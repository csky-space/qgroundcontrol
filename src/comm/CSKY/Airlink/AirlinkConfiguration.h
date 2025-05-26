#ifndef AIRLINKLINK_H
#define AIRLINKLINK_H

#include <UDPLink.h>

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

}

#endif // AIRLINKLINK_H
