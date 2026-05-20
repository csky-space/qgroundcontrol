/// @file ServoController.h

#pragma once

#include <array>
#include <bitset>

#include <QLoggingCategory>
#include <cstdint>
#include "Vehicle.h"
#include "QmlObjectListModel.h"

Q_DECLARE_LOGGING_CATEGORY(ServoControllerLog)

class MavlinkProtocol;

class Servo : public QObject {
    Q_OBJECT

    QString _name;
    quint16 _index;
    quint16 _function;
    quint16 _value;
    quint16 _minValue;
    quint16 _maxValue;
    bool    _reversed;
    float   _normalizedValue;

public:
    Servo(const QString& name, quint16 index, quint16 displayIndex, quint16 startValue, quint16 minValue, quint16 maxValue, bool reversed);

    Q_PROPERTY(QString name               READ name               NOTIFY nameChanged               FINAL)
    Q_PROPERTY(quint16 index              READ index              NOTIFY indexChanged              FINAL)
    Q_PROPERTY(quint16 function           READ function           NOTIFY functionChanged                )
    Q_PROPERTY(quint16 value              READ value              NOTIFY valueChanged                   )
    Q_PROPERTY(quint16 minValue           READ minValue           NOTIFY minValueChanged                )
    Q_PROPERTY(quint16 maxValue           READ maxValue           NOTIFY maxValueChanged                )
    Q_PROPERTY(bool    reversed           READ reversed           NOTIFY reversedChanged                )
    Q_PROPERTY(float   normalizedValue    READ normalizedValue    NOTIFY normalizedValueChanged         )

    const QString& name            () const;
    quint16        index           () const;
    quint16        function        () const;
    quint16        value           () const;
    quint16        minValue        () const;
    quint16        maxValue        () const;
    bool           reversed        () const;
    float          normalizedValue () const;

    void setFunction        (quint16 displayIndex);
    void setValue           (quint16 value);
    void setMinValue        (quint16 minValue);
    void setMaxValue        (quint16 maxValue);
    void setReversed        (bool reversed);

signals:
    void nameChanged            ();
    void indexChanged           ();
    void functionChanged        ();
    void valueChanged           ();
    void minValueChanged        ();
    void maxValueChanged        ();
    void reversedChanged        ();
    void normalizedValueChanged ();

public slots:
    void onFunctionParameterChanged(QVariant value);
    void onMinParameterChanged(QVariant value);
    void onMaxParameterChanged(QVariant value);
    void onReversedParameterChanged(QVariant value);
};

class ServoController : public QObject
{
    Q_OBJECT

public:
    ServoController (MAVLinkProtocol* mavlink, Vehicle* vehicle);
    ~ServoController();

    Q_PROPERTY(bool          initialized                READ initialized                NOTIFY initializedChanged)
    Q_PROPERTY(QVariantList  servoModel                 READ servoModel                 NOTIFY servoModelChanged)
    Q_PROPERTY(QStringList   servoAssignmentsModel      READ servoAssignmentsModel      NOTIFY servoAssignmentsModelChanged)
    Q_PROPERTY(QVector<int>  servoAssignments           READ servoAssignments           NOTIFY servoAssignmentsChanged)
    Q_PROPERTY(QVector<bool> servoStates                READ servoStates                NOTIFY servoStatesChanged)
    Q_PROPERTY(QStringList   dropConfigValidationErrors READ dropConfigValidationErrors NOTIFY dropConfigValidationErrorsChanged)

    bool          initialized                () const;
    QVariantList  servoModel                 () const;
    QStringList   servoAssignmentsModel      () const;
    QVector<int>  servoAssignments           () const;
    QVector<bool> servoStates                () const;
    QStringList   dropConfigValidationErrors () const;

    Q_INVOKABLE void setDropState(int servoIndex, bool state);
    Q_INVOKABLE void toggleDesiredDropState(int servoIndex);
    Q_INVOKABLE void setServoGroup(int servoIndex, int groupIndex);
    Q_INVOKABLE void resetGroupAssignments();
    Q_INVOKABLE void validateDropConfiguration();

    void saveSettings();
    void loadSettings();

private slots:
    void _mavlinkMessageReceived(const mavlink_message_t& message);
    void _onParametersReadyChanged(bool parametersReady);
    void _onSetServoListCommandAccepted(QVector<int> servoIndexes, bool desiredState);

signals:
    void initializedChanged                ();
    void servoModelChanged                 ();
    void servoAssignmentsModelChanged      ();
    void servoAssignmentsChanged           ();
    void servoStatesChanged                ();
    void dropConfigValidationErrorsChanged ();

private:
    void   _handleServoOutputRaw      (const mavlink_message_t& msg);
    bool   _hasServo                  (uint16_t index);
    Servo* _findServo                 (uint16_t index);

    MAVLinkProtocol*   _mavlink      = nullptr;
    Vehicle*           _vehicle      = nullptr;
    const QVector<int> _servoIndexes = { 7, 8, 9, 10 };
    static const int   _servoCount   = 16;

    bool               _initialized;
    QVariantList       _servoModel;
    QStringList        _servoAssignmentsModel;
    QVector<int>       _servoAssignments;
    QVector<bool>      _servoStates;
    QStringList        _dropConfigValidationErrors;

    std::array<uint16_t, _servoCount> _servoOutputsRaw;

    static void _handleDoSetServoListCommandAck(void* resultHandlerData, int compId, const mavlink_command_ack_t& ack, Vehicle::MavCmdResultFailureCode_t failureCode);

    static const char* _settingsGroup;
    static const char* _servoAssignmentKey;
};
