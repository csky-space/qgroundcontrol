#ifndef C_AIRLINK_TELEMETRY_H
#define C_AIRLINK_TELEMETRY_H

#include <QObject>

class QThread;

namespace CSKY {
class AirlinkTelemetry : public QObject{
    Q_OBJECT
public:
    explicit AirlinkTelemetry(QObject* parent = nullptr);

    //void _configureUdpSettings();
    //void _sendLoginMsgToAirLink();
    //bool _stillConnecting();
    //void _setConnectFlag(bool connect);
private:
    QThread* telemetryThread;
    //std::atomic<bool> _needToConnect{false};
signals:
public slots:
    //void connect();
private slots:
};
}

#endif
