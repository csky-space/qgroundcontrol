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

QTimer* AirlinkStreamBridgeManager::createReplyTimer(size_t timeout, const std::function<void()>& onTimeout, QNetworkReply* replyParent) const {
    QTimer* replyTimeout = new QTimer(replyParent);
    replyTimeout->setSingleShot(true);
    replyTimeout->start(timeout);
    connect(replyTimeout, &QTimer::timeout, this, onTimeout);
    return replyTimeout;
}

void AirlinkStreamBridgeManager::baseRequest(QNetworkRequest& request, const QString& reqType, QJsonDocument& jsonDoc,
                                             const std::function<void(QByteArray replyData, QNetworkReply::NetworkError err)>& onReplyFinished,
                                             size_t timeout, const std::function<void()>& onTimeout) {
    QPointer<QNetworkReply> reply = nullptr;
    if((reqType != "GET") && (reqType != "HEAD"))
        reply = manager.sendCustomRequest(request, reqType.toLatin1(), jsonDoc.toJson(QJsonDocument::Compact).trimmed());
    else
        reply = manager.sendCustomRequest(request, reqType.toLatin1());
    QTimer* replyTimeoutTimer = createReplyTimer(timeout, [reply, onTimeout](){
        if(reply){
            reply->abort();
            onTimeout();
        }
    }, reply);
    connect(reply, &QNetworkReply::finished, replyTimeoutTimer, &QTimer::stop, Qt::QueuedConnection);
    connect(reply, &QNetworkReply::finished, this, [reply, onReplyFinished](){
        if(reply) {
            QByteArray data = reply->readAll();
            QNetworkReply::NetworkError error = reply->error();
            onReplyFinished(data, error);
        }
    }, Qt::QueuedConnection);
    connect(reply, &QNetworkReply::finished, reply, &QNetworkReply::deleteLater, Qt::QueuedConnection);
}

void AirlinkStreamBridgeManager::createWebrtcDefault(QString hostName, QString modemName, QString password, quint16 port) {
    QJsonObject obj;
    obj["hostName"] = hostName;
    obj["modemName"] = modemName;
    obj["password"] = password;
    obj["UDPPort"] = port;
    QJsonDocument d(obj);
    baseRequest(createWebrtcDefaultRequest, "POST", d,
                [this](QByteArray data, QNetworkReply::NetworkError error){
                    qCDebug(AirlinkStreamBridgeManagerLog) << "emitting createWebrtcCompleted";
                    emit createWebrtcCompleted(data, error);
                }, 30000);
}

void AirlinkStreamBridgeManager::enableVideoTransmit() {
    QJsonObject obj;
    QJsonDocument d(obj);
    baseRequest(enableVideoTransmitRequest, "PUT", d,
                [this](QByteArray data, QNetworkReply::NetworkError error){
                    qCDebug(AirlinkStreamBridgeManagerLog) << "emitting enableVideoTransmitCompleted";
                    emit enableVideoTransmitCompleted(data, error);
                }, 1000);
}

void AirlinkStreamBridgeManager::isWebrtcReceiverConnected() {
    QJsonDocument d;
    baseRequest(isWebrtcReceiverConnectedRequest, "GET", d,
                [this](QByteArray data, QNetworkReply::NetworkError error){
                    qCDebug(AirlinkStreamBridgeManagerLog) << "emitting isWebrtcReceiverConnectedCompleted";
                    emit isWebrtcReceiverConnectedCompleted(data, error);
                }, 1000);
}

void AirlinkStreamBridgeManager::openPeer() {
    QJsonObject obj;
    QJsonDocument d(obj);
    baseRequest(openPeerRequest, "PUT", d,
                [this](QByteArray data, QNetworkReply::NetworkError error){
                    qCDebug(AirlinkStreamBridgeManagerLog) << "emitting openPeerCompleted";
                    emit openPeerCompleted(data, error);
                }, 30000);
}

void AirlinkStreamBridgeManager::closePeer() {
    qCDebug(AirlinkStreamBridgeManagerLog) << "closePeer";
    QJsonObject obj;
    QJsonDocument d(obj);
    baseRequest(closePeerRequest, "PUT", d,
                [this](QByteArray data, QNetworkReply::NetworkError error){
                    qCDebug(AirlinkStreamBridgeManagerLog) << "emitting closePeerCompleted";
                    emit closePeerCompleted(data, error);
                }, 3000);
}

void AirlinkStreamBridgeManager::sendAsbServicePort(quint16 port) {
    QJsonObject obj;
    obj["protocol"] = "UDP";
    obj["address"] = "127.0.0.1";
    obj["UDPPort"] = port;
    QJsonDocument d(obj);

    baseRequest(sendAsbServicePortRequest, "POST", d,
                [this](QByteArray data, QNetworkReply::NetworkError error){
                    qCDebug(AirlinkStreamBridgeManagerLog) << "emitting sendAsbServicePortCompleted";
                    emit sendAsbServicePortCompleted(data, error);
                }, 3000);
}

void AirlinkStreamBridgeManager::checkAlive() {
    QJsonDocument d;
    baseRequest(checkAliveRequest, "GET", d,
                [this](QByteArray data, QNetworkReply::NetworkError error){
                    qCDebug(AirlinkStreamBridgeManagerLog) << "emitting checkAliveCompleted";
                    emit checkAliveCompleted(data, error);
                }, 1000);
}
}


