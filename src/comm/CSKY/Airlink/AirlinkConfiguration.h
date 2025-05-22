#ifndef AIRLINKLINK_H
#define AIRLINKLINK_H

#include <UDPLink.h>
#include <QMutex>

namespace CSKY {

class AirlinkConfiguration : public UDPConfiguration
{
    Q_OBJECT
public:
    Q_PROPERTY(QString username     READ username       WRITE setUsername   NOTIFY usernameChanged)
    Q_PROPERTY(QString password     READ password       WRITE setPassword   NOTIFY passwordChanged)
    Q_PROPERTY(QString modemName    READ modemName      WRITE setModemName  NOTIFY modemNameChanged)
    Q_PROPERTY(QString online       READ online         WRITE setOnline     NOTIFY onlineChanged)

    AirlinkConfiguration(const QString& name);
    AirlinkConfiguration(AirlinkConfiguration* source);
    ~AirlinkConfiguration();

    QString username    () const { return _username; }
    QString password    () const { return _password; }
    QString modemName   () const { return _modemName; }
    QString online      () const { return _online; }

    void setUsername    (QString username);
    void setPassword    (QString password);
    void setModemName   (QString modemName);
    void setOnline      (QString online);

    /// LinkConfiguration overrides
#ifdef QGC_AIRLINK_ENABLED
    LinkType    type                 (void) override { return LinkConfiguration::TypeAirlink; }
#endif
    void        loadSettings         (QSettings& settings, const QString& root) override;
    void        saveSettings         (QSettings& settings, const QString& root) override;
    QString     settingsURL          (void) override { return "AirlinkSettings.qml"; }
    QString     settingsTitle        (void) override { return tr("Airlink Link Settings"); }
    void        copyFrom             (LinkConfiguration* source) override;


signals:
    void usernameChanged    (void);
    void passwordChanged    (void);
    void modemNameChanged   (void);
    void onlineChanged      (void);

private:
    void _copyFrom          (LinkConfiguration *source);


    QString _username;
    QString _password;
    QString _modemName;
    QString _online;

    const QString _usernameSettingsKey = "username";
    const QString _passwordSettingsKey = "password";
    const QString _modemNameSettingsKey = "modemName";
};

class Airlink : public UDPLink
{
    Q_OBJECT
public:
    Airlink(SharedLinkConfigurationPtr& config);
    virtual ~Airlink();

    void disconnect (void) override;
    std::shared_ptr<AirlinkConfiguration> getConfig() const;
private:
    void findSelf();
    /// LinkInterface overrides
    bool _connect(void) override;

    void _configureUdpSettings();
    void _sendLoginMsgToAirLink();
    bool _stillConnecting();
    void _setConnectFlag(bool connect);

    QMutex _mutex;
    /// Access this varible only with _mutex locked
    bool _needToConnect {false};
    std::shared_ptr<UDPLink> connectedLink = nullptr;
signals:
    void airlinkConnected(Airlink* link);
    void airlinkDisconnected(Airlink* link);
private slots:
    void retranslateSelfConnected();
    void retranslateSelfDisconnected();
};

}

#endif // AIRLINKLINK_H
