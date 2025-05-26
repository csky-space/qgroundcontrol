#ifndef AIRLINK_H
#define AIRLINK_H

#include <UDPLink.h>

class Fact;

namespace CSKY {

class AirlinkConfiguration;
class AirlinkManager;
class AirlinkStreamBridgeManager;


class Airlink : public UDPLink
{
    Q_OBJECT
public:
    Airlink(SharedLinkConfigurationPtr& config);
    virtual ~Airlink();

    void disconnect (void) override;
    std::shared_ptr<AirlinkConfiguration> getConfig() const;
    void setWebrtcCreated(bool created);
    bool webrtcCreated() const;
    void setAsbEnabled(Fact* asbEnabled);
    void setAsbPort(Fact* asbPort);
private:
    bool webtrcReceiverCreated = false;
    QMutex _mutex;
    /// Access this varible only with _mutex locked
    bool _needToConnect {false};
    std::shared_ptr<UDPLink> connectedLink = nullptr;
    AirlinkManager* airlinkManager = nullptr;
    AirlinkStreamBridgeManager* asbManager = nullptr;
    Fact* asbEnabled = nullptr;
    Fact* asbPort = nullptr;

    void findSelf();
    /// LinkInterface overrides
    bool _connect(void) override;

    void _configureUdpSettings();
    void _sendLoginMsgToAirLink();
    bool _stillConnecting();
    void _setConnectFlag(bool connect);


signals:
    void airlinkConnected(Airlink* link = nullptr);
    void airlinkDisconnected(Airlink* link = nullptr);

    void createWebrtcDefault(QString hostName, QString modemName, QString password, quint16 port);
    void enableVideoTransmit();
    void isWebrtcReceiverConnected();
    void openPeer();
    void closePeer();
private slots:

    void connectVideo();
    void disconnectVideo();
public slots:
    void retranslateSelfConnected();
    void retranslateSelfDisconnected();
    void asbClosed();
};
}

#endif // AIRLINK_H
