import QtQuick 2.15
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

    RowLayout {
        id: rowLayout
        anchors.fill: parent
        clip: true
        spacing: margins
        Rectangle {
            anchors.fill: barVisible ? parent : image
            color: qgcPal.window
            opacity: 0.5
            z: -1
            radius: 8
        }
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
               Bar {
                   id: barX
                   label: "X"
                   value: vibeX
               }

               Bar {
                   id: barY
                   label: "Y"
                   value: vibeY
               }

               Bar {
                   id: barZ
                   label: "Z"
                   value: vibeZ
               }
           }
        }

        Rectangle {
            visible: !barVisible
            clip: true
            anchors.margins: margins * 2

            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.preferredWidth: _root.width * 0.7
            color: "transparent"
        }

        QGCColoredImage {
            property real factor: 0.5
            property color added: (vibeX < barX.lowCheck) && (vibeY < barY.lowCheck) && (vibeZ < barZ.lowCheck) ? "white" : Math.max(Math.max(vibeX, vibeY), vibeZ) < barX.highCheck ? "yellow" : "red"

            id: image
            source: "/vibration/vibration.svg"
            fillMode: Image.PreserveAspectFit
            sourceSize.width: _root.width * 0.3
            color: added

            //Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.maximumHeight: (_root.width * 0.3) / (16/9)
            Layout.maximumWidth: _root.width * 0.3
            Layout.minimumWidth: _root.width * 0.3
            Layout.alignment: Qt.AlignRight

            MouseArea {
                anchors.fill: parent

                onClicked: {
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
            //   anchors.fill: parent
            //   color: "green"
            //}
        }
    }
}
