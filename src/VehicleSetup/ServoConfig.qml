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

    Component {
        id: pageComponent

        ColumnLayout {
            GridLayout {
                columns: 6

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
                            id:             servoInfoLayout

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
                                        text:                qsTr("min: ") + servo.minValue
                                        Layout.alignment:    Qt.AlignVCenter
                                        Layout.minimumWidth: 60
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
                                        text:                qsTr("max: ") + servo.maxValue
                                        Layout.alignment:    Qt.AlignVCenter
                                        Layout.minimumWidth: 60
                                    }
                                }

                                RowLayout {
                                    id: servoReversedLayout

                                    QGCLabel {
                                        text:                qsTr("Reversed")
                                        Layout.alignment:    Qt.AlignVCenter
                                        Layout.minimumWidth: 60
                                    }

                                    QGCCheckBox {
                                        checked:          servo.reversed
                                        enabled:          false
                                        Layout.alignment: Qt.AlignVCenter
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
