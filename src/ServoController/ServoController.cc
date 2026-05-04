#include "ServoController.h"

#include <QGCApplication.h>
#include <ParameterManager.h>

QGC_LOGGING_CATEGORY(ServoControllerLog, "ServoControllerLog")

const QString& Servo::name() const {
    return _name;
}

quint16 Servo::index() const {
    return _index;
}

quint16 Servo::displayIndex() const {
    return _displayIndex;
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

void Servo::setName(const QString& name) {
    _name = name;
    emit nameChanged();
}

void Servo::setIndex(quint16 index) {
    _index = index;
    emit indexChanged();
}

void Servo::setDisplayIndex(quint16 displayIndex) {
    _displayIndex = displayIndex;
    emit displayIndexChanged();
}

void Servo::setValue(quint16 value) {
    _value = value;
    emit valueChanged();
}

void Servo::setMinValue(quint16 minValue) {
    _minValue = minValue;
    emit minValueChanged();
}

void Servo::setMaxValue(quint16 maxValue) {
    _maxValue = maxValue;
    emit maxValueChanged();
}

void Servo::setReversed(bool reversed) {
    _reversed = reversed;
    emit reversedChanged();
}

Servo::Servo(const QString& name, quint16 index, quint16 displayIndex, quint16 startValue, quint16 minValue, quint16 maxValue, bool reversed)
    : _name(name)
    , _index(index)
    , _displayIndex(displayIndex)
    , _value(startValue)
    , _minValue(minValue)
    , _maxValue(maxValue)
    , _reversed(reversed)
{}

ServoController::ServoController(MAVLinkProtocol* mavlink, Vehicle* vehicle)
    : _mavlink(mavlink)
    , _vehicle(vehicle)
{
    QQmlEngine::setObjectOwnership(this, QQmlEngine::CppOwnership);
    connect(_vehicle, &Vehicle::mavlinkMessageReceived, this, &ServoController::_mavlinkMessageReceived);
    connect(_vehicle->parameterManager(), &ParameterManager::parametersReadyChanged, this, &ServoController::_onParametersReadyChanged);
    emit desiredOpenStatesChanged();
    emit dropControlEnabledChanged();
}

ServoController::~ServoController() {

}

void ServoController::close() {
    for (int32_t servoIndex = 0; servoIndex < _desiredOpenStates.size(); servoIndex++) {
        _desiredOpenStates[servoIndex] = false;
    }
}

void ServoController::setDesiredOpenStates(QVector<bool> states) {
    if (states.size() != _desiredOpenStates.size()) {
        return;
    }
    _desiredOpenStates = states;
}

QVariantList ServoController::servoModel() const {
    return _servoModel;
}

QVector<bool> ServoController::desiredOpenStates() const {
    return _desiredOpenStates;
}

int ServoController::servoCount() const {
    return _servoCount;
}

QVector<int> ServoController::rcAssignments() const {
    return _rcAssignments;
}

bool ServoController::dropControlEnabled() const {
    return _dropControlEnabled;
}

int ServoController::getDesiredOutput(int outputIndex) {
    if (outputIndex < 0 || outputIndex >= _servoCount) {
        return 1000;
    }
    int rcIndex = outputIndex + 13;
    for (int i = 0; i < _servoCount; i++) {
         if (_rcAssignments[i] != rcIndex) continue;
        Servo* servo = findServo(_servoIndexes[i] - 1);
        if (!servo) continue;
        // Reversed isn't needed for RCIN_SCALED
        // if (servo->reversed()) {
        //     return _desiredOpenStates[i] ? servo->minValue() : servo->maxValue();
        // }
        // else {
        //     return _desiredOpenStates[i] ? servo->maxValue() : servo->minValue();
        // }
        return _desiredOpenStates[i] ? servo->maxValue() : servo->minValue();
    }
    return 1000;
}

void ServoController::toggleDesiredDropState(int index) {
    if (index < 0 || index >= _servoCount) {
        return;
    }
    bool newState = !_desiredOpenStates[index];
    int assignment = _rcAssignments[index];
    for(int i = 0; i < _servoCount; i++) {
        if (_rcAssignments[i] != assignment) {
            continue;
        }
        _desiredOpenStates[i] = newState;
    }
    emit desiredOpenStatesChanged();
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

    for (int32_t i = 0; i < _servoIndexes.size(); i++) {
        QString parameterName = QString("SERVO%1_FUNCTION").arg(_servoIndexes[i]);
        if (!_vehicle->parameterManager()->parameterExists(_vehicle->defaultComponentId(), parameterName)) {
            qCDebug(ServoControllerLog) << "Servo Controller failed to initialize because of missing parameters for servo:" << _servoIndexes[i];
            return;
        }
    }

    for (int32_t i = 0; i < _servoIndexes.size(); i++) {
        QString parameterName = QString("SERVO%1_FUNCTION").arg(_servoIndexes[i]);
        Fact* parameter = _vehicle->parameterManager()->getParameter(_vehicle->defaultComponentId(), parameterName);
        connect(parameter, &Fact::vehicleUpdated, this, &ServoController::_onVehicleParameterUpdated);
    }

    _updateRCAssignments();
    _dropControlEnabled = true;
    emit dropControlEnabledChanged();

    _initialized = true;
}

void ServoController::_onVehicleParameterUpdated(QVariant value) {
    _updateRCAssignments();
}

void ServoController::_handleServoOutputRaw(const mavlink_message_t& msg) {
    if (!_vehicle->parameterManager()->parametersReady()) {
        return;
    }

    mavlink_msg_servo_output_raw_decode(&msg, &_sor);

    const uint8_t* base = reinterpret_cast<const uint8_t*>(&_sor);
    size_t offset1 = offsetof(mavlink_servo_output_raw_t, servo1_raw);
    memcpy(_servoOutputsRaw.data(), base + offset1, 8 * sizeof(uint16_t));
    size_t offset2 = offsetof(mavlink_servo_output_raw_t, servo9_raw);
    memcpy(_servoOutputsRaw.data() + 8, base + offset2, 8 * sizeof(uint16_t));

    for(int i = 0; i < _servoIndexes.size(); i++) {
        _updateServo(_servoIndexes[i]);
    }
}

void ServoController::_updateServo(int index) {
    QString parameterName = QString("SERVO%1_FUNCTION").arg(index);
    QString minValueParameterName = QString("SERVO%1_MIN").arg(index);
    QString maxValueParameterName = QString("SERVO%1_MAX").arg(index);
    QString parameterReversed = QString("SERVO%1_REVERSED").arg(index);

    Fact* parameter = _vehicle->parameterManager()->getParameter(_vehicle->defaultComponentId(), parameterName);
    Fact* valueReversedParameter = _vehicle->parameterManager()->getParameter(_vehicle->defaultComponentId(), parameterReversed);
    Fact* minValueParameter = _vehicle->parameterManager()->getParameter(_vehicle->defaultComponentId(), minValueParameterName);
    Fact* maxValueParameter = _vehicle->parameterManager()->getParameter(_vehicle->defaultComponentId(), maxValueParameterName);

    quint16 maxValue = std::numeric_limits<uint16_t>::max();
    quint16 minValue = 0;
    bool reversed = false;
    uint16_t function = 0;

    if(valueReversedParameter) {
        reversed = valueReversedParameter->rawValue().toBool();
    }
    if(minValueParameter) {
        minValue = minValueParameter->rawValue().toUInt();
    }
    if(maxValueParameter) {
        maxValue = maxValueParameter->rawValue().toUInt();
    }
    if(parameter) {
        function = parameter->rawValue().toUInt();
    }

    //qCDebug(ServoControllerLog) << "Servo min: " << minValue << ". servo value: " << _servoOutputsRaw[i] << ". servo max: " << maxValue << ". index: " << i;
    Servo* serv = findServo(index - 1);
    if(serv) {
        serv->setMinValue(minValue);
        serv->setMaxValue(maxValue);
        serv->setReversed(reversed);
        serv->setValue(_servoOutputsRaw[index - 1]);
    } else {
        serv = new Servo(QString("S%1").arg(index), index - 1, index, _servoOutputsRaw[index - 1], minValue, maxValue, reversed);
        _servoModel.append(QVariant::fromValue(serv));
        emit servoModelChanged();
    }
}

void ServoController::_updateRCAssignments() {
    for (int i = 0; i < _servoCount; i++) {
        QString parameterName = QString("SERVO%1_FUNCTION").arg(_servoIndexes[i]);
        Fact* parameter = _vehicle->parameterManager()->getParameter(_vehicle->defaultComponentId(), parameterName);
        if (!parameter) {
            _rcAssignments[i] = 0;
            continue;
        }
        int function = parameter->rawValue().toUInt();
        switch(static_cast<AllowedServoFunctions>(function)) {
        case AllowedServoFunctions::RCIN13Scaled:
            _rcAssignments[i] = 13;
            break;
        case AllowedServoFunctions::RCIN14Scaled:
            _rcAssignments[i] = 14;
            break;
        case AllowedServoFunctions::RCIN15Scaled:
            _rcAssignments[i] = 15;
            break;
        case AllowedServoFunctions::RCIN16Scaled:
            _rcAssignments[i] = 16;
            break;
        default:
            _rcAssignments[i] = 0;
            break;
        }
    }
    emit rcAssignmentsChanged();
}

bool ServoController::hasServo(uint16_t index) {
    for (uint16_t i = 0; i < _servoModel.size(); ++i) {
        if (_servoModel[i].value<Servo*>()->index() == index) {
            return true;
        }
    }
    return false;
}

Servo* ServoController::findServo(uint16_t index) {
    for (uint16_t i = 0; i < _servoModel.size(); ++i) {
        if (_servoModel[i].value<Servo*>()->index() == index) {
            return _servoModel[i].value<Servo*>();
        }
    }
    return nullptr;
}
