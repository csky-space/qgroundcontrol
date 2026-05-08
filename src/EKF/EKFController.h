/// @file EKFController.h

#pragma once

#include <QLoggingCategory>
#include <cstdint>
#include "Vehicle.h"

Q_DECLARE_LOGGING_CATEGORY(EKFLog)

class MavlinkProtocol;

class EKFController : public QObject
{
    Q_OBJECT
public:
    EKFController(MAVLinkProtocol* mavlink, Vehicle* vehicle);
    ~EKFController();

    //Q_PROPERTY(QmlObjectListModel*  vibrations         READ vibrations        CONSTANT)
    Q_PROPERTY(float velocityVariance READ velocityVariance NOTIFY velocityVarianceChanged)
    Q_PROPERTY(float posHorizVariance READ posHorizVariance NOTIFY posHorizVarianceChanged)
    Q_PROPERTY(float posVertVariance READ posVertVariance NOTIFY posVertVarianceChanged)
    Q_PROPERTY(float compassVariance READ compassVariance NOTIFY compassVarianceChanged)
    Q_PROPERTY(float terrainAltVariance READ terrainAltVariance NOTIFY terrainAltVarianceChanged)

    Q_INVOKABLE float velocityVariance() const;
    Q_INVOKABLE float posHorizVariance() const;
    Q_INVOKABLE float posVertVariance() const;
    Q_INVOKABLE float compassVariance() const;
    Q_INVOKABLE float terrainAltVariance() const;
private slots:
    void    _mavlinkMessageReceived(const mavlink_message_t& message);
signals:
    void velocityVarianceChanged();
    void posHorizVarianceChanged();
    void posVertVarianceChanged();
    void compassVarianceChanged();
    void terrainAltVarianceChanged();
private:
    void        _handleHeartbeat                    (const mavlink_message_t& message);
    void        _handleEKF_STATUS_REPORT     (const mavlink_message_t& message);

    float       _velocity_variance;
    float       _pos_horiz_variance;
    float       _pos_vert_variance;
    float    _compass_variance;
    float    _terrain_alt_variance;

    MAVLinkProtocol*    _mavlink            = nullptr;
    Vehicle*            _vehicle            = nullptr;
};
