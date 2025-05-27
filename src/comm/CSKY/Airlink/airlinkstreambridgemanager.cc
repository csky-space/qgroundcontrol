#include "airlinkstreambridgemanager.h"

#include <QGCLoggingCategory.h>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTimer>

QGC_LOGGING_CATEGORY(AirlinkStreamBridgeManagerLog, "AirlinkStreamBridgeManagerLog")

namespace CSKY {
AirlinkStreamBridgeManager::AirlinkStreamBridgeManager()
    : sslConfig(QSslConfiguration::defaultConfiguration())
    , manager(this)
{
    sslConfig.setProtocol(QSsl::TlsV1_2OrLater);
    sslConfig.setPeerVerifyMode(QSslSocket::VerifyNone);
    sslConfig.setSslOption(QSsl::SslOptionDisableLegacyRenegotiation, true);

    createWebrtcDefaultRequest.setUrl(QUrl(baseASBRequestsPath + baseWebrtcRequestsPath + "createDefaultReceiver"));
    createWebrtcDefaultRequest.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    createWebrtcDefaultRequest.setSslConfiguration(sslConfig);

    enableVideoTransmitRequest.setUrl(QUrl(baseASBRequestsPath + baseVideoRequestsPath + "enableVideoTransmit"));
    enableVideoTransmitRequest.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    enableVideoTransmitRequest.setSslConfiguration(sslConfig);

    isWebrtcReceiverConnectedRequest.setUrl(QUrl(baseASBRequestsPath + baseConnectionRequestsPath + "isConnected"));
    isWebrtcReceiverConnectedRequest.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    isWebrtcReceiverConnectedRequest.setSslConfiguration(sslConfig);

    openPeerRequest.setUrl(QUrl(baseASBRequestsPath + baseConnectionRequestsPath + "openPeer"));
    openPeerRequest.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    openPeerRequest.setSslConfiguration(sslConfig);

    closePeerRequest.setUrl(QUrl(baseASBRequestsPath + baseConnectionRequestsPath + "closePeer"));
    closePeerRequest.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    closePeerRequest.setSslConfiguration(sslConfig);

    sendAsbServicePortRequest.setUrl(QUrl(baseASBRequestsPath + baseWebrtcRequestsPath + "setupOutputProtocol"));
    sendAsbServicePortRequest.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    sendAsbServicePortRequest.setSslConfiguration(sslConfig);

    checkAliveRequest.setUrl(QUrl(baseASBRequestsPath));
    checkAliveRequest.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    checkAliveRequest.setSslConfiguration(sslConfig);
}

AirlinkStreamBridgeManager::~AirlinkStreamBridgeManager() {

}

void AirlinkStreamBridgeManager::createWebrtcDefault(QString hostName, QString modemName, QString password, quint16 port) {
    QJsonObject obj;
    obj["hostName"] = hostName;
    obj["modemName"] = modemName;
    obj["password"] = password;
    obj["UDPPort"] = port;
    QJsonDocument d(obj);

    createWebrtcDefaultReply = manager.post(createWebrtcDefaultRequest, d.toJson(QJsonDocument::Compact).trimmed());
    QTimer* replyTimeout = new QTimer(createWebrtcDefaultReply);
    replyTimeout->setSingleShot(true);
    replyTimeout->start(30000);
    connect(replyTimeout, &QTimer::timeout, this, [this](){
        if(createWebrtcDefaultReply) {
            createWebrtcDefaultReply->deleteLater();
            createWebrtcDefaultReply = nullptr;
        }
    });
    connect(createWebrtcDefaultReply, &QNetworkReply::finished, this, [this, replyTimeout](){
        replyTimeout->stop();
        qCDebug(AirlinkStreamBridgeManagerLog) << "emitting createWebrtcDefaultReply";
        emit createWebrtcCompleted(createWebrtcDefaultReply->readAll(), createWebrtcDefaultReply->error());
        createWebrtcDefaultReply->deleteLater();
        createWebrtcDefaultReply = nullptr;
    });
}

void AirlinkStreamBridgeManager::enableVideoTransmit() {
    enableVideoTransmitReply = manager.put(enableVideoTransmitRequest, "{}");
    QTimer* replyTimeout = new QTimer(enableVideoTransmitReply);
    replyTimeout->setSingleShot(true);
    replyTimeout->start(1000);
    connect(replyTimeout, &QTimer::timeout, [this](){
        if(enableVideoTransmitReply) {
            enableVideoTransmitReply->deleteLater();
            enableVideoTransmitReply = nullptr;
        }
    });
    connect(enableVideoTransmitReply, &QNetworkReply::finished, this, [this, replyTimeout](){
        replyTimeout->stop();
        emit enableVideoTransmitCompleted(enableVideoTransmitReply->readAll(), enableVideoTransmitReply->error());
        enableVideoTransmitReply->deleteLater();
        enableVideoTransmitReply = nullptr;
    });
}

void AirlinkStreamBridgeManager::isWebrtcReceiverConnected() {
    isWebrtcReceiverConnectedReply = manager.get(isWebrtcReceiverConnectedRequest);
    QTimer* replyTimeout = new QTimer(isWebrtcReceiverConnectedReply);
    replyTimeout->setSingleShot(true);
    replyTimeout->start(1000);
    connect(replyTimeout, &QTimer::timeout, [this](){
        if(isWebrtcReceiverConnectedReply) {
            isWebrtcReceiverConnectedReply->deleteLater();
            isWebrtcReceiverConnectedReply = nullptr;
        }
    });
    connect(isWebrtcReceiverConnectedReply, &QNetworkReply::finished, this, [this, replyTimeout](){
        replyTimeout->stop();
        qCDebug(AirlinkStreamBridgeManagerLog) << "emitting isWebrtcReceiverConnectedReply";
        emit isWebrtcReceiverConnectedCompleted(isWebrtcReceiverConnectedReply->readAll(), isWebrtcReceiverConnectedReply->error());
        isWebrtcReceiverConnectedReply->abort();
        isWebrtcReceiverConnectedReply->deleteLater();
    });
}

void AirlinkStreamBridgeManager::openPeer() {
    openPeerReply = manager.put(openPeerRequest, "{}");
    QTimer* replyTimeout = new QTimer(openPeerReply);
    replyTimeout->setSingleShot(true);
    replyTimeout->start(30000);
    connect(replyTimeout, &QTimer::timeout, [this](){
        qCDebug(AirlinkStreamBridgeManagerLog) << "open peer reply abort";
        if(openPeerReply) {
            openPeerReply->deleteLater();
            openPeerReply = nullptr;
        }

    });
    connect(openPeerReply, &QNetworkReply::finished, this, [this, replyTimeout](){
        replyTimeout->stop();
        qCDebug(AirlinkStreamBridgeManagerLog) << "emitting openPeerReply";
        if(openPeerReply && openPeerReply->isOpen() && openPeerReply->isReadable()) {
            emit openPeerCompleted(openPeerReply->readAll(), openPeerReply->error());
            openPeerReply->deleteLater();
            openPeerReply = nullptr;
        }

    });
}

void AirlinkStreamBridgeManager::closePeer() {
    qCDebug(AirlinkStreamBridgeManagerLog) << "closePeer";
    closePeerReply = manager.put(closePeerRequest, "{}");
    QTimer* replyTimeout = new QTimer(closePeerReply);
    replyTimeout->setSingleShot(true);
    replyTimeout->start(3000);
    connect(replyTimeout, &QTimer::timeout, [this](){
        if(closePeerReply) {
            closePeerReply->deleteLater();
            closePeerReply = nullptr;
        }
    });
    connect(closePeerReply, &QNetworkReply::finished, this, [this, replyTimeout](){
        replyTimeout->stop();
        qCDebug(AirlinkStreamBridgeManagerLog) << "emitting closePeerReply";
        emit closePeerCompleted(closePeerReply->readAll(), closePeerReply->error());
        closePeerReply->deleteLater();
        closePeerReply = nullptr;
    });
}

void AirlinkStreamBridgeManager::sendAsbServicePort(quint16 port) {
    QJsonObject obj;
    obj["protocol"] = "UDP";
    obj["address"] = "127.0.0.1";
    obj["UDPPort"] = port;
    QJsonDocument d(obj);

    sendAsbServicePortReply = manager.post(sendAsbServicePortRequest, d.toJson(QJsonDocument::Compact));
    QTimer* replyTimeout = new QTimer(sendAsbServicePortReply);
    replyTimeout->setSingleShot(true);
    replyTimeout->start(3000);
    connect(replyTimeout, &QTimer::timeout, [this](){
        if(sendAsbServicePortReply) {
            sendAsbServicePortReply->deleteLater();
            sendAsbServicePortReply = nullptr;
        }
    });
    connect(sendAsbServicePortReply, &QNetworkReply::finished, this, [this, replyTimeout](){
        replyTimeout->stop();
        qCDebug(AirlinkStreamBridgeManagerLog) << "emitting sendAsbServicePortReply";
        emit sendAsbServicePortCompleted(sendAsbServicePortReply->readAll(), sendAsbServicePortReply->error());
        sendAsbServicePortReply->deleteLater();
        sendAsbServicePortReply = nullptr;
    });
}

void AirlinkStreamBridgeManager::checkAlive() {
    checkAliveReply = manager.get(checkAliveRequest);
    QTimer* replyTimeout = new QTimer(checkAliveReply);
    replyTimeout->setSingleShot(true);
    replyTimeout->start(1000);
    connect(replyTimeout, &QTimer::timeout, [this](){
        qCDebug(AirlinkStreamBridgeManagerLog) << "abort checkAliveReply";
        if(checkAliveReply) {
            checkAliveReply->deleteLater();
            checkAliveReply = nullptr;
        }
    });
    connect(checkAliveReply, &QNetworkReply::finished, this, [this, replyTimeout](){
        replyTimeout->stop();
        qCDebug(AirlinkStreamBridgeManagerLog) << "emitting checkAliveReply";
        emit checkAliveCompleted(checkAliveReply->readAll(), checkAliveReply->error());
        checkAliveReply->deleteLater();
        checkAliveReply = nullptr;
    });
}
}


