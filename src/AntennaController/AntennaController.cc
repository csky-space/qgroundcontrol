#include "AntennaController.h"

#include <ParameterManager.h>
#include <QGCApplication.h>
#include <cmath>

QGC_LOGGING_CATEGORY(AntennaControllerLog, "AntennaControllerLog")

const char* AntennaController::_settingsGroup = "AntennaController";
const char* AntennaController::_addressKey    = "address";
const char* AntennaController::_portKey       = "port";
const char* AntennaController::_loginKey      = "login";
const char* AntennaController::_passwordKey   = "password";

AntennaController::AntennaController(QGCApplication *app, QGCToolbox *toolbox)
    : QGCTool(app, toolbox) {
    _loadSettings();
}

AntennaController::~AntennaController() {}

void AntennaController::setAddress(QString address) {
    _address = address;
    _saveSettings();
    emit addressChanged();
}

void AntennaController::setPort(quint16 port) {
    _port = port;
    _saveSettings();
    emit portChanged();
}

void AntennaController::setLogin(QString login) {
    _login = login;
    _updateToken();
    _saveSettings();
    emit loginChanged();
}

void AntennaController::setPassword(QString password) {
    _password = password;
    _updateToken();
    _saveSettings();
    emit passwordChanged();
}

void AntennaController::setToolbox(QGCToolbox *toolbox) {
    QGCTool::setToolbox(toolbox);
    QQmlEngine::setObjectOwnership(this, QQmlEngine::CppOwnership);
    qmlRegisterUncreatableType<AntennaController>("QGroundControl.antennaController", 1, 0, "AntennaController", "Reference only");
}

void AntennaController::sendSetAngleCommand(qint32 angle) {
    QByteArray buffer(6, 0);
    buffer[0] = char(0xFA);
    buffer[1] = char(0x11);
    buffer[2] = static_cast<char>( angle        & 0xFF);
    buffer[3] = static_cast<char>((angle >> 8)  & 0xFF);
    buffer[4] = static_cast<char>((angle >> 16) & 0xFF);
    buffer[5] = static_cast<char>((angle >> 24) & 0xFF);

    qCDebug(AntennaControllerLog) << "Sending set angle request. angle:" << angle;
    _sendCommand(buffer);
}

void AntennaController::sendSetAntennaCommand(qint8 antennaIndex) {
    QByteArray buffer(3, 0);
    buffer[0] = char(0xFA);
    buffer[1] = char(0x12);
    buffer[2] = static_cast<char>(antennaIndex);

    qCDebug(AntennaControllerLog) << "Sending set antenna request. antenna:" << antennaIndex;
    _sendCommand(buffer);
}

void AntennaController::resetSettings() {
    if (_address != _defaultAddress) {
        _address = _defaultAddress;
        emit addressChanged();
    }

    if (_port != _defaultPort) {
        _port = _defaultPort;
        emit portChanged();
    }

    if (_login != _defaultLogin) {
        _login = _defaultLogin;
        emit loginChanged();
    }

    if (_password != _defaultPassword) {
        _password = _defaultPassword;
        emit passwordChanged();
    }

    qCDebug(AntennaControllerLog) << "settings are set to default";

    _saveSettings();
}

// void MultiVehicleManager::onAuthRequired(QNetworkReply* reply, QAuthenticator* authenticator) {
//     qCDebug(AntennaControllerLog) << "auth event fallback";
//     qCDebug(AntennaControllerLog) << "realm" << authenticator->realm();

//     if (!authenticator->user().isEmpty()) {
//         qCDebug(AntennaControllerLog) << "wrong user or password" << authenticator->user() << " " << authenticator->password();
//         return;
//     }

//     authenticator->setUser("Operator");
//     authenticator->setPassword("167124");
// }

QString AntennaController::address() const {
    return _address;
}

quint16 AntennaController::port() const {
    return _port;
}

