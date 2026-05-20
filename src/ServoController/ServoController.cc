#include "ServoController.h"

#include <QGCApplication.h>
#include <ParameterManager.h>

QGC_LOGGING_CATEGORY(ServoControllerLog, "ServoControllerLog")

const char* ServoController::_settingsGroup      = "Vehicle%1ServoController";
const char* ServoController::_servoAssignmentKey = "servo%1Assignment";

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

    _servoAssignmentsModel.push_back("None");
    for (int i = 0; i < _servoIndexes.size(); i++) {
        _servoAssignmentsModel.push_back("Servo Group " + QString::number(i + 1));
    }
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

QStringList ServoController::servoAssignmentsModel() const {
    return _servoAssignmentsModel;
}

QVector<int> ServoController::servoAssignments() const {
    return _servoAssignments;
}

QVector<bool> ServoController::servoStates() const {
    return _servoStates;
}

QStringList ServoController::dropConfigValidationErrors () const {
    return _dropConfigValidationErrors;
}

void ServoController::_handleDoSetServoListCommandAck(void* resultHandlerData, int compId, const mavlink_command_ack_t& ack, Vehicle::MavCmdResultFailureCode_t failureCode) {
    DoSetServoListHandlerData* handlerData = static_cast<DoSetServoListHandlerData*>(resultHandlerData);
    if (ack.result == MAV_RESULT_ACCEPTED) {
        handlerData->controller->_onSetServoListCommandAccepted(handlerData->servoIndexes, handlerData->desiredState);
    }
    delete handlerData;
}

void ServoController::_onSetServoListCommandAccepted(QVector<int> servoIndexes, bool desiredState) {
    for(int i = 0; i < _servoStates.size(); i++) {
        Servo* servo = qvariant_cast<Servo*>(_servoModel[i]);
        if (servoIndexes.contains(servo->index() + 1)) {
            _servoStates[i] = desiredState;
        }
    }
    emit servoStatesChanged();
}

void ServoController::setDropState(int servoIndex, bool state) {
    if (servoIndex < 0 || servoIndex >= _servoModel.size()) {
        qCDebug(ServoControllerLog) << "ServoController::setDropState: servo index out of range:" << servoIndex;
        return;
    }

    Servo* mainServo = qvariant_cast<Servo*>(_servoModel[servoIndex]);

    float desiredValue = 1000.0f;
    if (mainServo->reversed()) {
        desiredValue = state ? mainServo->minValue() : mainServo->maxValue();
    }
    else {
        desiredValue = state ? mainServo->maxValue() : mainServo->minValue();
    }

    _vehicle->sendMavCommand(_vehicle->defaultComponentId(), MAV_CMD_DO_SET_SERVO, true, mainServo->index() + 1, desiredValue);

    _servoStates[servoIndex] = state;
    emit servoStatesChanged();
}

void ServoController::toggleDesiredDropState(int servoIndex) {
    if (servoIndex < 0 || servoIndex >= _servoModel.size()) {
        qCDebug(ServoControllerLog) << "ServoController::toggleDesiredDropState: servo index out of range:" << servoIndex;
        return;
    }

    int servoGroup = _servoAssignments[servoIndex];
    if (servoGroup == 0) {
        qCDebug(ServoControllerLog) << "ServoController::toggleDesiredDropState: servo group is not set:" << servoIndex;
        return;
    }

    Servo* mainServo = qvariant_cast<Servo*>(_servoModel[servoIndex]);
    bool newState = !_servoStates[servoIndex];

    float desiredValue = 1000.0f;
    if (mainServo->reversed()) {
        desiredValue = newState ? mainServo->minValue() : mainServo->maxValue();
    }
    else {
        desiredValue = newState ? mainServo->maxValue() : mainServo->minValue();
    }

    QVector<int> servosInGroup;
    for (int i = 0; i < _servoAssignments.size(); i++) {
        if (_servoAssignments[i] == servoGroup) {
            Servo* servo = qvariant_cast<Servo*>(_servoModel[i]);
            servosInGroup.push_back(servo->index() + 1);
        }
    }

    const int MAX_SERVOS_PERCOMMAND = 6;
    if (servosInGroup.size() > MAX_SERVOS_PERCOMMAND) {
        qCDebug(ServoControllerLog) << "ServoController::toggleDesiredDropState: too many servos in group, only first 6 are handled:" << servoGroup;
    }

    for (int i = servosInGroup.size(); i < MAX_SERVOS_PERCOMMAND; i++) {
        servosInGroup.push_back(-1);
    }

    DoSetServoListHandlerData* handlerData = new DoSetServoListHandlerData(this, servosInGroup, newState);

    Vehicle::MavCmdAckHandlerInfo_t handlerInfo = {};
    handlerInfo.resultHandler       = _handleDoSetServoListCommandAck;
    handlerInfo.resultHandlerData   = handlerData;

    _vehicle->sendMavCommandWithHandler(&handlerInfo, _vehicle->defaultComponentId(), MAV_CMD_DO_SET_SERVO_LIST,
        desiredValue,
        servosInGroup[0], servosInGroup[1], servosInGroup[2],
        servosInGroup[3], servosInGroup[4], servosInGroup[5]
    );
}

