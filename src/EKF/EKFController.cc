#include "EKFController.h"

#include "MAVLinkProtocol.h"
#include "Vehicle.h"
#include "QGCApplication.h"
#include "SettingsManager.h"
#include "ParameterManager.h"

#include <Eigen/Eigen>

QGC_LOGGING_CATEGORY(EKFLog, "EKFLog")

EKFController::EKFController(MAVLinkProtocol* mavlink, Vehicle* vehicle)
    : _velocity_variance(0.0f)
    , _pos_horiz_variance(0.0f)
    , _pos_vert_variance(0.0f)
    , _compass_variance(0.0f)
    , _terrain_alt_variance(0.0f)
    , _mavlink(mavlink)
    , _vehicle(vehicle)

{
    QQmlEngine::setObjectOwnership(this, QQmlEngine::CppOwnership);
    connect(_vehicle, &Vehicle::mavlinkMessageReceived, this, &EKFController::_mavlinkMessageReceived);
}

EKFController::~EKFController()
{
}

float EKFController::velocityVariance() const {
    return _velocity_variance;
}
float EKFController::posHorizVariance() const {
    return _pos_horiz_variance;
}
float EKFController::posVertVariance() const {
    return _pos_vert_variance;
}
float EKFController::compassVariance() const {
    return _compass_variance;
}
float EKFController::terrainAltVariance() const {
    return _terrain_alt_variance;
}

void
EKFController::_mavlinkMessageReceived(const mavlink_message_t& message)
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
    case MAVLINK_MSG_ID_EKF_STATUS_REPORT:
        _handleEKF_STATUS_REPORT(message);
        break;
        break;
    }
}

void
EKFController::_handleHeartbeat(const mavlink_message_t& message)
{

}

void
EKFController::_handleEKF_STATUS_REPORT(const mavlink_message_t& message)
{
    mavlink_ekf_status_report_t ekf;
    mavlink_msg_ekf_status_report_decode(&message, &ekf);
    _velocity_variance = ekf.velocity_variance;
    emit velocityVarianceChanged();
    _pos_horiz_variance = ekf.pos_horiz_variance;
    emit posHorizVarianceChanged();
    _pos_vert_variance = ekf.pos_vert_variance;
    emit posVertVarianceChanged();
    _compass_variance = ekf.compass_variance;
    emit compassVarianceChanged();
    _terrain_alt_variance = ekf.terrain_alt_variance;
    emit terrainAltVarianceChanged();
    qCDebug(EKFLog) << "_velocity_variance: " << _velocity_variance << "; _pos_horiz_variance:" << _pos_horiz_variance << "; _pos_vert_variance:" << _pos_vert_variance
                    << "; _compass_variance:" << _compass_variance << "; _terrain_alt_variance:" << _terrain_alt_variance;
}
