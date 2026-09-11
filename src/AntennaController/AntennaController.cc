#include "AntennaController.h"

#include <QGCApplication.h>

QGC_LOGGING_CATEGORY(AntennaControllerLog, "AntennaControllerLog")

const char* AntennaController::_settingsGroup = "AntennaController";
const char* AntennaController::_addressKey    = "address";
const char* AntennaController::_portKey       = "port";
const char* AntennaController::_loginKey      = "login";
const char* AntennaController::_passwordKey   = "password";
const char* AntennaController::_minAngleKey   = "minAngle";
const char* AntennaController::_maxAngleKey   = "maxAngle";

AntennaController::AntennaController(QGCApplication *app, QGCToolbox *toolbox)
    : QGCTool(app, toolbox) {
    _loadSettings();

    _getDeviceSettings([this](const QByteArray& response) {
        _onSettingRequestGetAntennaID(response);
    });
}

AntennaController::~AntennaController() {}

void AntennaController::setAddress(QString address) {
    if (!_isValidIP(address)) {
        qCDebug(AntennaControllerLog) << "Address is not a valid IP:" << address;
        return;
    }
    if (_address == address) {
        emit addressChanged();
        return;
    }
    qCDebug(AntennaControllerLog) << "Address changed:" << _address << "=>" << address;
    _address = address;
    _saveSettings();
    emit addressChanged();
}

void AntennaController::setPort(quint16 port) {
    if (_port == port) {
        emit portChanged();
        return;
    }
    qCDebug(AntennaControllerLog) << "Port changed:" << _port << "=>" << port;
    _port = port;
    _saveSettings();
    emit portChanged();
}

void AntennaController::setLogin(QString login) {
    if (_login == login) {
        emit loginChanged();
        return;
    }
    qCDebug(AntennaControllerLog) << "Login changed:" << _login << "=>" << login;
    _login = login;
    _saveSettings();
    emit loginChanged();
}

void AntennaController::setPassword(QString password) {
    if (_password == password) {
        emit passwordChanged();
        return;
    }
    qCDebug(AntennaControllerLog) << "Password changed.";
    _password = password;
    _saveSettings();
    emit passwordChanged();
}

void AntennaController::setMinAngle(qint32 angle) {
    if (angle > _maxAngle) {
        angle = _maxAngle;
        qCDebug(AntennaControllerLog) << "New min angle is clamped to max angle:" << _maxAngle;
    }
    if (angle < _minAngleLimit || angle > _maxAngleLimit) {
        qCDebug(AntennaControllerLog) << "New min angle is out of range:" << angle;
        angle = std::clamp(angle, _minAngleLimit, _maxAngleLimit);
        qCDebug(AntennaControllerLog) << "New min angle is clamped to value:" << angle;
    }
    if (_minAngle == angle) {
        emit minAngleChanged();
        return;
    }
    qCDebug(AntennaControllerLog) << "Min angle changed:" << _minAngle << "=>" << angle;
    _minAngle = angle;
    _saveSettings();
    emit minAngleChanged();
}

void AntennaController::setMaxAngle(qint32 angle) {
    if (angle < _minAngle) {
        angle = _minAngle;
        qCDebug(AntennaControllerLog) << "New max angle is clamped to min angle:" << _minAngle;
    }
    if (angle < _minAngleLimit || angle > _maxAngleLimit) {
        qCDebug(AntennaControllerLog) << "New max angle is out of range:" << angle;
        angle = std::clamp(angle, _minAngleLimit, _maxAngleLimit);
        qCDebug(AntennaControllerLog) << "New max angle clamped to value:" << angle;
    }
    if (_maxAngle == angle) {
        emit maxAngleChanged();
        return;
    }
    qCDebug(AntennaControllerLog) << "Max angle changed:" << _maxAngle << "=>" << angle;
    _maxAngle = angle;
    _saveSettings();
    emit maxAngleChanged();
}

