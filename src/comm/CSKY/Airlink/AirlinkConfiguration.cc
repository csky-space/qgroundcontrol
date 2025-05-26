#include "AirlinkConfiguration.h"

#include <QGCApplication.h>
#include <QGCLoggingCategory.h>
#include <SettingsManager.h>


QGC_LOGGING_CATEGORY(AirlinkConfigurationLog, "AirlinkConfigurationLog")

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
    qCDebug(AirlinkConfigurationLog) << "airlink modemName set: " << modemName;
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

}
