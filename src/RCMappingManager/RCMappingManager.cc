#include "RCMappingManager.h"
#include "RCMappingSourceType.h"
#include "QGCApplication.h"

#include <QQmlEngine>

#include "Vehicle.h"
#include "ServoController/ServoController.h"
#include "Joystick.h"

Q_LOGGING_CATEGORY(RCMappingManagerLog, "RCMappingManagerLog")

const char* RCMappingManager::_settingsGroup = "RCMappingManager";
const char* RCMappingManager::_isAdvancedModeKey = "MappingManagerAdvancedMode"; 
const char* RCMappingManager::_sourceIndexKey =  "Mapping%1SourceIndex";
const char* RCMappingManager::_optionIndexKey =  "Mapping%1OptionIndex";
const char* RCMappingManager::_manualSetFlagKey = "Mapping%1ManualSet";
const char* RCMappingManager::_inMinKey = "Mapping%1InMin";
const char* RCMappingManager::_inMaxKey = "Mapping%1InMax";
const char* RCMappingManager::_outMinKey = "Mapping%1OutMin";
const char* RCMappingManager::_outMaxKey = "Mapping%1OutMax";
const char* RCMappingManager::_isReversedKey = "Mapping%1IsReversed";
const char* RCMappingManager::_isClampingKey = "Mapping%1IsClamping";

RCMappingManager::RCMappingManager(Joystick* joystick)
    : _joystick(joystick)
    , _sourceIndexes(cMaxRcChannels)
    , _optionIndexes(cMaxRcChannels) {
    QQmlEngine::setObjectOwnership(this, QQmlEngine::CppOwnership);
}

RCMappingManager::~RCMappingManager() {
    for (int32_t i = 0; i < _mappersModel.size(); i++) {
        RCRangeMapper* mapper = _mappersModel[i].value<RCRangeMapper*>();
        mapper->deleteLater();
    }
    for (int32_t i = 0; i < _calibrationTimers.size(); i++) {
        _calibrationTimers[i]->deleteLater();
    }
}

void RCMappingManager::init() {
    // initialize flags
    for (int32_t i = 0; i < RCMappingManager::cMaxRcChannels; i++) {
        _manualSetFlags.push_back(false);
    }

    // Initialize sources
    int32_t sourcesCount = static_cast<int32_t>(RCMappingSourceType::COUNT);
    for (int32_t sourceIndex = 0; sourceIndex < sourcesCount; sourceIndex++) {
        _sourcesList.push_back(toString(static_cast<RCMappingSourceType>(sourceIndex)));
    }
    _sourcesList.push_back("Unset");

    // Initialize options
    _optionsList.resize(sourcesCount + 1);
    for (int32_t axisIndex = 0; axisIndex < _joystick->axisCount(); axisIndex++) {
        _optionsList[static_cast<int32_t>(RCMappingSourceType::JoystickAxis)].push_back(QString("Axis") + QString::number(axisIndex));
    }
    for (int32_t buttonIndex = 0; buttonIndex < _joystick->totalButtonCount(); buttonIndex++) {
        _optionsList[static_cast<int32_t>(RCMappingSourceType::JoystickButton)].push_back(QString("Button") + QString::number(buttonIndex));
    }

    // initialize mappers
    for (int32_t i = 0; i < RCMappingManager::cMaxRcChannels; i++) {
        RCRangeMapper* mapper = new RCRangeMapper();
        QQmlEngine::setObjectOwnership(mapper, QQmlEngine::CppOwnership);
        _mappersModel.append(QVariant::fromValue(mapper));
    }

    // init calibration timers
    for (int32_t i = 0; i < RCMappingManager::cMaxRcChannels; i++) {
        _calibrationSeconds.push_back(0);
        _calibrationTimers.push_back(new QTimer());
        _calibrationTimers[i]->setInterval(1000);
        connect(_calibrationTimers[i], &QTimer::timeout, [this, i]() {
            if (_calibrationSeconds[i] > 0) {
                _calibrationSeconds[i] -= 1;
            }
            if(_calibrationSeconds[i] == 0) {
                _calibrationTimers[i]->stop();
                _saveSettings();
            }
            emit this->calibrationSecondsChanged();
        });
    }

    emit sourcesListChanged();
    emit optionsListChanged();
    emit mappersModelChanged();

    resetToDefault(false);

    _loadSettings();

    connect(_joystick, &Joystick::axisValuesPolled, this, &RCMappingManager::onAxisValuesPolled);
    connect(_joystick, &Joystick::buttonValuesPolled, this, &RCMappingManager::onButtonValuesPolled);
}