void AntennaController::setCurrentAngle(qint32 angle) {
    if (_isBusy) {
        qCDebug(AntennaControllerLog) << "Can not set current angle. Antenna controller is busy.";
        return;
    }

    if (angle < _minAngle || angle > _maxAngle) {
        qCDebug(AntennaControllerLog) << "New angle is out of range:" << angle;
        angle = std::clamp(angle, _minAngle, _maxAngle);
        qCDebug(AntennaControllerLog) << "New angle clamped to value:" << angle;
    }

    QByteArray buffer(6, 0);
    buffer[0] = char(0xFA);
    buffer[1] = char(0x11);
    buffer[2] = static_cast<char>( angle        & 0xFF);
    buffer[3] = static_cast<char>((angle >> 8)  & 0xFF);
    buffer[4] = static_cast<char>((angle >> 16) & 0xFF);
    buffer[5] = static_cast<char>((angle >> 24) & 0xFF);

    qCDebug(AntennaControllerLog) << "Sending set angle request. angle:" << angle;

    _sendCommand(buffer, [this, angle](const QByteArray& response){
        if (_currentAngle != angle) {
            qCDebug(AntennaControllerLog) << "Current angle changed:" << _currentAngle << "=>" << angle;
            _currentAngle = angle;
        }
        emit currentAngleChanged();
    });
}

void AntennaController::setCurrentAntenna(qint32 index) {
    if (_isBusy) {
        qCDebug(AntennaControllerLog) << "Can not change curent antenna. Waiting for response.";
        return;
    }

    if (index >= _antennasCount) {
        qCDebug(AntennaControllerLog) << "Antenna index is out of range:" << index << "/" << _antennasCount;
        return;
    }

    const qint32 id = index + 1;

    QByteArray buffer(3, 0);
    buffer[0] = char(0xFA);
    buffer[1] = char(0x12);
    buffer[2] = static_cast<char>(id);

    qCDebug(AntennaControllerLog) << "Sending set antenna request. antenna id:" << id;
    _sendCommand(buffer, [this, index](const QByteArray&) {
        if (_currentAntenna != index) {
            qCDebug(AntennaControllerLog) << "Current antenna changed:" << _currentAntenna << "=>" << index;
            _currentAntenna = index;
        }
        emit currentAntennaChanged();
    });
}

void AntennaController::setToolbox(QGCToolbox *toolbox) {
    QGCTool::setToolbox(toolbox);
    QQmlEngine::setObjectOwnership(this, QQmlEngine::CppOwnership);
    qmlRegisterUncreatableType<AntennaController>("QGroundControl.antennaController", 1, 0, "AntennaController", "Reference only");
}

void AntennaController::resetAccessSettings() {
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

    qCDebug(AntennaControllerLog) << "access settings are set to default";

    _saveSettings();
}

