#ifndef C_DATA_LOSS_TESTER_H
#define C_DATA_LOSS_TESTER_H

#include <QLoggingCategory>
#include <QObject>
#include <QVariant>
#include <QQmlEngine>
#include <QTcpSocket>

#include "QGCToolbox.h"

Q_DECLARE_LOGGING_CATEGORY(DataLossTesterLog)

class DataLossTester : public QGCTool {
    Q_OBJECT

public:
    DataLossTester(QGCApplication* app, QGCToolbox* toolbox);
    virtual ~DataLossTester();

    virtual void setToolbox(QGCToolbox *toolbox);

    Q_PROPERTY(QString address            READ address            WRITE setAddress           NOTIFY addressChanged)
    Q_PROPERTY(quint16 port               READ port               WRITE setPort              NOTIFY portChanged)
    Q_PROPERTY(quint32 packetsPerTest     READ packetsPerTest     WRITE setPacketsPerTest    NOTIFY packetsPerTestChanged)
    Q_PROPERTY(quint32 sendInterval       READ sendInterval       WRITE setSendInterval      NOTIFY sendIntervalChanged)
    Q_PROPERTY(quint32 waitInterval       READ waitInterval       WRITE setWaitInterval      NOTIFY waitIntervalChanged)
    Q_PROPERTY(bool    shouldWait         READ shouldWait         WRITE setShouldWait        NOTIFY shouldWaitChanged)
    Q_PROPERTY(qint32  packetsSent        READ packetsSent                                   NOTIFY packetsSentChanged)
    Q_PROPERTY(qint32  packetsReceived    READ packetsReceived                               NOTIFY packetsReceivedChanged)
    Q_PROPERTY(qint32  packetsLost        READ packetsLost                                   NOTIFY packetsLostChanged)
    Q_PROPERTY(bool    connected          READ connected                                     NOTIFY connectedChanged)

    QString address() const;
    quint16 port() const;
    quint32 packetsPerTest() const;
    quint32 sendInterval() const;
    quint32 waitInterval() const;
    bool    shouldWait() const;
    qint32  packetsSent() const;
    qint32  packetsReceived() const;
    qint32  packetsLost() const;
    bool    connected() const;

    void setAddress(QString address);
    void setPort(quint16 port);
    void setPacketsPerTest(quint32 count);
    void setSendInterval(quint32 ms);
    void setWaitInterval(quint32 ms);
    void setShouldWait(bool state);

    Q_INVOKABLE void startSendingMessages();

signals:
    void addressChanged();
    void portChanged();
    void packetsPerTestChanged();
    void sendIntervalChanged();
    void waitIntervalChanged();
    void shouldWaitChanged();
    void packetsSentChanged();
    void packetsReceivedChanged();
    void packetsLostChanged();
    void connectedChanged();

private slots:
    void _onReadyRead();
    void _onSocketDisconnected();
    void _onSocketError(QAbstractSocket::SocketError error);
    void _onSendMessageTimeout();
    void _onWaitResponseTimeout();

private:
    QTimer _sendMessagesTimer;
    QTimer _waitResponseTimer;
    QTimer _disconnectTimer;
    bool   _shouldWaitForResponse = false;
    bool   _isWaitingForResponse = false;
    quint32 _sendInterval = 1000;
    quint32 _waitResponseInterval = 250;
    quint32 _disconnectInterval = 1000;
    quint32 _packetsPerTest = 5;
    quint32 _testPacketsSent = 0;

    QTcpSocket* _socket        = nullptr;
    QByteArray  _rxBuffer;
    bool        _connected     = false;    

    QString _address           = "127.0.0.1";
    quint16 _port              = 9000;
    qint32  _packetsSent       = 0;
    qint32  _packetsReceived   = 0;
    qint32  _packetsLost       = 0;

    QVector<QByteArray> _cachedMessages;
    quint32             _currentMessageIndex = 0;

    void _cacheMessages();
    void _connectToServer();
    void _disconnectFromServer();

    void _sendNextMessage();
    void _stopSendingMessages();
};

#endif // C_DATA_LOSS_TESTER_H
