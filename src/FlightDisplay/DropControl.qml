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

// DropControl
ToolStripAction {
    text:       qsTr(_dropActionText ? _dropActionText : "Error")
    iconSource: "qrc:/qmlimages/drop.svg"
    enabled:    _activeVehicle ? !QGroundControl.multiVehicleManager.activeVehicle.parameterManager.missingParameters : false

    property var _activeVehicle:  QGroundControl.multiVehicleManager.activeVehicle
    property var _dropController: _activeVehicle ? _activeVehicle.dropController : null
    property var _dropActionText: _dropController ? _dropController.dropModesModel[_dropController.activeModeIndex] : "Drop"

    dropPanelComponent: Rectangle {
        id:     dropControl
        height: instrumentPanel ? instrumentPanel._heightAttComp * 0.75 : 0
        width:  instrumentPanel ? instrumentPanel._heightAttComp * 3 : 0
        color:  Qt.rgba(qgcPal.window.r, qgcPal.window.g, qgcPal.window.b, 0.6)

        GridLayout {
            id:            buttonsGrid
            anchors.fill:  parent
            columns:       3
            columnSpacing: 2

            Repeater {
                id:    buttonsRepeater
                model: _dropController ? _dropController.dropModesModel : 0

                Button {
                    required property int index

                    id:                     dropBtn
                    Layout.preferredWidth:  (buttonsGrid.width - 6) / buttonsGrid.columns
                    Layout.preferredHeight: Layout.preferredWidth * 0.75
                    enabled:                !_dropController.isInterfaceLocked
                    opacity:                !_dropController.isInterfaceLocked ? 1 : 0.75

                    background: Rectangle {
                        anchors.fill:   parent
                        radius:         2
                        border.color:   "#555"
                        border.width:   1
                        color:          _dropController.activeModeIndex === index
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
                            text:               _dropController.dropModesModel[index]
                        }
                    }

                    onClicked: {
                        _dropController.setDropModeIndex(index);
                    }
                }
            }
        }
    }
}
