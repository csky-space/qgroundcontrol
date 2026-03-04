import QtQuick 2.11
import QtQuick.Layouts 1.11
import QtGraphicalEffects 1.15

import QGroundControl 1.0
import QGroundControl.Controls 1.0
import QGroundControl.MultiVehicleManager 1.0
import QGroundControl.ScreenTools 1.0
import QGroundControl.Palette 1.0

Item {
    id: _root

    property var activeVehicle: QGroundControl.multiVehicleManager.activeVehicle
    property var vibration: activeVehicle ? activeVehicle.vibrationController : null


    property bool barVisible: false
    property real vibeX: vibration ? vibration.xVibration : 0
    property real vibeY: vibration ? vibration.yVibration : 0
    property real vibeZ: vibration ? vibration.zVibration : 0
    property int clipping1: vibration ? vibration.clipping1 : 0
    property int clipping2: vibration ? vibration.clipping2 : 0
    property int clipping3: vibration ? vibration.clipping3 : 0

    property real margins: ScreenTools.defaultFontPixelWidth
    property real maxValue: 100
    property real barAreaHeight: height * 0.8

    readonly property real columnBaseWidth: (width - margins * 2) / 3

    //implicitWidth: rowLayout.implicitWidth
    //implicitHeight: rowLayout.implicitHeight

    Rectangle {
        anchors.fill: parent
        color: qgcPal.window
        opacity: barVisible ? 0.5 : 0.0
        z: -1
        radius: 8
    }

    RowLayout {
        id: rowLayout
        anchors.fill: parent
        clip: true
        spacing: margins
        //Layout.fillWidth: true
        ColumnLayout {
            id: barColumnLayout
            visible: barVisible
            spacing: margins * 2
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.preferredWidth: _root.width * 0.7
            GridLayout {
                id: barGrid
                columns: 3
                columnSpacing: margins
                rowSpacing: margins * 2
                Layout.fillWidth: true
                Layout.fillHeight: true
                ColumnLayout {
                    Layout.fillWidth: true
                    Layout.preferredWidth: 1
                    spacing: 5
                    //
                    Item {
                        Layout.fillWidth: true
                        Layout.fillHeight: true


                        Rectangle {
                            id: barX
                            anchors.bottom: parent.bottom
                            width: parent.width
                            height: (Math.min(vibeX, maxValue) / maxValue) * parent.height
                            color: vibeX < 30 ? "#222222" : vibeX < 60 ? "yellow" : "red"
                            radius: 6
                            border.color: "white"
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

                        Rectangle {
                            id: line30x
                            anchors.bottom: parent.bottom
                            anchors.bottomMargin: 0.3 * parent.height
                            width: parent.width
                            height: 2
                            color: "black"
                        }

                        Text {
                            anchors.top: line30x.bottom
                            anchors.left: line30x.left
                            anchors.rightMargin: 2
                            text: "30"
                            font.pixelSize: 8
                            color: "white"
                        }

                        Rectangle {
                            id: line60x
                            anchors.bottom: parent.bottom
                            anchors.bottomMargin: 0.6 * parent.height
                            width: parent.width
                            height: 2
                            color: "black"
                        }
                        Text {
                            anchors.top: line60x.bottom
                            anchors.left: line60x.left
                            anchors.rightMargin: 2
                            text: "60"
                            font.pixelSize: 8
                            color: "white"
                        }

                        Text {
                            id: xText
                            anchors.bottom: barX.bottom
                            anchors.horizontalCenter: parent.horizontalCenter
                            text: "X"
                            font.pixelSize: 10
                            color: "white"
                        }
                        Text {
                            anchors.bottom: xText.top
                            anchors.bottomMargin: 4
                            anchors.horizontalCenter: barX.horizontalCenter
                            text: vibeX.toFixed(1)
                            font.pixelSize: 10
                            color: "white"
                        }
                    }
                }

                ColumnLayout {
                    Layout.fillWidth: true
                    Layout.preferredWidth: 1
                    spacing: 5
                    //
                    Item {
                        Layout.fillWidth: true
                        Layout.fillHeight: true


                        Rectangle {
                            id: barY
                            anchors.bottom: parent.bottom
                            width: parent.width
                            height: (Math.min(vibeY, maxValue) / maxValue) * parent.height
                            color: vibeY < 30 ? "#222222" : vibeY < 60 ? "yellow" : "red"
                            radius: 6
                            border.color: "white"
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

                        Rectangle {
                            id: line30y
                            anchors.bottom: parent.bottom
                            anchors.bottomMargin: 0.3 * parent.height
                            width: parent.width
                            height: 2
                            color: "black"
                        }
                        Text {
                            anchors.top: line30y.bottom
                            anchors.left: line30y.left
                            anchors.rightMargin: 2
                            text: "30"
                            font.pixelSize: 8
                            color: "white"
                        }

                        Rectangle {
                            id: line60y
                            anchors.bottom: parent.bottom
                            anchors.bottomMargin: 0.6 * parent.height
                            width: parent.width
                            height: 2
                            color: "black"
                        }
                        Text {
                            anchors.top: line60y.bottom
                            anchors.left: line60y.left
                            anchors.rightMargin: 2
                            text: "60"
                            font.pixelSize: 8
                            color: "white"
                        }

                        Text {
                            id: yText
                            anchors.bottom: barY.bottom
                            anchors.horizontalCenter: parent.horizontalCenter
                            text: "Y"
                            font.pixelSize: 10
                            color: "white"
                        }
                        Text {
                            anchors.bottom: yText.top
                            anchors.bottomMargin: 4
                            anchors.horizontalCenter: barY.horizontalCenter
                            text: vibeY.toFixed(1)
                            font.pixelSize: 10
                            color: "white"
                        }
                    }
                }

                ColumnLayout {
                    Layout.fillWidth: true
                    Layout.preferredWidth: 1
                    spacing: 5
                    //
                    Item {
                        Layout.fillWidth: true
                        Layout.fillHeight: true


                        Rectangle {
                            id: barZ
                            anchors.bottom: parent.bottom
                            width: parent.width
                            height: (Math.min(vibeZ, maxValue) / maxValue) * parent.height
                            color: vibeZ < 30 ? "#222222" : vibeZ < 60 ? "yellow" : "red"
                            radius: 6
                            border.color: "white"
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

                        Rectangle {
                            id: line30z
                            anchors.bottom: parent.bottom
                            anchors.bottomMargin: 0.3 * parent.height
                            width: parent.width
                            height: 2
                            color: "black"
                        }
                        Text {
                            anchors.top: line30z.bottom
                            anchors.left: line30z.left
                            anchors.rightMargin: 2
                            text: "30"
                            font.pixelSize: 8
                            color: "white"
                        }

                        Rectangle {
                            id: line60z
                            anchors.bottom: parent.bottom
                            anchors.bottomMargin: 0.6 * parent.height
                            width: parent.width
                            height: 2
                            color: "black"
                        }
                        Text {
                            anchors.top: line60z.bottom
                            anchors.left: line60z.left
                            anchors.rightMargin: 2
                            text: "60"
                            font.pixelSize: 8
                            color: "white"
                        }
                        Text {
                            id: zText
                            anchors.bottom: barZ.bottom
                            anchors.horizontalCenter: parent.horizontalCenter
                            text: "Z"
                            font.pixelSize: 10
                            color: "white"
                        }
                        Text {
                            anchors.bottom: zText.top
                            anchors.bottomMargin: 4
                            anchors.horizontalCenter: barZ.horizontalCenter
                            text: vibeZ.toFixed(1)
                            font.pixelSize: 10
                            color: "white"
                        }

                    }


                }
            }
            //RowLayout {
            //    width: parent.width
            //    spacing: margins
            //    Layout.preferredWidth: _root.width * 0.7
            //    Layout.fillWidth: true
            //    Layout.fillHeight: true
            //    QGCLabel {
            //        horizontalAlignment: Text.AlignHCenter
            //        text: vibration ? qsTr("CLP1: ") + clipping1 : ""
            //        color: "white"
            //    }
            //    QGCLabel {
            //        horizontalAlignment: Text.AlignHCenter
            //        text: vibration ? qsTr("CLP2: ") + clipping2 : ""
            //        color: "white"
            //    }
            //    QGCLabel {
            //        horizontalAlignment: Text.AlignHCenter
            //        text: vibration ? qsTr("CLP3: ") + clipping3 : ""
            //        color: "white"
            //    }
            //}
        }

        //Rectangle {
        //    anchors.fill: parent
        //    color: "red"
        //}

        QGCColoredImage {
            property real factor: 0.5
            property color added: Qt.rgba(
                Math.min((barX.color.r + barY.color.r + barZ.color.r), 1.0),
                Math.min((barX.color.g + barY.color.g + barZ.color.g), 1.0),
                Math.min((barX.color.b + barY.color.b + barZ.color.b), 1.0),
                1
            )

            id: image
            source: "/vibration/vibration.svg"
            fillMode: Image.PreserveAspectFit
            sourceSize.height: _root.height * 0.3
            color: added
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.maximumHeight: width / (16/9)

            Layout.preferredWidth: _root.width * 0.3

            //Layout.alignment: Qt.AlignRight
            MouseArea {
                anchors.fill: parent

                onClicked: {
                    console.debug("barX.color:", barX.color)
                        console.debug("barY.color:", barY.color)
                        console.debug("barZ.color:", barZ.color)
                    barVisible = !barVisible
                }
            }

            Rectangle {
                anchors.fill: parent
                color: qgcPal.window
                opacity: !barVisible ? 0.5 : 0.0
                z: -1
                radius: 8
            }

            //Rectangle {
            //    anchors.fill: parent
            //    color: "green"
            //}
        }
    }
    onScaleChanged: {
        console.debug("barX size: (x:" + barX.width + ";y:" + barX.height + ")")
        console.debug("barY size: (x:" + barY.width + ";y:" + barY.height + ")")
        console.debug("barZ size: (x:" + barZ.width + ";y:" + barZ.height + ")")
    }
}
