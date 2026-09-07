#include "ServoController.h"

#include <QGCApplication.h>
#include <ParameterManager.h>
#include <cmath>          

QGC_LOGGING_CATEGORY(ServoControllerLog, "ServoControllerLog")

Servo::Servo(const QString& name, quint16 index,
             Fact* functionFact, Fact* minValueFact, Fact* maxValueFact, Fact* reversedFact)
    : _name(name)
    , _index(index)
    , _functionFact(functionFact)
    , _minValueFact(minValueFact)
    , _maxValueFact(maxValueFact)
    , _reversedFact(reversedFact)
    , _value(0)
{
    _function  = _functionFact ? _functionFact->rawValue().toUInt() : 0;
    _minValue  = _minValueFact ? _minValueFact->rawValue().toUInt() : 1000;
    _maxValue  = _maxValueFact ? _maxValueFact->rawValue().toUInt() : 2000;
    _reversed  = _reversedFact ? _reversedFact->rawValue().toBool() : false;
    _normalizedValue = 0.0f;
}

const QString& Servo::name() const {
    return _name;
}

quint16 Servo::index() const {
    return _index;
}

quint16 Servo::value() const {
    return _value;
}

float Servo::normalizedValue() const {
    return _normalizedValue;
}

Fact* Servo::functionFact() const {
    return _functionFact;
}

Fact* Servo::minValueFact() const {
    return _minValueFact;
}

Fact* Servo::maxValueFact() const {
    return _maxValueFact;
}

Fact* Servo::reversedFact() const {
    return _reversedFact;
}

void Servo::setValue(quint16 value) {
    if (_value != value) {
        _value = value;
        float range = static_cast<float>(_maxValue - _minValue);
        if (range > 0.0f) {
            _normalizedValue = fminf(1.0f, fmaxf(0.0f, static_cast<float>(_value - _minValue) / range));
        } else {
            _normalizedValue = 0.0f;
        }
        emit valueChanged();
        emit normalizedValueChanged();
    }
}

void Servo::onFunctionParameterChanged(QVariant value) {
    quint16 newFunc = value.toUInt();
    if (_function != newFunc) {
        _function = newFunc;
        emit functionFactChanged(); 
    }
}

void Servo::onMinParameterChanged(QVariant value) {
    quint16 newMin = value.toUInt();
    if (_minValue != newMin) {
        _minValue = newMin;
        float range = static_cast<float>(_maxValue - _minValue);
        if (range > 0.0f) {
            _normalizedValue = fminf(1.0f, fmaxf(0.0f, static_cast<float>(_value - _minValue) / range));
        } else {
            _normalizedValue = 0.0f;
        }
        emit minValueFactChanged();
        emit normalizedValueChanged(); 
    }
}

void Servo::onMaxParameterChanged(QVariant value) {
    quint16 newMax = value.toUInt();
    if (_maxValue != newMax) {
        _maxValue = newMax;
        float range = static_cast<float>(_maxValue - _minValue);
        if (range > 0.0f) {
            _normalizedValue = fminf(1.0f, fmaxf(0.0f, static_cast<float>(_value - _minValue) / range));
        } else {
            _normalizedValue = 0.0f;
        }
        emit maxValueFactChanged();
        emit normalizedValueChanged();
    }
}

void Servo::onReversedParameterChanged(QVariant value) {
    bool newRev = value.toBool();
    if (_reversed != newRev) {
        _reversed = newRev;
        emit reversedFactChanged();
    }
}

ServoController::ServoController(MAVLinkProtocol* mavlink, Vehicle* vehicle)
    : _mavlink(mavlink)
    , _vehicle(vehicle)
    , _initialized(false)
{
    QQmlEngine::setObjectOwnership(this, QQmlEngine::CppOwnership);
    connect(_vehicle, &Vehicle::mavlinkMessageReceived, this, &ServoController::_mavlinkMessageReceived);
    connect(_vehicle->parameterManager(), &ParameterManager::parametersReadyChanged,
            this, &ServoController::_onParametersReadyChanged);
}

ServoController::~ServoController() {
    for (int i = 0; i < _servoModel.size(); ++i) {
        qvariant_cast<Servo*>(_servoModel[i])->deleteLater();
    }
}

bool ServoController::initialized() const {
    return _initialized;
}

QVariantList ServoController::servoModel() const {
    return _servoModel;
}

