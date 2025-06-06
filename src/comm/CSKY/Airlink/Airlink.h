#ifndef AIRLINK_H
#define AIRLINK_H

#include <QNetworkReply>

#include <UDPLink.h>

class Fact;

namespace CSKY {

class AirlinkConfiguration;
class AirlinkManager;
class AirlinkStreamBridgeManager;
class AirlinkVideo;


class Airlink : public UDPLink
{
    Q_OBJECT
public:
    Airlink(SharedLinkConfigurationPtr& config);
    virtual ~Airlink();

    void disconnect (void) override;
    std::shared_ptr<AirlinkConfiguration> getConfig() const;
    void setAsbEnabled(Fact* asbEnabled);
    void setAsbPort(Fact* asbPort);
private:
    QThread* _videoThread;
    AirlinkVideo* _video;

    QMetaObject::Connection* onAddAirlinkConnection = new QMetaObject::Connection;
    QMetaObject::Connection* onRemoveAirlinkConnection = new QMetaObject::Connection;
    void setConnections();
    void unsetConnections();

    QMutex _mutex;
    /// Access this varible only with _mutex locked
    std::atomic<bool> _needToConnect{false};
    std::shared_ptr<UDPLink> connectedLink = nullptr;
    AirlinkManager* airlinkManager = nullptr;
    AirlinkStreamBridgeManager* asbManager = nullptr;
    Fact* asbEnabled = nullptr;
    Fact* asbPort = nullptr;

    /// LinkInterface overrides
    bool _connect(void) override;

    void _configureUdpSettings();
    void _sendLoginMsgToAirLink();
    bool _stillConnecting();
    void _setConnectFlag(bool connect);
signals:
    void airlinkConnected(Airlink* link = nullptr);
    void airlinkDisconnected(Airlink* link = nullptr);
    void blockUI(QByteArray replyData = {}, QNetworkReply::NetworkError err = QNetworkReply::NoError);

    void connectVideoReady(QString modemName, QString password, quint16 port);
    void disconnectVideoReady();

    void _asbClosed();
    void videoDisconnected();
private slots:

    void connectVideo();
    void disconnectVideo();
public slots:
    void emitVideoDisconnected();
    void asbClosed(Airlink* airlink);
};
}

#endif // AIRLINK_H
