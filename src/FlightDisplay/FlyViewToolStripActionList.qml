/****************************************************************************
 *
 * (c) 2009-2020 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/
import QtQuick                  2.12
import QtQuick.Controls         2.4
import QtQuick.Layouts          1.12

import QtQml.Models 2.12

import QGroundControl                1.0
import QGroundControl.Controls       1.0
import QGroundControl.FactSystem     1.0

ToolStripActionList {
    id: _root

    signal displayPreFlightChecklist
    
    property var  joyManager:         QGroundControl.joystickManager
    property var  activeJoystick:     joyManager.activeJoystick
    property bool isSendingRC:        activeJoystick ? activeJoystick.isSendingRC : false
    property bool isCrosshairEnabled: QGroundControl.videoManager.crosshairEnabled

    model: [
        ToolStripAction {
            text:           qsTr("Plan")
            iconSource:     "/qmlimages/Plan.svg"
            onTriggered:    mainWindow.showPlanView()
        },
        PreFlightCheckListShowAction { onTriggered: displayPreFlightChecklist() },
        DropControl { },
        ToolStripAction {
            text:          qsTr("Crosshair")
            iconSource:    isCrosshairEnabled ? "/qmlimages/crosshairIconActive.svg" : "/qmlimages/crosshairIcon.svg"
            fullColorIcon: true
            onTriggered:   {
                QGroundControl.videoManager.crosshairEnabled = !QGroundControl.videoManager.crosshairEnabled;
            }
        },
        ToolStripAction {
            id:             turboModeAction
            text:           qsTr("Turbo")
            iconSource:     _activeVehicle && _activeVehicle.turboModeActive ? "/qmlimages/turboActive.svg" : "/qmlimages/turbo.svg"
            fullColorIcon:  true
            enabled:        _activeVehicle ? _activeVehicle.turboModeEnabled : false

            property var  _activeVehicle: QGroundControl.multiVehicleManager.activeVehicle

            dropPanelComponent:  ColumnLayout {
                QGCButton {
                    text:             "Enable"
                    Layout.fillWidth: true
                    enabled: _activeVehicle ? !_activeVehicle.isSwitchingTurboMode : false

                    onClicked: {
                        if (turboModeAction._activeVehicle) {
                            turboModeAction._activeVehicle.setTurboModeState(true);
                        }
                    }
                }

                QGCButton {
                    text:             "Disable"
                    Layout.fillWidth: true
                    enabled: _activeVehicle ? !_activeVehicle.isSwitchingTurboMode : false

                    onClicked: {
                        if (turboModeAction._activeVehicle) {
                            turboModeAction._activeVehicle.setTurboModeState(false);
                        }
                    }
                }
            }
        },
        ReturnCourse { },
        GuidedActionTakeoff { },
        GuidedActionLand { },
        GuidedActionRTL { },
        GuidedActionPause { },
        GuidedActionActionList { },
        GuidedActionGripper { },
        ToolStripAction {
            text:           qsTr("Joystick")
            iconSource:     isSendingRC ? "/qmlimages/gamepadSwitcherActive.svg" : "/qmlimages/gamepadSwitcher.svg"
            fullColorIcon:  true
            onTriggered:    {
                if (activeJoystick) {
                    activeJoystick.isSendingRC = !activeJoystick.isSendingRC;
                }
            }
        }
    ]
}
