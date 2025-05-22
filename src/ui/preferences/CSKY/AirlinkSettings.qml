/****************************************************************************
 *
 * (c) 2009-2020 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/


import QtMultimedia 5.15
import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Dialogs 1.3
import QtQuick.Layouts 1.15
import QtLocation 5.15
import QtPositioning 5.15

import QGroundControl 1.0
import QGroundControl.Controllers 1.0
import QGroundControl.Controls 1.0
import QGroundControl.FactControls 1.0
import QGroundControl.FactSystem 1.0
import QGroundControl.Palette 1.0
import QGroundControl.ScreenTools 1.0
import QGroundControl.SettingsManager 1.0

ColumnLayout {
    spacing: _rowSpacing

    property Fact _loginFact:   QGroundControl.settingsManager.appSettings.loginAirLink
    property Fact _passFact:    QGroundControl.settingsManager.appSettings.passAirLink

    function saveSettings() {
        // No need
    }

    function updateConnectionName(modem) {
            nameField.text = "Airlink " + modem
    }

    GridLayout {
        columns:        2
        columnSpacing:  _colSpacing
        rowSpacing:     _rowSpacing

        QGCLabel {
            text: qsTr("Login:")
        }

        QGCTextField {
            id:                     loginField
            text:                   _loginFact.rawValue
            focus:                  true
            Layout.preferredWidth:  _secondColumnWidth
            onTextChanged:          subEditConfig.username = loginField.text
        }


        QGCLabel {
            text: qsTr("Password:")
        }

        QGCTextField {
            id:                     passwordField
            text:                   _passFact.rawValue
            focus:                  true
            Layout.preferredWidth:  _secondColumnWidth
            echoMode:               TextInput.Password
            onTextChanged:          subEditConfig.password = passwordField.text
        }
    }
    QGCLabel {
        text: "Forgot Your AirLink Password?"
        font.underline: true
        Layout.columnSpan:  2
        MouseArea {
            anchors.fill:   parent
            hoverEnabled:   true
            cursorShape:    Qt.PointingHandCursor
            onClicked:      Qt.openUrlExternally("https://" + QGroundControl.airlinkManager.airlinkHost + "/forgot-pass")
        }
    }

    RowLayout {
        spacing:  _colSpacing
        QGCLabel {
            wrapMode:               Text.WordWrap
            text:                   qsTr("Don't have an account?")
        }
        QGCLabel {
            font.underline:         true
            wrapMode:               Text.WordWrap
            text:                   qsTr("Register")
            MouseArea {
                anchors.fill:   parent
                hoverEnabled:   true
                cursorShape:    Qt.PointingHandCursor
                onClicked:      Qt.openUrlExternally("https://" + QGroundControl.airlinkManager.airlinkHost + "/registration")
            }
        }
    }

    QGCLabel {
        text: qsTr("List of available devices")
    }

    RowLayout {
        QGCComboBox {
            id: drones
            Layout.fillWidth: true
            model: {
                    var out        = []
                    var sortedIndices  = []
                    var list       = QGroundControl.airlinkManager.droneList
                    var onlineList = QGroundControl.airlinkManager.droneOnlineList

                    for (var i = 0; i < list.length; i++) {
                        if (onlineList[i]) {
                            out.push(list[i] + " online")
                            sortedIndices.push(i)
                        }
                    }

                    for (i = 0; i < list.length; i++) {
                        if (!onlineList[i]) {
                            out.push(list[i])
                            sortedIndices.push(i)
                        }
                    }
                    return out
                }

            onActivated: {
                updateConnectionName(drones.model[index])
            }
            Connections {
                target: QGroundControl.airlinkManager
                // model update does not trigger onActivated, so we catch first element manually
                onDroneListChanged: {
                    if (drones.model[0] !== undefined) {
                        drones.currentIndex = 0
                        updateConnectionName(drones.model[0])
                    }
                }
            }
        }
        QGCButton {
            text:       qsTr("Refresh")
            onClicked:  {
                QGroundControl.airlinkManager.updateDroneList(loginField.text, passwordField.text)
                refreshHint.visible = false
                QGroundControl.airlinkManager.updateCredentials(loginField.text, passwordField.text)
            }
        }
    }

    QGCLabel {
        id:                     refreshHint
        Layout.fillWidth:       true
        font.pointSize:         ScreenTools.smallFontPointSize
        wrapMode:               Text.WordWrap
        text:                   qsTr("Click \"Refresh\" to authorize")
    }
}
