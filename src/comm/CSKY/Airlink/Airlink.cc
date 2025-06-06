#include "Airlink.h"

#include <QGC.h>
#include <QGCApplication.h>
#include <AppSettings.h>
#include <SettingsManager.h>
#include <LinkManager.h>
#include <VideoManager.h>

#include "AirlinkManager.h"
#include "AirlinkConfiguration.h"
#include "AirlinkVideo.h"

QGC_LOGGING_CATEGORY(AirlinkLog, "AirlinkLog")

namespace CSKY {
Airlink::Airlink(SharedLinkConfigurationPtr &config)
    : UDPLink(config)
    , _videoThread(new QThread())
{
    qCInfo(AirlinkLog) << "Airlink created";
    _configureUdpSettings();
#ifdef QGC_AIRLINK_ENABLED
    airlinkManager = qgcApp()->toolbox()->airlinkManager();
    asbManager = &airlinkManager->getASBManager();
    _video = new AirlinkVideo(asbManager, airlinkManager);
    _video->moveToThread(_videoThread);
#endif
}

Airlink::~Airlink()
{
    unsetConnections();
    _video->deleteLater();
    if(_videoThread->isRunning())
        _videoThread->quit();
    _videoThread->deleteLater();
}

void Airlink::disconnect()
{
    disconnectVideo();
    qCDebug(AirlinkLog) << "Disconnecting airlink telemetry\n";


    _setConnectFlag(false);
    UDPLink::disconnect();
    unsetConnections();
}

std::shared_ptr<AirlinkConfiguration> Airlink::getConfig() const {
    return std::dynamic_pointer_cast<AirlinkConfiguration>(_config);
}

void Airlink::setAsbEnabled(Fact* asbEnabled) {
    this->asbEnabled = asbEnabled;
}

void Airlink::setAsbPort(Fact* asbPort) {
    this->asbPort = asbPort;
}

void Airlink::setConnections() {
#ifdef QGC_AIRLINK_ENABLED
    connect(airlinkManager, &AirlinkManager::asbClosed, this, &Airlink::asbClosed);
    connect(this, &Airlink::_asbClosed, _video, &AirlinkVideo::asbFailed);
    connect(airlinkManager, &AirlinkManager::asbEnabledTrue, this, &Airlink::connectVideo);
    connect(airlinkManager, &AirlinkManager::asbEnabledFalse, this, &Airlink::disconnectVideo);

    *onAddAirlinkConnection = connect(this, &Airlink::connected, airlinkManager, [this](){
        airlinkManager->addAirlink(this);
        _videoThread->start(QThread::HighPriority);
    });
    connect(_videoThread, &QThread::started, this, &Airlink::connectVideo);
    connect(this, &Airlink::connectVideoReady, _video, &AirlinkVideo::_connect);

    *onRemoveAirlinkConnection = connect(this, &Airlink::disconnectVideoReady, airlinkManager, [this](){
        airlinkManager->removeAirlink(this);
    });
    connect(this, &Airlink::disconnectVideoReady, _video, &AirlinkVideo::_disconnect);
    connect(_video, &AirlinkVideo::disconnected, _videoThread, &QThread::quit);

    connect(this, &Airlink::blockUI, airlinkManager, &AirlinkManager::blockUI);
#endif
}

void Airlink::unsetConnections() {
#ifdef QGC_AIRLINK_ENABLED
    QObject::disconnect(airlinkManager, &AirlinkManager::asbClosed, this, &Airlink::asbClosed);
    QObject::disconnect(this, &Airlink::_asbClosed, _video, &AirlinkVideo::asbFailed);
    QObject::disconnect(airlinkManager, &AirlinkManager::asbEnabledTrue, this, &Airlink::connectVideo);
    QObject::disconnect(airlinkManager, &AirlinkManager::asbEnabledFalse, this, &Airlink::disconnectVideo);

    QObject::disconnect(*onAddAirlinkConnection);
    QObject::disconnect(_videoThread, &QThread::started, this, &Airlink::connectVideo);
    QObject::disconnect(this, &Airlink::connectVideoReady, _video, &AirlinkVideo::_connect);

    QObject::disconnect(*onRemoveAirlinkConnection);
    QObject::disconnect(this, &Airlink::disconnectVideoReady, _video, &AirlinkVideo::_disconnect);

    QObject::disconnect(this, &Airlink::blockUI, airlinkManager, &AirlinkManager::blockUI);
#endif
}

bool Airlink::_connect()
{
    setConnections();
    start(NormalPriority);

    QTimer *pendingTimer = new QTimer();
    pendingTimer->moveToThread(this);
    connect(this, &Airlink::destroyed, pendingTimer, &QTimer::deleteLater);
    connect(pendingTimer, &QTimer::timeout, this, [this, pendingTimer] {
        pendingTimer->setInterval(3000);
        if (_stillConnecting()) {
            qCDebug(AirlinkLog) << "Connecting...";
            _sendLoginMsgToAirLink();
        } else {
            qCDebug(AirlinkLog) << "Stopping...";
            pendingTimer->stop();
            pendingTimer->deleteLater();
        }
    });
    MAVLinkProtocol *mavlink = qgcApp()->toolbox()->mavlinkProtocol();
    auto conn = std::make_shared<QMetaObject::Connection>();
    *conn = connect(mavlink, &MAVLinkProtocol::messageReceived, this, [this, conn] (LinkInterface* linkSrc, mavlink_message_t message) {
        if ((this != linkSrc) || (message.msgid != MAVLINK_MSG_ID_AIRLINK_AUTH_RESPONSE)) {
            return;
        }
        mavlink_airlink_auth_response_t responseMsg;
        mavlink_msg_airlink_auth_response_decode(&message, &responseMsg);
        int answer = responseMsg.resp_type;
        if (answer != AIRLINK_AUTH_RESPONSE_TYPE::AIRLINK_AUTH_OK) {
            qCDebug(AirlinkLog) << "Airlink auth failed";
            return;
        }
        qCDebug(AirlinkLog) << "Connected successfully";
        QObject::disconnect(*conn);
        _setConnectFlag(false);
    });
    _setConnectFlag(true);
    QMetaObject::invokeMethod(pendingTimer, "start", Qt::QueuedConnection, Q_ARG(int, 0));

    return true;
}

void Airlink::_configureUdpSettings()
{
    static quint16 availablePort = 14550;
    QUdpSocket udpSocket;
    while (!udpSocket.bind(QHostAddress::LocalHost, availablePort))
        availablePort++;
    UDPConfiguration* udpConfig = dynamic_cast<UDPConfiguration*>(UDPLink::_config.get());
    udpConfig->addHost(AirlinkManager::airlinkHost, AirlinkManager::airlinkPort);
    udpConfig->setLocalPort(availablePort);
    udpConfig->setDynamic(false);
    availablePort += 1;
}

void Airlink::_sendLoginMsgToAirLink()
{
    __mavlink_airlink_auth_t auth;
    uint8_t buffer[MAVLINK_MAX_PACKET_LEN];
    mavlink_message_t mavmsg;
    AirlinkConfiguration* config = dynamic_cast<AirlinkConfiguration*>(_config.get());
    qCDebug(AirlinkLog) << "before conf gets";
    QString login = config->modemName();
    QString pass = config->password();

    std::fill(std::begin(auth.login), std::end(auth.login), 0);
    std::fill(std::begin(auth.password), std::end(auth.password), 0);
    qCDebug(AirlinkLog) << "before print authorization";
    snprintf(auth.login, sizeof(auth.login), "%s", login.toUtf8().constData());
    snprintf(auth.password, sizeof(auth.password), "%s", pass.toUtf8().constData());

    qCDebug(AirlinkLog) << "before auth";
    mavlink_msg_airlink_auth_pack(0, 0, &mavmsg, auth.login, auth.password);
    uint16_t len = mavlink_msg_to_send_buffer(buffer, &mavmsg);

    if (!_stillConnecting()) {
        qCDebug(AirlinkLog) << "Force exit from connection";
        return;
    }
    qCDebug(AirlinkLog) << "before write bytes";
    this->writeBytesThreadSafe((const char*)buffer, len);
    qCDebug(AirlinkLog) << "after write bytes";
}

bool Airlink::_stillConnecting()
{
    return _needToConnect.load(std::memory_order_acquire);
}

void Airlink::_setConnectFlag(bool connect)
{
    _needToConnect.store(connect, std::memory_order_release);
}

void Airlink::connectVideo() {
    if(asbEnabled == nullptr)
        asbEnabled = airlinkManager->getAsbEnabled();
    if(asbPort == nullptr)
        asbPort = airlinkManager->getPort();
#ifndef __ANDROID__
    if(airlinkManager->getAsbProcess().state() != QProcess::Running)
        return;
#endif
    if (asbEnabled->rawValue().toBool()) {
        qCDebug(AirlinkLog) << "asb is on";
        auto configuration = std::dynamic_pointer_cast<AirlinkConfiguration>(_config);
        if(!configuration) {
            asbEnabled->setRawValue(false);
            qCDebug(AirlinkLog) << "Airlink configuration doesn't exist yet";
            return;
        }
        emit connectVideoReady(configuration->modemName(), configuration->password(), asbPort->rawValue().toUInt());
    }
}

void Airlink::disconnectVideo() {
    qCDebug(AirlinkLog) << "disconnect video check for ours";
    qCDebug(AirlinkLog) << "Disconnect video?";
#ifndef __ANDROID__
    if (airlinkManager->getAsbProcess().state() == QProcess::NotRunning) {
        return;
    }
#endif
    emit disconnectVideoReady();
}

void Airlink::emitVideoDisconnected() {
    emit videoDisconnected();
}

void Airlink::asbClosed(Airlink* airlink) {
    if(airlink != this) {
        return;
    }
    emit _asbClosed();
}

}