float RCMappingManager::getMappedValue(int mappingIndex, float defaultValue, bool allowInvalidated) {
    if (mappingIndex < 0 || mappingIndex >= _mappersModel.size()) {
        qCWarning(RCMappingManagerLog) << "getMappingValue: Mapping index out of range:" << mappingIndex;
        return defaultValue;
    }

    RCRangeMapper* mapper = _mappersModel[mappingIndex].value<RCRangeMapper*>();

    if (mapper->isValueMapped() || allowInvalidated) {
        return mapper->mappedValue();
    }

    return defaultValue;
}

int RCMappingManager::mappingsCount() const {
    return _mappersModel.size();
}

bool RCMappingManager::isAdvancedMode() const {
    return _isAdvancedMode;
}

QStringList RCMappingManager::sourcesList() const {
    return _sourcesList;
}

QVector<QStringList> RCMappingManager::optionsList() const {
    return _optionsList;
}

QVector<int> RCMappingManager::sourceIndexes() const {
    return _sourceIndexes;
}

QVector<int> RCMappingManager::optionIndexes() const {
    return _optionIndexes;
}

QVariantList RCMappingManager::mappersModel() const {
    return _mappersModel;
}

QVector<bool> RCMappingManager::manualSetFlags() const {
    return _manualSetFlags;
}

QVector<int> RCMappingManager::calibrationSeconds() const {
    return _calibrationSeconds;
}

void RCMappingManager::setAdvancedMode(bool state) {
    _isAdvancedMode = state;
    emit isAdvancedModeChanged();
}

void RCMappingManager::setSourceIndex(int mappingIndex, int sourceIndex) {
    if (mappingIndex < 0 || mappingIndex >= _mappersModel.size()) {
        qCWarning(RCMappingManagerLog) << "setSourceIndex: Mapping index out of range:" << mappingIndex;
        return;
    }

    if (sourceIndex < 0 || sourceIndex >= _sourcesList.size()) {
        qCWarning(RCMappingManagerLog) << "setSourceIndex: Source index out of range:" << sourceIndex;
        return;
    }

    _sourceIndexes[mappingIndex] = sourceIndex;

    emit sourceIndexesChanged();
    emit optionIndexesChanged();

    _updateMapping(mappingIndex);
}

void RCMappingManager::setOptionIndex(int mappingIndex, int optionIndex) {
    if (mappingIndex < 0 || mappingIndex >= _mappersModel.size()) {
        qCWarning(RCMappingManagerLog) << "setOptionIndex: Mapping index out of range:" << mappingIndex;
        return;
    }

    if (optionIndex < 0 || optionIndex >= _optionsList[_sourceIndexes[mappingIndex]].size()) {
        qCWarning(RCMappingManagerLog) << "setOptionIndex: Option index out of range:" << optionIndex;
        return;
    }

    _optionIndexes[mappingIndex] = optionIndex;

    emit optionIndexesChanged();

    _updateMapping(mappingIndex);
}

void RCMappingManager::setManualSetFlag(int mappingIndex, bool state) {
    if (mappingIndex < 0 || mappingIndex >= _mappersModel.size()) {
        qCWarning(RCMappingManagerLog) << "setManualSetFlag: Mapping index out of range:" << mappingIndex;
        return;
    }

    _manualSetFlags[mappingIndex] = state;

    emit manualSetFlagsChanged();

    if (!state) {
        resetMapping(mappingIndex);
    } else {
        _saveSettings();
    }
}

void RCMappingManager::resetToDefault(bool save) {
    for (int32_t mappingIndex = 0; mappingIndex < cMaxRcChannels; mappingIndex++) {
        if (mappingIndex < 8) {
            _sourceIndexes[mappingIndex] = static_cast<int32_t>(RCMappingSourceType::JoystickAxis);
            _optionIndexes[mappingIndex] = mappingIndex;
        }
        else if (mappingIndex < 16) {
            _sourceIndexes[mappingIndex] = static_cast<int32_t>(RCMappingSourceType::JoystickButton);
            _optionIndexes[mappingIndex] = mappingIndex - 8;
        }
        else {
            _sourceIndexes[mappingIndex] = static_cast<int32_t>(RCMappingSourceType::COUNT);
            _optionIndexes[mappingIndex] = -1;
        }
        resetMapping(mappingIndex, false);
    }

    emit sourceIndexesChanged();
    emit optionIndexesChanged();

    if (save) {
        _saveSettings();
    }
}

