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

    QPointer<QNetworkReply> reply = manager.post(createWebrtcDefaultRequest, d.toJson(QJsonDocument::Compact).trimmed());
    QTimer* replyTimeout = new QTimer(reply);
    replyTimeout->setSingleShot(true);
    replyTimeout->start(30000);

    connect(replyTimeout, &QTimer::timeout, this, [reply]() {
        if(reply) {
            reply->abort();
        }
    });

    connect(reply, &QNetworkReply::finished, this, [this, replyTimeout, reply]() {
        if(replyTimeout->isActive()) {
            replyTimeout->stop();
        }

        if(reply) {
            qCDebug(AirlinkStreamBridgeManagerLog) << "emitting createWebrtcDefaultReply";
            QByteArray data = reply->readAll();
            QNetworkReply::NetworkError error = reply->error();
            reply->deleteLater();
            emit createWebrtcCompleted(data, error);
        }
    });
}

void AirlinkStreamBridgeManager::enableVideoTransmit() {
    QPointer<QNetworkReply> reply = manager.put(enableVideoTransmitRequest, "{}");
    QTimer* replyTimeout = new QTimer(reply);
    replyTimeout->setSingleShot(true);
    replyTimeout->start(1000);
    connect(replyTimeout, &QTimer::timeout, [reply](){
        if(reply) {
            reply->abort();
        }
    });
    connect(reply, &QNetworkReply::finished, this, [this, replyTimeout, reply](){
        if(replyTimeout->isActive()) {
            replyTimeout->stop();
        }
        if(reply) {
            QByteArray data = reply->readAll();
            QNetworkReply::NetworkError error = reply->error();
            reply->deleteLater();
            emit enableVideoTransmitCompleted(data, error);
        }
    });
}

void AirlinkStreamBridgeManager::isWebrtcReceiverConnected() {
    QPointer<QNetworkReply> reply = manager.get(isWebrtcReceiverConnectedRequest);
    QTimer* replyTimeout = new QTimer(reply);
    replyTimeout->setSingleShot(true);
    replyTimeout->start(1000);
    connect(replyTimeout, &QTimer::timeout, [reply](){
        if(reply) {
            reply->abort();
        }
    });
    connect(reply, &QNetworkReply::finished, this, [this, replyTimeout, reply](){
        if(replyTimeout->isActive()) {
            replyTimeout->stop();
        }
        if(reply) {
            qCDebug(AirlinkStreamBridgeManagerLog) << "emitting isWebrtcReceiverConnectedReply";
            QByteArray data = reply->readAll();
            QNetworkReply::NetworkError error = reply->error();
            reply->deleteLater();
            emit isWebrtcReceiverConnectedCompleted(data, error);
        }
    });
}

void AirlinkStreamBridgeManager::openPeer() {
    QPointer<QNetworkReply> reply = manager.put(openPeerRequest, "{}");
    QTimer* replyTimeout = new QTimer(reply);
    replyTimeout->setSingleShot(true);
    replyTimeout->start(30000);
    connect(replyTimeout, &QTimer::timeout, [reply](){
        qCDebug(AirlinkStreamBridgeManagerLog) << "open peer reply abort";
        if(reply) {
            reply->abort();
        }

    });
    connect(reply, &QNetworkReply::finished, this, [this, replyTimeout, reply](){
        if(replyTimeout->isActive()) {
            replyTimeout->stop();
        }

        qCDebug(AirlinkStreamBridgeManagerLog) << "emitting openPeerReply";
        if(reply) {
            QByteArray data = reply->readAll();
            QNetworkReply::NetworkError error = reply->error();
            reply->deleteLater();
            emit openPeerCompleted(data, error);
        }

    });
}

void AirlinkStreamBridgeManager::closePeer() {
    qCDebug(AirlinkStreamBridgeManagerLog) << "closePeer";
    QPointer<QNetworkReply> reply = manager.put(closePeerRequest, "{}");
    QTimer* replyTimeout = new QTimer(reply);
    replyTimeout->setSingleShot(true);
    replyTimeout->start(3000);
    connect(replyTimeout, &QTimer::timeout, [reply](){
        if(reply) {
            reply->abort();
        }
    });
    connect(reply, &QNetworkReply::finished, this, [this, replyTimeout, reply](){
        qCDebug(AirlinkStreamBridgeManagerLog) << "closePeerReply finished";
        if(replyTimeout->isActive()) {
            replyTimeout->stop();
        }
        if(reply) {
            qCDebug(AirlinkStreamBridgeManagerLog) << "emitting closePeerReply";
            QByteArray data = reply->readAll();
            QNetworkReply::NetworkError error = reply->error();
            reply->deleteLater();
            emit closePeerCompleted(data, error);
        }
    });
    qCDebug(AirlinkStreamBridgeManagerLog) << "closePeer end";
}

void AirlinkStreamBridgeManager::sendAsbServicePort(quint16 port) {
    QJsonObject obj;
    obj["protocol"] = "UDP";
    obj["address"] = "127.0.0.1";
    obj["UDPPort"] = port;
    QJsonDocument d(obj);

    QPointer<QNetworkReply> reply = manager.post(sendAsbServicePortRequest, d.toJson(QJsonDocument::Compact));
    QTimer* replyTimeout = new QTimer(reply);
    replyTimeout->setSingleShot(true);
    replyTimeout->start(3000);
    connect(replyTimeout, &QTimer::timeout, [reply](){
        if(reply) {
            reply->abort();
        }
    });
    connect(reply, &QNetworkReply::finished, this, [this, replyTimeout, reply](){
        if(replyTimeout->isActive()) {
            replyTimeout->stop();
        }
        if(reply) {
            qCDebug(AirlinkStreamBridgeManagerLog) << "emitting sendAsbServicePortReply";
            QByteArray data = reply->readAll();
            QNetworkReply::NetworkError error = reply->error();
            reply->deleteLater();
            emit sendAsbServicePortCompleted(data, error);
        }
    });
}

void AirlinkStreamBridgeManager::checkAlive() {
    QPointer<QNetworkReply> reply = manager.get(checkAliveRequest);
    QTimer* replyTimeout = new QTimer(reply);
    replyTimeout->setSingleShot(true);
    replyTimeout->start(1000);
    connect(replyTimeout, &QTimer::timeout, [reply](){
        qCDebug(AirlinkStreamBridgeManagerLog) << "abort checkAliveReply";
        if(reply) {
            reply->abort();
        }
    });
    connect(reply, &QNetworkReply::finished, this, [this, replyTimeout, reply](){
        if(replyTimeout->isActive()) {
            replyTimeout->stop();
        }
        if(reply) {
            qCDebug(AirlinkStreamBridgeManagerLog) << "emitting checkAliveReply";
            QByteArray data = reply->readAll();
            QNetworkReply::NetworkError error = reply->error();
            reply->deleteLater();
            emit checkAliveCompleted(data, error);
        }
    });
}
}


