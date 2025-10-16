#ifndef C_AIRLINK_VIDEO_H
#define C_AIRLINK_VIDEO_H

#include <QObject>
#include <QNetworkReply>
#include <QByteArray>

class QThread;

namespace CSKY {
class AirlinkStreamBridgeManager;
class AirlinkManager;
class Airlink;

class AirlinkVideo : public QObject {
    Q_OBJECT
public:
    explicit AirlinkVideo(AirlinkStreamBridgeManager* asbManager, AirlinkManager* airlinkManager, Airlink* modem, QObject* parent = nullptr);
    ~AirlinkVideo();

private:
    AirlinkStreamBridgeManager* _asbManager;
    AirlinkManager* _airlinkManager;
    Airlink* _modem;

    bool webtrcReceiverCreated;

    void setConnections();
    void unsetConnections();
signals:
    void createWebrtcDefault(QString hostName, QString modemName, QString password, quint16 port);
    void enableVideoTransmit();
    void isWebrtcReceiverConnected();
    void openPeer();
    void closePeer();
    void closePeerCompleted();

    void blockUI(QByteArray replyData = {}, QNetworkReply::NetworkError = QNetworkReply::NoError);
    void disconnected();
public slots:
    void _connect(QString modemName, QString password, quint16 port);
    void _disconnect();

    void webrtcCreated(QByteArray replyData, QNetworkReply::NetworkError err);
    void peerOpened(QByteArray replyData, QNetworkReply::NetworkError err);
    void peerClosed(QByteArray replyData, QNetworkReply::NetworkError err);

    void setWebrtcCreated(bool created);
    void asbFailed();
private slots:
};

}

#endif
