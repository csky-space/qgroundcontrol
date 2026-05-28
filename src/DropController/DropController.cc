#include "DropController.h"

#include <QGCApplication.h>
#include <ParameterManager.h>

QGC_LOGGING_CATEGORY(DropControllerLog, "DropControllerLog")

DropController::DropController(MAVLinkProtocol* mavlink, Vehicle* vehicle)
    : _mavlink(mavlink)
    , _vehicle(vehicle)
    , _initialized(false)
    , _isInterfaceLocked(true)
    , _activeModeIndex(-1) {
    QQmlEngine::setObjectOwnership(this, QQmlEngine::CppOwnership);
    connect(_vehicle->parameterManager(), &ParameterManager::parametersReadyChanged, this, &DropController::_onParametersReadyChanged);

    _paramUpdateTimer.setInterval(3000);
    connect(&_paramUpdateTimer, &QTimer::timeout, this, &DropController::_onParamUpdateTimeout);

    _dropModesModel = QStringList({
        "Drop 1",
        "Drop 2",
        "Drop 4"
    });
    emit dropModesModelChanged();
}

DropController::~DropController() {

}

bool DropController::initialized() const {
    return _initialized;
}

bool DropController::isInterfaceLocked() const {
    return _isInterfaceLocked;
}

QStringList DropController::dropModesModel() const {
    return _dropModesModel;
}

int DropController::activeModeIndex() const {
    return _activeModeIndex;
}

void DropController::setDropModeIndex(int index) {
    if (index < 0 || index >= _dropModesModel.size()) {
        qCDebug(DropControllerLog) << "setDropModeIndex: index is out of range.";
        return;
    }

    if (!_vehicle->parameterManager()->parameterExists(_vehicle->defaultComponentId(), "DROP_MODE")) {
        qCDebug(DropControllerLog) << "setDropModeIndex: DROP_MODE parameter does not exist.";
        return;
    }

    Fact* dropModeParameter = _vehicle->parameterManager()->getParameter(_vehicle->defaultComponentId(), "DROP_MODE");
    dropModeParameter->setRawValue(index);

    qCDebug(DropControllerLog) << "setDropModeIndex: param set request sent.";

    if (_activeModeIndex != index) {
        _isInterfaceLocked = true;
        emit isInterfaceLockedChanged();
        _paramUpdateTimer.start();
    }
    else {
        qCDebug(DropControllerLog) << "setDropModeIndex: index is matching active index. Interface is not locked.";
    }
}

void DropController::_onParametersReadyChanged(bool parametersReady) {
    if (_initialized || !parametersReady) {
        return;
    }

    if(!_vehicle->parameterManager()->parameterExists(_vehicle->defaultComponentId(), "DROP_MODE")) {
        qCDebug(DropControllerLog) << "DropController failed to initialize because of missing DROP_MODE parameter.";
        return;
    }

    Fact* dropModeParameter = _vehicle->parameterManager()->getParameter(_vehicle->defaultComponentId(), "DROP_MODE");
    connect(dropModeParameter, &Fact::vehicleUpdated, this, &DropController::_onDropModeParameterChanged);

    _activeModeIndex = dropModeParameter->rawValue().toInt();
    emit activeModeIndexChanged();

    _initialized = true;
    emit initializedChanged();

    _isInterfaceLocked = false;
    emit isInterfaceLockedChanged();

    qCDebug(DropControllerLog) << "DropController initialized.";
}

void DropController::_onDropModeParameterChanged() {
    _paramUpdateTimer.stop();

    Fact* dropModeParameter = _vehicle->parameterManager()->getParameter(_vehicle->defaultComponentId(), "DROP_MODE");
    _activeModeIndex = dropModeParameter->rawValue().toInt();

    qCDebug(DropControllerLog) << "_onDropModeParameterChanged: drop mode parameter updated:" << _activeModeIndex;

    emit activeModeIndexChanged();

    _isInterfaceLocked = false;
    emit isInterfaceLockedChanged();
}

void DropController::_onParamUpdateTimeout() {
    qCDebug(DropControllerLog) << "_onParamUpdateTimeout: parameter set request timed out.";
    _paramUpdateTimer.stop();

    _isInterfaceLocked = false;
    emit isInterfaceLockedChanged();
}
