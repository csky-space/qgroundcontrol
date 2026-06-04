/// @file RCMappingManager.h

#pragma once

#include "QGCLoggingCategory.h"
#include "Vehicle.h"

#include <QVector>
#include <QStringList>
#include <QVariantList>
#include <QmlObjectListModel.h>

#include "RCRangeMapper.h"

Q_DECLARE_LOGGING_CATEGORY(RCMappingManagerLog)

class Joystick;

class RCMappingManager : public QObject {
    Q_OBJECT

public:
    static const int cMaxRcChannels = Vehicle::cMaxRcChannels;

    RCMappingManager(Joystick* joystick);
    ~RCMappingManager();

    void init();

    float getMappedValue(int mappingIndex, float defaultValue = 1000.0f, bool allowInvalidated = true);

    Q_PROPERTY(int                  mappingsCount         READ mappingsCount         CONSTANT)
    Q_PROPERTY(bool                 isAdvancedMode        READ isAdvancedMode        NOTIFY isAdvancedModeChanged)
    Q_PROPERTY(QStringList          sourcesList           READ sourcesList           NOTIFY sourcesListChanged)
    Q_PROPERTY(QVector<QStringList> optionsList           READ optionsList           NOTIFY optionsListChanged)
    Q_PROPERTY(QVector<int>         sourceIndexes         READ sourceIndexes         NOTIFY sourceIndexesChanged)
    Q_PROPERTY(QVector<int>         optionIndexes         READ optionIndexes         NOTIFY optionIndexesChanged)
    Q_PROPERTY(QVariantList         mappersModel          READ mappersModel          NOTIFY mappersModelChanged)
    Q_PROPERTY(QVector<bool>        manualSetFlags        READ manualSetFlags        NOTIFY manualSetFlagsChanged)
    Q_PROPERTY(QVector<int>         calibrationSeconds    READ calibrationSeconds    NOTIFY calibrationSecondsChanged)

    int                  mappingsCount      () const;
    bool                 isAdvancedMode     () const;
    QStringList          sourcesList        () const;
    QVector<QStringList> optionsList        () const;
    QVector<int>         sourceIndexes      () const;
    QVector<int>         optionIndexes      () const;
    QVariantList         mappersModel       () const;
    QVector<bool>        manualSetFlags     () const;
    QVector<int>         calibrationSeconds () const;

    Q_INVOKABLE void setAdvancedMode  (bool state);

    Q_INVOKABLE void setSourceIndex   (int mappingIndex, int sourceIndex);
    Q_INVOKABLE void setOptionIndex   (int mappingIndex, int optionIndex);
    Q_INVOKABLE void setManualSetFlag (int mappingIndex, bool state);
    Q_INVOKABLE void resetMapping     (int mappingIndex, bool save = true);
    Q_INVOKABLE void resetToDefault   (bool save = true);

    Q_INVOKABLE void calibrateMapping       (int mappingIndex);
    Q_INVOKABLE void stopCalibratingMapping (int mappingIndex);

    Q_INVOKABLE void setInMin (int mappingIndex, float value);
    Q_INVOKABLE void setInMax (int mappingIndex, float value);
    Q_INVOKABLE void setOutMin (int mappingIndex, float value);
    Q_INVOKABLE void setOutMax (int mappingIndex, float value);
    Q_INVOKABLE void setIsReversed (int mappingIndex, bool state);
    Q_INVOKABLE void setIsClamping (int mappingIndex, bool state);

private:
    Joystick*        _joystick;

    bool             _isAdvancedMode = false;
    QVector<bool>    _manualSetFlags;
    QVector<QTimer*> _calibrationTimers;
    QVector<int>     _calibrationSeconds;

    QStringList          _sourcesList;
    QVector<QStringList> _optionsList;
    QVector<int>         _sourceIndexes;
    QVector<int>         _optionIndexes;
    QVariantList         _mappersModel;

    void _saveSettings();
    void _loadSettings();

    void _updateMappings      ();
    void _updateMapping       (int32_t mappingIndex);
    void _updateMappingRanges (int32_t mappingIndex);
    void _updateMappedValue   (int32_t mappingIndex);
    bool _isMappingValid      (int32_t mappingIndex);

    void _setMappingRawValue (int32_t mappingIndex, float value);

signals:
    void isAdvancedModeChanged     ();
    void sourcesListChanged        ();
    void optionsListChanged        ();
    void sourceIndexesChanged      ();
    void optionIndexesChanged      ();
    void mappersModelChanged       ();
    void manualSetFlagsChanged     ();
    void calibrationSecondsChanged ();

public slots:
    void onAxisValuesPolled   (QVector<int> values);
    void onButtonValuesPolled (QVector<int> values);

private:
    static const char* _settingsGroup;
    static const char* _isAdvancedModeKey; 
    static const char* _sourceIndexKey;
    static const char* _optionIndexKey;
    static const char* _manualSetFlagKey;
    static const char* _inMinKey;
    static const char* _inMaxKey;
    static const char* _outMinKey;
    static const char* _outMaxKey;
    static const char* _isReversedKey;
    static const char* _isClampingKey;
};
