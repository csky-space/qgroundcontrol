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
import QGroundControl.Controls      1.0
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
        anchors.margins:        _toolsMargin
        anchors.right:          parent.right
        width:                  _rightPanelWidth
        visible: _activeVehicle !== undefined ? true : false

        property real rightEdgeCenterInset: visible ? parent.width - x : 0

        state:                  _verticalCenter ? "verticalCenter" : "topAnchor"
        states: [
            State {
                name: "verticalCenter"
                AnchorChanges {
                    target:                 photoVideoControl
                    anchors.top:            undefined
                    anchors.verticalCenter: _root.verticalCenter
                }
            },
            State {
                name: "topAnchor"
                AnchorChanges {
                    target:                 photoVideoControl
                    anchors.verticalCenter: undefined
                    anchors.top:            instrumentPanel.bottom
                }
            }
        ]

        property bool _verticalCenter: !QGroundControl.settingsManager.flyViewSettings.alternateInstrumentPanel.rawValue
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

    EKFIndicator {
        id: ekfIndicator

        anchors.top: instrumentPanel.bottom
        anchors.topMargin: _toolsMargin
        anchors.right: instrumentPanel.right
        anchors.left: instrumentPanel.left
        anchors.bottomMargin: _toolsMargin

        height: instrumentPanel.height
        visible: _activeVehicle != null
        z: QGroundControl.zOrderWidgets
    }

    VibrationIndicator {
        id: vibrationIndicator

        anchors.top: ekfIndicator.bottom
        anchors.topMargin: _toolsMargin
        anchors.right: ekfIndicator.right
        anchors.left: ekfIndicator.left
        anchors.bottomMargin: _toolsMargin

        height: instrumentPanel.height
        visible: _activeVehicle != null
        z: QGroundControl.zOrderWidgets
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
        console.log("number before cast: " + number + ". reversed: " + reversed + ". min: " + inMin + ". max: " + inMax)
        number = Math.min(Math.max(number, inMin), inMax);

        let t = (number - inMin) / (inMax - inMin);
        if (isNaN(t)) t = 0;
        if (reversed) {
            let val = outMax - t * (outMax - outMin);
            console.log("number after cast: " + val + ". reversed: " + reversed)
            return val
        } else {
            let val = outMin + t * (outMax - outMin);
            console.log("number after cast: " + val + ". reversed: " + reversed)
            return val
        }
    }
    Rectangle {
        anchors.topMargin: _toolsMargin + parentToolInsets.topEdgeLeftInset
        anchors.rightMargin: _toolsMargin + parentToolInsets.topEdgeLeftInset

        anchors.top: toolStrip.bottom
        anchors.left: toolStrip.left
        //anchors.bottom: telemetryPanel.bottom
        //anchors.right: telemetryPanel.left

        height: instrumentPanel.height * 2
        width: instrumentPanel.width * 2

        ListModel {
            id: queueModel
            ListElement { value: "A" }
            ListElement { value: "B" }
            ListElement { value: "C" }
        }

        ListModel {
            id: servoButtonsModel
            ListElement { value: "A" }
        }

        ListModel {
            id: servoDropModeModel
            ListElement { checked: true;    name: "All" }
            ListElement { checked: false;   name: "Each 2" }
            ListElement { checked: false;   name: "Each 1" }
            ListElement { checked: false;   name: "Carpet" }
        }

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 10

            GridLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                rows: 2
                columns: servoDropModeRepeater.count
                columnSpacing: 5
                rowSpacing: 5

                Repeater {
                    id: servoDropModeRepeater
                    model: (_activeVehicle && _activeVehicle.servoController)
                           ? _activeVehicle.servoController.servoDropModes
                           : servoDropModeModel

                    delegate: CheckBox {
                        id: modeCheckbox
                        Layout.fillWidth: true
                        Layout.alignment: Qt.AlignHCenter

                        required property var model
                        required property int index

                        checked: (_activeVehicle && _activeVehicle.servoController) ? _activeVehicle.servoController.servoDropModes[index].checked : servoDropModeModel.get(index).checked

                        text: ""

                        onClicked: {
                            console.log("onClicked")
                            if(_activeVehicle && _activeVehicle.servoController) {
                                console.log("onClicked condition passed")
                                for (let i = 0; i < _activeVehicle.servoController.servoDropModes.length; ++i) {
                                    console.log("dropMode" + String(i) + " constraining...")
                                    _activeVehicle.servoController.servoDropModes[i].checked = (i === index)
                                }
                            }
                        }
                    }
                }
                Repeater {
                    model: (_activeVehicle && _activeVehicle.servoController)
                           ? _activeVehicle.servoController.servoDropModes
                           : servoDropModeModel
                    delegate: Text {
                        required property var model
                        required property int index

                        text: (_activeVehicle && _activeVehicle.servoController) ? _activeVehicle.servoController.servoDropModes[index].name : servoDropModeModel.get(index).name
                        font.pointSize: 12
                        color: "white"
                        horizontalAlignment: Text.AlignHCenter
                        Layout.fillWidth: true
                    }
                }
            }

            RowLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                spacing: 5
                Repeater {
                    id: servoButtonsRepeater
                    model: (_activeVehicle && _activeVehicle.servoController)
                           ? _activeVehicle.servoController.servoModel
                           : servoButtonsModel

                    //model: servoButtonsModel

                    Rectangle {
                        required property int index
                        required property var modelData
                        property string displayValue: {
                            if (typeof modelData === 'object' && modelData !== null) {
                                return modelData.name !== undefined ? modelData.name : ""
                            }

                            return modelData
                        }
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        Layout.preferredWidth: 0
                        //width: parent ? (parent.width - (servoButtonsRepeater.count - 1) * parent.spacing) / servoButtonsRepeater.count : 0
                        //height: parent.height
                        //color: {
                        //    if (index === 0) return "lightgreen"
                        //    else if (index === servoButtonsRepeater.count - 1) return "lightblue"
                        //    else return "lightgray"
                        //}
                        //border.color: "black"
                        //border.width: 1
                        //radius: 3

                        Rectangle {
                            anchors.left: parent.left
                            anchors.right: parent.right
                            anchors.bottom: parent.bottom

                            height: {
                                if(_activeVehicle && _activeVehicle.servoController) {
                                    var servo = _activeVehicle.servoController.servoModel[index];
                                    var val = servo.value;
                                    var minV = servo.minValue;
                                    var maxV = servo.maxValue;
                                    var rev = servo.reversed;
                                    console.log("index", index, "minV", minV, "maxV", maxV, "val", val);
                                    console.log("typeof minV: " + typeof(minV))
                                    return servoValueScale(val, minV, maxV, 0, parent.height, rev);
                                }
                                return 0
                            }
                            color: "green"
                            radius: 8
                        }

                        Text {
                            anchors.centerIn: parent
                            text: displayValue
                            font.pointSize: 14
                            font.bold: true
                        }

                        Behavior on opacity { NumberAnimation { duration: 200 } }
                        opacity: 1

                        radius: 8
                    }
                }
            }

            RowLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                spacing: 5
                Button {
                    text: "Drop"
                    onClicked: {
                        if(_activeVehicle && _activeVehicle.servoController) {
                            Qt.callLater(()=>_activeVehicle.servoController.drop())

                        }
                    }
                    Layout.alignment: horizontalCenter
                }

                Button {
                    text: "Close"
                    onClicked: {
                        if(_activeVehicle && _activeVehicle.servoController) {
                            Qt.callLater(()=>_activeVehicle.servoController.close())

                        }
                    }
                    Layout.alignment: horizontalCenter
                }
            }


        }

        z:                      QGroundControl.zOrderWidgets
        opacity: 1//0.5
        color: qgcPal.window
        visible: _activeVehicle !== undefined ? true : false
        radius: 8
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
