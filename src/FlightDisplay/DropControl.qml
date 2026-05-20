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
    property var _activeVehicle: QGroundControl.multiVehicleManager.activeVehicle
    property var _servoController: _activeVehicle ? _activeVehicle.servoController : undefined

    text:           qsTr("Drop")
    iconSource:     "qrc:/qmlimages/drop.svg"
    enabled:        _servoController && _servoController.initialized ? true : false

    dropPanelComponent: Rectangle {
        id:     dropControl
        height: instrumentPanel ? instrumentPanel._heightAttComp * 1.5 : 0
        width:  instrumentPanel ? instrumentPanel._heightAttComp * 1.5 : 0
        color:  Qt.rgba(qgcPal.window.r, qgcPal.window.g, qgcPal.window.b, 0.6)

        GridLayout {
            id:            buttonsGrid
            anchors.fill:  parent
            columns:       2
            columnSpacing: 2
            rowSpacing:    2

            Repeater {
                id:    servoButtonsRepeater
                model: _servoController ? _servoController.servoModel : 0

                Button {
                    required property int index

                    property var servo:      _servoController.servoModel[index]
                    property int assignment: _servoController.servoAssignments[index]

                    id:                     dropBtn
                    text:                   "Drop"
                    enabled:                assignment !== 0 ? true : false
                    opacity:                assignment !== 0 ? 1 : 0.7
                    Layout.preferredWidth:  (buttonsGrid.width - 4) / buttonsGrid.columns
                    Layout.preferredHeight: Layout.preferredWidth

                    background: Rectangle {
                        anchors.fill:   parent
                        radius:         2
                        border.color:   "#555"
                        border.width:   1
                        color:          _servoController.servoStates[index]
                                            ? dropBtn.down ? "#acac2c" : dropBtn.hovered ? "#aaaa3a" : "#a4a444"
                                            : dropBtn.down ? "#2c2c2c" : dropBtn.hovered ? "#3a3a3a" : "#444444"
                    }

                    contentItem: ColumnLayout {
                        Image {
                            Layout.preferredWidth:  24
                            Layout.preferredHeight: Layout.preferredWidth
                            Layout.alignment:       Qt.AlignHCenter
                            source:                 "qrc:/qmlimages/drop.svg"
                        }
                        Text {
                            Layout.alignment:   Qt.AlignHCenter
                            font.family:        "Helvetica"
                            font.pointSize:     8
                            color:              "#ffffff"
                            text:               "S" + (servo.index + 1) + ": " + (assignment !== 0 ? "G" + assignment : "-")
                        }
                    }

                    onClicked: {
                        _servoController.toggleDesiredDropState(index);
                    }

                    Rectangle {
                        height:         2
                        anchors.left:   parent.left
                        anchors.right:  parent.right
                        anchors.bottom: parent.bottom
                        radius:         2

                        Rectangle {
                            color:          "#20af20"
                            anchors.left:   parent.left
                            anchors.top:    parent.top
                            anchors.bottom: parent.bottom
                            width:          servo.normalizedValue * parent.width
                            radius:         2
                        }
                    }
                }
            }
        }
    }
}
