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

        height: instrumentPanel ? instrumentPanel._heightAttComp : 0
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

        height: instrumentPanel ? instrumentPanel._heightAttComp : 0
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
    Rectangle {
        id: dropControl
        anchors.topMargin: _toolsMargin + parentToolInsets.topEdgeLeftInset
        anchors.rightMargin: _toolsMargin + parentToolInsets.topEdgeLeftInset

        anchors.top: toolStrip.bottom
        anchors.left: toolStrip.left
        //anchors.bottom: telemetryPanel.bottom
        //anchors.right: telemetryPanel.left

        height: instrumentPanel ? instrumentPanel._heightAttComp * 2 : 0
        width: instrumentPanel ? instrumentPanel._heightAttComp * 4 : 0

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
        ButtonGroup {
            id: servoGroup
            exclusive: true
        }
        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 10
            spacing: 10

            GridLayout {
                Layout.fillWidth: true
                Layout.preferredHeight: implicitHeight
                columns: servoDropModeRepeater.count
                columnSpacing: 5
                rowSpacing: 5
                Layout.alignment: Qt.AlignHCenter

                Repeater {
                    id: servoDropModeRepeater
                    model: (_activeVehicle && _activeVehicle.servoController)
                           ? _activeVehicle.servoController.servoDropModes
                           : servoDropModeModel

                    delegate: Item {
                        Layout.fillWidth: true
                        //Layout.fillHeight: true
                        implicitHeight: content.implicitHeight
                        opacity: 1

                        //property var model
                        //property int index

                        ColumnLayout {
                            id: content
                            anchors.fill: parent
                            spacing: 5
                            implicitHeight: childrenRect.height

                            RadioButton {
                                id: modeRadio
                                Layout.preferredWidth: 24
                                Layout.alignment: Qt.AlignHCenter

                                //property var model
                                //property int index

                                checked: (_activeVehicle && _activeVehicle.servoController)
                                         ? _activeVehicle.servoController.servoDropModes[index].checked
                                         : servoDropModeModel.get(index).checked

                                ButtonGroup.group: servoGroup

                                text: ""

                                indicator: Rectangle {
                                    implicitWidth: 24
                                    implicitHeight: 24
                                    radius: 14
                                    border.color: modeRadio.down ? "darkgray" : "gray"
                                    Rectangle {
                                        width: 12
                                        height: 12
                                        radius: 14
                                        color: modeRadio.checked ? "#000000" : "transparent"
                                        anchors.centerIn: parent
                                    }
                                }


                                Component.onCompleted: {
                                    if (_activeVehicle && _activeVehicle.servoController) {
                                        checked = _activeVehicle.servoController.servoDropModes[index].checked
                                    } else {
                                        checked = servoDropModeModel.get(index).checked
                                    }
                                }

                                onClicked: {
                                    if (_activeVehicle && _activeVehicle.servoController) {
                                        for (let i = 0; i < _activeVehicle.servoController.servoDropModes.length; ++i) {
                                            _activeVehicle.servoController.servoDropModes[i].checked = (i === index)
                                        }
                                    } else {
                                        for (let i = 0; i < servoDropModeModel.count; ++i) {
                                            servoDropModeModel.get(i).checked = (i === index)
                                        }
                                    }
                                }
                                //onCheckedChanged: {
                                //    if (checked) {
                                //        if (_activeVehicle && _activeVehicle.servoController) {
                                //            for (let i = 0; i < _activeVehicle.servoController.servoDropModes.length; ++i) {
                                //                _activeVehicle.servoController.servoDropModes[i].checked = (i === index);
                                //            }
                                //        } else {
                                //            for (let i = 0; i < servoDropModeModel.count; ++i) {
                                //                servoDropModeModel.get(i).checked = (i === index);
                                //            }
                                //        }
                                //    }
                                //}
                            }

                            Text {
                                Layout.fillWidth: true
                                text: (_activeVehicle && _activeVehicle.servoController)
                                      ? _activeVehicle.servoController.servoDropModes[index].name
                                      : servoDropModeModel.get(index).name
                                font.pointSize: 12
                                color: "white"
                                horizontalAlignment: Text.AlignHCenter
                            }
                        }
                    }
                }
            }

            ColumnLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                spacing: 5
                clip: true
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

                        Rectangle {
                            anchors.left: parent.left
                            anchors.top: parent.top

                            height: parent.height
                            width: {
                                if(_activeVehicle && _activeVehicle.servoController) {
                                    var servo = _activeVehicle.servoController.servoModel[index];
                                    var val = servo.value;
                                    var minV = servo.minValue;
                                    var maxV = servo.maxValue;
                                    var rev = servo.reversed;
                                    return servoValueScale(val, minV, maxV, 0, parent.width, rev);
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

                        radius: 8
                    }
                }
            }

            RowLayout {
                Layout.fillWidth: true
                Layout.preferredHeight: implicitHeight
                spacing: 5
                Button {
                    opacity: 1
                    id: dropBtn
                    text: "Drop"

                    Layout.alignment: Qt.AlignHCenter
                    Layout.preferredWidth: 120
                    Layout.preferredHeight: 40

                    background: Rectangle {
                        anchors.fill: parent
                        radius: 12

                        color: dropBtn.down
                               ? "#2c2c2c"
                               : dropBtn.hovered
                                 ? "#3a3a3a"
                                 : "#444444"

                        border.color: "#555"
                        border.width: 1
                    }

                    contentItem: Image {
                        anchors.centerIn: parent
                        width: 24
                        height: 24

                        source: "qrc:/qmlimages/drop.svg"
                        fillMode: Image.PreserveAspectFit
                    }

                    onClicked: {
                        if(_activeVehicle && _activeVehicle.servoController) {
                            _activeVehicle.servoController.drop()
                        }
                    }
                }

                Button {
                    opacity: 1
                    id: closeBtn
                    text: "Close"

                    Layout.alignment: Qt.AlignHCenter
                    Layout.preferredWidth: 120
                    Layout.preferredHeight: 40

                    background: Rectangle {
                        anchors.fill: parent
                        radius: 12

                        color: closeBtn.down
                               ? "#2c2c2c"
                               : closeBtn.hovered
                                 ? "#3a3a3a"
                                 : "#444444"

                        border.color: "#555"
                        border.width: 1
                    }

                    contentItem: Image {
                        anchors.centerIn: parent
                        width: 24
                        height: 24

                        source: "qrc:/qmlimages/close.svg"
                        fillMode: Image.PreserveAspectFit
                    }
                    onClicked: {
                        if(_activeVehicle && _activeVehicle.servoController) {
                            _activeVehicle.servoController.close()
                        }
                    }
                }
            }
        }

        z:                      QGroundControl.zOrderWidgets

        color: Qt.rgba(qgcPal.window.r,
                       qgcPal.window.g,
                       qgcPal.window.b,
                       0.6)
        visible: _activeVehicle !== undefined ? true : false
        radius: 8
    }

    property Fact retYawFact: null
    property bool initialCourseSet: false
    property Fact headingFact: null

    function updatePositions() {
        var containerWidth = compassContainer.width
            var segmentWidth = compassStrip.segmentWidth
            var stepWidth = compassStrip.stepWidth
            if (segmentWidth <= 0) return

            var centerX = containerWidth / 2
            var pxPerDegree = segmentWidth / 360
            var middleOffset = segmentWidth



            var targetPx = returnYaw.targetCourseNormalized * pxPerDegree + stepWidth / 2
            var currentPx = returnYaw.currentCourse * pxPerDegree + stepWidth / 2
            var realPx = returnYaw.realCourse * pxPerDegree + stepWidth / 2
            currentMarker.x = centerX + (realPx - currentPx)
            compassStrip.x = centerX - (middleOffset + currentPx)

            //currentText.text = isNaN(returnYaw.realCourse) ? "" : returnYaw.realCourse.toFixed(1) + "°"
            currentText.x = currentMarker.x + currentMarker.width/2 - currentText.width/2

    }

    Timer {
        interval: 200
        running: true
        repeat: true
        onTriggered: {
            if (!_activeVehicle) return
            var pm = _activeVehicle.parameterManager

            if (pm.parametersReady && !_root.retYawFact) {
                var fact = pm.getParameter(_activeVehicle.defaultComponentId(), "COMP_RET_YAW")
                if (fact) {
                    _root.retYawFact = fact

                    if (!_root.initialCourseSet) {
                        returnYaw.currentCourse = normalizeTo360(fact.value)
                        returnYaw.rawTargetCourse = returnYaw.currentCourse
                        returnYaw.Timer.running = true
                        _root.initialCourseSet = true
                        _root.updatePositions()
                    }

                    fact.valueChanged.connect(function(newValue) {
                        console.log("course changed: " + newValue + ". returnYaw.isDragging: " + returnYaw.isDragging)
                        returnYaw.currentCourse = normalizeTo360(fact.value)
                        returnYaw.rawTargetCourse = returnYaw.currentCourse
                        returnYaw.Timer.running = true
                        _root.updatePositions()
                    })
                }
            }
        }
    }

    Timer {
        interval: 200
        running: true
        repeat: true
        onTriggered: {
            if (!_activeVehicle || _activeVehicle.heading === undefined) return;
            if (!_root.headingFact) {
                _root.headingFact = _activeVehicle.heading
                _root.headingFact.valueChanged.connect(function(newValue) {
                    returnYaw.realCourse = normalizeTo360(newValue)
                    _root.updatePositions()
                    console.log("realCourse: " + returnYaw.realCourse)
                })
            }
        }
    }

    function normalizeTo360(angle) {
        return (angle + 360) % 360
    }
    function normalizeCourse180(course) {
        let c = ((course + 180) % 360 + 360) % 360;
        return c - 180;
    }

    function sendCourseToDrone(course) {
        if (_root.retYawFact) {
            let _course = normalizeCourse180(course)
            console.log("set parameter to", _course)
            _root.retYawFact.value = _course
            //_root.localTargetCourse = course
        }
    }

    Rectangle {
        id: returnYaw
        anchors.topMargin: _toolsMargin + parentToolInsets.topEdgeLeftInset
        anchors.top: dropControl.bottom
        anchors.left: dropControl.left
        anchors.right: dropControl.right

        radius: 8

        height: dropControl.height / 2
        color: Qt.rgba(
            qgcPal.window.r,
            qgcPal.window.g,
            qgcPal.window.b,
            0.5
        )

        property real rawTargetCourse: 0
        readonly property real targetCourseNormalized: ((rawTargetCourse % 360) + 360) % 360
        property real currentCourse: 0
        property real realCourse: 0
        property bool isDragging: false

        Timer {
            interval: 20
            running: initialCourseSet
            repeat: true
            onTriggered: {
                var diff = returnYaw.targetCourseNormalized - returnYaw.currentCourse
                if (diff > 180) diff -= 360
                if (diff < -180) diff += 360
                var step = diff * 0.05
                if (Math.abs(step) < 0.5) step = diff
                returnYaw.currentCourse += step
                returnYaw.currentCourse = (returnYaw.currentCourse % 360 + 360) % 360

                _root.updatePositions()
            }
        }

        Rectangle {
            id: compassContainer
            width: parent.width
            height: parent.height
            anchors.top: parent.top
            color: Qt.rgba(
                qgcPal.window.r,
                qgcPal.window.g,
                qgcPal.window.b,
                1.0
            )

            border.color: qgcPal.windowShade
            border.width: 1
            radius: 8

            clip: true

            Item {
                id: compassStrip

                property int stepWidth: 30
                property int segmentWidth: stepWidth * 72

                width: segmentWidth * 3
                height: parent.height

                Row {
                    anchors.fill: parent
                    //anchors.verticalCenter: parent.verticalCenter
                    spacing: 0

                    Repeater {
                        model: 3
                        delegate: Item {
                            width: compassStrip.segmentWidth
                            height: parent.height

                            x: index * compassStrip.segmentWidth

                            Repeater {
                                model: 72

                                delegate: Item {
                                    width: compassStrip.stepWidth
                                    height: parent.height

                                    x: index * compassStrip.stepWidth
                                    property int angle: index * 5
                                    property bool is90: angle % 90 === 0
                                    property bool is10: angle % 10 === 0 && !is90

                                    Rectangle {
                                        width: 2
                                        height: is90 ? 16 : (is10 ? 12 : 6)
                                        color: is90 ? qgcPal.buttonHighlight : qgcPal.text
                                        z: 2
                                        anchors.horizontalCenter: parent.horizontalCenter
                                        anchors.top: parent.top
                                        anchors.topMargin: 4
                                    }

                                    Rectangle {
                                        width: 2
                                        height: is90 ? 16 : (is10 ? 12 : 6)
                                        color: is90 ? qgcPal.buttonHighlight : qgcPal.text
                                        z: 2
                                        anchors.horizontalCenter: parent.horizontalCenter
                                        anchors.bottom: parent.bottom
                                        anchors.bottomMargin: 4
                                    }

                                    Label {
                                        visible: angle % 10 === 0
                                        anchors.horizontalCenter: parent.horizontalCenter
                                        anchors.top: parent.top
                                        anchors.topMargin: 20
                                        z: 3

                                        text: {
                                            if (angle === 0) return "N"
                                            if (angle === 90) return "E"
                                            if (angle === 180) return "S"
                                            if (angle === 270) return "W"
                                            return angle + "°"
                                        }

                                        color: is90 ? qgcPal.buttonHighlight : qgcPal.text
                                        font.pixelSize: 12
                                    }
                                }
                            }
                        }
                    }
                }
            }

            Rectangle {
                id: targetMarker
                width: 2
                height: parent.height
                anchors.centerIn: parent
                color: "lime"
                z: 10
            }

            Rectangle {
                id: currentMarker
                width: 2
                height: parent.height
                color: "orange"
                z: 5

                visible: (x >= 0 && x <= compassContainer.width)
                //x: returnYaw.width / 2
            }

            // Text {
            //     id: targetText
            //     text: returnYaw.targetCourseNormalized.toFixed(0) + "°"
            //     color: "lime"
            //     font.pixelSize: 12
            //     font.bold: true

            //     x: targetMarker.x + targetMarker.width / 2 - width / 2
            //     y: targetMarker.y + targetMarker.height + 2
            //     z: 20
            // }

            MouseArea {
                id: dragArea
                anchors.fill: parent
                property real dragStartX: 0
                property real dragStartRaw: 0

                Timer {
                    id: debounceTimer
                    interval: 150
                    repeat: false
                    onTriggered: {
                        if (!returnYaw.isDragging) return
                        sendCourseToDrone(returnYaw.targetCourseNormalized)
                    }
                }

                onPressed: {
                    returnYaw.isDragging = true
                    dragArea.dragStartX = mouseX
                    dragArea.dragStartRaw = returnYaw.rawTargetCourse
                    debounceTimer.stop()
                }
                onPositionChanged: {
                    if (!returnYaw.isDragging) return
                    var fullRange = compassStrip.width - compassContainer.width
                    if (fullRange <= 0) return
                    var deltaX = mouseX - dragArea.dragStartX
                    var deltaCourse = (deltaX / fullRange) * 360

                    returnYaw.rawTargetCourse = dragArea.dragStartRaw + deltaCourse
                    debounceTimer.restart()
                }
                onReleased: {
                    returnYaw.isDragging = false
                    debounceTimer.stop()
                    sendCourseToDrone(returnYaw.targetCourseNormalized)
                }
                focus: true
                Keys.onReleased: { }
                onWheel: function(wheel) {
                    var delta = wheel.angleDelta.y / 120
                    var courseStep = 5
                    returnYaw.rawTargetCourse = normalizeTo360(returnYaw.rawTargetCourse + delta * courseStep)
                    sendCourseToDrone(returnYaw.targetCourseNormalized)
                    wheel.accepted = true
                }
            }
        }
        Item {
            id: overlayLayer
            anchors.fill: parent
            z: 999

            Text {
                id: targetText
                text: returnYaw.targetCourseNormalized.toFixed(1) + "°"
                color: "lime"
                font.pixelSize: 12
                font.bold: true

                x: targetMarker.x + targetMarker.width / 2 - width / 2
                y: targetMarker.height - height - 2   // 🔥 внутри контейнера

                z: 20
            }

            Text {
                id: currentText
                text: (returnYaw.realCourse) ? returnYaw.realCourse.toFixed(1) : 0
                color: "orange"
                font.pixelSize: 12
                font.bold: true

                //x: currentMarker.x + currentMarker.width/2 - width/2
                y: 2
                z: 20
                visible: currentMarker.visible
            }
        }

        onCurrentCourseChanged:  _root.updatePositions()
        onRawTargetCourseChanged:  _root.updatePositions()
        Component.onCompleted:  _root.updatePositions()
        onWidthChanged:  _root.updatePositions()
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
