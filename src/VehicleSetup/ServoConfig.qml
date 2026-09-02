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
    id:                 servoPage
    pageComponent:      pageComponent
    pageName:           qsTr("Servo")
    pageDescription:    ""

    property var activeVehicle:     QGroundControl.multiVehicleManager.activeVehicle
    property var servoController:   activeVehicle.servoController
    property bool isEditingEnabled: false

    Component {
        id: pageComponent

        ColumnLayout {
            spacing: 10
            anchors.fill: parent
            anchors.margins: 10

            RowLayout {
                Layout.fillWidth: true

                QGCCheckBox {
                    id: editToggle
                    text: qsTr("Enable editing")
                    checked: servoPage.isEditingEnabled
                    onClicked: {
                        servoPage.isEditingEnabled = checked
                    }
                }
            }

            GridLayout {
                columns: 4
                columnSpacing: 8
                rowSpacing: 8
                Layout.fillWidth: true

                Repeater {
                    model: servoController.servoModel

                    Rectangle {
                        required property int index

                        implicitWidth:  servoInfoLayout.implicitWidth
                        implicitHeight: servoInfoLayout.implicitHeight
                        color:          "#00000000"
                        border.width:   1
                        border.color:   "#ffffff"
                        radius:         4

                        property var servo: servoController.servoModel[index]

                        ColumnLayout {
                            id: servoInfoLayout

                            ColumnLayout {
                                Layout.margins: 6

                                QGCLabel {
                                    text:                qsTr("Servo ") + (servo.index + 1)
                                    Layout.alignment:    Qt.AlignVCenter
                                    Layout.minimumWidth: 60
                                    font.pointSize:     ScreenTools.largeFontPointSize
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
                                        text: qsTr("Reversed")
                                        fact: servo.reversedFact
                                        Layout.alignment:    Qt.AlignVCenter
                                        enabled:             servoPage.isEditingEnabled   // добавлено
                                    }

                                    Item {
                                        Layout.fillWidth: true
                                        height: 1
                                    }

                                    RowLayout {
                                        Layout.alignment:    Qt.AlignVCenter

                                        QGCLabel {
                                            text:                qsTr("function:")
                                            Layout.alignment:    Qt.AlignVCenter
                                        }

                                        FactComboBox {
                                            fact: servo.functionFact
                                            Layout.preferredWidth: 120
                                            Layout.alignment:    Qt.AlignVCenter
                                            enabled:             servoPage.isEditingEnabled   // добавлено
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}