void AntennaController::resetConfiguration() {
    if (_minAngle != _defaultMinAngle) {
        _minAngle = _defaultMinAngle;
        emit minAngleChanged();
    }

    if (_maxAngle != _defaultMaxAngle) {
        _maxAngle = _defaultMaxAngle;
        emit maxAngleChanged();
    }

    qCDebug(AntennaControllerLog) << "configuration is set to default";

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

bool AntennaController::isBusy() const {
    return _isBusy;
}

qint32 AntennaController::minAngle() const {
    return _minAngle;
}

qint32 AntennaController::maxAngle() const {
    return _maxAngle;
}

qint32 AntennaController::currentAngle() const {
    return _currentAngle;
}

qint32 AntennaController::currentAntenna() const {
    return _currentAntenna;
}

void AntennaController::_saveSettings() {
    QSettings settings;
    settings.beginGroup(_settingsGroup);

    settings.setValue(QString(_addressKey), _address);
    settings.setValue(QString(_portKey), _port);
    settings.setValue(QString(_loginKey), _login);
    settings.setValue(QString(_passwordKey), _password);
    settings.setValue(QString(_minAngleKey), _minAngle);
    settings.setValue(QString(_maxAngleKey), _maxAngle);

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

    qint32 newMinAngle = settings.value(_minAngleKey, _defaultMinAngle).toInt(&ok);
    if (ok) {
        _minAngle = newMinAngle;
        emit minAngleChanged();
    }

    qint32 newMaxAngle = settings.value(_maxAngleKey, _defaultMaxAngle).toInt(&ok);
    if (ok) {
        _maxAngle = newMaxAngle;
        emit maxAngleChanged();
    }

    qCDebug(AntennaControllerLog) << "settings loaded";
}

void AntennaController::_sendCommand(const QByteArray& bytes, std::function<void(const QByteArray& response)> callback) {
    if (_isBusy) {
        qCDebug(AntennaControllerLog) << "Sending command request failed. Waiting for response.";
        return;
    }

    _isBusy = true;
    emit isBusyChanged();

    QNetworkRequest request = _createRequest("api/plugins/command");
    QNetworkReply *reply = m_networkManager.post(request, bytes);
    _connectReply(reply, callback);
}

void AntennaController::_getDeviceSettings(std::function<void(const QByteArray& response)> callback) {
    if (_isBusy) {
        qCDebug(AntennaControllerLog) << "Sending get device settings request failed. Waiting for response.";
        return;
    }

    _isBusy = true;
    emit isBusyChanged();

    QNetworkRequest request = _createRequest("api/getDeviceSetting");
    QNetworkReply* reply = m_networkManager.get(request);
    _connectReply(reply, callback);
}

QNetworkRequest AntennaController::_createRequest(const QString& apiAddress) {
    const QUrl url(QString("http://%1:%2/" + apiAddress).arg(_address).arg(_port));

    qCDebug(AntennaControllerLog) << "Creating request. url:" << url;

    const QString concatenated = _login + ":" + _password;

    QNetworkRequest request(url);
    request.setTransferTimeout(_requestTimeout);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/octet-stream");
    request.setRawHeader("Authorization", "Basic " + concatenated.toUtf8().toBase64());

    return request;
}

void AntennaController::_connectReply(QNetworkReply* reply, std::function<void(const QByteArray& response)> callback) {
    QObject::connect(reply, &QNetworkReply::finished, [this, reply, callback]() {
        _isBusy = false;

        if (reply->error() == QNetworkReply::NoError) {
            QByteArray response = reply->readAll();
            qCDebug(AntennaControllerLog) << "Request success:" << response;
            if (callback) {
                callback(response);
            }
        } else {
            qCDebug(AntennaControllerLog) << "Request error:" << reply->errorString();
        }

        reply->deleteLater();

        if (_isBusy == false) {
            emit isBusyChanged();
        }
    });

    QObject::connect(reply, &QNetworkReply::errorOccurred, [](QNetworkReply::NetworkError code) {
        if (code == QNetworkReply::TimeoutError) {
            qCDebug(AntennaControllerLog) << "Request timeout";
        } else {
            qCDebug(AntennaControllerLog) << "Request network error:" << code;
        }
    });
}

bool AntennaController::_isValidIP(const QString &ipString) {
    QHostAddress address;
    return address.setAddress(ipString);
}

void AntennaController::_onSettingRequestGetAntennaID(const QByteArray& bytes) {
    QString stringResponse(bytes);
    QStringList lines = stringResponse.split('\n');
    qCDebug(AntennaControllerLog) << "Succesfully received device setting. Lines:" << lines.count();
    if (lines.count() < 10) {
        qCDebug(AntennaControllerLog) << "Error: get device setings response is expected to have at least 10 lines.";
        return;
    }

    bool ok = false;
    quint32 antennaFlag = lines[9].toInt(&ok);
    if (ok) {
        if (antennaFlag == 0) {
            _currentAntenna = 0;
            emit currentAntennaChanged();
        } else if (antennaFlag == 1) {
            _currentAntenna = 1;
            emit currentAntennaChanged();
        } else {
            qCDebug(AntennaControllerLog) << "Error: unexpected antenna flag state:" << antennaFlag;
        }
    }
    else {
        qCDebug(AntennaControllerLog) << "Error: failed to check antenna flag.";
    }
}
