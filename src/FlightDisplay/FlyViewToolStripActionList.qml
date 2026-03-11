/****************************************************************************
 *
 * (c) 2009-2020 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/

import QtQml.Models 2.12

import QGroundControl           1.0
import QGroundControl.Controls  1.0

ToolStripActionList {
    id: _root

    signal displayPreFlightChecklist
    property var joyManager: QGroundControl.joystickManager
    property var activeJoystick: joyManager.activeJoystick
    property bool isSendingRC: activeJoystick ? activeJoystick.isSendingRC : false
    model: [
        ToolStripAction {
            text:           qsTr("Plan")
            iconSource:     "/qmlimages/Plan.svg"
            onTriggered:    mainWindow.showPlanView()
        },
        PreFlightCheckListShowAction { onTriggered: displayPreFlightChecklist() },
        GuidedActionTakeoff { },
        GuidedActionLand { },
        GuidedActionRTL { },
        GuidedActionPause { },
        GuidedActionActionList { },
        GuidedActionGripper { },
        ToolStripAction {
            text:           qsTr("Joystick")
            iconSource:     (activeJoystick !== undefined) && isSendingRC ? "/qmlimages/gamepadSwitcherActive.svg" : "/qmlimages/gamepadSwitcher.svg"
            fullColorIcon: true
            onTriggered:    {
                if (activeJoystick) {
                    activeJoystick.isSendingRC = !activeJoystick.isSendingRC;
                }
            }
        }
    ]
}
