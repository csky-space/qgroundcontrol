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
import QGroundControl.FactSystem    1.0
import QGroundControl.FactControls  1.0
import QGroundControl.Controls      1.0
import QGroundControl.ScreenTools   1.0
import QGroundControl.Controllers   1.0

AnalyzePage {
    id:                 dataLossPage
    pageComponent:      pageComponent
    pageDescription:    qsTr("Test mavlink loss.")
    allowPopout:        true

    QGCPalette { id:qgcPal; colorGroupEnabled: true }

    Component {
        id: pageComponent

        GridLayout {
            id:            statsLayout
            columns:       2
            columnSpacing: ScreenTools.defaultFontPixelWidth
            rowSpacing:    ScreenTools.defaultFontPixelHeight * 0.25

            function pct(value) {
                 const total = QGroundControl.dataLossTester.packetsSent;
                 if (total <= 0) return "0.00";
                 return (100.0 * value / total).toFixed(2);
             }

            QGCLabel {
                text: qsTr("Address:")
            }
            QGCTextField {
                Layout.columnSpan:  1
                Layout.fillWidth:   true
                text:               QGroundControl.dataLossTester.address
                placeholderText:    qsTr("127.0.0.1")
                enabled:            !QGroundControl.dataLossTester.connected

                onEditingFinished:  QGroundControl.dataLossTester.address = text
            }

            QGCLabel {
                text: qsTr("Port:")
            }
            QGCTextField {
                Layout.columnSpan:  1
                Layout.fillWidth:   true
                text:               QGroundControl.dataLossTester.port
                placeholderText:    qsTr("9000")
                enabled:            !QGroundControl.dataLossTester.connected

                onEditingFinished:  {
                    QGroundControl.dataLossTester.port = parseInt(text);
                }
            }

            QGCLabel {
                text: qsTr("Packets per test:")
            }
            QGCTextField {
                Layout.columnSpan:  1
                Layout.fillWidth:   true
                text:               QGroundControl.dataLossTester.packetsPerTest
                placeholderText:    qsTr("10000")
                enabled:            !QGroundControl.dataLossTester.connected

                onEditingFinished:  {
                    QGroundControl.dataLossTester.packetsPerTest = parseInt(text);
                }
            }

            QGCLabel {
                text: qsTr("Send interval (ms):")
            }
            QGCTextField {
                Layout.columnSpan:  1
                Layout.fillWidth:   true
                text:               QGroundControl.dataLossTester.sendInterval
                placeholderText:    qsTr("20")
                enabled:            !QGroundControl.dataLossTester.connected

                onEditingFinished:  {
                    QGroundControl.dataLossTester.sendInterval = parseInt(text);
                }
            }

            QGCCheckBox {
                Layout.columnSpan:  2
                text:               qsTr("Wait echo response")
                checked:            QGroundControl.dataLossTester.shouldWait === true
                enabled:            !QGroundControl.dataLossTester.connected

                onClicked:          {
                    QGroundControl.dataLossTester.shouldWait = checked
                }
            }

            QGCLabel {
                text: qsTr("Wait interval (ms):")
                visible:            QGroundControl.dataLossTester.shouldWait === true
            }
            QGCTextField {
                Layout.columnSpan:  1
                Layout.fillWidth:   true
                text:               QGroundControl.dataLossTester.waitInterval
                placeholderText:    qsTr("200")
                enabled:            !QGroundControl.dataLossTester.connected
                visible:            QGroundControl.dataLossTester.shouldWait === true

                onEditingFinished:  {
                    QGroundControl.dataLossTester.waitInterval = parseInt(text);
                }
            }

            QGCButton {
                Layout.columnSpan:  1
                Layout.fillWidth:   true
                text:               qsTr("Start Test")
                onClicked:          QGroundControl.dataLossTester.startSendingMessages()
                enabled:            !QGroundControl.dataLossTester.connected
            }

            QGCButton {
                Layout.columnSpan:  1
                Layout.fillWidth:   true
                text:               qsTr("Stop Test")
                onClicked:          QGroundControl.dataLossTester.stopSendingMessages()
                enabled:            QGroundControl.dataLossTester.connected
            }

            QGCLabel { text: qsTr("packets sent:") }
            QGCLabel {
                text: QGroundControl.dataLossTester.packetsSent
            }

            QGCLabel { text: qsTr("packets received:") }
            QGCLabel {
                text: QGroundControl.dataLossTester.packetsReceived
                      + "  (" + statsLayout.pct(QGroundControl.dataLossTester.packetsReceived) + " %)"
                color: statsLayout.pct(QGroundControl.dataLossTester.packetsReceived) >= 99.0
                       ? qgcPal.colorGreen
                       : qgcPal.text
            }

            QGCLabel { text: qsTr("wait timeouts:") }
            QGCLabel {
                text: QGroundControl.dataLossTester.waitTimeouts
                      + "  (" + statsLayout.pct(QGroundControl.dataLossTester.waitTimeouts) + " %)"
                color: QGroundControl.dataLossTester.waitTimeouts > 0
                       ? qgcPal.colorOrange
                       : qgcPal.text
            }

            QGCLabel { text: qsTr("lost by sequence:") }
            QGCLabel {
                text: QGroundControl.dataLossTester.packetsLostBySequence
                      + "  (" + statsLayout.pct(QGroundControl.dataLossTester.packetsLostBySequence) + " %)"
                color: QGroundControl.dataLossTester.packetsLostBySequence > 0
                       ? qgcPal.colorOrange
                       : qgcPal.text
            }
        }
    }
}

