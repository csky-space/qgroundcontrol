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
    std::atomic<bool> _needToConnect{false};
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
private slots:

    void connectVideo(Airlink* airlink = nullptr);
    void disconnectVideo(Airlink* airlink = nullptr);
public slots:
    void retranslateSelfConnected();
    void retranslateSelfDisconnected();
    void asbClosed(Airlink* airlink);
};
}

#endif // AIRLINK_H
