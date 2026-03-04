/****************************************************************************
 *
 * (c) 2009-2024 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/

import QtQuick          2.11
import QtQuick.Layouts  1.11
import QtCharts         2.15
import QtGraphicalEffects 1.15

import QGroundControl                     1.0
import QGroundControl.Controls            1.0
import QGroundControl.MultiVehicleManager 1.0
import QGroundControl.ScreenTools         1.0
import QGroundControl.Palette             1.0
import QGroundControl.FactSystem          1.0
import QGroundControl.FactControls        1.0

Item {
    id: _root

    property real size:     _defaultSize
    width:  size
    height: size

    Rectangle {
        anchors.fill: parent
        color: "red"
        opacity: 0.5
        z: 9999
    }

    anchors.top: parent.top
    anchors.bottom: parent.bottom

    width: rowLayout.implicitWidth
    height: rowLayout.implicitHeight

    property var activeVehicle: QGroundControl.multiVehicleManager.activeVehicle
    property var vibration: activeVehicle ? activeVehicle.vibrationController : null

    property real vibeX: vibration ? vibration.xVibration : 0
    property real vibeY: vibration ? vibration.yVibration : 0
    property real vibeZ: vibration ? vibration.zVibration : 0
    property int clipping1: vibration ? vibration.clipping1 : 0
    property int clipping2: vibration ? vibration.clipping2 : 0
    property int clipping3: vibration ? vibration.clipping3 : 0

    property bool showIndicator: true
    property bool settingsPanelVisible: false

    property real margins: ScreenTools.defaultFontPixelWidth
    property real panelRadius: ScreenTools.defaultFontPixelWidth * 0.5
    property real buttonHeight: height * 1.6
    property real separatorHeight: buttonHeight * 0.9

    // Максимальное значение для масштабирования (защита от нуля)
    //property real maxValue: Math.max(vibeX, vibeY, vibeZ, 1)
    property real maxValue: 100
    property real barAreaHeight: _root.height * 0.6

    RowLayout {
        id: rowLayout
        anchors.fill: parent
        spacing: margins

        QGCColoredImage {
            id: vibrationIndicatorIcon
            Layout.preferredWidth: height
            Layout.fillHeight: true
            source: "/vibration/vibration.svg"
            fillMode: Image.PreserveAspectFit
            sourceSize.height: height
            opacity: 1.0
            color: qgcPal.buttonText
        }

        Rectangle {
            Layout.fillHeight: true
            Layout.preferredWidth: mainLayout.implicitWidth + margins * 2
            visible: true
            //clip: true
            color: "transparent"

            GridLayout {
                id: mainLayout
                anchors.fill: parent
                anchors.margins: margins
                rowSpacing: margins
                columnSpacing: margins
                columns: 4

                Column {
                    spacing: 5

                    Item {
                        width: 40
                        height: barAreaHeight   // используем корневое свойство

                        Rectangle {
                            id: xline30
                            anchors.bottom: parent.bottom
                            anchors.bottomMargin: 0.3 * parent.height
                            width: parent.width
                            height: 1
                            color: "black"
                        }
                        Text {
                            anchors.left: xline30.left
                            anchors.leftMargin: 2
                            anchors.verticalCenter: xline30.verticalCenter
                            text: "30"
                            font.pixelSize: 8
                            color: "white"
                        }

                        Rectangle {
                            id: xline60
                            anchors.bottom: parent.bottom
                            anchors.bottomMargin: 0.6 * parent.height
                            width: parent.width
                            height: 1
                            color: "black"
                        }
                        Text {
                            anchors.left: xline60.left
                            anchors.leftMargin: 2
                            anchors.verticalCenter: xline60.verticalCenter
                            text: "60"
                            font.pixelSize: 8
                            color: "white"
                        }

                        Rectangle {
                            id: barX
                            anchors.bottom: parent.bottom
                            width: parent.width
                            height: (Math.min(vibeX, maxValue) / maxValue) * parent.height
                            color: "red"
                            radius: 6
                            border.color: "#1976d2"
                            border.width: 1

                            layer.enabled: true
                            layer.effect: DropShadow {
                                transparentBorder: true
                                color: "#80000000"
                                radius: 8
                                samples: 16
                            }

                            Behavior on height {
                                NumberAnimation { duration: 300; easing.type: Easing.OutQuad }
                            }
                        }

                        // Значение над столбиком
                        Text {
                            anchors.bottom: barX.bottom
                            anchors.bottomMargin: 4
                            anchors.horizontalCenter: barX.horizontalCenter
                            text: vibeX.toFixed(1)
                            font.pixelSize: 12
                            color: "white"
                        }
                    }

                    // Подпись "X" внизу
                    Text {
                        anchors.horizontalCenter: parent.horizontalCenter
                        text: "X"
                        font.pixelSize: 10
                        color: "white"
                    }
                }

                Column {
                    spacing: 5

                    Item {
                        width: 40
                        height: barAreaHeight   // используем корневое свойство

                        Rectangle {
                            id: yline30
                            anchors.bottom: parent.bottom
                            anchors.bottomMargin: 0.3 * parent.height
                            width: parent.width
                            height: 1
                            color: "black"
                        }
                        Text {
                            anchors.left: yline30.left
                            anchors.leftMargin: 2
                            anchors.verticalCenter: yline30.verticalCenter
                            text: "30"
                            font.pixelSize: 8
                            color: "white"
                        }

                        Rectangle {
                            id: yline60
                            anchors.bottom: parent.bottom
                            anchors.bottomMargin: 0.6 * parent.height
                            width: parent.width
                            height: 1
                            color: "black"
                        }
                        Text {
                            anchors.left: yline60.left
                            anchors.leftMargin: 2
                            anchors.verticalCenter: yline60.verticalCenter
                            text: "60"
                            font.pixelSize: 8
                            color: "white"
                        }

                        Rectangle {
                            id: barY
                            anchors.bottom: parent.bottom
                            width: parent.width
                            height: (Math.min(vibeX, maxValue) / maxValue) * parent.height
                            color: "green"
                            radius: 6
                            border.color: "#1976d2"
                            border.width: 1

                            layer.enabled: true
                            layer.effect: DropShadow {
                                transparentBorder: true
                                color: "#80000000"
                                radius: 8
                                samples: 16
                            }

                            Behavior on height {
                                NumberAnimation { duration: 300; easing.type: Easing.OutQuad }
                            }
                        }

                        Text {
                            anchors.bottom: barY.bottom
                            anchors.bottomMargin: 4
                            anchors.horizontalCenter: barY.horizontalCenter
                            text: vibeY.toFixed(1)
                            font.pixelSize: 12
                            color: "white"
                        }
                    }

                    Text {
                        anchors.horizontalCenter: parent.horizontalCenter
                        text: "Y"
                        font.pixelSize: 10
                        color: "white"
                    }
                }
                Column {
                    spacing: 5

                    Item {
                        width: 40
                        height: barAreaHeight
                        Rectangle {
                            id: zline30
                            anchors.bottom: parent.bottom
                            anchors.bottomMargin: 0.3 * parent.height
                            width: parent.width
                            height: 1
                            color: "black"
                        }
                        Text {
                            anchors.left: zline30.left
                            anchors.leftMargin: 2
                            anchors.verticalCenter: zline30.verticalCenter
                            text: "30"
                            font.pixelSize: 8
                            color: "white"
                        }

                        Rectangle {
                            id: zline60
                            anchors.bottom: parent.bottom
                            anchors.bottomMargin: 0.6 * parent.height
                            width: parent.width
                            height: 1
                            color: "black"
                        }
                        Text {
                            anchors.left: zline60.left
                            anchors.leftMargin: 2
                            anchors.verticalCenter: zline60.verticalCenter
                            text: "60"
                            font.pixelSize: 8
                            color: "white"
                        }
                        Rectangle {
                            id: barZ
                            anchors.bottom: parent.bottom
                            width: parent.width
                            height: (Math.min(vibeX, maxValue) / maxValue) * parent.height
                            color: "blue"
                            radius: 6
                            border.color: "#d35400"
                            border.width: 1

                            layer.enabled: true
                            layer.effect: DropShadow {
                                transparentBorder: true
                                color: "#80000000"
                                radius: 8
                                samples: 16
                            }

                            Behavior on height {
                                NumberAnimation { duration: 300; easing.type: Easing.OutQuad }
                            }
                        }

                        Text {
                            anchors.bottom: barZ.bottom
                            anchors.bottomMargin: 4
                            anchors.horizontalCenter: barZ.horizontalCenter
                            text: vibeZ.toFixed(1)
                            font.pixelSize: 12
                            color: "white"
                        }
                    }

                    Text {
                        anchors.horizontalCenter: parent.horizontalCenter
                        text: "Z"
                        font.pixelSize: 10
                        color: "white"
                    }
                }
            }
        }
        QGCLabel {
            id:                     clipping1Label
            text:                   vibration ? qsTr("CLP1: ") + clipping1 : ""
        }
        QGCLabel {
            id:                     clipping2Label
            text:                   vibration ? qsTr("CLP2: ") + clipping2 : ""
        }
        QGCLabel {
            id:                     clipping3Label
            text:                   vibration ? qsTr("CLP3: ") + clipping3 : ""
        }
    }

    MouseArea {
        anchors.fill: parent
        onClicked: {
            //mainWindow.showIndicatorPopup(_root, vibrationControlsPopup, false)
        }
    }
}
