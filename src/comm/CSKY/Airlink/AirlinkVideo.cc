#include "AirlinkVideo.h"

#include <QThread>

#include <QGCLoggingCategory.h>
#include <QGCApplication.h>
#include <QGCToolbox.h>
#include <VideoManager.h>

#include "airlinkstreambridgemanager.h"
#include "AirlinkManager.h"
#include "Airlink.h"
#include "AirlinkConfiguration.h"

QGC_LOGGING_CATEGORY(AirlinkVideoLog, "AirlinkVideoLog")

namespace CSKY {

AirlinkVideo::AirlinkVideo(AirlinkStreamBridgeManager* asbManager, AirlinkManager* airlinkManager, Airlink* modem, QObject* parent)
    : QObject(parent)
    , _videoRunningWatchdog(this)
    , _asbManager(asbManager)
    , _airlinkManager(airlinkManager)
    , _modem(modem)
    , _webrtcReceiverCreated(false)
{
    if (!_asbManager || !_airlinkManager || !_modem) {
        qCWarning(AirlinkVideoLog) << "AirlinkVideo created with null pointers";
    }
}

AirlinkVideo::~AirlinkVideo() {
    unsetConnections();

    disconnect(this, nullptr, nullptr, nullptr);

    qCDebug(AirlinkVideoLog) << "AirlinkVideo destroyed";
}

void AirlinkVideo::setConnections() {
    if (!_asbManager || !_airlinkManager) {
        qCWarning(AirlinkVideoLog) << "Cannot set connections: null manager pointers";
        return;
    }

    connect(this, &AirlinkVideo::blockUI, _airlinkManager, &AirlinkManager::blockUI);

    connect(this, &AirlinkVideo::createWebrtcDefault, _asbManager, &AirlinkStreamBridgeManager::createWebrtcDefault, Qt::QueuedConnection);
    connect(this, &AirlinkVideo::isWebrtcReceiverConnected, _asbManager, &AirlinkStreamBridgeManager::isWebrtcReceiverConnected, Qt::QueuedConnection);
    _videoRunningWatchdog.start(2000);
    connect(this, &AirlinkVideo::isVideoRunning, _asbManager, &AirlinkStreamBridgeManager::isRunning, Qt::QueuedConnection);
    connect(&_videoRunningWatchdog, &QTimer::timeout, this, [this](){
        emit isVideoRunning();
    });
    connect(_asbManager, &AirlinkStreamBridgeManager::videoIsRunningCompleted, this, [this](QByteArray replyData, QNetworkReply::NetworkError err){
        if((err != QNetworkReply::NoError) && _airlinkManager->getAsbEnabled()->rawValue().toBool() && _modem->isConnected()) {
            AirlinkConfiguration* conf = dynamic_cast<AirlinkConfiguration*>(_modem->getConfig().get());
            emit createWebrtcDefault(_modem->getHost(), conf->modemName(), conf->password(), _airlinkManager->getPort()->rawValue().toInt());
        }
    });
    connect(this, &AirlinkVideo::openPeer, _asbManager, &AirlinkStreamBridgeManager::openPeer, Qt::QueuedConnection);
    connect(this, &AirlinkVideo::closePeer, _asbManager, &AirlinkStreamBridgeManager::closePeer, Qt::QueuedConnection);

    connect(_asbManager, &AirlinkStreamBridgeManager::createWebrtcCompleted, _airlinkManager, &AirlinkManager::unblockUI, Qt::QueuedConnection);
    connect(_asbManager, &AirlinkStreamBridgeManager::createWebrtcCompleted, this, &AirlinkVideo::webrtcCreated, Qt::QueuedConnection);

    connect(_asbManager, &AirlinkStreamBridgeManager::openPeerCompleted, _airlinkManager, &AirlinkManager::unblockUI, Qt::QueuedConnection);
    connect(_asbManager, &AirlinkStreamBridgeManager::openPeerCompleted, this, &AirlinkVideo::peerOpened, Qt::QueuedConnection);

    connect(_asbManager, &AirlinkStreamBridgeManager::closePeerCompleted, _airlinkManager, &AirlinkManager::unblockUI, Qt::QueuedConnection);
    connect(_asbManager, &AirlinkStreamBridgeManager::closePeerCompleted, this, &AirlinkVideo::peerClosed, Qt::QueuedConnection);

    _connectionsEstablished = true;
    emit qtConnectionsEstablished();
    emit qtConnectionsStateChanged(_connectionsEstablished);
}

void AirlinkVideo::unsetConnections() {
    if (!_asbManager || !_airlinkManager) {
        return;
    }
    disconnect(this, &AirlinkVideo::blockUI, _airlinkManager, &AirlinkManager::blockUI);

    disconnect(this, &AirlinkVideo::createWebrtcDefault, _asbManager, &AirlinkStreamBridgeManager::createWebrtcDefault);
    disconnect(this, &AirlinkVideo::isWebrtcReceiverConnected, _asbManager, &AirlinkStreamBridgeManager::isWebrtcReceiverConnected);
    disconnect(this, &AirlinkVideo::isVideoRunning, _asbManager, &AirlinkStreamBridgeManager::isRunning);
    disconnect(this, &AirlinkVideo::openPeer, _asbManager, &AirlinkStreamBridgeManager::openPeer);
    disconnect(this, &AirlinkVideo::closePeer, _asbManager, &AirlinkStreamBridgeManager::closePeer);

    disconnect(_asbManager, &AirlinkStreamBridgeManager::createWebrtcCompleted, _airlinkManager, &AirlinkManager::unblockUI);
    disconnect(_asbManager, &AirlinkStreamBridgeManager::createWebrtcCompleted, this, &AirlinkVideo::webrtcCreated);

    disconnect(_asbManager, &AirlinkStreamBridgeManager::openPeerCompleted, _airlinkManager, &AirlinkManager::unblockUI);
    disconnect(_asbManager, &AirlinkStreamBridgeManager::openPeerCompleted, this, &AirlinkVideo::peerOpened);

    disconnect(_asbManager, &AirlinkStreamBridgeManager::closePeerCompleted, _airlinkManager, &AirlinkManager::unblockUI);
    disconnect(_asbManager, &AirlinkStreamBridgeManager::closePeerCompleted, this, &AirlinkVideo::peerClosed);
    _connectionsEstablished = false;
    emit qtConnectionsUnstablished();
    emit qtConnectionsStateChanged(_connectionsEstablished);
}

void AirlinkVideo::_connect(QString modemName, QString password, quint16 port) {
    if (!_asbManager || !_airlinkManager || !_modem) {
        qCWarning(AirlinkVideoLog) << "Cannot connect video: null pointers";
        return;
    }

    if (modemName.isEmpty() || password.isEmpty()) {
        qCWarning(AirlinkVideoLog) << "Cannot connect video: empty modem name or password";
        return;
    }

    setConnections();
    qCDebug(AirlinkVideoLog) << "asb is on";
    if(!_webrtcReceiverCreated) {
        qCDebug(AirlinkVideoLog) << "Airlink video connecting for " << modemName;
        emit blockUI();

        emit createWebrtcDefault(_modem->getHost(), modemName, password, port);
    }
    else {
        emit blockUI();
        emit openPeer();
    }
}

void AirlinkVideo::_disconnect() {
    qCDebug(AirlinkVideoLog) << "disconnect video check for ours";
    qCDebug(AirlinkVideoLog) << "Disconnect video?";

    if (!_asbManager) {
        qCWarning(AirlinkVideoLog) << "Cannot disconnect: ASB manager is null";
        return;
    }

    //emit blockUI();
    qCDebug(AirlinkVideoLog) << "Disconnect video";
    emit closePeer();
}

void AirlinkVideo::webrtcCreated(QByteArray replyData, QNetworkReply::NetworkError err) {
    if((err == QNetworkReply::NoError) || (err == QNetworkReply::TimeoutError)) {
        qCDebug(AirlinkVideoLog) << "WebRTC receiver created successfully";
        _webrtcReceiverCreated = true;

        emit blockUI();
        emit openPeer();
    } else {
        _webrtcReceiverCreated = false;
        qCWarning(AirlinkVideoLog) << "WebRTC creation failed. Error:" << err
                                   << "Response:" << replyData;

        emit connectionFailed(tr("WebRTC creation failed: %1").arg(err));
    }
}

void AirlinkVideo::peerOpened(QByteArray replyData, QNetworkReply::NetworkError err) {
    if (err == QNetworkReply::NoError) {
        qCDebug(AirlinkVideoLog) << "Peer opened successfully";

        VideoManager* videoManager = qgcApp()->toolbox()->videoManager();
        if (videoManager) {
            videoManager->stopVideo();
        } else {
            qCWarning(AirlinkVideoLog) << "Video manager not available";
        }
        emit isVideoRunning();
        //emit videoConnected();
    } else {
        qCWarning(AirlinkVideoLog) << "Failed to open peer. Error:" << err
                                   << "Response:" << replyData;
        emit connectionFailed(tr("Failed to open peer: %1").arg(err));
    }
}

void AirlinkVideo::peerClosed(QByteArray replyData, QNetworkReply::NetworkError err) {
    if (err == QNetworkReply::NoError) {
        qCDebug(AirlinkVideoLog) << "Peer closed successfully";
    } else {
        qCWarning(AirlinkVideoLog) << "Peer closure completed with error:" << err
                                   << "Response:" << replyData;
    }

    _webrtcReceiverCreated = false;
    unsetConnections();
    //emit videoDisconnected();
}

void AirlinkVideo::setWebrtcCreated(bool created) {
    _webrtcReceiverCreated = created;
}

void AirlinkVideo::asbFailed() {
    qCWarning(AirlinkVideoLog) << "ASB connection failed";
    _webrtcReceiverCreated = false;

    if (_airlinkManager) {
        QMetaObject::invokeMethod(_airlinkManager, "unblockUI", Qt::QueuedConnection);
    }

    emit connectionFailed(tr("ASB connection failed"));
    //emit videoDisconnected();
}

}
