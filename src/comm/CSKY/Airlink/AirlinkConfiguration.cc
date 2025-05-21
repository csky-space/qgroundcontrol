#include "AirlinkConfiguration.h"


#include <QGC.h>
#include <QGCApplication.h>
#include <AppSettings.h>
#include <SettingsManager.h>
#include <LinkManager.h>
#include <QGCLoggingCategory.h>

#include "AirlinkManager.h"

QGC_LOGGING_CATEGORY(AirlinkConfigurationLog, "AirlinkConfigurationLog")
QGC_LOGGING_CATEGORY(AirlinkLog, "AirlinkLog")

namespace CSKY {

AirlinkConfiguration::AirlinkConfiguration(const QString &name) : UDPConfiguration(name)
{
    qCInfo(AirlinkConfigurationLog) << "Airlink configuration created";
}

AirlinkConfiguration::AirlinkConfiguration(AirlinkConfiguration *source) : UDPConfiguration(source)
{
    qCInfo(AirlinkConfigurationLog) << "Airlink configuration created";
    _copyFrom(source);
}

AirlinkConfiguration::~AirlinkConfiguration()
{
}

void AirlinkConfiguration::setUsername(QString username)
{
    _username = username;
}

void AirlinkConfiguration::setPassword(QString password)
{
    _password = password;
}

void AirlinkConfiguration::setModemName(QString modemName)
{
    _modemName = modemName;
}

void AirlinkConfiguration::setOnline(QString online) {
    _online = online;
}

void AirlinkConfiguration::loadSettings(QSettings &settings, const QString &root)
{
    AppSettings *appSettings = qgcApp()->toolbox()->settingsManager()->appSettings();
    settings.beginGroup(root);
    _username = settings.value(_usernameSettingsKey, appSettings->loginAirLink()->rawValueString()).toString();
    _password = settings.value(_passwordSettingsKey, appSettings->passAirLink()->rawValueString()).toString();
    _modemName = settings.value(_modemNameSettingsKey).toString();
    settings.endGroup();
}

void AirlinkConfiguration::saveSettings(QSettings &settings, const QString &root)
{
    settings.beginGroup(root);
    settings.setValue(_usernameSettingsKey, _username);
    settings.setValue(_passwordSettingsKey, _password);
    settings.setValue(_modemNameSettingsKey, _modemName);
    settings.endGroup();
}

void AirlinkConfiguration::copyFrom(LinkConfiguration *source)
{
    //LinkConfiguration::copyFrom(source);
    auto* udpSource = qobject_cast<UDPConfiguration*>(source);
    if (udpSource) {
        UDPConfiguration::copyFrom(source);
    }
    _copyFrom(source);
}

void AirlinkConfiguration::_copyFrom(LinkConfiguration *source)
{
    auto* airlinkSource = qobject_cast<AirlinkConfiguration*>(source);
    if (airlinkSource) {
        _username = airlinkSource->username();
        _password = airlinkSource->password();
        _modemName = airlinkSource->modemName();
        _online = airlinkSource->online();
    } else {
        qCWarning(AirlinkConfigurationLog) << "Internal error: cannot read AirlinkConfiguration from given source";
    }
}


Airlink::Airlink(SharedLinkConfigurationPtr &config) : UDPLink(config)
{
    qCInfo(AirlinkLog) << "Airlink created";
    _configureUdpSettings();
#ifdef QGC_AIRLINK_ENABLED
    AirlinkManager* manager = qgcApp()->toolbox()->airlinkManager();

    connect(this, &Airlink::connected, manager, &AirlinkManager::connectVideo);
    connect(this, &Airlink::disconnected, manager, &AirlinkManager::disconnectVideo);
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
    if(connectedLink) {
        connectedLink->disconnect();
        connectedLink = nullptr;
    }
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
            findSelf();
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
    QString login = config->modemName();
    QString pass = config->password();

    std::fill(std::begin(auth.login), std::end(auth.login), 0);
    std::fill(std::begin(auth.password), std::end(auth.password), 0);

    snprintf(auth.login, sizeof(auth.login), "%s", login.toUtf8().constData());
    snprintf(auth.password, sizeof(auth.password), "%s", pass.toUtf8().constData());

    mavlink_msg_airlink_auth_pack(0, 0, &mavmsg, auth.login, auth.password);
    uint16_t len = mavlink_msg_to_send_buffer(buffer, &mavmsg);

    if (!_stillConnecting()) {
        qCDebug(AirlinkLog) << "Force exit from connection";
        return;
    }

    this->writeBytesThreadSafe((const char*)buffer, len);
}

bool Airlink::_stillConnecting()
{
    QMutexLocker locker(&_mutex);
    return _needToConnect;
}

void Airlink::_setConnectFlag(bool connect)
{
    QMutexLocker locker(&_mutex);
    _needToConnect = connect;
}

}
