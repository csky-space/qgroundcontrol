#include "airlinkstreambridgemanager.h"

#include <QGCLoggingCategory.h>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTimer>
#include <QPointer>

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

    QPointer<QNetworkReply> safeReply = manager.post(createWebrtcDefaultRequest, d.toJson(QJsonDocument::Compact).trimmed());
    QTimer* replyTimeout = new QTimer(safeReply);
    replyTimeout->setSingleShot(true);
    replyTimeout->start(30000);

    connect(replyTimeout, &QTimer::timeout, this, [safeReply]() {
        if(safeReply) {
            safeReply->abort();
            safeReply->deleteLater();
        }
    });

    connect(safeReply, &QNetworkReply::finished, this, [this, replyTimeout, safeReply]() {
        replyTimeout->stop();
        if(safeReply) {
            qCDebug(AirlinkStreamBridgeManagerLog) << "emitting createWebrtcDefaultReply";
            emit createWebrtcCompleted(safeReply->readAll(), safeReply->error());
            safeReply->deleteLater();
        }
    });
}

void AirlinkStreamBridgeManager::enableVideoTransmit() {
    QPointer<QNetworkReply> enableVideoTransmitReply = manager.put(enableVideoTransmitRequest, "{}");
    QTimer* replyTimeout = new QTimer(enableVideoTransmitReply);
    replyTimeout->setSingleShot(true);
    replyTimeout->start(1000);
    connect(replyTimeout, &QTimer::timeout, [enableVideoTransmitReply](){
        if(enableVideoTransmitReply) {
            enableVideoTransmitReply->abort();
            enableVideoTransmitReply->deleteLater();
        }
    });
    connect(enableVideoTransmitReply, &QNetworkReply::finished, this, [this, replyTimeout, enableVideoTransmitReply](){
        replyTimeout->stop();
        emit enableVideoTransmitCompleted(enableVideoTransmitReply->readAll(), enableVideoTransmitReply->error());
        enableVideoTransmitReply->deleteLater();
    });
}

void AirlinkStreamBridgeManager::isWebrtcReceiverConnected() {
    QPointer<QNetworkReply> isWebrtcReceiverConnectedReply = manager.get(isWebrtcReceiverConnectedRequest);
    QTimer* replyTimeout = new QTimer(isWebrtcReceiverConnectedReply);
    replyTimeout->setSingleShot(true);
    replyTimeout->start(1000);
    connect(replyTimeout, &QTimer::timeout, [isWebrtcReceiverConnectedReply](){
        if(isWebrtcReceiverConnectedReply) {
            isWebrtcReceiverConnectedReply->abort();
            isWebrtcReceiverConnectedReply->deleteLater();
        }
    });
    connect(isWebrtcReceiverConnectedReply, &QNetworkReply::finished, this, [this, replyTimeout, isWebrtcReceiverConnectedReply](){
        replyTimeout->stop();
        qCDebug(AirlinkStreamBridgeManagerLog) << "emitting isWebrtcReceiverConnectedReply";
        emit isWebrtcReceiverConnectedCompleted(isWebrtcReceiverConnectedReply->readAll(), isWebrtcReceiverConnectedReply->error());
        isWebrtcReceiverConnectedReply->deleteLater();
    });
}

void AirlinkStreamBridgeManager::openPeer() {
    QPointer<QNetworkReply> openPeerReply = manager.put(openPeerRequest, "{}");
    QTimer* replyTimeout = new QTimer(openPeerReply);
    replyTimeout->setSingleShot(true);
    replyTimeout->start(30000);
    connect(replyTimeout, &QTimer::timeout, [openPeerReply](){
        qCDebug(AirlinkStreamBridgeManagerLog) << "open peer reply abort";
        if(openPeerReply) {
            openPeerReply->abort();
            openPeerReply->deleteLater();
        }

    });
    connect(openPeerReply, &QNetworkReply::finished, this, [this, replyTimeout, openPeerReply](){
        replyTimeout->stop();
        qCDebug(AirlinkStreamBridgeManagerLog) << "emitting openPeerReply";
        if(openPeerReply && openPeerReply->isOpen() && openPeerReply->isReadable()) {
            emit openPeerCompleted(openPeerReply->readAll(), openPeerReply->error());
            openPeerReply->deleteLater();
        }

    });
}

void AirlinkStreamBridgeManager::closePeer() {
    qCDebug(AirlinkStreamBridgeManagerLog) << "closePeer";
    QPointer<QNetworkReply> closePeerReply = manager.put(closePeerRequest, "{}");
    QTimer* replyTimeout = new QTimer(closePeerReply);
    replyTimeout->setSingleShot(true);
    replyTimeout->start(3000);
    connect(replyTimeout, &QTimer::timeout, [closePeerReply](){
        if(closePeerReply) {
            closePeerReply->abort();
            closePeerReply->deleteLater();
        }
    });
    connect(closePeerReply, &QNetworkReply::finished, this, [this, replyTimeout, closePeerReply](){
        replyTimeout->stop();
        qCDebug(AirlinkStreamBridgeManagerLog) << "emitting closePeerReply";
        emit closePeerCompleted(closePeerReply->readAll(), closePeerReply->error());
        closePeerReply->deleteLater();
    });
}

void AirlinkStreamBridgeManager::sendAsbServicePort(quint16 port) {
    QJsonObject obj;
    obj["protocol"] = "UDP";
    obj["address"] = "127.0.0.1";
    obj["UDPPort"] = port;
    QJsonDocument d(obj);

    QPointer<QNetworkReply> sendAsbServicePortReply = manager.post(sendAsbServicePortRequest, d.toJson(QJsonDocument::Compact));
    QTimer* replyTimeout = new QTimer(sendAsbServicePortReply);
    replyTimeout->setSingleShot(true);
    replyTimeout->start(3000);
    connect(replyTimeout, &QTimer::timeout, [sendAsbServicePortReply](){
        if(sendAsbServicePortReply) {
            sendAsbServicePortReply->abort();
            sendAsbServicePortReply->deleteLater();
        }
    });
    connect(sendAsbServicePortReply, &QNetworkReply::finished, this, [this, replyTimeout, sendAsbServicePortReply](){
        replyTimeout->stop();
        qCDebug(AirlinkStreamBridgeManagerLog) << "emitting sendAsbServicePortReply";
        emit sendAsbServicePortCompleted(sendAsbServicePortReply->readAll(), sendAsbServicePortReply->error());
        sendAsbServicePortReply->deleteLater();
    });
}

void AirlinkStreamBridgeManager::checkAlive() {
    QPointer<QNetworkReply> checkAliveReply = manager.get(checkAliveRequest);
    QTimer* replyTimeout = new QTimer(checkAliveReply);
    replyTimeout->setSingleShot(true);
    replyTimeout->start(1000);
    connect(replyTimeout, &QTimer::timeout, [checkAliveReply](){
        qCDebug(AirlinkStreamBridgeManagerLog) << "abort checkAliveReply";
        if(checkAliveReply) {
            checkAliveReply->abort();
            checkAliveReply->deleteLater();
        }
    });
    connect(checkAliveReply, &QNetworkReply::finished, this, [this, replyTimeout, checkAliveReply](){
        replyTimeout->stop();
        qCDebug(AirlinkStreamBridgeManagerLog) << "emitting checkAliveReply";
        emit checkAliveCompleted(checkAliveReply->readAll(), checkAliveReply->error());
        checkAliveReply->deleteLater();
    });
}
}


