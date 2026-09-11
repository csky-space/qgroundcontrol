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
import QtQuick.Shapes           1.12

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
    id:         returnCourseIcon
    text:       "Antenna";
    iconSource: "qrc:/qmlimages/icon-antenna.svg"
    enabled:    true

    dropPanelComponent: Rectangle {
        id:     antennaDropPanel
        color:  "#00000000"
        radius: 10

        property int dialSize:     180;
        property int targetAngle:  0;
        property int currentAngle: QGroundControl.antennaController ? QGroundControl.antennaController.currentAngle : 0;
        property int minAngle:     QGroundControl.antennaController ? QGroundControl.antennaController.minAngle : -180;
        property int maxAngle:     QGroundControl.antennaController ? QGroundControl.antennaController.maxAngle : 180;

        width:  mainLayout.implicitWidth
        height: mainLayout.implicitHeight

        function normalizeTo360(angle) {
            return (angle + 360) % 360;
        }

        function normalizeTo180(angle) {
            return (((angle + 180) % 360 + 360) % 360) - 180;
        }

        ColumnLayout {
            id:           mainLayout
            anchors.fill: parent
            spacing:      10

            Rectangle {
                Layout.preferredWidth:  antennaDropPanel.dialSize
                Layout.preferredHeight: antennaDropPanel.dialSize
                Layout.alignment:       Qt.AlignHCenter
                color:                  "#00000000"

                Canvas {
                    id:           dirCanvas
                    anchors.fill: parent

                    onPaint: {
                        const ctx = getContext("2d");
                        const centerX = width / 2;
                        const centerY = height / 2;
                        const radius = (width - 2) / 2;

                        ctx.reset();

                        ctx.lineWidth = 1;
                        ctx.strokeStyle = "#ffffff";
                        ctx.fillStyle = "#80000000";

                        if ((antennaDropPanel.maxAngle - antennaDropPanel.minAngle) < 360) {
                            ctx.beginPath();
                            ctx.moveTo(centerX, centerY);
                            if (antennaDropPanel.maxAngle == antennaDropPanel.minAngle) {
                                ctx.arc(centerX, centerY, radius, 0, Math.PI * 2, true);
                            }
                            else {
                                const minAngleRadians = (antennaDropPanel.minAngle - 90) * Math.PI / 180;
                                const maxAngleRadians = (antennaDropPanel.maxAngle - 90) * Math.PI / 180;
                                ctx.arc(centerX, centerY, radius, minAngleRadians, maxAngleRadians, true);
                            }
                            ctx.closePath();
                            ctx.fill();
                        }

                        ctx.beginPath();
                        ctx.arc(centerX, centerY, radius, 0, 2 * Math.PI, false);
                        ctx.closePath();
                        ctx.stroke();

                        for (let deg = 0; deg < 360; deg += 10) {
                            const rad = deg * Math.PI / 180;
                            const cRad = Math.cos(rad);
                            const sRad = Math.sin(rad);

                            let lineWidth = 1;
                            let tickLength = radius * 0.075;
                            if (deg % 30 === 0) {
                                lineWidth = 2;
                                tickLength = radius * 0.15;
                            }
                            const innerRadius = radius - tickLength;

                            const outerX = centerX + radius * sRad;
                            const outerY = centerY - radius * cRad;

                            const innerX = centerX + innerRadius * sRad;
                            const innerY = centerY - innerRadius * cRad;

                            ctx.lineWidth = lineWidth;

                            ctx.beginPath();
                            ctx.moveTo(outerX, outerY);
                            ctx.lineTo(innerX, innerY);
                            ctx.stroke();
                        }

                        const labelAngles = [0, 30, 60, 90, 120, 150, -30, -60, -90, -120, -150, 180];
                        const labelOffset = radius * 0.25;
                        ctx.fillStyle = "#ffffff";
                        ctx.font = "10px sans-serif";
                        ctx.textAlign = "center";
                        ctx.textBaseline = "middle";

                        for (let i = 0; i < labelAngles.length; i++) {
                            const deg = labelAngles[i];
                            const rad = deg * Math.PI / 180;
                            const x = centerX + (radius - labelOffset) * Math.sin(rad);
                            const y = centerY - (radius - labelOffset) * Math.cos(rad);
                            let label = deg.toString();
                            if (deg === 180) label = "±180";
                            ctx.fillText(label, x, y);
                        }
                    }

                    function _requestRedraw() {
                        dirCanvas.requestPaint();
                        console.log("redraw", antennaDropPanel.minAngle, antennaDropPanel.maxAngle)
                    }

                    Connections {
                        target:  QGroundControl.antennaController ? QGroundControl.antennaController : null
                        onMinAngleChanged: {
                            dirCanvas._requestRedraw();
                        }
                    }

                    Connections {
                        target:  QGroundControl.antennaController ? QGroundControl.antennaController : null
                        onMaxAngleChanged: {
                            dirCanvas._requestRedraw();
                        }
                    }
                }

                Shape {
                    id:               currentDirMark
                    width:            size
                    height:           size
                    anchors.centerIn: parent

                    rotation: antennaDropPanel.currentAngle

                    property real size:                antennaDropPanel.dialSize * 0.075
                    property real currentAngleRadians: antennaDropPanel.currentAngle * (Math.PI/180.0)
                    property real marksRadius:         antennaDropPanel.dialSize / 2.25

                    ShapePath {
                        strokeColor: "#80ff0000"
                        strokeWidth: 1
                        fillColor:   "#80ff0000"

                        startX: currentDirMark.width / 2
                        startY: 0

                        PathLine { x: currentDirMark.width; y: currentDirMark.height }
                        PathLine { x: 0; y: currentDirMark.height }
                        PathLine { x: currentDirMark.width / 2; y: 0 }
                    }

                    transform: Translate {
                        x: currentDirMark.marksRadius * Math.sin(currentDirMark.currentAngleRadians)
                        y: -currentDirMark.marksRadius * Math.cos(currentDirMark.currentAngleRadians)
                    }
                }

                Shape {
                    id:               dirMark
                    width:            size
                    height:           size
                    anchors.centerIn: parent

                    rotation: antennaDropPanel.targetAngle

                    property real size:               antennaDropPanel.dialSize * 0.075
                    property real targetAngleRadians: antennaDropPanel.targetAngle * (Math.PI/180.0)
                    property real marksRadius:        antennaDropPanel.dialSize / 2.25

                    ShapePath {
                        strokeColor: "#8000ff00"
                        strokeWidth: 1
                        fillColor:   "#8000ff00"

                        startX: dirMark.width / 2
                        startY: 0

                        PathLine { x: dirMark.width; y: dirMark.height }
                        PathLine { x: 0; y: dirMark.height }
                        PathLine { x: dirMark.width / 2; y: 0 }
                    }

                    transform: Translate {
                        x: dirMark.marksRadius * Math.sin(dirMark.targetAngleRadians)
                        y: -dirMark.marksRadius * Math.cos(dirMark.targetAngleRadians)
                    }
                }

                MouseArea {
                    id:           dialMouseArea
                    anchors.fill: parent

                    function setAngleFromMousePosition(x, y) {
                        var offsetX = x - (width / 2);
                        var offsetY = (height / 2) - y;
                        var r = Math.sqrt(offsetX * offsetX + offsetY * offsetY);

                        if (r < 10 || r > width / 2) {
                            return;
                        }

                        var theta = Math.atan2(offsetY, offsetX) * (180 / Math.PI);
                        var normalizedAngle = -antennaDropPanel.normalizeTo180(theta - 90);

                        normalizedAngle = Math.min(Math.max(Math.round(normalizedAngle), antennaDropPanel.minAngle), antennaDropPanel.maxAngle);

                        antennaDropPanel.targetAngle = normalizedAngle;

                        angleInput.text = normalizedAngle.toFixed(0);
                    }

                    onPressed: {
                        angleInput.focus = false;
                    }

                    onPositionChanged: (mouse) => {
                        setAngleFromMousePosition(mouse.x, mouse.y);
                    }

                    onReleased: (mouse) => {
                        setAngleFromMousePosition(mouse.x, mouse.y);
                    }
                }


                ColumnLayout {
                    anchors.centerIn: parent

                    RowLayout {
                        Shape {
                            id:               legendTargetMark
                            width:            antennaDropPanel.dialSize * 0.025
                            height:           antennaDropPanel.dialSize * 0.025
                            Layout.alignment: Qt.AlignVCenter

                            ShapePath {
                                strokeColor: "#8000ff00"
                                strokeWidth: 1
                                fillColor:   "#8000ff00"

                                startX: legendTargetMark.width / 2
                                startY: 0

                                PathLine { x: legendTargetMark.width; y: legendTargetMark.height }
                                PathLine { x: 0; y: legendTargetMark.height }
                                PathLine { x: legendTargetMark.width / 2; y: 0 }
                            }
                        }
                        QGCLabel {
                            id:               targetLabel
                            text:             "Traget"
                            color:            qgcPal.text
                            font.pointSize:   ScreenTools.smallFontPointSize
                            Layout.alignment: Qt.AlignVCenter
                        }
                    }
                    QGCTextField {
                        id:                    angleInput
                        text:                  antennaDropPanel.targetAngle.toFixed(0)
                        unitsLabel:            "°"
                        Layout.preferredWidth: 50

                        onEditingFinished: {
                            let value = parseFloat(text) ?? 0.0;
                            if (isNaN(value)) value = 0.0;
                            value = Math.min(Math.max(Math.round(value), antennaDropPanel.minAngle), antennaDropPanel.maxAngle);
                            text = value
                            antennaDropPanel.targetAngle = value;
                        }
                    }

                    RowLayout {
                        Shape {
                            id:               legendLastSendMark
                            width:            antennaDropPanel.dialSize * 0.025
                            height:           antennaDropPanel.dialSize * 0.025
                            Layout.alignment: Qt.AlignVCenter

                            ShapePath {
                                strokeColor: "#80ff0000"
                                strokeWidth: 1
                                fillColor:   "#80ff0000"

                                startX: legendLastSendMark.width / 2
                                startY: 0

                                PathLine { x: legendLastSendMark.width; y: legendLastSendMark.height }
                                PathLine { x: 0; y: legendLastSendMark.height }
                                PathLine { x: legendLastSendMark.width / 2; y: 0 }
                            }
                        }
                        QGCLabel {
                            id:               lastSentLabel
                            text:             "Last sent"
                            color:            qgcPal.text
                            font.pointSize:   ScreenTools.smallFontPointSize
                            Layout.alignment: Qt.AlignVCenter
                        }
                    }
                    QGCTextField {
                        id:                    lastSentInput
                        text:                  antennaDropPanel.currentAngle.toFixed(0)
                        unitsLabel:            "°"
                        Layout.preferredWidth: 50
                        enabled:               false
                    }
                }
            }

            RowLayout {
                Layout.alignment: Qt.AlignHCenter

                QGCButton {
                    text:      "Send"
                    enabled:   QGroundControl.antennaController ? !QGroundControl.antennaController.isBusy : false
                    onClicked: {
                        if (QGroundControl.antennaController) {
                            QGroundControl.antennaController.setCurrentAngle(Math.round(antennaDropPanel.targetAngle));
                        }
                    }
                }
                QGCButton {
                    text:      "To zero"
                    enabled:   QGroundControl.antennaController ? !QGroundControl.antennaController.isBusy : false
                    onClicked: {
                        if (QGroundControl.antennaController) {
                            antennaDropPanel.targetAngle = 0;
                            QGroundControl.antennaController.setCurrentAngle(0);
                        }
                    }
                }
            }

            RowLayout {
                Layout.alignment: Qt.AlignCenter
                spacing:          8

                QGCButton {
                    text:      "Antenna 1"
                    enabled:   QGroundControl.antennaController ? !QGroundControl.antennaController.isBusy : false
                    primary:   QGroundControl.antennaController ? QGroundControl.antennaController.currentAntenna === 0 : false
                    onClicked: {
                        if (QGroundControl.antennaController) {
                            QGroundControl.antennaController.setCurrentAntenna(0);
                        }
                    }
                }

                QGCButton {
                    text:      "Antenna 2"
                    enabled:   QGroundControl.antennaController ? !QGroundControl.antennaController.isBusy : false
                    primary:   QGroundControl.antennaController ? QGroundControl.antennaController.currentAntenna === 1 : false
                    onClicked: {
                        if (QGroundControl.antennaController) {
                            QGroundControl.antennaController.setCurrentAntenna(1);
                        }
                    }
                }
            }
        }
    }
}
