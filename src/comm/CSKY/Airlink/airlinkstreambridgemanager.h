#ifndef AIRLINKSTREAMBRIDGEMANAGER_H
#define AIRLINKSTREAMBRIDGEMANAGER_H

#include <qsslconfiguration.h>
#include <QNetworkRequest>
#include <QNetworkAccessManager>
#include <QNetworkReply>

namespace CSKY {
class AirlinkStreamBridgeManager : public QObject {
    Q_OBJECT
public:
    AirlinkStreamBridgeManager();
    ~AirlinkStreamBridgeManager();
private:
    QSslConfiguration sslConfig;

    QString baseASBRequestsPath = "https://localhost:8443/";
    QString baseWebrtcRequestsPath = "Webrtc/";
    QString baseAppRequestsPath = "App/";
    QString baseConnectionRequestsPath = "Connection/";
    QString baseVideoRequestsPath = "Video/";

    QNetworkAccessManager manager;
    QNetworkRequest createWebrtcDefaultRequest;
    QNetworkReply* createWebrtcDefaultReply = nullptr;
    QNetworkRequest enableVideoTransmitRequest;
    QNetworkReply* enableVideoTransmitReply = nullptr;
    QNetworkRequest isWebrtcReceiverConnectedRequest;
    QNetworkReply* isWebrtcReceiverConnectedReply = nullptr;
    QNetworkRequest openPeerRequest;
    QNetworkReply* openPeerReply = nullptr;
    QNetworkRequest closePeerRequest;
    QNetworkReply* closePeerReply = nullptr;
    QNetworkRequest sendAsbServicePortRequest;
    QNetworkReply* sendAsbServicePortReply = nullptr;
    QNetworkRequest checkAliveRequest;
    QNetworkReply* checkAliveReply = nullptr;


signals:
    void createWebrtcCompleted(QByteArray replyData, QNetworkReply::NetworkError err);
    void enableVideoTransmitCompleted(QByteArray replyData, QNetworkReply::NetworkError err);
    void isWebrtcReceiverConnectedCompleted(QByteArray replyData, QNetworkReply::NetworkError err);
    void openPeerCompleted(QByteArray replyData, QNetworkReply::NetworkError err);
    void closePeerCompleted(QByteArray replyData, QNetworkReply::NetworkError err);
    void sendAsbServicePortCompleted(QByteArray replyData, QNetworkReply::NetworkError err);
    void checkAliveCompleted(QByteArray replyData, QNetworkReply::NetworkError err);
public slots:
    //requests
    void createWebrtcDefault(QString hostName, QString modemName, QString password, quint16 port);
    void enableVideoTransmit();
    void isWebrtcReceiverConnected();
    void openPeer();
    void closePeer();
    void sendAsbServicePort(quint16 port);
    void checkAlive();
};
}

#endif // AIRLINKSTREAMBRIDGEMANAGER_H