QString AntennaController::login() const {
    return _login;
}

QString AntennaController::password() const {
    return _password;
}

void AntennaController::_saveSettings() {
    QSettings settings;
    settings.beginGroup(_settingsGroup);

    settings.setValue(QString(_addressKey), _address);
    settings.setValue(QString(_portKey), _port);
    settings.setValue(QString(_loginKey), _login);
    settings.setValue(QString(_passwordKey), _password);

    qCDebug(AntennaControllerLog) << "settings saved";
}

void AntennaController::_loadSettings() {
    QSettings settings;
    settings.beginGroup(_settingsGroup);

    QString newAddress = settings.value(_addressKey, _defaultAddress).toString();
    _address = newAddress;
    emit addressChanged();

    bool ok = false;
    quint16 newPort = static_cast<quint16>(settings.value(_portKey, _defaultPort).toUInt(&ok));
    if (ok) {
        _port = newPort;
        emit portChanged();
    }

    QString newLogin = settings.value(_loginKey, _defaultLogin).toString();
    _login = newLogin;
    emit loginChanged();

    QString newPassword = settings.value(_passwordKey, _defaultPassword).toString();
    _password = newPassword;
    emit passwordChanged();

    _updateToken();

    qCDebug(AntennaControllerLog) << "settings loaded";
}

void AntennaController::_sendCommand(const QByteArray& bytes) {
    const QUrl url(QString("http://%1:%2/api/plugins/command").arg(_address).arg(_port));

    qCDebug(AntennaControllerLog) << "Sending command. url:" << url;

    QNetworkRequest request(url);
    request.setTransferTimeout(5000);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/octet-stream");
    request.setRawHeader("Authorization", _token.toUtf8());

    QNetworkReply *reply = m_networkManager.post(request, bytes);

    QObject::connect(reply, &QNetworkReply::finished, [reply]() {
        if (reply->error() == QNetworkReply::NoError) {
            QByteArray response = reply->readAll();
            qCDebug(AntennaControllerLog) << "Command request success:" << response;
        } else {
            qCDebug(AntennaControllerLog) << "Command request error:" << reply->errorString();
        }
        reply->deleteLater();
    });

    QObject::connect(reply, &QNetworkReply::errorOccurred, [](QNetworkReply::NetworkError code) {
        if (code == QNetworkReply::TimeoutError) {
            qCDebug(AntennaControllerLog) << "Command request timeout";
        } else {
            qCDebug(AntennaControllerLog) << "Command request network error:" << code;
        }
    });
}

void AntennaController::_getDeviceSettings() {
    const QUrl url(QString("http://%1:%2/api/getDeviceSetting").arg(_address).arg(_port));

    qCDebug(AntennaControllerLog) << "Sending command. url:" << url;

    QNetworkRequest request(url);
    request.setTransferTimeout(5000);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/octet-stream");
    request.setRawHeader("Authorization", _token.toUtf8());

    QNetworkReply *reply = m_networkManager.get(request);

    QObject::connect(reply, &QNetworkReply::finished, [reply]() {
        if (reply->error() == QNetworkReply::NoError) {
            QByteArray response = reply->readAll();
            qCDebug(AntennaControllerLog) << "Get device settings request success:" << response;
        } else {
            qCDebug(AntennaControllerLog) << "Get device settings request error:" << reply->errorString();
        }
        reply->deleteLater();
    });

    QObject::connect(reply, &QNetworkReply::errorOccurred, [](QNetworkReply::NetworkError code) {
        if (code == QNetworkReply::TimeoutError) {
            qCDebug(AntennaControllerLog) << "Get device settings request timeout";
        } else {
            qCDebug(AntennaControllerLog) << "Get device settings request network error:" << code;
        }
    });
}

void AntennaController::_updateToken() {
    const QString concatenated = _login + ":" + _password;
    _token = concatenated.toUtf8().toBase64();
}
