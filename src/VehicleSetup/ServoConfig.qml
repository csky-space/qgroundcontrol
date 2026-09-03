/****************************************************************************
 *
 * (c) 2009-2020 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/

import QtQuick                      2.11
import QtQuick.Controls             2.4
import QtQuick.Dialogs              1.3
import QtQuick.Layouts              1.11

import QGroundControl               1.0
import QGroundControl.Palette       1.0
import QGroundControl.Controls      1.0
import QGroundControl.ScreenTools   1.0
import QGroundControl.Controllers   1.0
import QGroundControl.FactSystem    1.0
import QGroundControl.FactControls  1.0

/// Servo Config
SetupPage {
    id:              servoPage
    pageComponent:   pageComponent
    pageName:        qsTr("Servo")
    pageDescription: ""
    anchors.fill:    parent

    property var activeVehicle:     QGroundControl.multiVehicleManager.activeVehicle
    property var servoController:   activeVehicle.servoController
    property bool isEditingEnabled: false

    Component {
        id: pageComponent

        ColumnLayout {
            spacing: 10
            width:   servoPage.width
            height:  servoPage.height

            RowLayout {
                Layout.fillWidth: true

                QGCCheckBox {
                    id:        editToggle
                    text:      qsTr("Enable editing")
                    checked:   servoPage.isEditingEnabled
                    onClicked: {
                        servoPage.isEditingEnabled = checked
                    }
                }
            }

            GridView {
                id:                gridView
                model:             servoController.servoModel
                Layout.fillWidth:  true
                Layout.fillHeight: true
                cellWidth:         320
                cellHeight:        120
                flow:              GridView.FlowLeftToRight
                clip:              true

                delegate: Rectangle {
                    color:        "#00000000"
                    border.width: 1
                    border.color: "#ffffff"
                    radius:       4
                    width:        gridView.cellWidth - 10
                    height:       gridView.cellHeight - 10
                    x:            5
                    y:            5

                    required property int index
                    property var servo: servoController.servoModel[index]

                    ColumnLayout {
                        anchors.fill:    parent
                        anchors.margins: 6

                        QGCLabel {
                            text:                qsTr("Servo ") + (servo.index + 1)
                            Layout.alignment:    Qt.AlignVCenter
                            Layout.minimumWidth: 60
                            font.pointSize:      ScreenTools.largeFontPointSize
                        }

                        RowLayout {
                            id: servoStateLayout

                            QGCLabel {
                                text:                qsTr("min:")
                                Layout.alignment:    Qt.AlignVCenter
                                Layout.minimumWidth: 20
                            }

                            FactTextField {
                                fact:                  servo.minValueFact
                                Layout.alignment:      Qt.AlignVCenter
                                Layout.preferredWidth: 70
                                enabled:               servoPage.isEditingEnabled
                            }

                            Rectangle {
                                height:           ScreenTools.defaultFontPixelHeight
                                width:            120
                                color:            "#333333"
                                Layout.fillWidth: true
                                Layout.alignment: Qt.AlignVCenter
                                radius:           4

                                Rectangle {
                                    color:          "#20af20"
                                    anchors.left:   parent.left
                                    anchors.top:    parent.top
                                    anchors.bottom: parent.bottom
                                    width:          servo.normalizedValue * parent.width
                                    radius:         4
                                }

                                QGCLabel {
                                    text:             servo.value
                                    anchors.centerIn: parent
                                }
                            }

                            QGCLabel {
                                text:                qsTr("max:")
                                Layout.alignment:    Qt.AlignVCenter
                                Layout.minimumWidth: 20
                            }

                            FactTextField {
                                fact:                  servo.maxValueFact
                                Layout.alignment:      Qt.AlignVCenter
                                Layout.preferredWidth: 70
                                enabled:               servoPage.isEditingEnabled
                            }
                        }

                        RowLayout {
                            FactCheckBox {
                                text:             qsTr("Reversed")
                                fact:             servo.reversedFact
                                Layout.alignment: Qt.AlignVCenter
                                enabled:          servoPage.isEditingEnabled
                            }

                            Item {
                                Layout.fillWidth: true
                                height:           1
                            }

                            RowLayout {
                                Layout.alignment: Qt.AlignVCenter

                                QGCLabel {
                                    text:             qsTr("function:")
                                    Layout.alignment: Qt.AlignVCenter
                                }

                                FactComboBox {
                                    fact:                  servo.functionFact
                                    Layout.preferredWidth: 120
                                    Layout.alignment:      Qt.AlignVCenter
                                    enabled:               servoPage.isEditingEnabled
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}