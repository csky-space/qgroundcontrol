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
    text:           courseInitialized ? vehicleReturnCourse.toFixed(1) + "°" : "-";
    iconSource:     "qrc:/qmlimages/return.svg"
    enabled:        courseInitialized && _activeVehicle ? true : false

    property var  _activeVehicle:      QGroundControl.multiVehicleManager.activeVehicle
    property var  _paramaterManager:   _activeVehicle ? _activeVehicle.parameterManager : undefined
    property bool courseInitialized:   false
    property real vehicleReturnCourse: 180

    function normalizeTo360(angle) {
        return (angle + 360) % 360
    }

    function normalizeCourse180(course) {
        let c = ((course + 180) % 360 + 360) % 360;
        return c - 180;
    }

    function courseParameterChangedCallback() {
        console.log("course changed callback called")
        var fact = _paramaterManager.getParameter(_activeVehicle.defaultComponentId(), "COMP_RET_YAW");
        vehicleReturnCourse = normalizeTo360(fact.value);
        courseInitialized = true;
    }

    function fetchCourseFromParameters(readyState = true) {
        console.log("fetchCourseFromParameters called. parametersReady:", readyState)
        if (readyState) {
            if (_paramaterManager.parameterExists(_activeVehicle.defaultComponentId(), "COMP_RET_YAW")) {
                var fact = _paramaterManager.getParameter(_activeVehicle.defaultComponentId(), "COMP_RET_YAW");
                vehicleReturnCourse = normalizeTo360(fact.value);
                courseInitialized = true;
                fact.vehicleUpdated.connect(courseParameterChangedCallback);
            }
        }
    }

    function handleVehicleChanged(vehicle) {
        console.log("vehicle changed called")
        if (_activeVehicle) {
            _paramaterManager.parametersReadyChanged.disconnect(fetchCourseFromParameters);
            if (_paramaterManager.parameterExists(_activeVehicle.defaultComponentId(), "COMP_RET_YAW")) {
                var fact = _paramaterManager.getParameter(_activeVehicle.defaultComponentId(), "COMP_RET_YAW");
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
        var multiVehicleManager = QGroundControl.multiVehicleManager;
        multiVehicleManager.activeVehicleChanged.connect(handleVehicleChanged);
        if (multiVehicleManager.activeVehicle) {
            handleVehicleChanged(activeVehicle);
        }    
    }

    dropPanelComponent: Rectangle {
        id:       returnCourse
        height:   instrumentPanel ? instrumentPanel._heightAttComp : 0
        width:    instrumentPanel ? instrumentPanel._heightAttComp * 4 : 0

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

            // var targetPx = returnYaw.targetCourseNormalized * pxPerDegree + stepWidth / 2
            // var realPx = returnYaw.realCourse * pxPerDegree + stepWidth / 2

            var offsetPx = normalizeCourse180(returnYaw.realCourse - returnYaw.currentCourse) * pxPerDegree
            var currentPx = returnYaw.currentCourse * pxPerDegree + stepWidth / 2
            currentMarker.x = centerX + offsetPx;
            compassStrip.x = centerX - (middleOffset + currentPx)

            //currentText.text = isNaN(returnYaw.realCourse) ? "" : returnYaw.realCourse.toFixed(1) + "°"
            currentText.x = currentMarker.x + currentMarker.width/2 - currentText.width/2
        }

        function sendCourseToDrone(course) {
            if (returnCourse.retYawFact) {
                returnCourse.retYawFact.value = course
                let _course = normalizeCourse180(course)
                console.log("set parameter to", _course)
            }
        }

        Timer {
            interval: 200
            running:  true
            repeat:   true

            function _valueChangedCallback(newValue) {
                console.log("course changed: " + newValue + ". returnYaw.isDragging: " + returnYaw.isDragging)
                returnYaw.currentCourse = normalizeTo360(returnCourse.retYawFact.value)
                returnYaw.rawTargetCourse = returnYaw.currentCourse
                returnYaw.Timer.running = true
                returnCourse.updatePositions()
            }

            onTriggered: {
                if (!returnCourse || !_activeVehicle) return

                if (_paramaterManager.parametersReady && !returnCourse.retYawFact) {
                    var fact = _paramaterManager.getParameter(_activeVehicle.defaultComponentId(), "COMP_RET_YAW")
                    if (fact) {
                        returnCourse.retYawFact = fact

                        if (!returnCourse.initialCourseSet) {
                            returnYaw.currentCourse = normalizeTo360(fact.value)
                            returnYaw.rawTargetCourse = returnYaw.currentCourse
                            returnYaw.Timer.running = true
                            returnCourse.initialCourseSet = true
                            returnCourse.updatePositions()
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
            running: true
            repeat: true

            function _valueChangedCallback(newValue) {
                returnYaw.realCourse = normalizeTo360(newValue)
                returnCourse.updatePositions()
                console.log("realCourse: " + returnYaw.realCourse)
            }

            onTriggered: {
                if (!returnCourse || !_activeVehicle || _activeVehicle.heading === undefined) return;
                if (!returnCourse.headingFact) {
                    returnCourse.headingFact = _activeVehicle.heading
                    returnCourse.headingFact.valueChanged.connect(_valueChangedCallback)
                }
            }

            Component.onDestruction: {
                if (returnCourse && returnCourse.headingFact) {
                    returnCourse.headingFact.valueChanged.disconnect(_valueChangedCallback);
                }
            }
        }

        Rectangle {
            id: returnYaw
            anchors.topMargin:  _toolsMargin + parentToolInsets.topEdgeLeftInset
            anchors.left:       toolStrip.left
            anchors.top:        toolStrip.bottom
            height:             instrumentPanel ? instrumentPanel._heightAttComp : 0
            width:              instrumentPanel ? instrumentPanel._heightAttComp * 4 : 0

            radius: 8
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
                id:       animationTimer
                interval: 20
                running: initialCourseSet
                repeat: true
                onTriggered: {
                    var diff = returnYaw.targetCourseNormalized - returnYaw.currentCourse
                    if (diff > 180) diff -= 360
                    if (diff < -180) diff += 360
                    var segmentWidth = compassStrip.segmentWidth
                    var pxPerDegree = segmentWidth / 360
                    var step = diff * pxPerDegree * (animationTimer.interval / wheelDebounceTimer.interval)
                    returnYaw.currentCourse += step
                    returnYaw.currentCourse = (returnYaw.currentCourse % 360 + 360) % 360
                    returnCourse.updatePositions()
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
                        id: dragDebounceTimer
                        interval: 500
                        repeat: false
                        onTriggered: {
                            if (returnYaw.isDragging) {
                                sendCourseToDrone(returnYaw.targetCourseNormalized)
                            }
                        }
                    }

                    Timer {
                        id: wheelDebounceTimer
                        interval: 500
                        repeat: false
                        onTriggered: {
                            sendCourseToDrone(returnYaw.targetCourseNormalized)
                        }
                    }

                    onPressed: {
                        returnYaw.isDragging = true
                        dragArea.dragStartX = mouseX
                        dragArea.dragStartRaw = returnYaw.rawTargetCourse
                        dragDebounceTimer.stop()
                    }

                    onPositionChanged: {
                        if (!returnYaw.isDragging) return
                        var fullRange = compassStrip.width - compassContainer.width
                        if (fullRange <= 0) return
                        var deltaX = mouseX - dragArea.dragStartX
                        var deltaCourse = (deltaX / fullRange) * 360
                        returnYaw.rawTargetCourse = dragArea.dragStartRaw + deltaCourse
                        dragDebounceTimer.restart()
                    }

                    onReleased: {
                        returnYaw.isDragging = false
                        dragDebounceTimer.stop()
                        sendCourseToDrone(returnYaw.targetCourseNormalized)
                    }

                    focus: true
                    Keys.onReleased: { }
                    onWheel: function(wheel) {
                        var delta = wheel.angleDelta.y / 120
                        var courseStep = 5
                        returnYaw.rawTargetCourse = normalizeTo360(returnYaw.rawTargetCourse + delta * courseStep)
                        wheelDebounceTimer.restart()
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

            onCurrentCourseChanged:  returnCourse.updatePositions()
            onRawTargetCourseChanged:  returnCourse.updatePositions()
            Component.onCompleted:  returnCourse.updatePositions()
            onWidthChanged:  returnCourse.updatePositions()
        }
    }
}
