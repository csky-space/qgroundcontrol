/// @file ServoController.h

#pragma once

#include <array>
#include <bitset>
#include <cstdint>

#include <QLoggingCategory>
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

    bool          initialized                () const;
    QVariantList  servoModel                 () const;

private slots:
    void _mavlinkMessageReceived(const mavlink_message_t& message);
    void _onParametersReadyChanged(bool parametersReady);

signals:
    void initializedChanged                ();
    void servoModelChanged                 ();

private:
    void   _handleServoOutputRaw      (const mavlink_message_t& msg);
    bool   _hasServo                  (uint16_t index);
    Servo* _findServo                 (uint16_t index);

    MAVLinkProtocol*   _mavlink      = nullptr;
    Vehicle*           _vehicle      = nullptr;

    bool               _initialized;
    QVariantList       _servoModel;

    inline static const int _servoCount = 16;
    std::array<uint16_t, _servoCount> _servoOutputsRaw;
};
