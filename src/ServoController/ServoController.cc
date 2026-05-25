#include "ServoController.h"

#include <QGCApplication.h>
#include <ParameterManager.h>

QGC_LOGGING_CATEGORY(ServoControllerLog, "ServoControllerLog")

struct DoSetServoListHandlerData {
    ServoController* controller;
    QVector<int> servoIndexes;
    bool desiredState;

    DoSetServoListHandlerData() = delete;
    DoSetServoListHandlerData(ServoController* controller, const QVector<int>& servoIndexes, bool desiredState) :
        controller(controller), servoIndexes(servoIndexes), desiredState(desiredState) {}
};

Servo::Servo(const QString& name, quint16 index, quint16 function, quint16 startValue, quint16 minValue, quint16 maxValue, bool reversed)
    : _name(name)
    , _index(index)
    , _function(function)
    , _value(startValue)
    , _minValue(minValue)
    , _maxValue(maxValue)
    , _reversed(reversed)
    , _normalizedValue(fminf(1.0f, fmaxf(0.0f, static_cast<float>(startValue - minValue) / static_cast<float>(maxValue - minValue))))
{}

const QString& Servo::name() const {
    return _name;
}

quint16 Servo::index() const {
    return _index;
}

quint16 Servo::function() const {
    return _function;
}

quint16 Servo::value() const {
    return _value;
}

quint16 Servo::minValue() const {
    return _minValue;
}

quint16 Servo::maxValue() const {
    return _maxValue;
}

bool Servo::reversed() const {
    return _reversed;
}

float Servo::normalizedValue () const {
    return _normalizedValue;
}

void Servo::setFunction(quint16 function) {
    _function = function;
    emit functionChanged();
}

void Servo::setValue(quint16 value) {
    _value = value;
    _normalizedValue = fminf(1.0f, fmaxf(0.0f, static_cast<float>(_value - _minValue) / static_cast<float>(_maxValue - _minValue)));
    emit valueChanged();
    emit normalizedValueChanged();
}

void Servo::setMinValue(quint16 minValue) {
    _minValue = minValue;
    _normalizedValue = fminf(1.0f, fmaxf(0.0f, static_cast<float>(_value - _minValue) / static_cast<float>(_maxValue - _minValue)));
    emit minValueChanged();
    emit normalizedValueChanged();
}

void Servo::setMaxValue(quint16 maxValue) {
    _maxValue = maxValue;
    _normalizedValue = fminf(1.0f, fmaxf(0.0f, static_cast<float>(_value - _minValue) / static_cast<float>(_maxValue - _minValue)));
    emit maxValueChanged();
    emit normalizedValueChanged();
}

void Servo::setReversed(bool reversed) {
    _reversed = reversed;
    emit reversedChanged();
}

void Servo::onFunctionParameterChanged(QVariant value) {
    _function = value.toUInt();
    emit functionChanged();
}

void Servo::onMinParameterChanged(QVariant value) {
    _minValue = value.toUInt();
    emit minValueChanged();
}

void Servo::onMaxParameterChanged(QVariant value) {
    _maxValue = value.toUInt();
    emit maxValueChanged();
}

void Servo::onReversedParameterChanged(QVariant value) {
    _reversed = value.toBool();
    emit reversedChanged();
}

ServoController::ServoController(MAVLinkProtocol* mavlink, Vehicle* vehicle)
    : _mavlink(mavlink)
    , _vehicle(vehicle)
    , _initialized(false) {
    QQmlEngine::setObjectOwnership(this, QQmlEngine::CppOwnership);
    connect(_vehicle, &Vehicle::mavlinkMessageReceived, this, &ServoController::_mavlinkMessageReceived);
    connect(_vehicle->parameterManager(), &ParameterManager::parametersReadyChanged, this, &ServoController::_onParametersReadyChanged);
}

