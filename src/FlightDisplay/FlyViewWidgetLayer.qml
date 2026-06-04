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
import QtQuick.Dialogs          1.3
import QtQuick.Layouts          1.12

import QtLocation               5.3
import QtPositioning            5.3
import QtQuick.Window           2.2
import QtQml.Models             2.1

import QGroundControl               1.0
import QGroundControl.Controls      1.0
import QGroundControl.Controllers   1.0
import QGroundControl.FactSystem    1.0
import QGroundControl.FlightDisplay 1.0
import QGroundControl.FlightMap     1.0
import QGroundControl.Palette       1.0
import QGroundControl.ScreenTools   1.0
import QGroundControl.Vehicle       1.0

// This is the ui overlay layer for the widgets/tools for Fly View
Item {
    id: _root

    property var    parentToolInsets
    property var    totalToolInsets:        _totalToolInsets
    property var    mapControl

    property var    _activeVehicle:         QGroundControl.multiVehicleManager.activeVehicle
    property var    _planMasterController:  globals.planMasterControllerFlyView
    property var    _missionController:     _planMasterController.missionController
    property var    _geoFenceController:    _planMasterController.geoFenceController
    property var    _rallyPointController:  _planMasterController.rallyPointController
    property var    _guidedController:      globals.guidedControllerFlyView
    property real   _margins:               ScreenTools.defaultFontPixelWidth / 2
    property real   _toolsMargin:           ScreenTools.defaultFontPixelWidth * 0.75
    property rect   _centerViewport:        Qt.rect(0, 0, width, height)
    property real   _rightPanelWidth:       ScreenTools.defaultFontPixelWidth * 30
    property alias  _gripperMenu:           gripperOptions

    QGCToolInsets {
        id:                     _totalToolInsets
        leftEdgeTopInset:       toolStrip.leftEdgeTopInset
        leftEdgeCenterInset:    parentToolInsets.leftEdgeCenterInset
        leftEdgeBottomInset:    virtualJoystickMultiTouch.visible ? virtualJoystickMultiTouch.leftEdgeBottomInset : parentToolInsets.leftEdgeBottomInset
        rightEdgeTopInset:      instrumentPanel.rightEdgeTopInset
        rightEdgeCenterInset:   (telemetryPanel.rightEdgeCenterInset > photoVideoControl.rightEdgeCenterInset) ? telemetryPanel.rightEdgeCenterInset : photoVideoControl.rightEdgeCenterInset
        rightEdgeBottomInset:   virtualJoystickMultiTouch.visible ? virtualJoystickMultiTouch.rightEdgeBottomInset : parentToolInsets.rightEdgeBottomInset
        topEdgeLeftInset:       toolStrip.topEdgeLeftInset
        topEdgeCenterInset:     mapScale.topEdgeCenterInset
        topEdgeRightInset:      instrumentPanel.topEdgeRightInset
        bottomEdgeLeftInset:    virtualJoystickMultiTouch.visible ? virtualJoystickMultiTouch.bottomEdgeLeftInset : parentToolInsets.bottomEdgeLeftInset
        bottomEdgeCenterInset:  telemetryPanel.bottomEdgeCenterInset
        bottomEdgeRightInset:   virtualJoystickMultiTouch.visible ? virtualJoystickMultiTouch.bottomEdgeRightInset : parentToolInsets.bottomEdgeRightInset
    }

    FlyViewMissionCompleteDialog {
        missionController:      _missionController
        geoFenceController:     _geoFenceController
        rallyPointController:   _rallyPointController
    }

    Row {
        id:                 multiVehiclePanelSelector
        anchors.margins:    _toolsMargin
        anchors.top:        parent.top
        anchors.right:      parent.right
        width:              _rightPanelWidth
        spacing:            ScreenTools.defaultFontPixelWidth
        visible:            QGroundControl.multiVehicleManager.vehicles.count > 1 && QGroundControl.corePlugin.options.flyView.showMultiVehicleList

        property bool showSingleVehiclePanel:  !visible || singleVehicleRadio.checked

        QGCMapPalette { id: mapPal; lightColors: true }

        QGCRadioButton {
            id:             singleVehicleRadio
            text:           qsTr("Single")
            checked:        true
            textColor:      mapPal.text
        }

        QGCRadioButton {
            text:           qsTr("Multi-Vehicle")
            textColor:      mapPal.text
        }
    }

    MultiVehicleList {
        anchors.margins:    _toolsMargin
        anchors.top:        multiVehiclePanelSelector.bottom
        anchors.right:      parent.right
        width:              _rightPanelWidth
        height:             parent.height - y - _toolsMargin
        visible:            !multiVehiclePanelSelector.showSingleVehiclePanel
    }


    GuidedActionConfirm {
        anchors.margins:            _toolsMargin
        anchors.top:                parent.top
        anchors.horizontalCenter:   parent.horizontalCenter
        z:                          QGroundControl.zOrderTopMost
        guidedController:           _guidedController
        guidedValueSlider:          _guidedValueSlider
    }

    FlyViewInstrumentPanel {
        id:                         instrumentPanel
        anchors.margins:            _toolsMargin
        anchors.top:                multiVehiclePanelSelector.visible ? multiVehiclePanelSelector.bottom : parent.top
        anchors.right:              parent.right
        width:                      _rightPanelWidth
        spacing:                    _toolsMargin
        visible:                    QGroundControl.corePlugin.options.flyView.showInstrumentPanel && multiVehiclePanelSelector.showSingleVehiclePanel
        availableHeight:            parent.height - y - _toolsMargin

        property real rightEdgeTopInset: visible ? parent.width - x : 0
        property real topEdgeRightInset: visible ? y + height : 0
    }

    PhotoVideoControl {
        id:                     photoVideoControl
        width:                  _rightPanelWidth
        visible:                _activeVehicle ? true : false

        anchors.margins:        _toolsMargin
        anchors.right:          parent.right
        anchors.top:            vibrationIndicator.bottom

        property real rightEdgeCenterInset: visible ? parent.width - x : 0
    }

    TelemetryValuesBar {
        id:                 telemetryPanel
        x:                  recalcXPosition()
        anchors.margins:    _toolsMargin

        property real bottomEdgeCenterInset: 0
        property real rightEdgeCenterInset: 0

        // States for custom layout support
        states: [
            State {
                name: "bottom"
                when: telemetryPanel.bottomMode

                AnchorChanges {
                    target: telemetryPanel
                    anchors.top: undefined
                    anchors.bottom: parent.bottom
                    anchors.right: undefined
                    anchors.verticalCenter: undefined
                }

                PropertyChanges {
                    target: telemetryPanel
                    x: recalcXPosition()
                    bottomEdgeCenterInset: visible ? parent.height-y : 0
                    rightEdgeCenterInset: 0
                }
            },

            State {
                name: "right-video"
                when: !telemetryPanel.bottomMode && photoVideoControl.visible

                AnchorChanges {
                    target: telemetryPanel
                    anchors.top: photoVideoControl.bottom
                    anchors.bottom: undefined
                    anchors.right: parent.right
                    anchors.verticalCenter: undefined
                }
                PropertyChanges {
                    target: telemetryPanel
                    bottomEdgeCenterInset: 0
                    rightEdgeCenterInset: visible ? parent.width - x : 0
                }
            },

            State {
                name: "right-novideo"
                when: !telemetryPanel.bottomMode && !photoVideoControl.visible

                AnchorChanges {
                    target: telemetryPanel
                    anchors.top: undefined
                    anchors.bottom: undefined
                    anchors.right: parent.right
                    anchors.verticalCenter: parent.verticalCenter
                }
                PropertyChanges {
                    target: telemetryPanel
                    bottomEdgeCenterInset: 0
                    rightEdgeCenterInset: visible ? parent.width - x : 0
                }
            }
        ]

        function recalcXPosition() {
            // First try centered
            var halfRootWidth   = _root.width / 2
            var halfPanelWidth  = telemetryPanel.width / 2
            var leftX           = (halfRootWidth - halfPanelWidth) - _toolsMargin
            var rightX          = (halfRootWidth + halfPanelWidth) + _toolsMargin
            if (leftX >= parentToolInsets.leftEdgeBottomInset || rightX <= parentToolInsets.rightEdgeBottomInset ) {
                // It will fit in the horizontalCenter
                return halfRootWidth - halfPanelWidth
            } else {
                // Anchor to left edge
                return parentToolInsets.leftEdgeBottomInset + _toolsMargin
            }
        }
    }

    Rectangle {
        id:                  cameraTiltControl
        width:               60
        radius:              4
        color:               "#80202020"
        anchors.top:         instrumentPanel.top
        anchors.bottom:      photoVideoControl.bottom
        anchors.right:       instrumentPanel.left
        anchors.rightMargin: 6
        visible:             initialized

        property var  activeVehicle:      QGroundControl.multiVehicleManager.activeVehicle
        property var  paramaterManager:   activeVehicle ? activeVehicle.parameterManager : null
        property bool parametersReady:    paramaterManager ? !paramaterManager.missingParameters : false
        property var  gimbalController:   activeVehicle ? activeVehicle.gimbalController : null
        property var  activeGimbal:       gimbalController ? gimbalController.activeGimbal : null
        property var  deviceId:           activeGimbal ? activeGimbal.deviceId.value : 0
        property var  gimbalReady:        activeGimbal ? deviceId > 0 : false
        property bool initialized:        parametersReady && gimbalReady
        property var  currentTilt:        activeGimbal ? activeGimbal.absolutePitch.rawValue : 0
        property var  currentYaw:         activeGimbal ? activeGimbal.bodyYaw.rawValue : 0
        property Fact tiltMinFact:        initialized ? paramaterManager.getParameter(-1, qsTr("MNT%1_PITCH_MIN").arg(deviceId)) : null
        property Fact tiltMaxFact:        initialized ? paramaterManager.getParameter(-1, qsTr("MNT%1_PITCH_MAX").arg(deviceId)) : null
        property real tiltMin:            tiltMinFact ? tiltMinFact.value : -90
        property real tiltMax:            tiltMaxFact ? tiltMaxFact.value : 10
        property real range:              tiltMax - tiltMin
        property real targetTilt:         currentTilt
        property bool isTargetVisible:    false

        Rectangle {
            anchors.fill:         parent
            color:                "#00000000"
            anchors.topMargin:    4
            anchors.bottomMargin: 36
            anchors.leftMargin:   4
            anchors.rightMargin:  4

            Rectangle {
                anchors.top:          parent.top
                anchors.left:         parent.left
                anchors.right:        parent.right
                height:               2
                color:                "#ffffff"
                radius:               2
            }

            Rectangle {
                anchors.bottom:       parent.bottom
                anchors.left:         parent.left
                anchors.right:        parent.right
                height:               2
                color:                "#ffffff"
                radius:               2
            }

            Rectangle {
                anchors.top:              parent.top
                anchors.bottom:           parent.bottom
                anchors.horizontalCenter: parent.horizontalCenter
                width:                    2
                color:                    "#ffffff"
            }

            Rectangle {
                anchors.left:                 parent.left
                anchors.right:                parent.right
                height:                       2
                color:                        "#ffffff"
                radius:                       2
                anchors.verticalCenter:       parent.bottom
                anchors.verticalCenterOffset: -(parent.height * position)

                property real position: -cameraTiltControl.tiltMin / cameraTiltControl.range
            }

            Rectangle {
                anchors.left:                 parent.left
                anchors.right:                parent.right
                anchors.leftMargin:           6
                anchors.rightMargin:          6
                height:                       4
                color:                        "#C02020"
                radius:                       2
                anchors.verticalCenter:       parent.bottom
                anchors.verticalCenterOffset: -(parent.height * position)
                visible:                      cameraTiltControl.isTargetVisible

                property real position: (cameraTiltControl.targetTilt - cameraTiltControl.tiltMin) / cameraTiltControl.range
            }

            Rectangle {
                anchors.left:                 parent.left
                anchors.right:                parent.right
                anchors.leftMargin:           6
                anchors.rightMargin:          6
                height:                       4
                color:                        "#20a020"
                radius:                       2
                anchors.verticalCenter:       parent.bottom
                anchors.verticalCenterOffset: -(parent.height * position)

                property real position: (cameraTiltControl.currentTilt - cameraTiltControl.tiltMin) / cameraTiltControl.range
            }

            MouseArea {
                anchors.fill: parent

                function updateTargetTiltFromMouse(mouseY) {
                    let normalized = 1.0 - Math.max(0.0, Math.min(1.0, mouseY / parent.height));
                    let targetAngle = normalized * cameraTiltControl.range + cameraTiltControl.tiltMin;
                    cameraTiltControl.targetTilt = targetAngle;
                }

                onPressed: (mouse) => {
                    updateTargetTiltFromMouse(mouse.y);
                    cameraTiltControl.isTargetVisible = true;
                }

                onPositionChanged: (mouse) => {
                    updateTargetTiltFromMouse(mouse.y);
                }

                onReleased: (mouse) => {
                    updateTargetTiltFromMouse(mouse.y);
                    if (cameraTiltControl.gimbalController) {
                        cameraTiltControl.gimbalController.sendPitchBodyYaw(cameraTiltControl.targetTilt, cameraTiltControl.currentYaw)
                    }
                    cameraTiltControl.isTargetVisible = false;
                }
            }
        }

        Row {
            anchors.bottom:           parent.bottom
            anchors.bottomMargin:     18
            anchors.horizontalCenter: parent.horizontalCenter

            QGCLabel {
                text: "C: "
                color: "#20a020"
            }
            QGCLabel {
                text: cameraTiltControl.currentTilt.toFixed(1)
                width: 24
            }
        }

        Row {
            anchors.bottom:           parent.bottom
            anchors.bottomMargin:     2
            anchors.horizontalCenter: parent.horizontalCenter
            visible:                  cameraTiltControl.isTargetVisible

            QGCLabel {
                text:  "T: "
                color: "#C02020"
            }
            QGCLabel {
                text: cameraTiltControl.targetTilt.toFixed(1)
                width: 24
            }
        }
    }


    EKFIndicator {
        id:        ekfIndicator
        height:    instrumentPanel ? instrumentPanel._heightAttComp : 0
        visible:   _activeVehicle ? true : false
        z:         QGroundControl.zOrderWidgets

        anchors.top:          instrumentPanel.bottom
        anchors.topMargin:    _toolsMargin
        anchors.right:        instrumentPanel.right
        anchors.left:         instrumentPanel.left
        anchors.bottomMargin: _toolsMargin
    }

    VibrationIndicator {
        id:        vibrationIndicator
        height:    instrumentPanel ? instrumentPanel._heightAttComp : 0
        visible:   _activeVehicle ? true : false
        z:         QGroundControl.zOrderWidgets

        anchors.top:          ekfIndicator.bottom
        anchors.topMargin:    _toolsMargin
        anchors.right:        ekfIndicator.right
        anchors.left:         ekfIndicator.left
        anchors.bottomMargin: _toolsMargin
    }

    //-- Virtual Joystick
    Loader {
        id:                         virtualJoystickMultiTouch
        z:                          QGroundControl.zOrderTopMost + 1
        width:                      parent.width  - (_pipOverlay.width / 2)
        height:                     Math.min(parent.height * 0.25, ScreenTools.defaultFontPixelWidth * 16)
        visible:                    _virtualJoystickEnabled && !QGroundControl.videoManager.fullScreen && !(_activeVehicle ? _activeVehicle.usingHighLatencyLink : false)
        anchors.bottom:             parent.bottom
        anchors.bottomMargin:       parentToolInsets.leftEdgeBottomInset + ScreenTools.defaultFontPixelHeight * 2
        anchors.horizontalCenter:   parent.horizontalCenter
        source:                     "qrc:/qml/VirtualJoystick.qml"
        active:                     _virtualJoystickEnabled && !(_activeVehicle ? _activeVehicle.usingHighLatencyLink : false)

        property bool autoCenterThrottle: QGroundControl.settingsManager.appSettings.virtualJoystickAutoCenterThrottle.rawValue

        property bool _virtualJoystickEnabled: QGroundControl.settingsManager.appSettings.virtualJoystick.rawValue

        property real bottomEdgeLeftInset: parent.height-y
        property real bottomEdgeRightInset: parent.height-y

        // Width is difficult to access directly hence this hack which may not work in all circumstances
        property real leftEdgeBottomInset: visible ? bottomEdgeLeftInset + width/18 - ScreenTools.defaultFontPixelHeight*2 : 0
        property real rightEdgeBottomInset: visible ? bottomEdgeRightInset + width/18 - ScreenTools.defaultFontPixelHeight*2 : 0
    }

    FlyViewToolStrip {
        id:                     toolStrip
        anchors.leftMargin:     _toolsMargin + parentToolInsets.leftEdgeCenterInset
        anchors.topMargin:      _toolsMargin + parentToolInsets.topEdgeLeftInset
        anchors.left:           parent.left
        anchors.top:            parent.top
        z:                      QGroundControl.zOrderWidgets
        maxHeight:              parent.height - y - parentToolInsets.bottomEdgeLeftInset - _toolsMargin
        visible:                !QGroundControl.videoManager.fullScreen

        onDisplayPreFlightChecklist: preFlightChecklistPopup.createObject(mainWindow).open()


        property real topEdgeLeftInset: visible ? y + height : 0
        property real leftEdgeTopInset: visible ? x + width : 0
    }

    function servoValueScale(number, inMin, inMax, outMin, outMax, reversed) {
        number = Math.min(Math.max(number, inMin), inMax);

        let t = (number - inMin) / (inMax - inMin);
        if (isNaN(t)) t = 0;
        if (reversed) {
            let val = outMax - t * (outMax - outMin);
            return val
        } else {
            let val = outMin + t * (outMax - outMin);
            return val
        }
    }

    GripperMenu {
        id: gripperOptions
    }

    VehicleWarnings {
        anchors.centerIn:   parent
        z:                  QGroundControl.zOrderTopMost
    }

    MapScale {
        id:                 mapScale
        anchors.margins:    _toolsMargin
        anchors.left:       toolStrip.right
        anchors.top:        parent.top
        mapControl:         _mapControl
        buttonsOnLeft:      true
        visible:            !ScreenTools.isTinyScreen && QGroundControl.corePlugin.options.flyView.showMapScale && mapControl.pipState.state === mapControl.pipState.fullState

        property real topEdgeCenterInset: visible ? y + height : 0
    }

    Component {
        id: preFlightChecklistPopup
        FlyViewPreFlightChecklistPopup {
        }
    }
}
