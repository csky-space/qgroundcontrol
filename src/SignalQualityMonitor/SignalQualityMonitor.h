#ifndef C_SIGNAL_QUALITY_MONITOR_H
#define C_SIGNAL_QUALITY_MONITOR_H

#include <QObject>
#include <QTimer>
#include <QUrl>
#include <QUdpSocket>
#include <QStringList>

#include "QGCLoggingCategory.h"
#include "QGCToolbox.h"

Q_DECLARE_LOGGING_CATEGORY(SignalQualityMonitorLog)

class SignalQuality : public QObject {
    Q_OBJECT

public:
    enum class SignalStatus : int {
        STATUS_UNINITIALIZED = 0,
        STATUS_OK,
        STATUS_ERROR
    };
    Q_ENUM(SignalStatus)

    SignalQuality() = default;

    Q_PROPERTY(float      snr           READ snr           NOTIFY snrChanged)
    Q_PROPERTY(float      gain          READ gain          NOTIFY gainChanged)
    Q_PROPERTY(int        snrStatus     READ snrStatus     NOTIFY snrStatusChanged)
    Q_PROPERTY(int        gainStatus    READ gainStatus    NOTIFY gainStatusChanged)

    float   snr        () const;
    float   gain       () const;
    int     snrStatus  () const;
    int     gainStatus () const;

    void setSNR        (float value);
    void setGain       (float value);

private:
    float        _snr        = 0.0f;
    float        _gain       = 0.0f;
    SignalStatus _snrStatus  = SignalStatus::STATUS_UNINITIALIZED;
    SignalStatus _gainStatus = SignalStatus::STATUS_UNINITIALIZED;

signals:
    void snrChanged();
    void gainChanged();
    void snrStatusChanged();
    void gainStatusChanged();
};

class SignalQualityMonitor : public QGCTool {
    Q_OBJECT

public:
    enum class SignalMonitorStatus : int {
        STATUS_UNINITIALIZED = 0,
        STATUS_OK,
        STATUS_ERROR,
        STATUS_CONNECTION_TIMEOUT
    };
    Q_ENUM(SignalMonitorStatus)

    SignalQualityMonitor(QGCApplication* app, QGCToolbox* toolbox);
    virtual ~SignalQualityMonitor();

    virtual void setToolbox(QGCToolbox *toolbox);

    Q_PROPERTY(int             status          READ status          NOTIFY statusChanged)
    Q_PROPERTY(QVariantList    signalsModel    READ signalsModel    NOTIFY signalsModelChanged)
    Q_PROPERTY(QStringList     dataKeys        READ dataKeys        NOTIFY dataKeysChanged)
    Q_PROPERTY(QStringList     dataValues      READ dataValues      NOTIFY dataValuesChanged)

    int          status       () const;
    QVariantList signalsModel () const;
    QStringList  dataKeys     () const;
    QStringList  dataValues   () const;

private:
    QUdpSocket* _monitoredSocket;
    QTimer      _dataRecieveTimer;

    SignalMonitorStatus _status = SignalMonitorStatus::STATUS_UNINITIALIZED;
    QVariantList        _signalsModel;
    QStringList         _dataKeys;
    QStringList         _dataValues;

    void _parseMessageJSON(const QByteArray& data);

private slots:
    void _onMonitoredSocketReadyRead();
    void _onDataRecieveTimeout();

signals:
    void statusChanged();
    void signalsModelChanged();
    void dataKeysChanged();
    void dataValuesChanged();
};

#endif // C_SIGNAL_QUALITY_MONITOR_H