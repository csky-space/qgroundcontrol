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
    id:             returnCourseIcon
    text:           "Antenna";
    iconSource:     "qrc:/qmlimages/icon-antenna.svg"
    enabled:        true

    dropPanelComponent: Rectangle {
        id:       antennaDropPanel
        color:    "#00000000"

        property real targetAngle: 0;
        property real dialSize: 180

        width:    mainLayout.implicitWidth
        height:   mainLayout.implicitHeight

        function normalizeTo360(angle) {
            return (angle + 360) % 360;
        }

        function normalizeTo180(angle) {
            return (((angle + 180) % 360 + 360) % 360) - 180;
        }

        ColumnLayout {
            id: mainLayout
            anchors.fill: parent
            spacing:      10

            Rectangle {
                Layout.preferredWidth:  antennaDropPanel.dialSize
                Layout.preferredHeight: antennaDropPanel.dialSize / 2
                Layout.alignment:       Qt.AlignHCenter
                color:                  "#00000000" // Make transparent so it doesn't block the dial image

                Image {
                    source:             "/qmlimages/antenna-direction-dial.svg"
                    mipmap:             true
                    fillMode:           Image.PreserveAspectFit
                    anchors.fill:       parent
                    sourceSize.height:  parent.height

                    Shape {
                        id:               dirMark
                        width:            size
                        height:           size
                        anchors.left:     parent.horizontalCenter
                        anchors.bottom:   parent.bottom

                        rotation: antennaDropPanel.targetAngle

                        property real size: antennaDropPanel.dialSize * 0.075
                        property real targetAngleRadians: antennaDropPanel.targetAngle * (Math.PI/180.0)
                        property real marksRadius: antennaDropPanel.dialSize / 2.5 // Safe reference directly to dialSize

                        ShapePath {
                            strokeColor: "#8000ff00"
                            strokeWidth: 1
                            fillColor: "#8000ff00"

                            startX: dirMark.width / 2
                            startY: 0

                            PathLine { x: dirMark.width; y: dirMark.height }
                            PathLine { x: 0; y: dirMark.height }
                            PathLine { x: dirMark.width / 2; y: 0 }
                        }

                        transform: Translate {
                            x: dirMark.marksRadius * Math.sin(dirMark.targetAngleRadians) - dirMark.size / 2
                            y: -dirMark.marksRadius * Math.cos(dirMark.targetAngleRadians) + 2
                        }
                    }

                    MouseArea {
                        id:           dialMouseArea
                        anchors.fill: parent

                        function setAngleFromMousePosition(x, y) {
                            var offsetX = x - (width / 2);
                            var offsetY = (height - dirMark.size / 2) - y;
                            var r = Math.sqrt(offsetX * offsetX + offsetY * offsetY);

                            if (r < 10 || r > width / 2) {
                                return;
                            }

                            var theta = Math.atan2(offsetY, offsetX) * (180 / Math.PI);
                            var normalizedAngle = -antennaDropPanel.normalizeTo180(theta - 90);

                            normalizedAngle = Math.min(Math.max(normalizedAngle, -90), 90);

                            console.log(normalizedAngle);

                            antennaDropPanel.targetAngle = normalizedAngle;
                        }

                        onPositionChanged: (mouse) => {
                            setAngleFromMousePosition(mouse.x, mouse.y);
                        }

                        onReleased: (mouse) => {
                            setAngleFromMousePosition(mouse.x, mouse.y);
                        }
                    }

                    QGCTextField {
                        id:                       altitudeInput
                        text:                     antennaDropPanel.targetAngle.toFixed(2)
                        unitsLabel:               "°"
                        width:                    60
                        anchors.horizontalCenter: parent.horizontalCenter
                        anchors.bottom:           parent.bottom

                        onEditingFinished: {
                            let value = parseFloat(text) ?? 0.0;
                            if (isNaN(value)) value = 0.0;
                            value = Math.min(Math.max(value, -90), 90);
                            antennaDropPanel.targetAngle = value;
                        }
                    }
                }
            }

            RowLayout {
                Layout.alignment: Qt.AlignHCenter
                spacing:          8

                QGCButton {
                    text: "Узкий"
                }

                QGCButton {
                    text: "Широкий"
                }
            }
        }
    }
}