void ServoController::setServoGroup(int servoIndex, int groupIndex) {
    if (servoIndex < 0 || servoIndex >= _servoAssignments.size()) {
        qCDebug(ServoControllerLog) << "ServoController::setServoGroup: servo index out of range:" << servoIndex;
        return;
    }
    _servoAssignments[servoIndex] = groupIndex;

    saveSettings();

    emit servoAssignmentsChanged();

    validateDropConfiguration();
}

void ServoController::saveSettings() {
    QSettings settings;
    settings.beginGroup(QString(_settingsGroup).arg(_vehicle->id()));
    for (int32_t i = 0; i < _servoModel.size(); i++) {
        Servo* servo = qvariant_cast<Servo*>(_servoModel[i]);
        settings.setValue(QString(_servoAssignmentKey).arg(servo->index() + 1), _servoAssignments[i]);
    }
}

void ServoController::loadSettings() {
    QSettings settings;
    settings.beginGroup(QString(_settingsGroup).arg(_vehicle->id()));

    for (int32_t i = 0; i < _servoModel.size(); i++) {
        Servo* servo = qvariant_cast<Servo*>(_servoModel[i]);
        _servoAssignments[i] = settings.value(QString(_servoAssignmentKey).arg(servo->index() + 1)).toInt();
    }

    emit servoAssignmentsChanged();
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

    for (int servoIndex : _servoIndexes) {
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

    for (int servoIndex : _servoIndexes) {
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

    _servoAssignments.resize(_servoModel.size());
    _servoStates.resize(_servoModel.size());
    for (int i = 0; i < _servoModel.size(); i++) {
        _servoAssignments[i] = 0;
        _servoStates[i] = false;
    }

    _initialized = true;

    loadSettings();

    emit servoStatesChanged();
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

    for(int servoIndex : _servoIndexes) {
        Servo* serv = _findServo(servoIndex - 1);
        if (serv) {
            serv->setValue(_servoOutputsRaw[servoIndex - 1]);
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

void ServoController::resetGroupAssignments() {
    for (int i = 0; i < _servoAssignments.size(); i++) {
        _servoAssignments[i] = 0;
    }
    
    saveSettings();

    emit servoAssignmentsChanged();

    validateDropConfiguration();
}

void ServoController::validateDropConfiguration() {
    _dropConfigValidationErrors.clear();
    for (int groupIndex = 1; groupIndex <= _servoModel.size(); groupIndex++) {
        Servo* firstServo = nullptr;
        for (int servoIndex = 0; servoIndex < _servoModel.size(); servoIndex++) {
            if (_servoAssignments[servoIndex] != groupIndex) continue;
            if (firstServo == nullptr) {
                firstServo = qvariant_cast<Servo*>(_servoModel[servoIndex]);
                continue;
            }
            Servo* currentServo = qvariant_cast<Servo*>(_servoModel[servoIndex]);
            if (currentServo->minValue() != firstServo->minValue()) {
                _dropConfigValidationErrors.push_back(QString(
                    "Parameter SERVO%1_MIN doesn't match SERVO%2_MIN"
                ).arg(currentServo->index() + 1).arg(firstServo->index() + 1));
            }
            if (currentServo->maxValue() != firstServo->maxValue()) {
                _dropConfigValidationErrors.push_back(QString(
                    "Parameter SERVO%1_MAX doesn't match SERVO%2_MAX"
                ).arg(currentServo->index() + 1).arg(firstServo->index() + 1));
            }
            if (currentServo->reversed() != firstServo->reversed()) {
                _dropConfigValidationErrors.push_back(QString(
                    "Parameter SERVO%1_REVERSED doesn't match SERVO%2_REVERSED"
                ).arg(currentServo->index() + 1).arg(firstServo->index() + 1));
            }
        }
    }

    if(_dropConfigValidationErrors.size() > 0) {
        qCDebug(ServoControllerLog) << _dropConfigValidationErrors;
    }

    emit dropConfigValidationErrorsChanged();
}