void RCMappingManager::_saveSettings() {
    QSettings settings;

    settings.beginGroup(_settingsGroup);
    settings.setValue(QString(_isAdvancedModeKey), _isAdvancedMode);

    for (int mappingIndex = 0; mappingIndex < cMaxRcChannels; mappingIndex++) {
        settings.setValue(QString(_sourceIndexKey).arg(mappingIndex), _sourceIndexes[mappingIndex]);
        settings.setValue(QString(_optionIndexKey).arg(mappingIndex), _optionIndexes[mappingIndex]);
        settings.setValue(QString(_manualSetFlagKey).arg(mappingIndex), _manualSetFlags[mappingIndex]);
    }

    for (int mappingIndex = 0; mappingIndex < cMaxRcChannels; mappingIndex++) {
        RCRangeMapper* mapper = _mappersModel[mappingIndex].value<RCRangeMapper*>();
        settings.setValue(QString(_inMinKey).arg(mappingIndex), mapper->inMin());
        settings.setValue(QString(_inMaxKey).arg(mappingIndex), mapper->inMax());
        settings.setValue(QString(_outMinKey).arg(mappingIndex), mapper->outMin());
        settings.setValue(QString(_outMaxKey).arg(mappingIndex), mapper->outMax());
        settings.setValue(QString(_isReversedKey).arg(mappingIndex), mapper->isReversed());
        settings.setValue(QString(_isClampingKey).arg(mappingIndex), mapper->IsClamping());
    }
}

void RCMappingManager::_loadSettings() {
    QSettings settings;

    _isAdvancedMode = settings.value(QString(_isAdvancedModeKey), false).toBool();
    emit isAdvancedModeChanged();

    for (int mappingIndex = 0; mappingIndex < cMaxRcChannels; mappingIndex++) {
        _sourceIndexes[mappingIndex] = settings.value(QString(_sourceIndexKey).arg(mappingIndex), _sourceIndexes[mappingIndex]).toInt();
        _optionIndexes[mappingIndex] = settings.value(QString(_optionIndexKey).arg(mappingIndex), _optionIndexes[mappingIndex]).toInt();
        _manualSetFlags[mappingIndex] = settings.value(QString(_manualSetFlagKey).arg(mappingIndex), _manualSetFlags[mappingIndex]).toBool();
    }

    for (int mappingIndex = 0; mappingIndex < cMaxRcChannels; mappingIndex++) {
        RCRangeMapper* mapper = _mappersModel[mappingIndex].value<RCRangeMapper*>();
        float inMin = settings.value(QString(_inMinKey).arg(mappingIndex), mapper->inMin()).toFloat();
        float inMax = settings.value(QString(_inMaxKey).arg(mappingIndex), mapper->inMax()).toFloat();
        float outMin = settings.value(QString(_outMinKey).arg(mappingIndex), mapper->outMin()).toFloat();
        float outMax = settings.value(QString(_outMaxKey).arg(mappingIndex), mapper->outMax()).toFloat();
        mapper->setRanges(inMin, inMax, outMin, outMax);

        bool isReversed = settings.value(QString(_isReversedKey).arg(mappingIndex), mapper->isReversed()).toBool();
        mapper->setIsReversed(isReversed);

        bool IsClamping = settings.value(QString(_isClampingKey).arg(mappingIndex), mapper->IsClamping()).toBool();
        mapper->setIsClamping(IsClamping);
    }

    _updateMappings();
}

void RCMappingManager::_updateMappings() {
    for (int32_t mappingIndex = 0; mappingIndex < cMaxRcChannels; mappingIndex++) {
        _updateMapping(mappingIndex);
    }
}

void RCMappingManager::_updateMapping(int32_t mappingIndex) {
    if (mappingIndex < 0 || mappingIndex >= cMaxRcChannels) {
        qCWarning(RCMappingManagerLog) << "_updateMapping: Mapping index out of range:" << mappingIndex;
        return;
    }

    if (_manualSetFlags[mappingIndex]) {
        _updateMappingRanges(mappingIndex);
    }
    else {
        resetMapping(mappingIndex);
    }
}

