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
    property var ekf: activeVehicle ? activeVehicle.ekfController : null


    property bool barVisible: false

    property real velVar: ekf ? ekf.velocityVariance : 0
    property real posHorVar: ekf ? ekf.posHorizVariance : 0
    property real posVertVar: ekf ? ekf.posVertVariance : 0
    property real compVar: ekf ? ekf.compassVariance : 0
    property real terrAltVar: ekf ? ekf.terrainAltVariance : 0

    property real margins: ScreenTools.defaultFontPixelWidth
    property real maxValue: 100
    property real barAreaHeight: height * 0.8

    readonly property real columnBaseWidth: (width - margins * 2) / 3

    Rectangle {
        id: areaBackground
        anchors.fill: parent
        color: qgcPal.window
        opacity: 0.5
        z: -1
        radius: 8
        visible: barVisible
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
                columns: 5
                columnSpacing: margins
                rowSpacing: margins * 2
                Layout.fillWidth: true
                Layout.fillHeight: true

                Bar {
                    id: velocityVar
                    label: "Vel"
                    value: velVar
                    lowCheck: 0.5
                    highCheck: 0.8
                    maxValue: 1.0
                }

                Bar {
                    id: positionHorVar
                    label: "PosH"
                    value: posHorVar
                    lowCheck: 0.5
                    highCheck: 0.8
                    maxValue: 1.0
                }

                Bar {
                    id: positionVertVar
                    label: "PosV"
                    value: posVertVar
                    lowCheck: 0.5
                    highCheck: 0.8
                    maxValue: 1.0
                }

                Bar {
                    id: compassVar
                    label: "CMP"
                    value: compVar
                    lowCheck: 0.5
                    highCheck: 0.8
                    maxValue: 1.0
                }

                Bar {
                    id: terrVar
                    label: "TALT"
                    value: terrAltVar
                    lowCheck: 0.5
                    highCheck: 0.8
                    maxValue: 1.0
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

        QGCColoredImage {
            property real factor: 0.5
            property color added: (velVar < velocityVar.lowCheck) && (posHorVar < positionHorVar.lowCheck) && (posVertVar < positionVertVar.lowCheck)
                                  && (compVar < compassVar.lowCheck) && (terrAltVar < terrVar.lowCheck) ? "white" :
                                    Math.max(Math.max(Math.max(Math.max(velVar, posHorVar), posVertVar), compVar), terrAltVar) < velocityVar.highCheck ? "yellow" : "red"

            id: image
            source: "/EKF/EKF.svg"
            fillMode: Image.PreserveAspectFit
            sourceSize.width: _root.width
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
            //    anchors.fill: parent
            //    color: "green"
            //}
        }
    }
}
