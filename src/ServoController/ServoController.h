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
    quint16 _displayIndex;
    quint16 _value;
    quint16 _minValue;
    quint16 _maxValue;
    bool    _reversed;

public:
    Q_PROPERTY(QString  name            READ name           NOTIFY nameChanged          FINAL)
    Q_PROPERTY(quint16  index           READ index          NOTIFY indexChanged         FINAL)
    Q_PROPERTY(quint16  displayIndex    READ displayIndex   NOTIFY displayIndexChanged  FINAL)
    Q_PROPERTY(quint16  value           READ value          NOTIFY valueChanged         FINAL)
    Q_PROPERTY(quint16  minValue        READ minValue       NOTIFY minValueChanged      FINAL)
    Q_PROPERTY(quint16  maxValue        READ maxValue       NOTIFY maxValueChanged      FINAL)
    Q_PROPERTY(bool     reversed        READ reversed       NOTIFY reversedChanged      FINAL)

    const QString&  name        ()  const;
    quint16         index       ()  const;
    quint16         displayIndex()  const;
    quint16         value       ()  const;
    quint16         minValue    ()  const;
    quint16         maxValue    ()  const;
    bool            reversed    ()  const;

    void setName                (const QString& name);
    void setIndex               (quint16 index);
    void setDisplayIndex        (quint16 displayIndex);
    void setValue               (quint16 value);
    void setMinValue            (quint16 minValue);
    void setMaxValue            (quint16 maxValue);
    void setReversed            (bool reversed);

signals:
    void nameChanged        ();
    void indexChanged       ();
    void displayIndexChanged();
    void valueChanged       ();
    void minValueChanged    ();
    void maxValueChanged    ();
    void reversedChanged    ();

public:
    Servo(const QString& name, quint16 index, quint16 displayIndex, quint16 startValue, quint16 minValue, quint16 maxValue, bool reversed);
};

class ServoController : public QObject
{
    Q_OBJECT

    bool               _initialized        = false;
    const int          _servoCount         = 4;
    bool               _dropControlEnabled = false;
    const QVector<int> _servoIndexes       = { 7, 8, 9, 10 };
    QVector<bool>      _desiredOpenStates  = { false, false, false, false };
    QVector<int>       _rcAssignments      = { 0, 0, 0, 0 };

public:
    ServoController (MAVLinkProtocol* mavlink, Vehicle* vehicle);
    ~ServoController();

    int32_t getDesiredValue(int32_t servoIndex);

    Q_PROPERTY(QVariantList     servoModel           READ servoModel           NOTIFY servoModelChanged)
    Q_PROPERTY(QVector<bool>    desiredOpenStates    READ desiredOpenStates    WRITE  setDesiredOpenStates      NOTIFY desiredOpenStatesChanged)
    Q_PROPERTY(int              servoCount           READ servoCount           CONSTANT)
    Q_PROPERTY(QVector<int>     rcAssignments        READ rcAssignments        NOTIFY rcAssignmentsChanged)
    Q_PROPERTY(bool             dropControlEnabled   READ dropControlEnabled   NOTIFY dropControlEnabledChanged)

    QVariantList        servoModel         () const;
    QVector<bool>       desiredOpenStates  () const;
    int                 servoCount         () const;
    QVector<int>        rcAssignments      () const;
    bool                dropControlEnabled () const;

    int getDesiredOutput(int outputIndex);

    Q_INVOKABLE void toggleDesiredDropState(int index);

private slots:
    void _mavlinkMessageReceived(const mavlink_message_t& message);
    void _onParametersReadyChanged(bool parametersReady);
    void _onVehicleParameterUpdated(QVariant value);

public slots:
    void close();
    void setDesiredOpenStates(QVector<bool> states);

signals:
    void servoModelChanged         ();
    void desiredOpenStatesChanged  ();
    void rcAssignmentsChanged      ();
    void dropControlEnabledChanged ();
    void desiredValueSet           (int servoIndex, int value);

private:
    void   _handleServoOutputRaw (const mavlink_message_t& msg);
    void   _updateServo          (int index);
    void   _updateRCAssignments  ();
    bool   hasServo              (uint16_t index);
    Servo* findServo             (uint16_t index);

    std::array<uint16_t, 16>    _servoOutputsRaw;
    QVariantList                _servoModel;
    mavlink_servo_output_raw_t  _sor;

    MAVLinkProtocol*            _mavlink            = nullptr;
    Vehicle*                    _vehicle            = nullptr;
};
