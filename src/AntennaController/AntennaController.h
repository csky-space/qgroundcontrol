/// @file AntennaController.h

#pragma once

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
    Q_PROPERTY(QString address           READ address           WRITE setAddress           NOTIFY addressChanged)
    Q_PROPERTY(quint16 port              READ port              WRITE setPort              NOTIFY portChanged)
    Q_PROPERTY(QString login             READ login             WRITE setLogin             NOTIFY loginChanged)
    Q_PROPERTY(QString password          READ password          WRITE setPassword          NOTIFY passwordChanged)
    Q_PROPERTY(bool    isBusy            READ isBusy                                       NOTIFY isBusyChanged)
    Q_PROPERTY(qint32  minAngle          READ minAngle          WRITE setMinAngle          NOTIFY minAngleChanged)
    Q_PROPERTY(qint32  maxAngle          READ maxAngle          WRITE setMaxAngle          NOTIFY maxAngleChanged)
    Q_PROPERTY(qint32  currentAngle      READ currentAngle      WRITE setCurrentAngle      NOTIFY currentAngleChanged)
    Q_PROPERTY(qint32  currentAntenna    READ currentAntenna    WRITE setCurrentAntenna    NOTIFY currentAntennaChanged)

    Q_INVOKABLE void setAddress        (QString address);
    Q_INVOKABLE void setPort           (quint16 port);
    Q_INVOKABLE void setLogin          (QString login);
    Q_INVOKABLE void setPassword       (QString password);
    Q_INVOKABLE void setMinAngle       (qint32 angle);
    Q_INVOKABLE void setMaxAngle       (qint32 angle);
    Q_INVOKABLE void setCurrentAngle   (qint32 angle);
    Q_INVOKABLE void setCurrentAntenna (qint32 index);

    Q_INVOKABLE void resetAccessSettings   ();
    Q_INVOKABLE void resetConfiguration    ();

    // void onAuthRequired(QNetworkReply* reply, QAuthenticator* authenticator);

public:
    AntennaController(QGCApplication* app, QGCToolbox* toolbox);
    virtual ~AntennaController();

    void setToolbox(QGCToolbox *toolbox);

    QString address        () const;
    quint16 port           () const;
    QString login          () const;
    QString password       () const;
    bool    isBusy         () const;
    qint32  minAngle       () const;
    qint32  maxAngle       () const;
    qint32  currentAngle   () const;
    qint32  currentAntenna () const;

private:
    QNetworkAccessManager m_networkManager;

    void _saveSettings        ();
    void _loadSettings        ();
    void _sendCommand         (const QByteArray& bytes, std::function<void(const QByteArray& response)> callback = nullptr);
    void _getDeviceSettings   (std::function<void(const QByteArray& response)> callback);
    void _connectReply        (QNetworkReply* reply, std::function<void(const QByteArray& response)> callback = nullptr);
    bool _isValidIP           (const QString &ipString);

    void _onSettingRequestGetAntennaID(const QByteArray& bytes);

    QNetworkRequest _createRequest(const QString& apiAddress);

signals:
    void addressChanged        ();
    void portChanged           ();
    void loginChanged          ();
    void passwordChanged       ();
    void isBusyChanged         ();
    void minAngleChanged       ();
    void maxAngleChanged       ();
    void currentAngleChanged   ();
    void currentAntennaChanged ();

private:
    static inline const QString _defaultAddress  = "192.168.2.2";
    static inline const quint16 _defaultPort     = 8000;
    static inline const QString _defaultLogin    = "Operator";
    static inline const QString _defaultPassword = "167124";
    static inline const qint32  _defaultMinAngle = -165;
    static inline const qint32  _defaultMaxAngle = 165;
    static inline const qint32  _minAngleLimit   = -180;
    static inline const qint32  _maxAngleLimit   = 180;
    static inline const qint32  _antennasCount   = 2;
    static inline const quint64 _requestTimeout  = 3000;

    QString _address        = _defaultAddress;
    quint16 _port           = _defaultPort;
    QString _login          = _defaultLogin;
    QString _password       = _defaultPassword;
    bool    _isBusy         = false;
    qint32  _minAngle       = _defaultMinAngle;
    qint32  _maxAngle       = _defaultMaxAngle;
    qint32  _currentAntenna = -1;
    qint32  _currentAngle   = 0;

    static const char* _settingsGroup;
    static const char* _addressKey;
    static const char* _portKey;
    static const char* _loginKey;
    static const char* _passwordKey;
    static const char* _minAngleKey;
    static const char* _maxAngleKey;
};
