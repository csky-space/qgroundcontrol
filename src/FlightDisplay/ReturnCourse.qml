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

ToolStripAction {
    id:             returnCourseIcon
    text:           courseInitialized ? vehicleReturnCourse.toFixed(1) + "°" : "-"
    iconSource:     "qrc:/qmlimages/return.svg"
    enabled:        courseInitialized && _activeVehicle ? true : false

    property var  _activeVehicle:      QGroundControl.multiVehicleManager.activeVehicle
    property var  _paramaterManager:   _activeVehicle ? _activeVehicle.parameterManager : undefined
    property bool courseInitialized:   false
    property real vehicleReturnCourse: 180

    function normalizeTo360(angle) { return (angle + 360) % 360 }

    function normalizeCourse180(course) {
        let c = ((course + 180) % 360 + 360) % 360
        return c - 180
    }

    function courseParameterChangedCallback() {
        const fact = _paramaterManager.getParameter(_activeVehicle.defaultComponentId(), "COMP_RET_YAW");
        vehicleReturnCourse = normalizeTo360(fact.value);
        courseInitialized = true;
    }

    function fetchCourseFromParameters(readyState = true) {
        if (readyState) {
            if (_paramaterManager.parameterExists(_activeVehicle.defaultComponentId(), "COMP_RET_YAW")) {
                const fact = _paramaterManager.getParameter(_activeVehicle.defaultComponentId(), "COMP_RET_YAW");
                vehicleReturnCourse = normalizeTo360(fact.value);
                courseInitialized = true;
                fact.vehicleUpdated.connect(courseParameterChangedCallback);
            }
        }
    }

    function handleVehicleChanged(vehicle) {
        if (_activeVehicle) {
            _paramaterManager.parametersReadyChanged.disconnect(fetchCourseFromParameters);
            if (_paramaterManager.parameterExists(_activeVehicle.defaultComponentId(), "COMP_RET_YAW")) {
                const fact = _paramaterManager.getParameter(_activeVehicle.defaultComponentId(), "COMP_RET_YAW");
                fact.vehicleUpdated.disconnect(courseParameterChangedCallback);
            }
        }
        _activeVehicle = vehicle;
        if (_activeVehicle) {
            _paramaterManager.parametersReadyChanged.connect(fetchCourseFromParameters);
            fetchCourseFromParameters();
        } else {
            courseInitialized = false;
        }
    }

    Component.onCompleted: {
        const mvm = QGroundControl.multiVehicleManager;
        mvm.activeVehicleChanged.connect(handleVehicleChanged);
        if (mvm.activeVehicle) handleVehicleChanged(activeVehicle);
    }

    dropPanelComponent: ColumnLayout {
        id: returnCourse
        spacing: 0

        property Fact retYawFact: null
        property bool initialCourseSet: false
        property Fact headingFact: null

        function updatePositions() {
            if (!compassContainer || !compassCanvas || !currentMarker) {
                return;
            }
            const centerX  = compassContainer.width / 2;
            const pxPerDeg = compassContainer.pxPerDegree;
            if (pxPerDeg <= 0) return;

            const offset = normalizeCourse180(returnYaw.realCourse - returnYaw.currentCourse);
            currentMarker.x = centerX + offset * pxPerDeg - currentMarker.width / 2;

            compassCanvas.requestPaint();

            if (currentText) {
                currentText.x = currentMarker.x + currentMarker.width / 2 - currentText.width / 2;
            }
        }

        function sendCourseToDrone(course) {
            if (returnCourse.retYawFact) {
                returnCourse.retYawFact.value = course;
                console.log("set parameter to", normalizeCourse180(course));
            }
        }

        Item {
            Layout.fillWidth:  true
            height:            16
        }

        Rectangle {
            id: returnYaw

            Layout.preferredWidth:  compassContainer.implicitWidth
            Layout.preferredHeight: compassContainer.implicitHeight
            Layout.alignment:       Qt.AlignLeft
            radius: 4
            color: Qt.rgba(qgcPal.window.r, qgcPal.window.g, qgcPal.window.b, 0.5)

            property real rawTargetCourse: 0
            readonly property real targetCourseNormalized: ((rawTargetCourse % 360) + 360) % 360
            property real currentCourse: 0
            property real realCourse: 0
            property bool isDragging: false

            Timer {
                id:       animationTimer
                interval: 20
                running:  returnCourse.initialCourseSet
                repeat:   true
                onTriggered: {
                    let diff = returnYaw.targetCourseNormalized - returnYaw.currentCourse;
                    if (diff >  180) diff -= 360;
                    if (diff < -180) diff += 360;

                    if (Math.abs(diff) > 179.5) {
                        diff = diff > 0 ? 179.5 : -179.5;
                    }

                    const step = diff * 6 * (animationTimer.interval / wheelDebounceTimer.interval);
                    returnYaw.currentCourse = normalizeTo360(returnYaw.currentCourse + step);
                    returnCourse.updatePositions();
                }
            }

            Rectangle {
                id: compassContainer
                x: 0
                y: 0
                width:  returnYaw.width
                height: returnYaw.height
                implicitWidth:  360
                implicitHeight: 80
                color: Qt.rgba(qgcPal.window.r, qgcPal.window.g, qgcPal.window.b, 1.0)
                border.color: qgcPal.windowShade
                border.width: 1
                radius: 4
                clip: true

                property real pxPerDegree: 6

                Canvas {
                    id: compassCanvas
                    x: 0
                    y: 0
                    width:  parent.width
                    height: parent.height

                    antialiasing: true
                    renderStrategy: Canvas.Cooperative

                    property color tickColor:      qgcPal.text
                    property color majorTickColor: qgcPal.buttonHighlight

                    onTickColorChanged:      requestPaint()
                    onMajorTickColorChanged: requestPaint()

                    Connections {
                        target: returnYaw
                        function onCurrentCourseChanged() {
                            compassCanvas.requestPaint()
                        }
                    }

                    onWidthChanged:   requestPaint()
                    onHeightChanged:  requestPaint()
                    onVisibleChanged: if (visible) requestPaint()
                    Component.onCompleted: requestPaint()

                    onPaint: {
                        const ctx = getContext("2d");
                        ctx.reset();
                        ctx.clearRect(0, 0, width, height);

                        const centerX  = width / 2;
                        const course   = returnYaw.currentCourse;
                        const pxPerDeg = compassContainer.pxPerDegree;
                        if (pxPerDeg <= 0 || height <= 0){
                            return;
                        }

                        const halfRangeDeg = (width / 2) / pxPerDeg + 10;
                        const startDeg = Math.floor((course - halfRangeDeg) / 5) * 5;
                        const endDeg   = Math.ceil ((course + halfRangeDeg) / 5) * 5;

                        ctx.textAlign    = "center";
                        ctx.textBaseline = "top";
                        ctx.font = "12px sans-serif";

                        for (let deg = startDeg; deg <= endDeg; deg += 5) {
                            const x = centerX + (deg - course) * pxPerDeg;
                            if (x < -30 || x > width + 30) continue;

                            const normDeg = ((deg % 360) + 360) % 360;
                            const is90 = (normDeg % 90) === 0;
                            const is10 = (normDeg % 10) === 0;
                            const tickHeight = is90 ? 16 : (is10 ? 12 : 6);

                            ctx.fillStyle = is90 ? majorTickColor : tickColor;
                            ctx.fillRect(x - 1, 4, 2, tickHeight);
                            ctx.fillRect(x - 1, height - 4 - tickHeight, 2, tickHeight);

                            if (normDeg % 10 === 0) {
                                let label;
                                if (normDeg === 0)        label = "N";
                                else if (normDeg === 90)  label = "E";
                                else if (normDeg === 180) label = "S";
                                else if (normDeg === 270) label = "W";
                                else                      label = normDeg + "°";

                                ctx.fillStyle = is90 ? majorTickColor : tickColor;
                                ctx.fillText(label, x, 20);
                            }
                        }
                    }
                }

                Rectangle {
                    id: targetMarker
                    x: (compassContainer.width - width) / 2
                    y: 0
                    width: 2
                    height: compassContainer.height
                    color: "lime"
                    z: 10
                }

                Rectangle {
                    id: currentMarker
                    y: 0
                    width: 2
                    height: compassContainer.height
                    color: "orange"
                    z: 5
                    visible: (x >= 0 && x <= compassContainer.width)
                }

                MouseArea {
                    id: dragArea
                    x: 0
                    y: 0
                    width:  parent.width
                    height: parent.height
                    focus: true

                    property real dragStartX: 0
                    property real dragStartRaw: 0

                    Timer {
                        id: dragDebounceTimer
                        interval: 500
                        repeat: false
                        onTriggered: {
                            if (returnYaw.isDragging) {
                                returnCourse.sendCourseToDrone(returnYaw.targetCourseNormalized);
                            }
                        }
                    }

                    Timer {
                        id: wheelDebounceTimer
                        interval: 500
                        repeat: false
                        onTriggered: returnCourse.sendCourseToDrone(returnYaw.targetCourseNormalized)
                    }

                    onPressed: {
                        returnYaw.isDragging   = true;
                        dragArea.dragStartX    = mouseX;
                        dragArea.dragStartRaw  = normalizeTo360(returnYaw.rawTargetCourse);
                        dragDebounceTimer.stop()
                    }

                    onPositionChanged: {
                        if (!returnYaw.isDragging) {
                            return
                        }

                        const fullRange = width
                        if (fullRange <= 0) {
                            return;
                        }

                        const deltaX = mouseX - dragArea.dragStartX
                        const sensitivity = 0.15;
                        returnYaw.rawTargetCourse = normalizeTo360(
                            dragArea.dragStartRaw + sensitivity * (deltaX / fullRange) * 360
                        );

                        dragDebounceTimer.restart();
                    }

                    onReleased: {
                        returnYaw.isDragging = false;
                        dragDebounceTimer.stop();
                        returnCourse.sendCourseToDrone(returnYaw.targetCourseNormalized);
                    }

                    Keys.onReleased: { }
                    onWheel: function(wheel) {
                        const delta = wheel.angleDelta.y / 120;
                        returnYaw.rawTargetCourse = normalizeTo360(returnYaw.rawTargetCourse + delta * 5);
                        wheelDebounceTimer.restart();
                        wheel.accepted = true;
                    }
                }
            }

            Item {
                id: overlayLayer
                x: 0
                y: 0
                width:  returnYaw.width
                height: returnYaw.height
                z: 999

                Text {
                    id: targetText
                    text: returnYaw.targetCourseNormalized.toFixed(1) + "°"
                    color: "lime"
                    font.pixelSize: 12
                    font.bold: true
                    x: targetMarker.x + targetMarker.width / 2 - width / 2
                    y: targetMarker.height + 2
                    z: 20
                }

                Text {
                    id: currentText
                    text: returnYaw.realCourse ? returnYaw.realCourse.toFixed(1) : 0
                    color: "orange"
                    font.pixelSize: 12
                    font.bold: true
                    y: -height - 2
                    z: 20
                    visible: currentMarker.visible
                }
            }

            onCurrentCourseChanged:   returnCourse.updatePositions()
            onRawTargetCourseChanged: returnCourse.updatePositions()
            Component.onCompleted:    returnCourse.updatePositions()
            onWidthChanged:           returnCourse.updatePositions()
        }

        Item {
            Layout.fillWidth:  true
            height:            20
        }

        RowLayout {
            RowLayout {
                QGCLabel { text: "Target:" }
                QGCTextField {
                    id: targetField
                    text: returnYaw.targetCourseNormalized.toFixed(1)
                    inputMethodHints: Qt.ImhFormattedNumbersOnly
                    Layout.preferredWidth: 80

                    onEditingFinished: {
                        const v = parseFloat(targetField.text.replace(",", "."));
                        if (!isNaN(v)) {
                            returnYaw.rawTargetCourse = v;
                            returnCourse.sendCourseToDrone(returnYaw.targetCourseNormalized);
                        }
                        targetField.text = returnYaw.targetCourseNormalized.toFixed(1);
                    }

                    Connections {
                        target: returnYaw
                        function onRawTargetCourseChanged() {
                            targetField.text = returnYaw.targetCourseNormalized.toFixed(1);
                        }
                    }
                }
            }

            Item { Layout.fillWidth: true }

            RowLayout {
                QGCLabel { text: "Current:" }
                QGCTextField {
                    text: returnYaw.realCourse.toFixed(1) + "°"
                    enabled: false
                }
            }
        }

        Timer {
            interval: 200
            running:  true
            repeat:   true

            function _valueChangedCallback(newValue) {
                returnYaw.currentCourse    = normalizeTo360(returnCourse.retYawFact.value);
                returnYaw.rawTargetCourse  = returnYaw.currentCourse;
                returnCourse.updatePositions();
            }

            onTriggered: {
                if (!returnCourse || !_activeVehicle) {
                    return;
                }
                if (_paramaterManager.parametersReady && !returnCourse.retYawFact) {
                    const fact = _paramaterManager.getParameter(_activeVehicle.defaultComponentId(), "COMP_RET_YAW");
                    if (fact) {
                        returnCourse.retYawFact = fact;
                        if (!returnCourse.initialCourseSet) {
                            returnYaw.currentCourse   = normalizeTo360(fact.value);
                            returnYaw.rawTargetCourse = returnYaw.currentCourse;
                            returnCourse.initialCourseSet = true;
                            returnCourse.updatePositions();
                        }
                        fact.valueChanged.connect(_valueChangedCallback);
                    }
                }
            }

            Component.onDestruction: {
                if (returnCourse.retYawFact) {
                    returnCourse.retYawFact.valueChanged.disconnect(_valueChangedCallback);
                }
            }
        }

        Timer {
            interval: 200
            running:  true
            repeat:   true

            function _valueChangedCallback(newValue) {
                returnYaw.realCourse = normalizeTo360(newValue);
                returnCourse.updatePositions();
            }

            onTriggered: {
                if (!returnCourse || !_activeVehicle || _activeVehicle.heading === undefined) {
                    return;
                }
                if (!returnCourse.headingFact) {
                    returnCourse.headingFact = _activeVehicle.heading;
                    returnCourse.headingFact.valueChanged.connect(_valueChangedCallback);
                    _valueChangedCallback(returnCourse.headingFact.value);
                }
            }

            Component.onDestruction: {
                if (returnCourse && returnCourse.headingFact) {
                    returnCourse.headingFact.valueChanged.disconnect(_valueChangedCallback);
                }
            }
        }
    }
}
