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
                    label: "PosHor"
                    value: posHorVar
                    lowCheck: 0.5
                    highCheck: 0.8
                    maxValue: 1.0
                }

                Bar {
                    id: positionVertVar
                    label: "PosVert"
                    value: posVertVar
                    lowCheck: 0.5
                    highCheck: 0.8
                    maxValue: 1.0
                }

                Bar {
                    id: compassVar
                    label: "Comp"
                    value: compVar
                    lowCheck: 0.5
                    highCheck: 0.8
                    maxValue: 1.0
                }

                Bar {
                    id: terrVar
                    label: "Terr"
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
                                  && (compVar < compassVar.lowCheck) && (terrAltVar < terrVar.lowCheck) ? "white" : Qt.rgba(
                Math.min((velocityVar.barRectangle.color.r + positionHorVar.barRectangle.color.r + positionVertVar.barRectangle.color.r + compassVar.barRectangle.color.r + terrVar.barRectangle.color.r) * (1.0/5), 1.0),
                Math.min((velocityVar.barRectangle.color.g + positionHorVar.barRectangle.color.g + positionVertVar.barRectangle.color.g + compassVar.barRectangle.color.g + terrVar.barRectangle.color.g) * (1.0/5), 1.0),
                Math.min((velocityVar.barRectangle.color.b + positionHorVar.barRectangle.color.b + positionVertVar.barRectangle.color.b + compassVar.barRectangle.color.b + terrVar.barRectangle.color.b) * (1.0/5), 1.0),
                1
            )

            id: image
            source: "/EKF/EKF.svg"
            fillMode: Image.PreserveAspectFit
            sourceSize.height: _root.height * 0.3
            color: added
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.maximumHeight: barVisible ? (_root.width * 0.3) / (16/9) : _root.width / (16/9)

            Layout.preferredWidth: barVisible ? _root.width * 0.3 : _root.width

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
