#include "airlinkstreambridgemanager.h"

#include <QGCLoggingCategory.h>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTimer>

QGC_LOGGING_CATEGORY(AirlinkStreamBridgeManagerLog, "AirlinkStreamBridgeManagerLog")

namespace CSKY {
AirlinkStreamBridgeManager::AirlinkStreamBridgeManager()
    : sslConfig(QSslConfiguration::defaultConfiguration())
    ,  manager(this)
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
    if(createWebrtcDefaultReply)
        createWebrtcDefaultReply->deleteLater();
    if(enableVideoTransmitReply)
        enableVideoTransmitReply->deleteLater();
    if(isWebrtcReceiverConnectedReply)
        isWebrtcReceiverConnectedReply->deleteLater();
    if(openPeerReply)
        openPeerReply->deleteLater();
    if(closePeerReply)
        closePeerReply->deleteLater();
    if(sendAsbServicePortReply)
        sendAsbServicePortReply->deleteLater();
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
    replyTimeout->start(5000);
    connect(replyTimeout, &QTimer::timeout, this, [this](){
        createWebrtcDefaultReply->abort();

    });
    connect(createWebrtcDefaultReply, &QNetworkReply::finished, this, [this, replyTimeout](){
        replyTimeout->stop();
        qCDebug(AirlinkStreamBridgeManagerLog) << "emitting createWebrtcDefaultReply";
        emit createWebrtcCompleted(createWebrtcDefaultReply->readAll(), createWebrtcDefaultReply->error());
    });
}

void AirlinkStreamBridgeManager::enableVideoTransmit() {
    enableVideoTransmitReply = manager.put(enableVideoTransmitRequest, "{}");
    QTimer* replyTimeout = new QTimer(enableVideoTransmitReply);
    replyTimeout->setSingleShot(true);
    replyTimeout->start(1000);
    connect(replyTimeout, &QTimer::timeout, [this](){
        enableVideoTransmitReply->abort();
    });
    connect(enableVideoTransmitReply, &QNetworkReply::finished, this, [this, replyTimeout](){
        replyTimeout->stop();
        emit enableVideoTransmitCompleted(enableVideoTransmitReply->readAll(), enableVideoTransmitReply->error());
    });
}

void AirlinkStreamBridgeManager::isWebrtcReceiverConnected() {
    isWebrtcReceiverConnectedReply = manager.get(isWebrtcReceiverConnectedRequest);
    QTimer* replyTimeout = new QTimer(isWebrtcReceiverConnectedReply);
    replyTimeout->setSingleShot(true);
    replyTimeout->start(1000);
    connect(replyTimeout, &QTimer::timeout, [this](){
        isWebrtcReceiverConnectedReply->abort();
    });
    connect(isWebrtcReceiverConnectedReply, &QNetworkReply::finished, this, [this, replyTimeout](){
        replyTimeout->stop();
        qCDebug(AirlinkStreamBridgeManagerLog) << "emitting isWebrtcReceiverConnectedReply";
        emit isWebrtcReceiverConnectedCompleted(isWebrtcReceiverConnectedReply->readAll(), isWebrtcReceiverConnectedReply->error());
    });
}

void AirlinkStreamBridgeManager::openPeer() {
    openPeerReply = manager.put(openPeerRequest, "{}");
    QTimer* replyTimeout = new QTimer(openPeerReply);
    replyTimeout->setSingleShot(true);
    replyTimeout->start(10000);
    connect(replyTimeout, &QTimer::timeout, [this](){
        qCDebug(AirlinkStreamBridgeManagerLog) << "open peer reply abort";
        openPeerReply->abort();
    });
    connect(openPeerReply, &QNetworkReply::finished, this, [this, replyTimeout](){
        replyTimeout->stop();
        qCDebug(AirlinkStreamBridgeManagerLog) << "emitting openPeerReply";
        emit openPeerCompleted(openPeerReply->readAll(), openPeerReply->error());
    });
}

void AirlinkStreamBridgeManager::closePeer() {
    closePeerReply = manager.put(closePeerRequest, "{}");
    QTimer* replyTimeout = new QTimer(closePeerReply);
    replyTimeout->setSingleShot(true);
    replyTimeout->start(3000);
    connect(replyTimeout, &QTimer::timeout, [this](){
        closePeerReply->abort();
    });
    connect(closePeerReply, &QNetworkReply::finished, this, [this, replyTimeout](){
        replyTimeout->stop();
        qCDebug(AirlinkStreamBridgeManagerLog) << "emitting closePeerReply";
        emit closePeerCompleted(closePeerReply->readAll(), closePeerReply->error());
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
        sendAsbServicePortReply->abort();
    });
    connect(sendAsbServicePortReply, &QNetworkReply::finished, this, [this, replyTimeout](){
        replyTimeout->stop();
        qCDebug(AirlinkStreamBridgeManagerLog) << "emitting sendAsbServicePortReply";
        emit sendAsbServicePortCompleted(sendAsbServicePortReply->readAll(), sendAsbServicePortReply->error());
    });
}

void AirlinkStreamBridgeManager::checkAlive() {
    checkAliveReply = manager.get(checkAliveRequest);
    QTimer* replyTimeout = new QTimer(checkAliveReply);
    replyTimeout->setSingleShot(true);
    replyTimeout->start(1000);
    connect(replyTimeout, &QTimer::timeout, [this](){
        qCDebug(AirlinkStreamBridgeManagerLog) << "abort checkAliveReply";
        checkAliveReply->abort();
    });
    connect(checkAliveReply, &QNetworkReply::finished, this, [this, replyTimeout](){
        replyTimeout->stop();
        qCDebug(AirlinkStreamBridgeManagerLog) << "emitting checkAliveReply";
        emit checkAliveCompleted(checkAliveReply->readAll(), checkAliveReply->error());
    });
}
}


