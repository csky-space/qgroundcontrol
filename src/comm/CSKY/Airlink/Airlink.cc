#include "Airlink.h"

#include <QGC.h>
#include <QGCApplication.h>
#include <AppSettings.h>
#include <SettingsManager.h>
#include <LinkManager.h>
#include <VideoManager.h>

#include "AirlinkManager.h"
#include "AirlinkConfiguration.h"

QGC_LOGGING_CATEGORY(AirlinkLog, "AirlinkLog")

namespace CSKY {
Airlink::Airlink(SharedLinkConfigurationPtr &config) : UDPLink(config)
{
    qCInfo(AirlinkLog) << "Airlink created";
    _configureUdpSettings();
#ifdef QGC_AIRLINK_ENABLED
    airlinkManager = qgcApp()->toolbox()->airlinkManager();
    asbManager = &airlinkManager->getASBManager();

    connect(airlinkManager, &AirlinkManager::asbClosed, this, &Airlink::asbClosed);
    connect(airlinkManager, &AirlinkManager::asbEnabledTrue, this, &Airlink::connectVideo);
    connect(airlinkManager, &AirlinkManager::asbEnabledFalse, this, &Airlink::disconnectVideo);

    connect(this, &Airlink::connected, airlinkManager, [this](){
        airlinkManager->addAirlink(this);
    });
    connect(this, &Airlink::connected, this, &Airlink::connectVideo);

    connect(this, &Airlink::disconnected, airlinkManager, [this](){
        qCDebug(AirlinkLog) << "disconnect video check for ours";
        qCDebug(AirlinkLog) << "Disconnect video?";
        if (airlinkManager->getAsbProcess().state() != QProcess::NotRunning) {
            airlinkManager->blockUI();
            qCDebug(AirlinkLog) << "Disconnect video";
            airlinkManager->closePeer();
        }
    });
    connect(this, &Airlink::disconnected, airlinkManager, [this](){
        airlinkManager->removeAirlink(this);
    });


    connect(this, &Airlink::blockUI, airlinkManager, &AirlinkManager::blockUI);
#endif
}

Airlink::~Airlink()
{
}

void Airlink::disconnect()
{
    qCDebug(AirlinkLog) << "Disconnecting airlink telemetry\n";


    _setConnectFlag(false);
    UDPLink::disconnect();
}

std::shared_ptr<AirlinkConfiguration> Airlink::getConfig() const {
    return std::dynamic_pointer_cast<AirlinkConfiguration>(_config);
}

void Airlink::setWebrtcCreated(bool created) {
    webtrcReceiverCreated = created;
}

bool Airlink::webrtcCreated() const {
    return webtrcReceiverCreated;
}

void Airlink::setAsbEnabled(Fact* asbEnabled) {
    this->asbEnabled = asbEnabled;
}

void Airlink::setAsbPort(Fact* asbPort) {
    this->asbPort = asbPort;
}

static bool is_ip(const QString& address)
{
    QHostAddress addr;

    return addr.setAddress(address);
}

static QString get_ip_address(const QString& address)
{
    if (is_ip(address)) {
        return address;
    }
    // Need to look it up
    QHostInfo info = QHostInfo::fromName(address);
    if (info.error() == QHostInfo::NoError) {
        QList<QHostAddress> hostAddresses = info.addresses();
        for (int i=0; i<hostAddresses.size(); i++) {
            // Exclude all IPv6 addresses
            if (!hostAddresses.at(i).toString().contains(":")) {
                return hostAddresses.at(i).toString();
            }
        }
    }
    return QString();
}

void Airlink::findSelf() {
    LinkManager* manager = qgcApp()->toolbox()->linkManager();
    if (!manager) {
        qCWarning(AirlinkLog) << "LinkManager is null!";
        return;
    }

    const QList<SharedLinkInterfacePtr> links = manager->links();
    qCDebug(AirlinkLog) << "Number of links:" << links.size();

    for (const auto& link : links) {
        if (!link || !link->linkConfiguration()) {
            qCDebug(AirlinkLog) << "Skipping invalid link or link configuration";
            continue;
        }

        qCDebug(AirlinkLog) << "Checking link:" << link->linkConfiguration()->name()
                            << "Type:" << link->linkConfiguration()->type();

        auto udpConfig = std::dynamic_pointer_cast<UDPConfiguration>(link->linkConfiguration());
        if (!udpConfig) {
            qCDebug(AirlinkLog) << "Not a UDPConfiguration, skipping";
            continue;
        }

        qCDebug(AirlinkLog) << "Found UDPConfiguration. Checking host list...";

        const QString targetHost = get_ip_address(AirlinkManager::airlinkHost + ":" + QString::number(AirlinkManager::airlinkPort));
        const auto hostList = udpConfig->hostList();

        for (const auto& host : hostList) {
            qCDebug(AirlinkLog) << "Checking host:" << host;
            if (host == targetHost && link->isConnected()) {
                qCDebug(AirlinkLog) << "Match found! Setting connect flag to true.";
                std::shared_ptr<UDPLink> udpLink = std::dynamic_pointer_cast<UDPLink>(link);
                if(udpLink)
                    connectedLink = std::dynamic_pointer_cast<UDPLink>(link);
                else
                    connectedLink = nullptr;
                _setConnectFlag(true);
                return;
            }
        }
    }

    qCDebug(AirlinkLog) << "No matching connected UDP link found.";
    _setConnectFlag(false);
}

bool Airlink::_connect()
{
    start(NormalPriority);
    QTimer *pendingTimer = new QTimer;
    connect(pendingTimer, &QTimer::timeout, [this, pendingTimer] {
        pendingTimer->setInterval(3000);
        if (_stillConnecting()) {
            qCDebug(AirlinkLog) << "Connecting...";
            _sendLoginMsgToAirLink();
        } else {
            qCDebug(AirlinkLog) << "Stopping...";
            pendingTimer->stop();
            pendingTimer->deleteLater();
        }
    });
    MAVLinkProtocol *mavlink = qgcApp()->toolbox()->mavlinkProtocol();
    auto conn = std::make_shared<QMetaObject::Connection>();
    *conn = connect(mavlink, &MAVLinkProtocol::messageReceived, [this, conn] (LinkInterface* linkSrc, mavlink_message_t message) {
        if ((this != linkSrc) || (message.msgid != MAVLINK_MSG_ID_AIRLINK_AUTH_RESPONSE)) {
            return;
        }
        mavlink_airlink_auth_response_t responseMsg;
        mavlink_msg_airlink_auth_response_decode(&message, &responseMsg);
        int answer = responseMsg.resp_type;
        if (answer != AIRLINK_AUTH_RESPONSE_TYPE::AIRLINK_AUTH_OK) {
            qCDebug(AirlinkLog) << "Airlink auth failed";
            return;
        }
        qCDebug(AirlinkLog) << "Connected successfully";
        QObject::disconnect(*conn);
        _setConnectFlag(false);
    });
    _setConnectFlag(true);
    pendingTimer->start(0);
    return true;
}

void Airlink::_configureUdpSettings()
{
    quint16 availablePort = 14550;
    QUdpSocket udpSocket;
    while (!udpSocket.bind(QHostAddress::LocalHost, availablePort))
        availablePort++;
    UDPConfiguration* udpConfig = dynamic_cast<UDPConfiguration*>(UDPLink::_config.get());
    udpConfig->addHost(AirlinkManager::airlinkHost, AirlinkManager::airlinkPort);
    udpConfig->setLocalPort(availablePort);
    udpConfig->setDynamic(false);
}

void Airlink::_sendLoginMsgToAirLink()
{
    __mavlink_airlink_auth_t auth;
    uint8_t buffer[MAVLINK_MAX_PACKET_LEN];
    mavlink_message_t mavmsg;
    AirlinkConfiguration* config = dynamic_cast<AirlinkConfiguration*>(_config.get());
    qCDebug(AirlinkLog) << "before conf gets";
    QString login = config->modemName();
    QString pass = config->password();

    std::fill(std::begin(auth.login), std::end(auth.login), 0);
    std::fill(std::begin(auth.password), std::end(auth.password), 0);
    qCDebug(AirlinkLog) << "before print authorization";
    snprintf(auth.login, sizeof(auth.login), "%s", login.toUtf8().constData());
    snprintf(auth.password, sizeof(auth.password), "%s", pass.toUtf8().constData());

    qCDebug(AirlinkLog) << "before auth";
    mavlink_msg_airlink_auth_pack(0, 0, &mavmsg, auth.login, auth.password);
    uint16_t len = mavlink_msg_to_send_buffer(buffer, &mavmsg);

    if (!_stillConnecting()) {
        qCDebug(AirlinkLog) << "Force exit from connection";
        return;
    }
    qCDebug(AirlinkLog) << "before write bytes";
    this->writeBytesThreadSafe((const char*)buffer, len);
    qCDebug(AirlinkLog) << "after write bytes";
}

bool Airlink::_stillConnecting()
{
    return _needToConnect.load(std::memory_order_acquire);
}

void Airlink::_setConnectFlag(bool connect)
{
    _needToConnect.store(connect, std::memory_order_release);
}

void Airlink::retranslateSelfConnected() {
    qCDebug(AirlinkLog) << "airlink connected retranslation";
    emit airlinkConnected(this);
}

void Airlink::retranslateSelfDisconnected() {
    qCDebug(AirlinkLog) << "airlink disconnected retranslation";
    emit airlinkDisconnected(this);
}

void Airlink::connectVideo() {
    if(!isConnected())
        return;
    if(asbEnabled == nullptr)
        asbEnabled = airlinkManager->getAsbEnabled();
    if(asbPort == nullptr)
        asbPort = airlinkManager->getPort();
    if ((airlinkManager->getAsbProcess().state() == QProcess::Running) && asbEnabled->rawValue().toBool()) {
        qCDebug(AirlinkLog) << "asb is on";
        auto configuration = std::dynamic_pointer_cast<AirlinkConfiguration>(_config);
        if(!configuration) {
            asbEnabled->setRawValue(false);
            qCDebug(AirlinkLog) << "Airlink configuration doesn't exist yet";
            return;
        }

        if(!webtrcReceiverCreated) {
            qCDebug(AirlinkLog()) << "Airlink video connecting for " << configuration->modemName();
            emit blockUI();

            emit  airlinkManager->createWebrtcDefault(AirlinkManager::airlinkHost, configuration->modemName(), configuration->password(), asbPort->rawValue().toUInt());
        }
        else {
            emit blockUI();
            emit airlinkManager->openPeer();
        }
    }
    else if ((airlinkManager->getAsbProcess().state() != QProcess::Running) && asbEnabled){
        asbEnabled->setRawValue(false);
        qCDebug(AirlinkLog) << "Airlink AirlinkStreamBridge didn't run";
    }
}

void Airlink::disconnectVideo() {
    qCDebug(AirlinkLog) << "disconnect video check for ours";
    qCDebug(AirlinkLog) << "Disconnect video?";
    if (airlinkManager->getAsbProcess().state() != QProcess::NotRunning) {
        emit blockUI();
        qCDebug(AirlinkLog) << "Disconnect video";
        emit airlinkManager->closePeer();
    }
}

void Airlink::asbClosed(Airlink* airlink) {
    if(airlink != this) {
        return;
    }
    webtrcReceiverCreated = false;
}
}
