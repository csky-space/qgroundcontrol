/// @file ServoController.h

#pragma once

#include <array>
#include <bitset>
#include <cstdint>

#include <QLoggingCategory>
#include <QObject>
#include <QVariant>
#include <QQmlEngine>

#include "Vehicle.h"
#include "Fact.h"

Q_DECLARE_LOGGING_CATEGORY(ServoControllerLog)

class MavlinkProtocol;
class ParameterManager;

class Servo : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString name               READ name               NOTIFY nameChanged               FINAL)
    Q_PROPERTY(quint16 index              READ index              NOTIFY indexChanged              FINAL)
    Q_PROPERTY(quint16 value              READ value              NOTIFY valueChanged                   )
    Q_PROPERTY(float   normalizedValue    READ normalizedValue    NOTIFY normalizedValueChanged         )

    Q_PROPERTY(Fact*   functionFact       READ functionFact       NOTIFY functionFactChanged            )
    Q_PROPERTY(Fact*   minValueFact       READ minValueFact       NOTIFY minValueFactChanged            )
    Q_PROPERTY(Fact*   maxValueFact       READ maxValueFact       NOTIFY maxValueFactChanged            )
    Q_PROPERTY(Fact*   reversedFact       READ reversedFact       NOTIFY reversedFactChanged            )

public:
    Servo(const QString& name, quint16 index,
          Fact* functionFact, Fact* minValueFact, Fact* maxValueFact, Fact* reversedFact);

    const QString& name            () const;
    quint16        index           () const;
    quint16        value           () const;
    float          normalizedValue () const;

    Fact* functionFact () const;
    Fact* minValueFact () const;
    Fact* maxValueFact () const;
    Fact* reversedFact () const;

    void setValue(quint16 value);

signals:
    void nameChanged            ();
    void indexChanged           ();
    void valueChanged           ();
    void normalizedValueChanged ();

    void functionFactChanged    ();
    void minValueFactChanged    ();
    void maxValueFactChanged    ();
    void reversedFactChanged    ();

public slots:
    void onFunctionParameterChanged(QVariant value);
    void onMinParameterChanged     (QVariant value);
    void onMaxParameterChanged     (QVariant value);
    void onReversedParameterChanged(QVariant value);

private:
    QString  _name;
    quint16  _index;

    Fact*    _functionFact;
    Fact*    _minValueFact;
    Fact*    _maxValueFact;
    Fact*    _reversedFact;

    quint16  _function;
    quint16  _minValue;
    quint16  _maxValue;
    bool     _reversed;

    quint16  _value;
    float    _normalizedValue;
};

class ServoController : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool          initialized    READ initialized    NOTIFY initializedChanged)
    Q_PROPERTY(QVariantList  servoModel     READ servoModel     NOTIFY servoModelChanged)

public:
    ServoController (MAVLinkProtocol* mavlink, Vehicle* vehicle);
    ~ServoController();

    bool          initialized () const;
    QVariantList  servoModel  () const;

private slots:
    void _mavlinkMessageReceived(const mavlink_message_t& message);
    void _onParametersReadyChanged(bool parametersReady);

signals:
    void initializedChanged ();
    void servoModelChanged  ();

private:
    void   _handleServoOutputRaw      (const mavlink_message_t& msg);
    bool   _hasServo                  (uint16_t index);
    Servo* _findServo                 (uint16_t index);

    MAVLinkProtocol*   _mavlink;
    Vehicle*           _vehicle;
    bool               _initialized;

    QVariantList       _servoModel;

    static constexpr int _servoCount = 16;
    std::array<uint16_t, _servoCount> _servoOutputsRaw;
};
