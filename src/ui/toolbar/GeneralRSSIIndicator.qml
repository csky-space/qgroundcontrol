/****************************************************************************
 *
 * (c) 2009-2020 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/

import QtQuick          2.11
import QtQuick.Layouts  1.11

import QGroundControl                       1.0
import QGroundControl.Controls              1.0
import QGroundControl.MultiVehicleManager   1.0
import QGroundControl.ScreenTools           1.0
import QGroundControl.Palette               1.0
import MAVLink                              1.0

//-------------------------------------------------------------------------
//-- General RSSI Indicator
Item {
    id:             _root
    anchors.top:    parent.top
    anchors.bottom: parent.bottom
    width:          indicatorRow.width
    visible:        status !== 0

    property bool showIndicator:        true
    property var  signalQualityMonitor: QGroundControl.signalQualityMonitor ? QGroundControl.signalQualityMonitor : null
    property var  status:               signalQualityMonitor ? signalQualityMonitor.status : 0

    RowLayout {
        id:               indicatorRow
        anchors.centerIn: parent

        Repeater {
            model: signalQualityMonitor ? signalQualityMonitor.signalsModel : 0

            ColumnLayout {
                required property int index

                id: indicatorCol

                property var letterCode: String.fromCharCode(65 + index)

                function statusToColor(status) {
                    if (_root.status === 2) {
                        return qgcPal.colorRed
                    } else if (_root.status === 3) {
                        return qgcPal.colorOrange
                    }

                    switch (status) {
                    case 0:
                    case 1:
                        return qgcPal.text
                    case 2:
                        return qgcPal.colorRed
                    case 3:
                        
                    default:
                        return qgcPal.text
                    }
                }

                RowLayout {
                    QGCLabel {
                        font.pointSize:         ScreenTools.mediumFontPointSize
                        text:                   "S" + letterCode + ":"
                        color:                  indicatorCol.statusToColor()
                        Layout.preferredWidth:  20
                    }

                    QGCLabel {
                        text:                   _root.status == 0 ? "-" : signalQualityMonitor.signalsModel[index].snr.toFixed(2)
                        font.pointSize:         ScreenTools.mediumFontPointSize
                        color:                  indicatorCol.statusToColor()
                        Layout.preferredWidth:  40
                    }
                }

                RowLayout {
                    QGCLabel {
                        font.pointSize:         ScreenTools.mediumFontPointSize
                        text:                   "G" + letterCode + ":"
                        color:                  indicatorCol.statusToColor()
                        Layout.preferredWidth:  20
                    }

                    QGCLabel {
                        text:                   _root.status == 0 ? "-" : signalQualityMonitor.signalsModel[index].gain.toFixed(2)
                        font.pointSize:         ScreenTools.mediumFontPointSize
                        color:                  indicatorCol.statusToColor()
                        Layout.preferredWidth:  40
                    }
                }
            }
        }
    }

    MouseArea {
        anchors.fill:   parent
        onClicked: {
            mainWindow.showIndicatorPopup(_root, indicatorPopup)
        }
    }

    Component {
        id: indicatorPopup

        Rectangle {
            width:          mainLayout.width   + mainLayout.anchors.margins * 2
            height:         mainLayout.height  + mainLayout.anchors.margins * 2
            radius:         ScreenTools.defaultFontPixelHeight / 2
            color:          qgcPal.window
            border.color:   qgcPal.text

            ColumnLayout {
                id:                 mainLayout
                anchors.margins:    ScreenTools.defaultFontPixelWidth
                anchors.top:        parent.top
                anchors.right:      parent.right
                spacing:            ScreenTools.defaultFontPixelHeight

                QGCLabel {
                    Layout.alignment:   Qt.AlignCenter
                    text:               qsTr("Signal Quality")
                    font.family:        ScreenTools.demiboldFontFamily
                }

                RowLayout {
                    spacing: ScreenTools.defaultFontPixelWidth

                    ColumnLayout {
                        Repeater {
                            model: signalQualityMonitor ? signalQualityMonitor.dataKeys : 0
                
                            QGCLabel { 
                                required property int index

                                Layout.preferredWidth: 80
                                text:                  signalQualityMonitor.dataKeys[index]
                            }
                        }
                    }
                    
                    ColumnLayout {
                        Repeater {
                            model: signalQualityMonitor ? signalQualityMonitor.dataValues : 0
                
                            QGCLabel { 
                                required property int index

                                property var value: signalQualityMonitor.dataValues[index]
                                property var parsedValue: parseFloat(value)

                                Layout.preferredWidth: 80
                                text:                  parsedValue ? parsedValue.toFixed(2) : value
                            }
                        }
                    }
                }
            }
        }
    }
}
