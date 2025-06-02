/****************************************************************************
 *
 * (c) 2009-2020 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/

#include "AirlinkManager.h"

#include <csignal>

#include <QDebug>
#include <QSettings>
#include <qnetworkaccessmanager.h>
#include <qvariant.h>
#include <QtConcurrent>
#include <QTimer>
#include <QThread>


#include <QGCLoggingCategory.h>
#include <QQuickWindow>


#include "Fact.h"
#include "LinkManager.h"
#include "QGCApplication.h"
#include "QGCCorePlugin.h"
#include "SettingsManager.h"
#include "UDPLink.h"
#include "AirlinkConfiguration.h"
#include "Airlink.h"
#include "VideoManager.h"


QGC_LOGGING_CATEGORY(AirlinkManagerLog, "AirlinkManagerLog")



namespace CSKY {



#if defined(QGC_AIRLINK_STAGE)
    const QString AirlinkManager::airlinkHostPrefix = "stage.";
#else
    const QString AirlinkManager::airlinkHostPrefix = "";
#endif
const QString AirlinkManager::airlinkHost = airlinkHostPrefix + "air-link.space";

AirlinkManager::AirlinkManager(QGCApplication *app, QGCToolbox *toolbox)
    : QGCTool(app, toolbox)
    , serverController("com/csky/airlinkstreambridge/mobile/ServerController")
    , connectTelemetryManager(this)
    , asbProcess(this)
{

    qDebug(AirlinkManagerLog) << "airlink host on: " << AirlinkManager::airlinkHost;

#ifdef __ANDROID__
    startAndroidASB();
#else
    asbPath = QCoreApplication::applicationDirPath() + QDir::separator() +  "AirlinkStreamBridge";
    asbProcess.setProcessEnvironment(QProcessEnvironment::systemEnvironment());
    asbProcess.setStandardInputFile(QProcess::nullDevice());
    asbProcess.setProcessChannelMode(QProcess::SeparateChannels);
    QProcess::execute("chmod", {"755", asbPath});

    asbProcess.setProgram(asbPath);
#endif

    requestsThread = new QThread();
    manager.moveToThread(requestsThread);
    requestsThread->start(QThread::LowPriority);
}

AirlinkManager::~AirlinkManager() {
#ifndef __ANDROID__
    stopWatchdog();
    if (asbProcess.state() != QProcess::NotRunning) {
        asbProcess.terminate();
        asbProcess.waitForFinished();
    }
#else
    serverController.callMethod<void>("close");
#endif
}

#ifdef __ANDROID__

void AirlinkManager::startAndroidASB() const {
    serverController.callMethod<void>("start");
}
#endif

void AirlinkManager::restartASBProcess() {
#ifndef __ANDROID__
    if (asbProcess.state() != QProcess::NotRunning) {
        asbProcess.terminate();
        asbProcess.waitForFinished();
    }

    qCDebug(AirlinkManagerLog) << "Starting AirlinkStreamBridge...";
    asbProcess.start();
#else
    startAndroidASB();
#endif

}

void AirlinkManager::setToolbox(QGCToolbox *toolbox) {
    QGCTool::setToolbox(toolbox);

    asbEnabled = toolbox->settingsManager()->asbSettings()->asbEnabled();
    asbAutotune = toolbox->settingsManager()->asbSettings()->asbAutotune();
    asbPort = toolbox->settingsManager()->asbSettings()->asbPort();

    videoUDPPort = toolbox->settingsManager()->videoSettings()->udpPort();
    videoSource = toolbox->settingsManager()->videoSettings()->videoSource();

    qgcVideoManager = toolbox->videoManager();
    auto linkManager = toolbox->linkManager();
    for(auto link : linkManager->links()) {
        if(std::shared_ptr<Airlink> airlink = std::dynamic_pointer_cast<Airlink>(link)) {
            airlink->setAsbEnabled(asbEnabled);
            airlink->setAsbPort(asbPort);
        }
    }

    for(auto airlinkIt = modems.keyValueBegin(); airlinkIt != modems.keyValueEnd(); ++airlinkIt) {
        airlinkIt->second->setAsbEnabled(asbEnabled);
        airlinkIt->second->setAsbPort(asbPort);
    }

    connect(asbEnabled, &Fact::rawValueChanged, this, &AirlinkManager::asbEnabledChanged);
    connect(videoUDPPort, &Fact::rawValueChanged, this, &AirlinkManager::portConstraint);
    connect(videoSource, &Fact::rawValueChanged, this, [this](QVariant value){
        if(asbAutotune->rawValue().toBool()) {
            setupVideoStream("");
        }
    });

    connect(asbPort, &Fact::rawValueChanged, this, &AirlinkManager::setupPort);
    connect(asbAutotune, &Fact::rawValueChanged, this, &AirlinkManager::asbAutotuneChanged);

#ifndef __ANDROID__
    asbProcess.start();
    asbProcess.waitForStarted(4000);
    if ((asbProcess.error() == QProcess::FailedToStart) ||
        (asbProcess.error() == QProcess::Crashed) ||
        (asbProcess.state() == QProcess::NotRunning)) {
        QMutexLocker locker(&processMutex);
        qCDebug(AirlinkManagerLog) << "asbProcess doesn't launched. Restarting...";
        asbProcess.terminate();
        asbProcess.waitForFinished();
        isRestarting = true;
        restartASBProcess();
        isRestarting = false;

    } else {
        connect(&asbProcess, &QProcess::errorOccurred, [](QProcess::ProcessError error) {
            qCDebug(AirlinkManagerLog) << "QProcess error:" << error;
        });
        connect(&asbProcess, &QProcess::readyReadStandardOutput, this, [this]() {
            qCDebug(AirlinkManagerLog) << asbProcess.readAllStandardOutput();
        });
        connect(&asbProcess, &QProcess::readyReadStandardError, this, [this]() {
            qCDebug(AirlinkManagerLog) << asbProcess.readAllStandardError();
        });

        //QObject::connect(&asbProcess, &QProcess::readyRead, [&]() {
        //    qDebug() << "Process output:" << asbProcess.readAll();
        //});
        //thread()->msleep(1500);
        //createWebrtcReceiver();
    }





#endif
    watchdogTimer = new QTimer(this);
    watchdogTimer->setInterval(5000);

    auto asbProcessWatchdog = connect(watchdogTimer, &QTimer::timeout, this, &AirlinkManager::checkAndRestartASB);
    QObject::connect(qApp, &QCoreApplication::aboutToQuit, [asbProcessWatchdog, this]() {
        qCDebug(AirlinkManagerLog) << "off watchdog on quiting...";
#ifndef __ANDROID__
        asbProcess.kill();
        asbProcess.waitForFinished();
#else
        serverController.callMethod<void>("close");
#endif
        QObject::disconnect(asbProcessWatchdog);
    });
    startWatchdog();
    _setConnects();
    QTimer::singleShot(0, this, [this]() {
        if(asbEnabled->rawValue().toBool()) {
            asbEnabledChanged(asbEnabled->rawValue());
        }
        if(asbAutotune->rawValue().toBool()) {
            asbAutotuneChanged(true);
        }
    });
}

QStringList AirlinkManager::droneList() const {
    return _vehiclesFromServer.keys();
}

QList<bool> AirlinkManager::droneOnlineList() const {
    return _vehiclesFromServer.values();
}

AirlinkStreamBridgeManager& AirlinkManager::getASBManager() {
    return manager;
}

QProcess& AirlinkManager::getAsbProcess() {
    return asbProcess;
}

Fact* AirlinkManager::getAsbEnabled() const {
    return asbEnabled;
}

Fact* AirlinkManager::getPort() const {
    return asbPort;
}

void AirlinkManager::updateDroneList(const QString &login,
                                     const QString &pass) {
                                        
    connectToAirLinkServer(login, pass);
}

bool AirlinkManager::isOnline(const QString &drone) {
    if (!_vehiclesFromServer.contains(drone)) {
      return false;
    } else {
      return _vehiclesFromServer[drone];
    }
}

void AirlinkManager::connectToAirLinkServer(const QString &login, const QString &pass) {
    qCDebug(AirlinkManagerLog) << "airlinkHost: " << airlinkHost;
    const QUrl url(QString("https://") + airlinkHost + "/api/gs/getModems");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QJsonObject obj;
    obj["login"] = login.trimmed();
    obj["password"] = pass.trimmed();
    QJsonDocument doc(obj);
    QByteArray data = doc.toJson();
    qCDebug(AirlinkManagerLog) << "request for modems: " << doc.toJson();
    _reply = connectTelemetryManager.post(request, data);

    QObject::connect(_reply, &QNetworkReply::finished, [this]() {
      _processReplyAirlinkServer(*_reply);
      _reply->deleteLater();
    });
}

void AirlinkManager::updateCredentials(const QString &login,
                                       const QString &pass) {
    _toolbox->settingsManager()->appSettings()->loginAirLink()->setRawValue(
      login);
    _toolbox->settingsManager()->appSettings()->passAirLink()->setRawValue(pass);
}

void signalHandler(int signal) {
    qDebug() << "signal handle";
    std::signal(signal, SIG_DFL);
    qgcApp()->mainRootWindow()->close();
    QEvent event{QEvent::Quit};
    qgcApp()->event(&event);
}

void AirlinkManager::_setConnects() {
    std::signal(SIGSEGV, signalHandler);
    std::signal(SIGABRT, signalHandler);
#ifndef Q_OS_LINUX
    std::signal(SIGINT, signalHandler);
    std::signal(SIGTERM, signalHandler);
#endif
    connect(qgcApp()->toolbox()->multiVehicleManager(), &MultiVehicleManager::activeVehicleAvailableChanged, this, [](bool realChanged){
        if(realChanged) {
            //auto link = qgcApp()->toolbox()->multiVehicleManager()->activeVehicle()->vehicleLinkManager()->primaryLink().lock();
            //QString linkName = link->linkConfiguration()->name();
            //Airlink* airlink = dynamic_cast<Airlink*>(link.get());
            //if(airlink && !modems.contains(linkName)) {
            //    LinkManager
            //}

        }

    });
    connect(this, &AirlinkManager::sendAsbServicePort, &manager, &AirlinkStreamBridgeManager::sendAsbServicePort);
    connect(this, &AirlinkManager::checkAlive, &manager, &AirlinkStreamBridgeManager::checkAlive);

    connect(&manager, &AirlinkStreamBridgeManager::sendAsbServicePortCompleted, this, &AirlinkManager::unblockUI);
    connect(&manager, &AirlinkStreamBridgeManager::sendAsbServicePortCompleted, this, [](QByteArray replyData, QNetworkReply::NetworkError err){

    });

    connect(&manager, &AirlinkStreamBridgeManager::checkAliveCompleted, this, [this](QByteArray replyData, QNetworkReply::NetworkError err) {
        if(err != QNetworkReply::NoError) {
            QMutexLocker locker(&processMutex);

            qCDebug(AirlinkManagerLog) << "[Watchdog] HTTP server unreachable. " << "Error: " << err <<  "Restarting ASB...";
            isRestarting = true;
            restartASBProcess();
            isRestarting = false;
        }
    });

    connect(this, &AirlinkManager::openPeer, &manager, &AirlinkStreamBridgeManager::openPeer);
    connect(this, &AirlinkManager::closePeer, &manager, &AirlinkStreamBridgeManager::closePeer);
    connect(this, &AirlinkManager::createWebrtcDefault, &manager, &AirlinkStreamBridgeManager::createWebrtcDefault);
    connect(this, &AirlinkManager::isWebrtcReceiverConnected, &manager, &AirlinkStreamBridgeManager::isWebrtcReceiverConnected);

    connect(&manager, &AirlinkStreamBridgeManager::createWebrtcCompleted, this, &AirlinkManager::unblockUI);

    connect(&manager, &AirlinkStreamBridgeManager::createWebrtcCompleted, this, [this](QByteArray replyData, QNetworkReply::NetworkError err){
        if(err == QNetworkReply::NoError) {
            qCDebug(AirlinkManagerLog) << "create webrtc completed";
            if(lastConnectedModem) {
                lastConnectedModem->setWebrtcCreated(true);
                blockUI();
                emit openPeer();
            }

        }else {
            lastConnectedModem->setWebrtcCreated(false);
            qCDebug(AirlinkManagerLog) << "create webrtc failed: " << replyData << ". err: " << err;
        }

    });

    connect(&manager, &AirlinkStreamBridgeManager::openPeerCompleted, this, &AirlinkManager::unblockUI);

    connect(&manager, &AirlinkStreamBridgeManager::openPeerCompleted, this, [](QByteArray replyData, QNetworkReply::NetworkError err){
        qCDebug(AirlinkManagerLog) << "peer opened";
        qgcApp()->toolbox()->videoManager()->stopVideo();
    });

    connect(&manager, &AirlinkStreamBridgeManager::closePeerCompleted, this, &AirlinkManager::unblockUI);

    connect(&manager, &AirlinkStreamBridgeManager::closePeerCompleted, this, [](QByteArray replyData, QNetworkReply::NetworkError err){
        qCDebug(AirlinkManagerLog) << "peer closed";
    });

    connect(&manager, &AirlinkStreamBridgeManager::checkAliveCompleted, this, [this](QByteArray replyData, QNetworkReply::NetworkError err) {
        if(lastConnectedModem)
            lastConnectedModem->setWebrtcCreated(false);
    });
    setFullBlock(false);
    emit fullBlockChanged(false);
}

void AirlinkManager::_parseAnswer(const QByteArray &ba) {
    _vehiclesFromServer.clear();
    for (const auto &arr : QJsonDocument::fromJson(ba)["modems"].toArray()) {
      droneModem = arr.toObject()["name"].toString();
      bool isOnline = arr.toObject()["isOnline"].toBool();
      _vehiclesFromServer[droneModem] = isOnline;
    }
    emit droneListChanged();
}

void AirlinkManager::_processReplyAirlinkServer(QNetworkReply &reply) {
    QByteArray ba = reply.readAll();
    qCDebug(AirlinkManagerLog) << "anwer to get modems with: " << ba;
    if (reply.error() == QNetworkReply::NoError) {
      if (!QJsonDocument::fromJson(ba)["modems"].toArray().isEmpty()) {
        _parseAnswer(ba);
      } else {
        qCDebug(AirlinkManagerLog) << "No airlink modems in answer: " << QString(ba);
      }
    } else {
      qCDebug(AirlinkManagerLog) << "Airlink auth - network error";
    }
}

void AirlinkManager::_findAirlinkConfiguration() {
    LinkManager* manager = _toolbox->linkManager();

    QList<SharedLinkInterfacePtr> links = manager->links();
    qCDebug(AirlinkManagerLog) << "links size is: " << links.size();
    for(auto& link : links) {
        qCDebug(AirlinkManagerLog) << "linkConfiguration name is: " << link->linkConfiguration()->name();
        qCDebug(AirlinkManagerLog) << link->linkConfiguration()->type();
        if(qobject_cast<AirlinkConfiguration*>(link->linkConfiguration().get())) {
            qCDebug(AirlinkManagerLog) << "is AirlinkConfiguration";
            config = std::dynamic_pointer_cast<AirlinkConfiguration>(link->linkConfiguration());
        }
        else if(qobject_cast<UDPConfiguration*>(link->linkConfiguration().get())) {
            qCDebug(AirlinkManagerLog) << "is UDPConfiguration";

        }
    }
}

void AirlinkManager::setupVideoStream(QVariant value) {
    if(asbAutotune->rawValue().toBool())
        videoSource->setRawValue(VideoSettings::videoSourceUDPH265);
}

void AirlinkManager::setupPort(QVariant value) {
    qCDebug(AirlinkManagerLog) << "set port";
    if(asbAutotune->rawValue().toBool()) {
        qCDebug(AirlinkManagerLog) << "port constraining to " << value;
        videoUDPPort->setRawValue(value);
    }
    blockUI();
    emit sendAsbServicePort(value.toUInt());

}

void AirlinkManager::portConstraint(QVariant value) {
    if(asbAutotune->rawValue().toBool()) {
        qDebug() << "port constraining to " << asbPort->rawValue().toUInt();
        videoUDPPort->setRawValue(asbPort->rawValue().toUInt());
    }
}

void AirlinkManager::startWatchdog() {
    if (!watchdogTimer->isActive()) {
        watchdogTimer->start();
    }
}

void AirlinkManager::stopWatchdog() {
    if(watchdogTimer && watchdogTimer->isActive()){
        watchdogTimer->stop();
    }
}

void AirlinkManager::terminateASB() {
    asbProcess.terminate();
    asbProcess.waitForFinished(3000);
}

void AirlinkManager::checkAndRestartASB() {
    QMutexLocker locker(&processMutex);

    if (isRestarting) {
        return;
    }
#ifndef __ANDROID__
    if (asbProcess.state() != QProcess::Running) {
        asbEnabled->setRawValue(false);
        emit asbClosed(lastConnectedModem);
        qCDebug(AirlinkManagerLog) << "[Watchdog] ASB process not running. Restarting...";
        isRestarting = true;
        restartASBProcess();
        isRestarting = false;
        return;
    }
    else {
        if(lastConnectedModem && lastConnectedModem->isConnected() && !lastConnectedModem->webrtcCreated() && !asbEnabled->rawValue().toBool()) {
            asbEnabled->setRawValue(true);
        }
    }
#endif
    emit checkAlive();
}

void AirlinkManager::asbEnabledChanged(QVariant value) {
    qCDebug(AirlinkManagerLog) << "check for possibility connect";
    if(value.toBool()) {
        qCDebug(AirlinkManagerLog)  << "asbEnabledTrue";
        emit asbEnabledTrue(lastConnectedModem);
    }else {
        qCDebug(AirlinkManagerLog)  << "asbEnabledFalse";
        emit asbEnabledFalse(lastConnectedModem);
    }
}

void AirlinkManager::asbAutotuneChanged(QVariant value) {
    if(value.toBool()) {
        videoUDPPort->setRawValue(asbPort->rawValue());
        setupVideoStream("");
    }
}

void AirlinkManager::blockUI(QByteArray replyData, QNetworkReply::NetworkError) {
    qCDebug(AirlinkManagerLog) << "block";
    setFullBlock(true);
    emit fullBlockChanged(true);
}

void AirlinkManager::unblockUI(QByteArray replyData, QNetworkReply::NetworkError) {
    qCDebug(AirlinkManagerLog) << "unblock";
    setFullBlock(false);
    emit fullBlockChanged(false);
}

void AirlinkManager::addAirlink(Airlink* airlink) {
    if(airlink) {

        //QString modemName = airlink->getConfig()->modemName();
        //if(!modems.contains(modemName)) {
        //    if(lastConnectedModem && lastConnectedModem->isConnected())
        //        lastConnectedModem->disconnect();
        //    modems[modemName] = airlink;
        //    lastConnectedModem = airlink;
        //    lastConnectedModem->setAsbEnabled(asbEnabled);
        //    lastConnectedModem->setAsbPort(asbPort);
        //    emit onConnectedAirlinkAdded(airlink);
        //}
        if(lastConnectedModem && lastConnectedModem->isConnected())
            lastConnectedModem->disconnect();
        lastConnectedModem = airlink;
    }

}

void AirlinkManager::removeAirlink(Airlink* airlink) {
    qDebug() << "remove airlink?";
    if(airlink) {
        lastConnectedModem = nullptr;
        //QString modemName = airlink->getConfig()->modemName();
        //if(modems.contains(modemName)) {
        //    qCDebug(AirlinkManagerLog) << "removing airlink";
        //    modems.remove(modemName);
        //    emit onDisconnectedAirlinkRemoved(airlink);
        //}
    }
}

}
