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

import QtQml.Models 2.12

import QGroundControl                1.0
import QGroundControl.Controls       1.0

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
            iconSource:     _turboModeEnabled ? "/qmlimages/turboActive.svg" : "/qmlimages/turbo.svg"
            fullColorIcon: true

            property var  _activeVehicle:   QGroundControl.multiVehicleManager.activeVehicle
            property bool _turboModeEnabled: _activeVehicle ? _activeVehicle.turboModeEnabled : false

            dropPanelComponent: Rectangle {
                height: instrumentPanel ? instrumentPanel._heightAttComp * 0.5 : 0
                width:  instrumentPanel ? instrumentPanel._heightAttComp * 2 : 0
                Button {
                    id:                 turboBtn
                    text:               "Toggle max angle 17.5°"
                    palette.buttonText: "white"
                    anchors.fill:       parent

                    background: Rectangle {
                        anchors.fill:   parent
                        radius:         2
                        border.color:   "#555"
                        border.width:   1
                        color:          turboBtn.down ? "#2c2c2c" : turboBtn.hovered ? "#3a3a3a" : "#444444"
                    }

                    onClicked: {
                        if(turboModeAction._activeVehicle) {
                            turboModeAction._activeVehicle.toggleTurboModeEnabled();
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
