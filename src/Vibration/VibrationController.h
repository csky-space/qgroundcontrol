/// @file VibrationController.h

#pragma once

#include <QLoggingCategory>
#include <cstdint>
#include "Vehicle.h"
#include "QmlObjectListModel.h"

Q_DECLARE_LOGGING_CATEGORY(VibrationLog)

class MavlinkProtocol;

class VibrationController : public QObject
{
    Q_OBJECT
public:
    VibrationController(MAVLinkProtocol* mavlink, Vehicle* vehicle);
    ~VibrationController();

    //Q_PROPERTY(QmlObjectListModel*  vibrations         READ vibrations        CONSTANT)
    Q_PROPERTY(float xVibration READ xVibration NOTIFY xVibrationChanged)
    Q_PROPERTY(float yVibration READ yVibration NOTIFY yVibrationChanged)
    Q_PROPERTY(float zVibration READ zVibration NOTIFY zVibrationChanged)
    Q_PROPERTY(unsigned int clipping1 READ clipping1 NOTIFY clipping1Changed)
    Q_PROPERTY(unsigned int clipping2 READ clipping2 NOTIFY clipping2Changed)
    Q_PROPERTY(unsigned int clipping3 READ clipping3 NOTIFY clipping3Changed)

    Q_PROPERTY(Fact* xVibrationFact READ xVibrationFact NOTIFY xVibrationFactChanged)
    Q_PROPERTY(Fact* yVibrationFact READ yVibrationFact NOTIFY yVibrationFactChanged)
    Q_PROPERTY(Fact* zVibrationFact READ zVibrationFact NOTIFY zVibrationFactChanged)

    Q_INVOKABLE float xVibration() const;
    Q_INVOKABLE float yVibration() const;
    Q_INVOKABLE float zVibration() const;
    Q_INVOKABLE unsigned int clipping1() const;
    Q_INVOKABLE unsigned int clipping2() const;
    Q_INVOKABLE unsigned int clipping3() const;

    Q_INVOKABLE Fact* xVibrationFact() const;
    Q_INVOKABLE Fact* yVibrationFact() const;
    Q_INVOKABLE Fact* zVibrationFact() const;
private slots:
    void    _mavlinkMessageReceived(const mavlink_message_t& message);
signals:
    void xVibrationChanged();
    void yVibrationChanged();
    void zVibrationChanged();
    void clipping1Changed();
    void clipping2Changed();
    void clipping3Changed();

    void xVibrationFactChanged();
    void yVibrationFactChanged();
    void zVibrationFactChanged();
private:
    void        _handleHeartbeat                    (const mavlink_message_t& message);
    void        _handleVibrationManagerInformation     (const mavlink_message_t& message);
    float       _xVibration;
    float       _yVibration;
    float       _zVibration;
    uint32_t    _clp1;
    uint32_t    _clp2;
    uint32_t    _clp3;

    MAVLinkProtocol*    _mavlink            = nullptr;
    Vehicle*            _vehicle            = nullptr;
};
