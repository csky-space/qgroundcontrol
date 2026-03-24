#include "airlinkstreambridgemanager.h"

#include <QGCLoggingCategory.h>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTimer>
#include <QPointer>
#include <QGCApplication.h>
#include <SettingsManager.h>

#include "AirlinkManager.h"

QGC_LOGGING_CATEGORY(AirlinkStreamBridgeManagerLog, "AirlinkStreamBridgeManagerLog")

namespace CSKY {
AirlinkStreamBridgeManager::AirlinkStreamBridgeManager(QObject* parent)
    : QObject(parent)
    , sslConfig(QSslConfiguration::defaultConfiguration())
    , manager(this)
    , codecWatchdogTimer(new QTimer())
    , currentCodec(VideoSettings::videoDisabled)
    , _initialPort(9050)
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

    setupTransportPolicy.setUrl(QUrl(baseASBRequestsPath + baseWebrtcRequestsPath + "setupTransportPolicy"));
    setupTransportPolicy.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    setupTransportPolicy.setSslConfiguration(sslConfig);

    sendAsbServicePortRequest.setUrl(QUrl(baseASBRequestsPath + baseWebrtcRequestsPath + "setupOutputProtocol"));
    sendAsbServicePortRequest.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    sendAsbServicePortRequest.setSslConfiguration(sslConfig);

    checkAliveRequest.setUrl(QUrl(baseASBRequestsPath));
    checkAliveRequest.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    checkAliveRequest.setSslConfiguration(sslConfig);

    getCodecRequest.setUrl(QUrl(baseASBRequestsPath + baseVideoRequestsPath + "getCodec"));
    getCodecRequest.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    getCodecRequest.setSslConfiguration(sslConfig);

    videoIsRunningRequest.setUrl(QUrl(baseASBRequestsPath + baseVideoRequestsPath + "isRunning"));
    videoIsRunningRequest.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    videoIsRunningRequest.setSslConfiguration(sslConfig);

    connect(codecWatchdogTimer.get(), &QTimer::timeout, this, [this](){
        getCodec();
    });
    connect(this, &AirlinkStreamBridgeManager::getCodecCompleted, this, [this](QByteArray replyData, QNetworkReply::NetworkError err){
        qCDebug(AirlinkStreamBridgeManagerLog) << "getCodecCompleted with error: " << err;
        if((err == QNetworkReply::NoError) && qgcApp()->toolbox()->airlinkManager()->getAutotuneEnabled()->rawValue().toBool()) {
            qCDebug(AirlinkStreamBridgeManagerLog) << "setup codec from ASB";
            QJsonDocument d = QJsonDocument::fromJson(replyData);
            if(!d.isEmpty() && d.object().contains("codec") && d.object()["codec"].isString() && !d.object()["codec"].toString().isEmpty()) {
                if(d["codec"].toString().contains("h264", Qt::CaseInsensitive)) {
                    qCDebug(AirlinkStreamBridgeManagerLog) << "current codec is: " << VideoSettings::videoSourceUDPH264;
                    currentCodec = VideoSettings::videoSourceUDPH264;
                }
                else if(d["codec"].toString().contains("h265", Qt::CaseInsensitive)) {
                    qCDebug(AirlinkStreamBridgeManagerLog) << "current codec is: " << VideoSettings::videoSourceUDPH265;
                    currentCodec = VideoSettings::videoSourceUDPH265;
                }
                else {
                    qCDebug(AirlinkStreamBridgeManagerLog) << "current codec is: " << VideoSettings::videoDisabled;
                    currentCodec = VideoSettings::videoDisabled;
                }
                qgcApp()->toolbox()->settingsManager()->videoSettings()->videoSource()->setRawValue(currentCodec);
            } else if (!d.isEmpty() && d.object().contains("codec") && d.object()["codec"].isString()){
                qCDebug(AirlinkStreamBridgeManagerLog) << "current codec is: " << d.object()["codec"].toString();
            }
            //d["codec"]

        }

    });

    connect(this, &AirlinkStreamBridgeManager::sendAsbServicePortCompleted, this, [](QByteArray replyData, QNetworkReply::NetworkError err){
        qCDebug(AirlinkStreamBridgeManagerLog) << "sendAsbServicePortCompleted with error: " << err;
        //if((err == QNetworkReply::NoError) && ) {
        //    qgcApp()->toolbox()->settingsManager()->videoSettings()->videoSource()->setRawValue(currentCodec);
        //}
    });