void ServoController::_mavlinkMessageReceived(const mavlink_message_t& message) {
    if (message.msgid == MAVLINK_MSG_ID_SERVO_OUTPUT_RAW) {
        _handleServoOutputRaw(message);
    }
}

void ServoController::_onParametersReadyChanged(bool parametersReady) {
    if (_initialized || !parametersReady) {
        return;
    }

    for (size_t servoIndex = 1; servoIndex <= _servoCount; ++servoIndex) {
        QString funcName  = QString("SERVO%1_FUNCTION").arg(servoIndex);
        QString minName   = QString("SERVO%1_MIN").arg(servoIndex);
        QString maxName   = QString("SERVO%1_MAX").arg(servoIndex);
        QString revName   = QString("SERVO%1_REVERSED").arg(servoIndex);

        if (!_vehicle->parameterManager()->parameterExists(_vehicle->defaultComponentId(), funcName) ||
            !_vehicle->parameterManager()->parameterExists(_vehicle->defaultComponentId(), minName)  ||
            !_vehicle->parameterManager()->parameterExists(_vehicle->defaultComponentId(), maxName)  ||
            !_vehicle->parameterManager()->parameterExists(_vehicle->defaultComponentId(), revName)) {
            qCDebug(ServoControllerLog) << "ServoController: missing parameters for servo" << servoIndex;
            return;
        }
    }

    for (size_t servoIndex = 1; servoIndex <= _servoCount; ++servoIndex) {
        QString funcName  = QString("SERVO%1_FUNCTION").arg(servoIndex);
        QString minName   = QString("SERVO%1_MIN").arg(servoIndex);
        QString maxName   = QString("SERVO%1_MAX").arg(servoIndex);
        QString revName   = QString("SERVO%1_REVERSED").arg(servoIndex);

        Fact* funcFact = _vehicle->parameterManager()->getParameter(_vehicle->defaultComponentId(), funcName);
        Fact* minFact  = _vehicle->parameterManager()->getParameter(_vehicle->defaultComponentId(), minName);
        Fact* maxFact  = _vehicle->parameterManager()->getParameter(_vehicle->defaultComponentId(), maxName);
        Fact* revFact  = _vehicle->parameterManager()->getParameter(_vehicle->defaultComponentId(), revName);

        Servo* servo = new Servo(QString("S%1").arg(servoIndex),
                                 static_cast<quint16>(servoIndex - 1),
                                 funcFact, minFact, maxFact, revFact);
        QQmlEngine::setObjectOwnership(servo, QQmlEngine::CppOwnership);

        connect(funcFact, &Fact::vehicleUpdated, servo, &Servo::onFunctionParameterChanged);
        connect(minFact,  &Fact::vehicleUpdated, servo, &Servo::onMinParameterChanged);
        connect(maxFact,  &Fact::vehicleUpdated, servo, &Servo::onMaxParameterChanged);
        connect(revFact,  &Fact::vehicleUpdated, servo, &Servo::onReversedParameterChanged);

        _servoModel.append(QVariant::fromValue(servo));
    }

    _initialized = true;
    emit servoModelChanged();
    emit initializedChanged();
}

void ServoController::_handleServoOutputRaw(const mavlink_message_t& msg) {
    if (!_initialized) {
        return;
    }

    mavlink_servo_output_raw_t sor;
    mavlink_msg_servo_output_raw_decode(&msg, &sor);

    const uint8_t* base = reinterpret_cast<const uint8_t*>(&sor);
    size_t offset1 = offsetof(mavlink_servo_output_raw_t, servo1_raw);
    memcpy(_servoOutputsRaw.data(), base + offset1, 8 * sizeof(uint16_t));
    size_t offset2 = offsetof(mavlink_servo_output_raw_t, servo9_raw);
    memcpy(_servoOutputsRaw.data() + 8, base + offset2, 8 * sizeof(uint16_t));

    for (size_t i = 0; i < _servoOutputsRaw.size(); ++i) {
        Servo* servo = _findServo(static_cast<uint16_t>(i));
        if (servo) {
            servo->setValue(_servoOutputsRaw[i]);
        }
    }
}

bool ServoController::_hasServo(uint16_t index) {
    return _findServo(index) != nullptr;
}

Servo* ServoController::_findServo(uint16_t index) {
    for (const QVariant& v : _servoModel) {
        Servo* s = v.value<Servo*>();
        if (s && s->index() == index) {
            return s;
        }
    }
    return nullptr;
}