void RCMappingManager::_updateMappingRanges(int32_t mappingIndex) {
    if (mappingIndex < 0 || mappingIndex >= cMaxRcChannels) {
        qCWarning(RCMappingManagerLog) << "_updateMappingRanges: Mapping index out of range:" << mappingIndex;
        return;
    }

    RCRangeMapper* mapper = _mappersModel[mappingIndex].value<RCRangeMapper*>();

    if (!_isMappingValid(mappingIndex)) {
        mapper->setRanges(INT16_MIN, INT16_MAX, 1000, 2000);
        mapper->resetValue();
    }
    else {
        switch(static_cast<RCMappingSourceType>(_sourceIndexes[mappingIndex])) {
            case RCMappingSourceType::JoystickButton: {
                mapper->setRanges(0, 1, 1000, 2000);
                break;
            }
            case RCMappingSourceType::JoystickAxis: {
                mapper->setRanges(INT16_MIN, INT16_MAX, 1000, 2000);
                break;
            }
            default:
                mapper->setRanges(INT16_MIN, INT16_MAX, 1000, 2000);
                mapper->resetValue();
                return;
        }
    }

    _saveSettings();
}

void RCMappingManager::calibrateMapping(int mappingIndex) {
    if (mappingIndex < 0 || mappingIndex >= cMaxRcChannels) {
        qCWarning(RCMappingManagerLog) << "calibrateMapping: Mapping index out of range:" << mappingIndex;
        return;
    }

    _calibrationSeconds[mappingIndex] = 8;
    emit calibrationSecondsChanged();

    _manualSetFlags[mappingIndex] = true;
    emit manualSetFlagsChanged();

    RCRangeMapper* mapper = _mappersModel[mappingIndex].value<RCRangeMapper*>();
    mapper->setInRange(std::numeric_limits<float>::infinity(), -std::numeric_limits<float>::infinity());

    _calibrationTimers[mappingIndex]->start();
}

void RCMappingManager::stopCalibratingMapping(int mappingIndex) {
    if (mappingIndex < 0 || mappingIndex >= cMaxRcChannels) {
        qCWarning(RCMappingManagerLog) << "stopCalibratingMapping: Mapping index out of range:" << mappingIndex;
        return;
    }

    _calibrationTimers[mappingIndex]->stop();

    _calibrationSeconds[mappingIndex] = 0;
    emit calibrationSecondsChanged();

    _saveSettings();
}

void RCMappingManager::setInMin(int mappingIndex, float value) {
    if (mappingIndex < 0 || mappingIndex >= cMaxRcChannels) {
        qCWarning(RCMappingManagerLog) << "setInMin: Mapping index out of range:" << mappingIndex;
        return;
    }

    RCRangeMapper* mapper = _mappersModel[mappingIndex].value<RCRangeMapper*>();
    mapper->setInMin(value);

    _saveSettings();
}

void RCMappingManager::setInMax(int mappingIndex, float value) {
    if (mappingIndex < 0 || mappingIndex >= cMaxRcChannels) {
        qCWarning(RCMappingManagerLog) << "setInMax: Mapping index out of range:" << mappingIndex;
        return;
    }

    RCRangeMapper* mapper = _mappersModel[mappingIndex].value<RCRangeMapper*>();
    mapper->setInMax(value);

    _saveSettings();
}

void RCMappingManager::setOutMin(int mappingIndex, float value) {
    if (mappingIndex < 0 || mappingIndex >= cMaxRcChannels) {
        qCWarning(RCMappingManagerLog) << "setOutMin: Mapping index out of range:" << mappingIndex;
        return;
    }

    RCRangeMapper* mapper = _mappersModel[mappingIndex].value<RCRangeMapper*>();
    mapper->setOutMin(value);

    _saveSettings();
}

void RCMappingManager::setOutMax(int mappingIndex, float value) {
    if (mappingIndex < 0 || mappingIndex >= cMaxRcChannels) {
        qCWarning(RCMappingManagerLog) << "setOutMax: Mapping index out of range:" << mappingIndex;
        return;
    }

    RCRangeMapper* mapper = _mappersModel[mappingIndex].value<RCRangeMapper*>();
    mapper->setOutMax(value);

    _saveSettings();
}

