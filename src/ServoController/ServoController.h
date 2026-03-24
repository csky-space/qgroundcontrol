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

class ServoMode : public QObject {
    Q_OBJECT

    QString _name;
    bool    _checked;
public:
    Q_PROPERTY(QString  name    READ name                       NOTIFY nameChanged)
    Q_PROPERTY(bool     checked READ checked WRITE setChecked   NOTIFY checkedChanged)

    const QString& name() const;
    bool checked() const;

    void setChecked(bool checked);
signals:
    void nameChanged();
    void checkedChanged();
public:
    ServoMode(const QString& name, bool checked);
};

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
public:
    ServoController (MAVLinkProtocol* mavlink, Vehicle* vehicle);
    ~ServoController();

    Q_PROPERTY(QVariantList servoModel      READ servoModel         NOTIFY servoModelChanged        CONSTANT)
    Q_PROPERTY(QVariantList servoDropModes  READ servoDropModes     NOTIFY servoDropModesChanged    CONSTANT)
    Q_PROPERTY(quint16      dropIndex       READ dropIndex          NOTIFY dropIndexChanged         CONSTANT)

    QVariantList        servoModel      ()  const;
    QVariantList        servoDropModes  ()  const;
    quint16             dropIndex       ()  const;

    static constexpr uint16_t servoDropFunction = 254;
private slots:
    void _mavlinkMessageReceived(const mavlink_message_t& message);
public slots:
    void drop();
signals:
    void servoModelChanged      ();
    void servoDropModesChanged  ();
    void dropIndexChanged       ();
private:
    void _handleServoOutputRaw  (const mavlink_message_t& msg);

    void _increaseDropIndex     (uint16_t incSize);

    QMetaObject::Connection     _cavetationTimerConnection;
    QTimer*                     _cavetationTimer;
    std::function<void()>       _sendCommandLongCallback;
    ServoMode*                  _allMode;
    ServoMode*                  _2Mode;
    ServoMode*                  _1Mode;
    ServoMode*                  _cavetMode;
    std::array<uint16_t, 16>    _servoOutputsRaw;
    QVariantList                _servoDropModes;
    QVariantList                _servoModel;
    mavlink_servo_output_raw_t  _sor;

    MAVLinkProtocol*            _mavlink            = nullptr;
    Vehicle*                    _vehicle            = nullptr;
    uint16_t                    _dropIndex;
};
