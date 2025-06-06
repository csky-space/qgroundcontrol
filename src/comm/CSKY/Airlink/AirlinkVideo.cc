#include "AirlinkVideo.h"

#include <QThread>

#include <QGCLoggingCategory.h>
#include <QGCApplication.h>
#include <QGCToolbox.h>
#include <VideoManager.h>

#include "airlinkstreambridgemanager.h"
#include "AirlinkManager.h"
#include "Airlink.h"

QGC_LOGGING_CATEGORY(AirlinkVideoLog, "AirlinkVideoLog")

namespace CSKY {

AirlinkVideo::AirlinkVideo(AirlinkStreamBridgeManager* asbManager, AirlinkManager* airlinkManager, QObject* parent)
    : QObject(parent)
    , _asbManager(asbManager)
    , _airlinkManager(airlinkManager)
    , webtrcReceiverCreated(false)
{

}

AirlinkVideo::~AirlinkVideo() {
}

void AirlinkVideo::setConnections() {
    connect(this, &AirlinkVideo::blockUI, _airlinkManager, &AirlinkManager::blockUI);

    connect(this, &AirlinkVideo::createWebrtcDefault, _asbManager, &AirlinkStreamBridgeManager::createWebrtcDefault, Qt::QueuedConnection);
    connect(this, &AirlinkVideo::isWebrtcReceiverConnected, _asbManager, &AirlinkStreamBridgeManager::isWebrtcReceiverConnected, Qt::QueuedConnection);
    connect(this, &AirlinkVideo::openPeer, _asbManager, &AirlinkStreamBridgeManager::openPeer, Qt::QueuedConnection);
    connect(this, &AirlinkVideo::closePeer, _asbManager, &AirlinkStreamBridgeManager::closePeer, Qt::QueuedConnection);

    connect(_asbManager, &AirlinkStreamBridgeManager::createWebrtcCompleted, _airlinkManager, &AirlinkManager::unblockUI, Qt::QueuedConnection);
    connect(_asbManager, &AirlinkStreamBridgeManager::createWebrtcCompleted, this, &AirlinkVideo::webrtcCreated, Qt::QueuedConnection);

    connect(_asbManager, &AirlinkStreamBridgeManager::openPeerCompleted, _airlinkManager, &AirlinkManager::unblockUI, Qt::QueuedConnection);
    connect(_asbManager, &AirlinkStreamBridgeManager::openPeerCompleted, this, &AirlinkVideo::peerOpened, Qt::QueuedConnection);

    connect(_asbManager, &AirlinkStreamBridgeManager::closePeerCompleted, _airlinkManager, &AirlinkManager::unblockUI, Qt::QueuedConnection);
    connect(_asbManager, &AirlinkStreamBridgeManager::closePeerCompleted, this, &AirlinkVideo::peerClosed, Qt::QueuedConnection);
}

void AirlinkVideo::unsetConnections() {
    disconnect(this, &AirlinkVideo::blockUI, _airlinkManager, &AirlinkManager::blockUI);

    disconnect(this, &AirlinkVideo::createWebrtcDefault, _asbManager, &AirlinkStreamBridgeManager::createWebrtcDefault);
    disconnect(this, &AirlinkVideo::isWebrtcReceiverConnected, _asbManager, &AirlinkStreamBridgeManager::isWebrtcReceiverConnected);
    disconnect(this, &AirlinkVideo::openPeer, _asbManager, &AirlinkStreamBridgeManager::openPeer);
    disconnect(this, &AirlinkVideo::closePeer, _asbManager, &AirlinkStreamBridgeManager::closePeer);

    disconnect(_asbManager, &AirlinkStreamBridgeManager::createWebrtcCompleted, _airlinkManager, &AirlinkManager::unblockUI);
    disconnect(_asbManager, &AirlinkStreamBridgeManager::createWebrtcCompleted, this, &AirlinkVideo::webrtcCreated);

    disconnect(_asbManager, &AirlinkStreamBridgeManager::openPeerCompleted, _airlinkManager, &AirlinkManager::unblockUI);
    disconnect(_asbManager, &AirlinkStreamBridgeManager::openPeerCompleted, this, &AirlinkVideo::peerOpened);

    disconnect(_asbManager, &AirlinkStreamBridgeManager::closePeerCompleted, _airlinkManager, &AirlinkManager::unblockUI);
    disconnect(_asbManager, &AirlinkStreamBridgeManager::closePeerCompleted, this, &AirlinkVideo::peerClosed);
}

void AirlinkVideo::_connect(QString modemName, QString password, quint16 port) {
    setConnections();
    qCDebug(AirlinkVideoLog) << "asb is on";
    if(!webtrcReceiverCreated) {
        qCDebug(AirlinkVideoLog) << "Airlink video connecting for " << modemName;
        emit blockUI();

        emit createWebrtcDefault(AirlinkManager::airlinkHost, modemName, password, port);
    }
    else {
        emit blockUI();
        emit openPeer();
    }
}

void AirlinkVideo::_disconnect() {
    qCDebug(AirlinkVideoLog) << "disconnect video check for ours";
    qCDebug(AirlinkVideoLog) << "Disconnect video?";

    emit blockUI();
    qCDebug(AirlinkVideoLog) << "Disconnect video";
    emit closePeer();
}

void AirlinkVideo::webrtcCreated(QByteArray replyData, QNetworkReply::NetworkError err) {
    if(err == QNetworkReply::NoError) {
        qCDebug(AirlinkVideoLog) << "create webrtc completed";
        webtrcReceiverCreated = true;
        emit blockUI();
        emit openPeer();

    }else {
        webtrcReceiverCreated = false;
        qCDebug(AirlinkVideoLog) << "create webrtc failed: " << replyData << ". err: " << err;
    }
}

void AirlinkVideo::peerOpened(QByteArray replyData, QNetworkReply::NetworkError err) {
    qCDebug(AirlinkVideoLog) << "peer opened";
    qgcApp()->toolbox()->videoManager()->stopVideo();
}

void AirlinkVideo::peerClosed(QByteArray replyData, QNetworkReply::NetworkError err) {
    qCDebug(AirlinkVideoLog) << "peer closed";
    unsetConnections();
    //emit closePeerCompleted();
}

void AirlinkVideo::setWebrtcCreated(bool created) {
    webtrcReceiverCreated = created;
}

void AirlinkVideo::asbFailed() {
    webtrcReceiverCreated = false;
}

}