    connect(this, &AirlinkStreamBridgeManager::videoIsRunningCompleted, this, [this](QByteArray replyData, QNetworkReply::NetworkError err){
        qCDebug(AirlinkStreamBridgeManagerLog) << "videoIsRunningCompleted with error: " << err;
        if(err == QNetworkReply::NoError) {
            qCDebug(AirlinkStreamBridgeManagerLog) << "setup codec from ASB";
            QJsonDocument d = QJsonDocument::fromJson(replyData);
            qCDebug(AirlinkStreamBridgeManagerLog) << "isRunning response: " << replyData;
            static qint16 port = qgcApp()->toolbox()->airlinkManager()->getPort()->rawValue().toInt();
            if(!d.isEmpty() && d.object().contains("isConnected") && d.object()["isConnected"].isBool() && d.object()["isConnected"].toBool()) {
                if(!_isRunning && qgcApp()->toolbox()->settingsManager()->asbSettings()->asbAutotune()->rawValue().toBool()) {
                    qCDebug(AirlinkStreamBridgeManagerLog) << "Increment port";
                    port += 1;
                    _selfSetupPort = true;
                    qgcApp()->toolbox()->airlinkManager()->getPort()->setRawValue(port);
                }
                _isRunning = true;
            } else {
                _isRunning = false;
            }
        }

    });
}

AirlinkStreamBridgeManager::~AirlinkStreamBridgeManager() {

}

void AirlinkStreamBridgeManager::startConstrainVideoCodec() {
    qCDebug(AirlinkStreamBridgeManagerLog) << "start constrain video codec";
    codecWatchdogTimer->start(500);
}

void AirlinkStreamBridgeManager::stopConstrainVideoCodec() {
    codecWatchdogTimer->stop();
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
    if(manager.thread() != thread())
        manager.moveToThread(thread());
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
    connect(reply, &QNetworkReply::finished, replyTimeoutTimer, &QTimer::stop);
    connect(reply, &QNetworkReply::finished, this, [reply, onReplyFinished](){
        if(reply) {
            QByteArray data = reply->readAll();
            QNetworkReply::NetworkError error = reply->error();
            onReplyFinished(data, error);
        }
    });
    connect(reply, &QNetworkReply::finished, reply, &QNetworkReply::deleteLater);
}

void AirlinkStreamBridgeManager::init() {
    connect(qgcApp()->toolbox()->airlinkManager()->getPort(), &Fact::rawValueChanged, this, [this](QVariant changedValue){
        if(!_selfSetupPort) {
            _initialPort = changedValue.toInt();
        }
        else {
            _selfSetupPort = false;
        }
    });
}

void AirlinkStreamBridgeManager::setToInitialPort() {
    qgcApp()->toolbox()->settingsManager()->asbSettings()->asbPort()->setRawValue(_initialPort);
}

void AirlinkStreamBridgeManager::createWebrtcDefault(QString hostName, QString modemName, QString password, quint16 port, QString policy) {
    QJsonObject obj;
    obj["hostName"] = hostName;
    obj["modemName"] = modemName;
    obj["password"] = password;
    obj["IcePolicy"] = policy;

    QJsonDocument d(obj);
    //qCDebug(AirlinkStreamBridgeManagerLog) << "create webrtc with: " << d.toJson();
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

void AirlinkStreamBridgeManager::sendAsbServiceTransportPolicy(const QString& policy) {
    QJsonObject obj;
    obj["IcePolicy"] = policy;
    QJsonDocument d(obj);

    baseRequest(setupTransportPolicy, "POST", d,
                [this](QByteArray data, QNetworkReply::NetworkError error){
                    qCDebug(AirlinkStreamBridgeManagerLog) << "emitting sendAsbServiceTransportPolicyCompleted";
                    emit sendAsbServiceTransportPolicyCompleted(data, error);
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

void AirlinkStreamBridgeManager::getCodec() {
    QJsonDocument d;
    baseRequest(getCodecRequest, "GET", d,
                [this](QByteArray data, QNetworkReply::NetworkError error){
                    qCDebug(AirlinkStreamBridgeManagerLog) << "emitting getCodecCompleted";
                    emit getCodecCompleted(data, error);
                }, 1000);
}

void AirlinkStreamBridgeManager::isRunning() {
    QJsonDocument d;
    baseRequest(videoIsRunningRequest, "GET", d,
                [this](QByteArray data, QNetworkReply::NetworkError error){
                    qCDebug(AirlinkStreamBridgeManagerLog) << "emitting videoIsRunningCompleted";
                    emit videoIsRunningCompleted(data, error);
                }, 1000);
}

}


