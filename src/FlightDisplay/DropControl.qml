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
    property var  _activeVehicle:                 QGroundControl.multiVehicleManager.activeVehicle

    text:           qsTr("Drop")
    iconSource:     "qrc:/qmlimages/drop.svg"
    enabled:        _activeVehicle ? !QGroundControl.multiVehicleManager.activeVehicle.parameterManager.missingParameters : false

    dropPanelComponent: Rectangle {
        id:     dropControl
        height: instrumentPanel ? instrumentPanel._heightAttComp * 0.75 : 0
        width:  instrumentPanel ? instrumentPanel._heightAttComp * 3 : 0
        color:  Qt.rgba(qgcPal.window.r, qgcPal.window.g, qgcPal.window.b, 0.6)

        property int activeMode: -1;
        property Fact dropModeFact: _activeVehicle.parameterManager.getParameter(_activeVehicle.defaultComponentId(), "DROP_MODE");

        function fetchParamValue() {
            dropControl.activeMode = dropControl.dropModeFact.value;
        }

        Component.onCompleted : {
            fetchParamValue();
        }

        Connections {
            target: _activeVehicle.parameterManager
            onParametersReadyChanged: function(parametersReady) {
                if (parametersReady) {
                    fetchParamValue();
                    return;
                } else {
                    dropControl.activeMode = -1;
                }
            }
        }

        Connections {
            target: dropModeFact
            onVehicleUpdated: {
                activeMode = dropModeFact.value;
            }
        }

        Timer {
            id: paramRequestTimer
            interval: 1000      
            running: true       
            repeat: true       
            onTriggered: {
                _activeVehicle.parameterManager.refreshParameter(_activeVehicle.defaultComponentId(), "DROP_MODE");                
            }
        }

        GridLayout {
            id:            buttonsGrid
            anchors.fill:  parent
            columns:       3
            columnSpacing: 2

            Repeater {
                id:    buttonsRepeater
                model: modelData

                property var modelData: [1, 2, 4]

                Button {
                    required property int index

                    property var modeName:  "Mode " + buttonsRepeater.modelData[index]

                    id:                     dropBtn
                    text:                   modeName
                    Layout.preferredWidth:  (buttonsGrid.width - 6) / buttonsGrid.columns
                    Layout.preferredHeight: Layout.preferredWidth * 0.75

                    background: Rectangle {
                        anchors.fill:   parent
                        radius:         2
                        border.color:   "#555"
                        border.width:   1
                        color:          dropControl.activeMode == index
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
                            text:               dropBtn.modeName
                        }
                    }

                    onClicked: {
                        dropControl.dropModeFact.value = index
                    }
                }
            }
        }
    }
}
