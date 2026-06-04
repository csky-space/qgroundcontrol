import QtQuick                      2.11
import QtQuick.Controls             2.4
import QtQuick.Dialogs              1.3
import QtQuick.Layouts              1.11

import QGroundControl               1.0
import QGroundControl.Palette       1.0
import QGroundControl.Controls      1.0
import QGroundControl.ScreenTools   1.0
import QGroundControl.Controllers   1.0
import QGroundControl.FactSystem    1.0
import QGroundControl.FactControls  1.0

ColumnLayout {
    visible: rcMappingManager ? true : false
    spacing: ScreenTools.defaultFontPixelHeight * 0.5

    property var activeJoystick:   QGroundControl.joystickManager.activeJoystick
    property var rcMappingManager: activeJoystick ? activeJoystick.rcMappingManager : null

    QGCCheckBox {
        id:      advancedModeCheck
        text:    qsTr("Advanced Mode")
        checked: rcMappingManager.isAdvancedMode

        onClicked: {
            rcMappingManager.setAdvancedMode(checked);
        }

        Connections {
            target: rcMappingManager
            onIsAdvancedModeChanged: {
                advancedModeCheck.checked = rcMappingManager.isAdvancedMode;
            }
        }
    }

    Loader {
        sourceComponent: rcMappingManager.isAdvancedMode ? advancedSettings : baseSettings
    }

    Component {
        id: baseSettings

        ColumnLayout {
            Repeater {
                model: rcMappingManager ? rcMappingManager.mappersModel : 0
                width: parent.width

                Rectangle {
                    required property int index

                    color:          Qt.rgba(0, 0, 0, 0)
                    border.color:   Qt.rgba(1, 1, 1, 1)
                    border.width:   1
                    radius:         ScreenTools.defaultFontPixelWidth * 0.5
                    implicitHeight: baseMappingRow.implicitHeight + 12
                    implicitWidth:  baseMappingRow.implicitWidth + 12

                    property var modelData: rcMappingManager.mappersModel[index]

                    RowLayout {
                        id:           baseMappingRow
                        spacing:      5
                        anchors.fill: parent
                        anchors.margins: 6

                        property int sourceIndex:    rcMappingManager.sourceIndexes[index];
                        property int optionIndex:    rcMappingManager.optionIndexes[index];
                        property bool isUnset:       sourceIndex < 0 || sourceIndex >= 2
                        property bool isValidSource: rcMappingManager.sourcesList[sourceIndex] != undefined
                        property bool isValidOption: isValidSource && rcMappingManager.optionsList[sourceIndex][optionIndex] != undefined
                        property bool isValid:       !isUnset && isValidSource && isValidOption

                        QGCLabel {
                            id:                    mappingLabel
                            text:                  (index + 1).toString()
                            Layout.preferredWidth: 16
                            Layout.alignment:      Qt.AlignVCenter
                        }

                        QGCComboBox {
                            width:                 ScreenTools.defaultFontPixelWidth * 26
                            model:                 sourcesModel ? sourcesModel : []
                            currentIndex:          sourceIndex !== undefined ? sourceIndex : -1
                            sizeToContents:        true
                            Layout.preferredWidth: 140
                            enabled:               !baseMappingRow.calibrating

                            property var sourcesModel: rcMappingManager.sourcesList
                            property var sourceIndex:  rcMappingManager.sourceIndexes[index]

                            onActivated: (newIndex) => {
                                rcMappingManager.setSourceIndex(index, newIndex);
                            }
                        }

                        QGCComboBox {
                            width:                 ScreenTools.defaultFontPixelWidth * 26
                            model:                 optionsModel ? optionsModel : []
                            currentIndex:          optionIndex !== undefined ? optionIndex : -1
                            sizeToContents:        true
                            Layout.preferredWidth: 140
                            enabled:               !baseMappingRow.isUnset && !baseMappingRow.calibrating

                            property var sourceIndex:  rcMappingManager.sourceIndexes[index]
                            property var optionsModel: rcMappingManager.optionsList[sourceIndex]
                            property var optionIndex:  rcMappingManager.optionIndexes[index]

                            onActivated: (newIndex) => {
                                rcMappingManager.setOptionIndex(index, newIndex);
                            }
                        }
                        
                        Rectangle {
                            height:           28
                            width:            200
                            color:            modelData.isValueMapped ? "#333333" : "#404020"
                            Layout.alignment: Qt.AlignVCenter
                            radius:           4

                            Rectangle {
                                color:          modelData.isValueMapped ? "#20af20" : "#c0c020"
                                anchors.left:   parent.left
                                anchors.top:    parent.top
                                anchors.bottom: parent.bottom
                                width:          modelData.normalizedValue * parent.width
                                radius:         4
                            }

                            QGCLabel {
                                text:             modelData.mappedValue.toFixed(1)
                                anchors.centerIn: parent
                            }
                        }

                        QGCCheckBox {
                            id:      baseMappingInverseCheck
                            text:    qsTr("Inverse")
                            checked: modelData.isReversed
                            enabled: baseMappingRow.isValid && !baseMappingRow.calibrating

                            onClicked: {
                                rcMappingManager.setIsReversed(mappingIndex, checked)
                            }

                            Connections {
                                target: modelData
                                onIsReversedChanged: {
                                    baseMappingInverseCheck.checked = modelData.isReversed;
                                }
                            }
                        }

                        QGCButton {
                            id:    resetMappingButton
                            text:  qsTr("Reset")
                            width: ScreenTools.defaultFontPixelWidth * 15

                            onClicked:  {
                                rcMappingManager.resetMapping(index);
                            }
                        }
                    }
                }
            }
        }
    }

    Component {
        id: advancedSettings

        ColumnLayout {
            Repeater {
                model: rcMappingManager ? rcMappingManager.mappersModel : 0
                width: parent.width

                Rectangle {
                    required property int index

                    id:             mappingRect
                    color:          Qt.rgba(0, 0, 0, 0)
                    border.color:   Qt.rgba(1, 1, 1, 1)
                    border.width:   1
                    radius:         ScreenTools.defaultFontPixelWidth * 0.5
                    implicitHeight: advancedMappingRow.implicitHeight + 12
                    implicitWidth:  advancedMappingRow.implicitWidth + 12

                    property var modelData: rcMappingManager.mappersModel[index]

                    RowLayout {
                        id:           advancedMappingRow
                        spacing:      5
                        anchors.fill: parent
                        anchors.margins: 6

                        property int sourceIndex:    rcMappingManager.sourceIndexes[index];
                        property int optionIndex:    rcMappingManager.optionIndexes[index];
                        property bool calibrating:   rcMappingManager.calibrationSeconds[index] > 0
                        property bool isUnset:       sourceIndex < 0 || sourceIndex >= 2
                        property bool isValidSource: rcMappingManager.sourcesList[sourceIndex] != undefined
                        property bool isValidOption: isValidSource && rcMappingManager.optionsList[sourceIndex][optionIndex] != undefined
                        property bool isValid:       !isUnset && isValidSource && isValidOption

                        QGCLabel {
                            id:                    mappingLabel
                            text:                  (index + 1).toString()
                            Layout.preferredWidth: 16
                            Layout.alignment:      Qt.AlignVCenter
                        }
                        ColumnLayout {
                            id:               mappingColumn
                            Layout.alignment: Qt.AlignVCenter

                            QGCComboBox {
                                id:                    sourceCombo
                                width:                 ScreenTools.defaultFontPixelWidth * 26
                                model:                 sourcesModel ? sourcesModel : []
                                currentIndex:          sourceIndex !== undefined ? sourceIndex : -1
                                sizeToContents:        true
                                Layout.preferredWidth: 140
                                enabled:               !advancedMappingRow.calibrating

                                property var sourcesModel: rcMappingManager.sourcesList
                                property var sourceIndex:  rcMappingManager.sourceIndexes[index]

                                onActivated: (newIndex) => {
                                    rcMappingManager.setSourceIndex(index, newIndex);
                                }
                            }
                            QGCComboBox {
                                id:                    optionCombo
                                width:                 ScreenTools.defaultFontPixelWidth * 26
                                model:                 optionsModel ? optionsModel : []
                                currentIndex:          optionIndex !== undefined ? optionIndex : -1
                                sizeToContents:        true
                                Layout.preferredWidth: 140
                                enabled:               !advancedMappingRow.isUnset && !advancedMappingRow.calibrating

                                property var sourceIndex:  rcMappingManager.sourceIndexes[index]
                                property var optionsModel: rcMappingManager.optionsList[sourceIndex]
                                property var optionIndex:  rcMappingManager.optionIndexes[index]

                                onActivated: (newIndex) => {
                                    rcMappingManager.setOptionIndex(index, newIndex);
                                }
                            }
                        }
                        ColumnLayout {
                            RowLayout {
                                QGCTextField {
                                    id:      inMinValue
                                    text:    modelData.inMin
                                    enabled: advancedMappingRow.isValid && !advancedMappingRow.calibrating

                                    onEditingFinished: {
                                        let value = parseFloat(text);
                                        rcMappingManager.setInMin(index, isNaN(value) ? 0.0 : value);
                                        rcMappingManager.setManualSetFlag(index, true);
                                    }
                                }

                                Rectangle {
                                    height:           28
                                    width:            200
                                    color:            modelData.isValueMapped ? "#333333" : "#404020"
                                    Layout.alignment: Qt.AlignVCenter
                                    radius:           4

                                    Rectangle {
                                        color:          modelData.isValueMapped ? "#20af20" : "#c0c020"
                                        anchors.left:   parent.left
                                        anchors.top:    parent.top
                                        anchors.bottom: parent.bottom
                                        width:          scale * parent.width
                                        radius:         4

                                        property real scale: modelData.isReversed ? (1.0 - modelData.normalizedValue) : modelData.normalizedValue
                                    }

                                    QGCLabel {
                                        text:             modelData.rawValue.toString()
                                        anchors.centerIn: parent
                                    }
                                }

                                QGCTextField {
                                    id:      inMaxValue
                                    text:    modelData.inMax
                                    enabled: advancedMappingRow.isValid && !advancedMappingRow.calibrating

                                    onEditingFinished: {
                                        let value = parseFloat(text);
                                        rcMappingManager.setInMax(index, isNaN(value) ? 0.0 : value);
                                        rcMappingManager.setManualSetFlag(index, true);
                                    }
                                }
                            }

                            RowLayout {
                                QGCTextField {
                                    id:      outMinValue
                                    text:    modelData.outMin
                                    enabled: advancedMappingRow.isValid && !advancedMappingRow.calibrating

                                    onEditingFinished: {
                                        let value = parseFloat(text);
                                        rcMappingManager.setOutMin(index, isNaN(value) ? 0.0 : value);
                                        rcMappingManager.setManualSetFlag(index, true);
                                    }
                                }

                                Rectangle {
                                    height:           28
                                    width:            200
                                    color:            modelData.isValueMapped ? "#333333" : "#404020"
                                    Layout.alignment: Qt.AlignVCenter
                                    radius:           4

                                    Rectangle {
                                        color:          modelData.isValueMapped ? "#20af20" : "#c0c020"
                                        anchors.left:   parent.left
                                        anchors.top:    parent.top
                                        anchors.bottom: parent.bottom
                                        width:          modelData.normalizedValue * parent.width
                                        radius:         4
                                    }

                                    QGCLabel {
                                        text:             modelData.mappedValue.toFixed(1)
                                        anchors.centerIn: parent
                                    }
                                }

                                QGCTextField {
                                    id:      outMaxValue
                                    text:    modelData.outMax
                                    enabled: advancedMappingRow.isValid && !advancedMappingRow.calibrating

                                    onEditingFinished: {
                                        let value = parseFloat(text);
                                        rcMappingManager.setOutMax(index, isNaN(value) ? 0.0 : value);
                                        rcMappingManager.setManualSetFlag(index, true);
                                    }
                                }
                            }
                        }

                        ColumnLayout {
                            QGCCheckBox {
                                id:      mappingInverseCheck
                                text:    qsTr("Inverse")
                                checked: modelData.isReversed
                                enabled: advancedMappingRow.isValid && !advancedMappingRow.calibrating

                                onClicked: {
                                    rcMappingManager.setIsReversed(index, checked)  
                                }

                                Connections {
                                    target: modelData
                                    onIsReversedChanged: {
                                        mappingInverseCheck.checked = modelData.isReversed;
                                    }
                                }
                            }

                            QGCCheckBox {
                                id:      mappingClampCheck
                                text:    qsTr("Clamp")
                                checked: modelData.IsClamping
                                enabled: advancedMappingRow.isValid && !advancedMappingRow.calibrating

                                onClicked: {
                                    rcMappingManager.setIsClamping(index, checked)  
                                }

                                Connections {
                                    target: modelData
                                    onIsClampingChanged: {
                                        mappingClampCheck.checked = modelData.IsClamping;
                                    }
                                }
                            }

                            QGCCheckBox {
                                id:      mappingManualCheck
                                text:    qsTr("Manual configuration")
                                checked: rcMappingManager.manualSetFlags[index]
                                enabled: advancedMappingRow.isValid && !advancedMappingRow.calibrating

                                onClicked: {
                                    rcMappingManager.setManualSetFlag(index, checked);
                                }

                                Connections {
                                    target: rcMappingManager
                                    onManualSetFlagsChanged: {
                                        mappingManualCheck.checked = rcMappingManager.manualSetFlags[index];
                                    }
                                }
                            }
                        }

                        QGCButton {
                            id:                    calibrateMappingButton
                            text:                  !advancedMappingRow.calibrating ? qsTr("Calibrate") : qsTr("Stop Calibration (" + seconds + "s)")
                            width:                 ScreenTools.defaultFontPixelWidth * 15
                            Layout.preferredWidth: 140
                            enabled:               advancedMappingRow.isValid

                            property var seconds: rcMappingManager.calibrationSeconds[index]

                            onClicked:  {
                                if (!advancedMappingRow.calibrating) {
                                    rcMappingManager.calibrateMapping(index);
                                }
                                else {
                                    rcMappingManager.stopCalibratingMapping(index);
                                }
                            }
                        }

                        QGCButton {
                            id:    resetMappingButton
                            text:  qsTr("Reset")
                            width: ScreenTools.defaultFontPixelWidth * 15

                            onClicked:  {
                                rcMappingManager.resetMapping(index);
                            }
                        }
                    }
                }
            }
        }
    }
    RowLayout {
        QGCButton {
            id:         resetMappingsButton
            text:       qsTr("Reset Mappings")
            width:      ScreenTools.defaultFontPixelWidth * 15
            onClicked:  {
                rcMappingManager.resetToDefault();
            }
        }
    }
}
