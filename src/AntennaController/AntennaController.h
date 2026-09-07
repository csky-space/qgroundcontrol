/// @file AntennaController.h

#pragma once

#include <array>
#include <bitset>
#include <cstdint>

#include <QLoggingCategory>
#include <QNetworkAccessManager>
#include <QObject>
#include <QVariant>
#include <QQmlEngine>

#include "QGCToolbox.h"

Q_DECLARE_LOGGING_CATEGORY(AntennaControllerLog)

class AntennaController : public QGCTool {
    Q_OBJECT

public:
    Q_PROPERTY(QString address     READ address     WRITE setAddress     NOTIFY addressChanged)
    Q_PROPERTY(quint16 port        READ port        WRITE setPort        NOTIFY portChanged)
    Q_PROPERTY(QString login       READ login       WRITE setLogin       NOTIFY loginChanged)
    Q_PROPERTY(QString password    READ password    WRITE setPassword    NOTIFY passwordChanged)

    Q_INVOKABLE void setAddress  (QString address);
    Q_INVOKABLE void setPort     (quint16 port);
    Q_INVOKABLE void setLogin    (QString login);
    Q_INVOKABLE void setPassword (QString password);

    Q_INVOKABLE void sendSetAngleCommand(qint32 angle);
    Q_INVOKABLE void sendSetAntennaCommand(qint8 antennaIndex);
    Q_INVOKABLE void resetSettings();

    // void onAuthRequired(QNetworkReply* reply, QAuthenticator* authenticator);

public:
    AntennaController(QGCApplication* app, QGCToolbox* toolbox);
    virtual ~AntennaController();

    void setToolbox(QGCToolbox *toolbox);

    QString address  () const;
    quint16 port     () const;
    QString login    () const;
    QString password () const;

private:
    QNetworkAccessManager m_networkManager;

    void _saveSettings();
    void _loadSettings();
    void _sendCommand(const QByteArray& bytes);
    void _getDeviceSettings();
    void _updateToken();

signals:
    void addressChanged  ();
    void portChanged     ();
    void loginChanged    ();
    void passwordChanged ();

private:
    static inline const QString _defaultAddress = "192.168.2.2";
    static inline const quint16 _defaultPort = 8000;
    static inline const QString _defaultLogin = "Operator";
    static inline const QString _defaultPassword = "167124";

    QString _address  = _defaultAddress;
    quint16 _port     = _defaultPort;
    QString _login    = _defaultLogin;
    QString _password = _defaultPassword;
    QString _token    = "";

    static const char* _settingsGroup;
    static const char* _addressKey;
    static const char* _portKey;
    static const char* _loginKey;
    static const char* _passwordKey;
};
