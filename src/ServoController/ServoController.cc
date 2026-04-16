#include "ServoController.h"

#include <QGCApplication.h>
#include <ParameterManager.h>

QGC_LOGGING_CATEGORY(ServoControllerLog, "ServoControllerLog")

const QString& ServoMode::name() const {
    return _name;
}

bool ServoMode::checked() const {
    return _checked;
}

void ServoMode::setChecked(bool checked) {
    qCDebug(ServoControllerLog) << "setChecked to " << checked << " for mode " << _name;
    _checked = checked;
    emit checkedChanged();
}

ServoMode::ServoMode(const QString& name, bool checked)
    : _name(name)
    , _checked(checked)
{}

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
    : _carpetationTimer(new QTimer(this))
    , _allMode(new ServoMode("All", false))
    , _2Mode(new ServoMode("2", false))
    , _1Mode(new ServoMode("1", true))
    , _carpetMode(new ServoMode("Carpet", false))
    , _mavlink(mavlink)
    , _vehicle(vehicle)
    , _dropIndex(0)

{
    QQmlEngine::setObjectOwnership(this, QQmlEngine::CppOwnership);
    connect(_vehicle, &Vehicle::mavlinkMessageReceived, this, &ServoController::_mavlinkMessageReceived);

    connect(_allMode, &ServoMode::checkedChanged, this, [this](){
        if(_allMode->checked()) {
            qCDebug(ServoControllerLog) << "All mode using";
            _sendCommandLongCallback = [this](){
                if(_servoModel.empty()) return;
                Servo* servo = _servoModel[0].value<Servo*>();
                if(servo) {
                    float pwm = static_cast<float>(servo->reversed() ? servo->minValue() : servo->maxValue());
                    _vehicle->sendMavCommand(_vehicle->compId(), MAV_CMD_DO_SET_SERVO, true, static_cast<float>(servo->index()+1), pwm, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
                    _increaseDropIndex(1);
                }
                for(size_t i = 1; i < _servoModel.size(); ++i) {
                    _commandQueue.enqueue([i,this](){
                        Servo* servo = _servoModel[i].value<Servo*>();
                        if(servo) {
                            float pwm = static_cast<float>(servo->reversed() ? servo->minValue() : servo->maxValue());
                            _vehicle->sendMavCommand(_vehicle->compId(), MAV_CMD_DO_SET_SERVO, true, static_cast<float>(servo->index()+1), pwm, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
                            _increaseDropIndex(1);
                        }
                    });
                }
            };
        }
    });
    connect(_2Mode, &ServoMode::checkedChanged, this, [this](){
        if(_2Mode->checked()) {
            qCDebug(ServoControllerLog) << "2 mode using";
            _sendCommandLongCallback = [this](){
                if(_dropIndex+1 >= _servoModel.size()) return;
                Servo* servo1 = _servoModel[_dropIndex].value<Servo*>();
                if(servo1) {
                    float pwm1 = static_cast<float>(servo1->reversed() ? servo1->minValue() : servo1->maxValue());
                    _vehicle->sendMavCommand(_vehicle->compId(), MAV_CMD_DO_SET_SERVO, true, static_cast<float>(servo1->index()+1), static_cast<float>(pwm1), 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
                    _increaseDropIndex(1);
                }
                _commandQueue.enqueue([this](){
                    Servo* servo = _servoModel[_dropIndex].value<Servo*>();
                    if(servo) {
                        float pwm = static_cast<float>(servo->reversed() ? servo->minValue() : servo->maxValue());
                        _vehicle->sendMavCommand(_vehicle->compId(), MAV_CMD_DO_SET_SERVO, true, static_cast<float>(servo->index()+1), pwm, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
                        _increaseDropIndex(1);
                    }
                });
            };
        }

    });
    connect(_1Mode, &ServoMode::checkedChanged, this, [this](){
        if(_1Mode->checked()) {
            qCDebug(ServoControllerLog) << "1 mode using";
            _sendCommandLongCallback = [this](){
                if(_dropIndex < _servoModel.size()) {
                    Servo* servo1 = _servoModel[_dropIndex].value<Servo*>();
                    if(servo1) {
                        float pwm1 = static_cast<float>(servo1->reversed() ? servo1->minValue() : servo1->maxValue());
                        qCDebug(ServoControllerLog) << "servo index: " << servo1->index() + 1 << ". drop pwm: " << pwm1 << ". min: " << servo1->minValue() << ". max: " << servo1->maxValue();
                        qCDebug(ServoControllerLog) << "target sys: " << _vehicle->id() << ". target comp: " << _vehicle->compId();
                        _vehicle->sendMavCommand(_vehicle->compId(), MAV_CMD_DO_SET_SERVO, true, static_cast<float>(servo1->index()+1), static_cast<float>(pwm1), 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
                    }
                    _increaseDropIndex(1);
                }
            };
        }

    });
    connect(_carpetMode, &ServoMode::checkedChanged, this, [this](){
        if(_carpetMode->checked()) {
            qCDebug(ServoControllerLog) << "Carpet mode using";
            _sendCommandLongCallback = [this](){
                _carpetationTimer->setInterval(2000);
                _carpetationTimer->start();
            };
        }
    });


    connect(_vehicle, &Vehicle::mavCommandResult, this, &ServoController::_sendQueuedComand);

    _carpetationTimerConnection = connect(_carpetationTimer, &QTimer::timeout, this, [this](){
        Servo* servo1 = _servoModel[_dropIndex].value<Servo*>();
        if(servo1) {
            float pwm1 = static_cast<float>(servo1->reversed() ? servo1->minValue() : servo1->maxValue());
            _vehicle->sendMavCommand(_vehicle->compId(), MAV_CMD_DO_SET_SERVO, true, static_cast<float>(servo1->index()+1), static_cast<float>(pwm1), 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
        }
        _increaseDropIndex(1);
        if(_dropIndex == 0) {
            _carpetationTimer->stop();
        }
    });

    _servoDropModes.append(QVariant::fromValue(_allMode));
    _servoDropModes.append(QVariant::fromValue(_2Mode));
    _servoDropModes.append(QVariant::fromValue(_1Mode));
    _servoDropModes.append(QVariant::fromValue(_carpetMode));
    _1Mode->setChecked(true);
}

ServoController::~ServoController() {

}

void ServoController::_sendQueuedComand(int vehicleId, int targetComponent, int command, int ackResult, int failureCode) {
    if(!_commandQueue.empty() && (vehicleId == _vehicle->id()) && (targetComponent == _vehicle->compId()) && (command == MAV_CMD_DO_SET_SERVO)) {
        _commandQueue.dequeue()();
    }
}

void ServoController::_enqueueLongCommand(const std::function<void()>& command) {
    if(command) {
        _commandQueue.enqueue(command);
    }
}

void ServoController::drop() {
    _sendCommandLongCallback();
}

void ServoController::close() {
    if(_servoModel.empty()) return;
    Servo* servo = _servoModel[0].value<Servo*>();
    if(servo) {
        float pwm = static_cast<float>(!servo->reversed() ? servo->minValue() : servo->maxValue());
        _vehicle->sendMavCommand(_vehicle->compId(), MAV_CMD_DO_SET_SERVO, true, static_cast<float>(servo->index()+1), pwm, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
        _increaseDropIndex(1);
    }
    for(size_t i = 1; i < _servoModel.size(); ++i) {
        _commandQueue.enqueue([i,this](){
            Servo* servo = _servoModel[i].value<Servo*>();
            if(servo) {
                float pwm = static_cast<float>(!servo->reversed() ? servo->minValue() : servo->maxValue());
                _vehicle->sendMavCommand(_vehicle->compId(), MAV_CMD_DO_SET_SERVO, true, static_cast<float>(servo->index()+1), pwm, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
                _increaseDropIndex(1);
            }
        });
    }
}

QVariantList ServoController::servoModel() const {
    return _servoModel;
}


QVariantList ServoController::servoDropModes() const {
    return _servoDropModes;
}

quint16 ServoController::dropIndex() const{
    return _dropIndex;
}

void ServoController::_mavlinkMessageReceived(const mavlink_message_t& message) {
    switch(message.msgid) {
    case MAVLINK_MSG_ID_SERVO_OUTPUT_RAW:
        _handleServoOutputRaw(message);
        break;
    }
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

    for(size_t i = 0; i < _servoOutputsRaw.size(); ++i) {
        QString parameterName = QString("SERVO%1_FUNCTION").arg(i+1);
        QString minValueParameterName = QString("SERVO%1_MIN").arg(i+1);
        QString maxValueParameterName = QString("SERVO%1_MAX").arg(i+1);
        QString parameterReversed = QString("SERVO%1_REVERSED").arg(i+1);

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

        if(parameter && ((i+1 == 7) || (i+1 == 8))) {
            //qCDebug(ServoControllerLog) << "Servo min: " << minValue << ". servo value: " << _servoOutputsRaw[i] << ". servo max: " << maxValue << ". index: " << i;
            Servo* serv = findServo(i);
            if(serv) {
                serv->setMinValue(minValue);
                serv->setMaxValue(maxValue);
                serv->setReversed(reversed);
                serv->setValue(_servoOutputsRaw[i]);
            } else {
                serv = new Servo(QString("S%1").arg(i+1), i, i+1, _servoOutputsRaw[i], minValue, maxValue, reversed);
                _servoModel.append(QVariant::fromValue(serv));
                emit servoModelChanged();
            }
        }
    }
}

void ServoController::_increaseDropIndex(uint16_t incSize) {
    _dropIndex = _dropIndex += incSize;
    if(_dropIndex >= _servoModel.size()) {
        _dropIndex = 0;
    }
    emit dropIndexChanged();
}

bool ServoController::hasServo(uint16_t index) {
    for(uint16_t i = 0; i < _servoModel.size(); ++i) {
        if(_servoModel[i].value<Servo*>()->index() == index) {
            return true;
        }
    }
    return false;
}

Servo* ServoController::findServo(uint16_t index) {
    for(uint16_t i = 0; i < _servoModel.size(); ++i) {
        if(_servoModel[i].value<Servo*>()->index() == index) {
            return _servoModel[i].value<Servo*>();
        }
    }
    return nullptr;
}
