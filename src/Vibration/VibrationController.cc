#include "VibrationController.h"

#include "MAVLinkProtocol.h"
#include "Vehicle.h"
#include "QGCApplication.h"
#include "SettingsManager.h"
#include "ParameterManager.h"

#include <Eigen/Eigen>

QGC_LOGGING_CATEGORY(VibrationLog, "VibrationLog")

VibrationController::VibrationController(MAVLinkProtocol* mavlink, Vehicle* vehicle)
    : _xVibration(0.0f)
    , _yVibration(0.0f)
    , _zVibration(0.0f)
    , _clp1(0)
    , _clp2(0)
    , _clp3(0)
    , _mavlink(mavlink)
    , _vehicle(vehicle)

{
    QQmlEngine::setObjectOwnership(this, QQmlEngine::CppOwnership);
    connect(_vehicle, &Vehicle::mavlinkMessageReceived, this, &VibrationController::_mavlinkMessageReceived);
}

VibrationController::~VibrationController()
{
}

float VibrationController::xVibration() const {
    return _xVibration;
}
float VibrationController::yVibration() const {
    return _yVibration;
}
float VibrationController::zVibration() const {
    return _zVibration;
}
unsigned int VibrationController::clipping1() const {
    return _clp1;
}
unsigned int VibrationController::clipping2() const {
    return _clp2;
}
unsigned int VibrationController::clipping3() const {
    return _clp3;
}

Fact* VibrationController::xVibrationFact() const {
    return qgcApp()->toolbox()->settingsManager()->vibrationControllerSettings()->VibrationX();
}
Fact* VibrationController::yVibrationFact() const {
    return qgcApp()->toolbox()->settingsManager()->vibrationControllerSettings()->VibrationY();
}
Fact* VibrationController::zVibrationFact() const {
    return qgcApp()->toolbox()->settingsManager()->vibrationControllerSettings()->VibrationZ();
}

void
VibrationController::_mavlinkMessageReceived(const mavlink_message_t& message)
{
    // Don't proceed until parameters are ready, otherwise the vibration controller handshake
    // could potentially not work due to the high traffic for parameters, mission download, etc
    if (!_vehicle->parameterManager()->parametersReady() ) {
        return;
    }
    switch (message.msgid) {
    case MAVLINK_MSG_ID_HEARTBEAT:
        _handleHeartbeat(message);
        break;
    case MAVLINK_MSG_ID_VIBRATION:
        _handleVibrationManagerInformation(message);
        break;
        break;
    }
}

void
VibrationController::_handleHeartbeat(const mavlink_message_t& message)
{

}

void
VibrationController::_handleVibrationManagerInformation(const mavlink_message_t& message)
{

    mavlink_vibration_t vibr;
    mavlink_msg_vibration_decode(&message, &vibr);
    _xVibration = vibr.vibration_x;
    emit xVibrationChanged();
    _yVibration = vibr.vibration_y;
    emit yVibrationChanged();
    _zVibration = vibr.vibration_z;
    emit zVibrationChanged();
    _clp1 = vibr.clipping_0;
    emit clipping1Changed();
    _clp2 = vibr.clipping_1;
    emit clipping2Changed();
    _clp3 = vibr.clipping_2;
    emit clipping3Changed();
    //settings->

   // _checkComplete(vibration, pairId);
}
