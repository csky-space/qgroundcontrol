#ifndef AIRLINKSTREAMBRIDGEMANAGER_H
#define AIRLINKSTREAMBRIDGEMANAGER_H

#include <functional>

#include <qsslconfiguration.h>
#include <QNetworkRequest>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QScopedPointer>

class QTimer;

namespace CSKY {
class AirlinkStreamBridgeManager : public QObject {
    Q_OBJECT
public:
    AirlinkStreamBridgeManager();
    ~AirlinkStreamBridgeManager();

    void startConstrainVideoCodec();
    void stopConstrainVideoCodec();
private:
    QTimer* createReplyTimer(size_t timeout, const std::function<void()>& onTimeout, QNetworkReply* replyParent) const;
    void baseRequest(QNetworkRequest& request, const QString& reqType, QJsonDocument& jsonDoc,
                     const std::function<void(QByteArray replyData, QNetworkReply::NetworkError err)>& onReplyFinished,
                     size_t timeout, const std::function<void()>& onTimeout = [](){});

    QSslConfiguration sslConfig;

    QString baseASBRequestsPath = "https://localhost:8443/";
    QString baseWebrtcRequestsPath = "Webrtc/";
    QString baseAppRequestsPath = "App/";
    QString baseConnectionRequestsPath = "Connection/";
    QString baseVideoRequestsPath = "Video/";

    QNetworkAccessManager manager;
    QNetworkRequest createWebrtcDefaultRequest;
    QNetworkRequest enableVideoTransmitRequest;
    QNetworkRequest isWebrtcReceiverConnectedRequest;
    QNetworkRequest openPeerRequest;
    QNetworkRequest setupTransportPolicy;

    QNetworkRequest closePeerRequest;
    QNetworkRequest sendAsbServicePortRequest;
    QNetworkRequest checkAliveRequest;
    QNetworkRequest getCodecRequest;
    QNetworkRequest videoIsRunningRequest;

    QScopedPointer<QTimer> codecWatchdogTimer;
    QString currentCodec;
    qint16 _initialPort;
    bool _selfSetupPort = false;
    bool _isRunning = false;

signals:
    void createWebrtcCompleted(QByteArray replyData, QNetworkReply::NetworkError err);
    void enableVideoTransmitCompleted(QByteArray replyData, QNetworkReply::NetworkError err);
    void isWebrtcReceiverConnectedCompleted(QByteArray replyData, QNetworkReply::NetworkError err);
    void openPeerCompleted(QByteArray replyData, QNetworkReply::NetworkError err);
    void closePeerCompleted(QByteArray replyData, QNetworkReply::NetworkError err);
    void sendAsbServicePortCompleted(QByteArray replyData, QNetworkReply::NetworkError err);
    void sendAsbServiceTransportPolicyCompleted(QByteArray replyData, QNetworkReply::NetworkError err);
    void checkAliveCompleted(QByteArray replyData, QNetworkReply::NetworkError err);
    void getCodecCompleted(QByteArray replyData, QNetworkReply::NetworkError err);
    void videoIsRunningCompleted(QByteArray replyData, QNetworkReply::NetworkError err);
public slots:
    //requests
    void init();
    void setToInitialPort();
    void createWebrtcDefault(QString hostName, QString modemName, QString password, quint16 port, QString policy);
    void enableVideoTransmit();
    void isWebrtcReceiverConnected();
    void openPeer();
    void closePeer();
    void sendAsbServicePort(quint16 port);
    void sendAsbServiceTransportPolicy(const QString& policy);
    void checkAlive();
    void getCodec();
    void isRunning();
};
}

#endif // AIRLINKSTREAMBRIDGEMANAGER_H