void RCMappingManager::setIsReversed(int mappingIndex, bool state) {
    if (mappingIndex < 0 || mappingIndex >= cMaxRcChannels) {
        qCWarning(RCMappingManagerLog) << "setIsReversed: Mapping index out of range:" << mappingIndex;
        return;
    }

    RCRangeMapper* mapper = _mappersModel[mappingIndex].value<RCRangeMapper*>();
    mapper->setIsReversed(state);

    _saveSettings();
}

void RCMappingManager::setIsClamping(int mappingIndex, bool state) {
    if (mappingIndex < 0 || mappingIndex >= cMaxRcChannels) {
        qCWarning(RCMappingManagerLog) << "setIsClamping: Mapping index out of range:" << mappingIndex;
        return;
    }

    RCRangeMapper* mapper = _mappersModel[mappingIndex].value<RCRangeMapper*>();
    mapper->setIsClamping(state);

    _saveSettings();
}

void RCMappingManager::resetMapping(int mappingIndex, bool save) {
    if (mappingIndex < 0 || mappingIndex >= cMaxRcChannels) {
        qCWarning(RCMappingManagerLog) << "resetMapping: Mapping index out of range:" << mappingIndex;
        return;
    }

    RCRangeMapper* mapper = _mappersModel[mappingIndex].value<RCRangeMapper*>();
    mapper->setIsReversed(false);
    mapper->setIsClamping(true);

    _updateMappingRanges(mappingIndex);

    mapper->resetValue();

    _manualSetFlags[mappingIndex] = false;
    emit manualSetFlagsChanged();

    _calibrationTimers[mappingIndex]->stop();
    _calibrationSeconds[mappingIndex] = 0;
    emit calibrationSecondsChanged();

    if (save) {
        _saveSettings();
    }
}

void RCMappingManager::_updateMappedValue(int32_t mappingIndex) {
    if (mappingIndex < 0 || mappingIndex >= cMaxRcChannels) {
        qCWarning(RCMappingManagerLog) << "_updateMappedValue: Mapping index out of range:" << mappingIndex;
        return;
    }

    RCRangeMapper* mapper = _mappersModel[mappingIndex].value<RCRangeMapper*>();
    mapper->remapValue();
}

bool RCMappingManager::_isMappingValid(int32_t mappingIndex) {
    return (
        mappingIndex >= 0 &&
        mappingIndex < cMaxRcChannels &&
        _sourceIndexes[mappingIndex] >= 0 &&
        _sourceIndexes[mappingIndex] < static_cast<int32_t>(RCMappingSourceType::COUNT) &&
        _optionIndexes[mappingIndex] >= 0 &&
        _optionIndexes[mappingIndex] < _optionsList[_sourceIndexes[mappingIndex]].size()
    ); 
}

void RCMappingManager::_setMappingRawValue(int32_t mappingIndex, float value) {
    if (mappingIndex < 0 || mappingIndex >= cMaxRcChannels) {
        qCWarning(RCMappingManagerLog) << "_setMappingRawValue: Mapping index out of range:" << mappingIndex;
        return;
    }

    RCRangeMapper* mapper = _mappersModel[mappingIndex].value<RCRangeMapper*>();

    if (_calibrationSeconds[mappingIndex] > 0) {
        mapper->expandInputToFit(value);
    }
    
    mapper->setRawValue(value);
}

void RCMappingManager::onAxisValuesPolled(QVector<int> values) {
    for (int mappingIndex = 0; mappingIndex < cMaxRcChannels; mappingIndex++) {
        if (_sourceIndexes[mappingIndex] != static_cast<int32_t>(RCMappingSourceType::JoystickAxis)) continue;
        int optionIndex = _optionIndexes[mappingIndex];
        if (optionIndex < 0 || optionIndex >= values.size()) continue;
        _setMappingRawValue(mappingIndex, static_cast<float>(values[optionIndex]));
    }
}

void RCMappingManager::onButtonValuesPolled(QVector<int> values) {
    for (int mappingIndex = 0; mappingIndex < cMaxRcChannels; mappingIndex++) {
        if (_sourceIndexes[mappingIndex] != static_cast<int32_t>(RCMappingSourceType::JoystickButton)) continue;
        int optionIndex = _optionIndexes[mappingIndex];
        if (optionIndex < 0 || optionIndex >= values.size()) continue;
        _setMappingRawValue(mappingIndex, static_cast<float>(values[optionIndex]));
    }
}