ServoController::~ServoController() {
    for (int i = 0; i < _servoModel.size(); i++) {
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
    switch(message.msgid) {
    case MAVLINK_MSG_ID_SERVO_OUTPUT_RAW:
        _handleServoOutputRaw(message);
        break;
    }
}

void ServoController::_onParametersReadyChanged(bool parametersReady) {
    if (_initialized || !parametersReady) {
        return;
    }

    for (int servoIndex = 1; servoIndex <= _servoOutputsRaw.size(); servoIndex++) {
        QString functionParameterName = QString("SERVO%1_FUNCTION").arg(servoIndex);
        QString minValueParameterName = QString("SERVO%1_MIN").arg(servoIndex);
        QString maxValueParameterName = QString("SERVO%1_MAX").arg(servoIndex);
        QString reversedParameterName = QString("SERVO%1_REVERSED").arg(servoIndex);

        if (!_vehicle->parameterManager()->parameterExists(_vehicle->defaultComponentId(), functionParameterName)) {
            qCDebug(ServoControllerLog) << "ServoController failed to initialize because of missing SERVOx_FUNCTION for servo:" << servoIndex;
            return;
        }

        if (!_vehicle->parameterManager()->parameterExists(_vehicle->defaultComponentId(), minValueParameterName)) {
            qCDebug(ServoControllerLog) << "ServoController failed to initialize because of missing SERVOx_MIN for servo:" << servoIndex;
            return;
        }

        if (!_vehicle->parameterManager()->parameterExists(_vehicle->defaultComponentId(), maxValueParameterName)) {
            qCDebug(ServoControllerLog) << "ServoController failed to initialize because of missing SERVOx_MAX for servo:" << servoIndex;
            return;
        }

        if (!_vehicle->parameterManager()->parameterExists(_vehicle->defaultComponentId(), reversedParameterName)) {
            qCDebug(ServoControllerLog) << "ServoController failed to initialize because of missing SERVOx_REVERSED for servo:" << servoIndex;
            return;
        }
    }

    for (int servoIndex = 1; servoIndex <= _servoOutputsRaw.size(); servoIndex++) {
        QString functionParameterName = QString("SERVO%1_FUNCTION").arg(servoIndex);
        QString minValueParameterName = QString("SERVO%1_MIN").arg(servoIndex);
        QString maxValueParameterName = QString("SERVO%1_MAX").arg(servoIndex);
        QString reversedParameterName = QString("SERVO%1_REVERSED").arg(servoIndex);

        Fact* functionParameter = _vehicle->parameterManager()->getParameter(_vehicle->defaultComponentId(), functionParameterName);
        Fact* minValueParameter = _vehicle->parameterManager()->getParameter(_vehicle->defaultComponentId(), minValueParameterName);
        Fact* maxValueParameter = _vehicle->parameterManager()->getParameter(_vehicle->defaultComponentId(), maxValueParameterName);
        Fact* reversedParameter = _vehicle->parameterManager()->getParameter(_vehicle->defaultComponentId(), reversedParameterName);

        quint16 rawValue = 0;
        uint16_t function = functionParameter->rawValue().toUInt();
        quint16 minValue = minValueParameter->rawValue().toUInt();
        quint16 maxValue = maxValueParameter->rawValue().toUInt();
        bool reversed = reversedParameter->rawValue().toBool();

        Servo* servo = new Servo(QString("S%1").arg(servoIndex), servoIndex - 1, function, rawValue, minValue, maxValue, reversed);
        QQmlEngine::setObjectOwnership(servo, QQmlEngine::CppOwnership);

        connect(functionParameter, &Fact::vehicleUpdated, servo, &Servo::onFunctionParameterChanged);
        connect(minValueParameter, &Fact::vehicleUpdated, servo, &Servo::onMinParameterChanged);
        connect(maxValueParameter, &Fact::vehicleUpdated, servo, &Servo::onMaxParameterChanged);
        connect(reversedParameter, &Fact::vehicleUpdated, servo, &Servo::onReversedParameterChanged);

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

    mavlink_servo_output_raw_t  _sor;
    mavlink_msg_servo_output_raw_decode(&msg, &_sor);

    const uint8_t* base = reinterpret_cast<const uint8_t*>(&_sor);
    size_t offset1 = offsetof(mavlink_servo_output_raw_t, servo1_raw);
    memcpy(_servoOutputsRaw.data(), base + offset1, 8 * sizeof(uint16_t));
    size_t offset2 = offsetof(mavlink_servo_output_raw_t, servo9_raw);
    memcpy(_servoOutputsRaw.data() + 8, base + offset2, 8 * sizeof(uint16_t));

    for(int servoIndex = 0; servoIndex < _servoOutputsRaw.size(); servoIndex++) {
        Servo* serv = _findServo(servoIndex);
        if (serv) {
            serv->setValue(_servoOutputsRaw[servoIndex]);
        }
    }
}

bool ServoController::_hasServo(uint16_t index) {
    for (uint16_t i = 0; i < _servoModel.size(); ++i) {
        if (_servoModel[i].value<Servo*>()->index() == index) {
            return true;
        }
    }
    return false;
}

Servo* ServoController::_findServo(uint16_t index) {
    for (uint16_t i = 0; i < _servoModel.size(); ++i) {
        if (_servoModel[i].value<Servo*>()->index() == index) {
            return _servoModel[i].value<Servo*>();
        }
    }
    return nullptr;
}
