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
            columns:       2
            columnSpacing: ScreenTools.defaultFontPixelWidth
            rowSpacing:    ScreenTools.defaultFontPixelHeight * 0.25

            QGCLabel { text: qsTr("Address:") }
            QGCTextField {
                Layout.columnSpan:  1
                Layout.fillWidth:   true
                text:               QGroundControl.dataLossTester.address
                placeholderText:    qsTr("127.0.0.1")

                onEditingFinished:  QGroundControl.dataLossTester.address = text
            }

            QGCLabel { text: qsTr("Port:") }
            QGCTextField {
                Layout.columnSpan:  1
                Layout.fillWidth:   true
                text:               QGroundControl.dataLossTester.port
                placeholderText:    qsTr("9000")

                onEditingFinished:  {
                    QGroundControl.dataLossTester.port = parseInt(text);
                }
            }

            QGCLabel { text: qsTr("Packets per test:") }
            QGCTextField {
                Layout.columnSpan:  1
                Layout.fillWidth:   true
                text:               QGroundControl.dataLossTester.packetsPerTest
                placeholderText:    qsTr("10000")

                onEditingFinished:  {
                    QGroundControl.dataLossTester.packetsPerTest = parseInt(text);
                }
            }

            QGCLabel { text: qsTr("Send interval:") }
            QGCTextField {
                Layout.columnSpan:  1
                Layout.fillWidth:   true
                text:               QGroundControl.dataLossTester.sendInterval
                placeholderText:    qsTr("20")

                onEditingFinished:  {
                    QGroundControl.dataLossTester.sendInterval = parseInt(text);
                }
            }

            QGCLabel { text: qsTr("Wait interval:") }
            QGCTextField {
                Layout.columnSpan:  1
                Layout.fillWidth:   true
                text:               QGroundControl.dataLossTester.waitInterval
                placeholderText:    qsTr("200")

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
                onClicked:          QGroundControl.dataLossTester.startSendingMessages()
                enabled:            QGroundControl.dataLossTester.connected
            }

            QGCCheckBox {
                Layout.columnSpan:  2
                text:               qsTr("Should wait")
                checked:            QGroundControl.dataLossTester.shouldWait === true

                onClicked:          {
                    QGroundControl.dataLossTester.setShouldWait = checked
                }
            }

            QGCLabel { text: qsTr("packets sent:") }
            QGCLabel { text: QGroundControl.dataLossTester.packetsSent }

            QGCLabel { text: qsTr("packets received:") }
            QGCLabel { text: QGroundControl.dataLossTester.packetsReceived }

            QGCLabel { text: qsTr("packets lost:") }
            QGCLabel { text: QGroundControl.dataLossTester.packetsLost }
        }
    }
}

