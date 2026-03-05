import QtQuick 2.15
import QtQuick.Layouts 1.11
import QtGraphicalEffects 1.15

ColumnLayout {
    property string label: ""
    property real value: 0.0
    property real maxValue: 100
    property real highCheck: 60
    property real lowCheck: 30

    property alias barRectangle: bar

    Layout.fillWidth: true
    Layout.preferredWidth: 1
    spacing: 5
    //
    Item {
        Layout.fillWidth: true
        Layout.fillHeight: true
        Rectangle {
            anchors.fill: parent
            color: "black"
        }

        Rectangle {
            id: bar
            anchors.bottom: parent.bottom
            width: parent.width
            height: (Math.min(value, maxValue) / maxValue) * parent.height
            color: value < lowCheck ? "#222222" : value < highCheck ? "yellow" : "red"
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
            id: line60
            anchors.bottom: parent.bottom
            anchors.bottomMargin: highCheck/maxValue * parent.height
            width: parent.width
            height: 2
            color: "white"
        }
        Text {
            anchors.top: line60.bottom
            anchors.left: line60.left
            anchors.rightMargin: 2
            text: highCheck
            font.pixelSize: 8
            color: "white"
        }

        Rectangle {
            id: line30
            anchors.bottom: parent.bottom
            anchors.bottomMargin: lowCheck/maxValue * parent.height
            width: parent.width
            height: 2
            color: "white"
        }
        Text {
            id: line30Text
            anchors.top: line30.bottom
            anchors.left: line30.left
            anchors.rightMargin: 2
            text: lowCheck
            font.pixelSize: 8
            color: "white"
        }
        Text {
            id: labelText
            anchors.top: bar.top
            anchors.horizontalCenter: bar.horizontalCenter

            text: label
            font.pixelSize: 10
            color: "white"
        }
    }
}
