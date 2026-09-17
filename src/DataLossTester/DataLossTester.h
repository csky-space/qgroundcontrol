#ifndef C_DATA_LOSS_TESTER_H
#define C_DATA_LOSS_TESTER_H

#include <functional>

#include <QLoggingCategory>
#include <QObject>
#include <QVariant>
#include <QQmlEngine>
#include <QTcpSocket>
#include <QTimer>
#include <QVector>

#include "QGCToolbox.h"
#include "QGCMAVLink.h"

Q_DECLARE_LOGGING_CATEGORY(DataLossTesterLog)

class TestMessageInfo : public QObject {
    Q_OBJECT

public:
    TestMessageInfo(quint32 id, QString name, std::function<void(mavlink_message_t&)> packCallback);

    Q_PROPERTY(quint32 id             READ id                                          CONSTANT)
    Q_PROPERTY(QString name           READ name                                        CONSTANT)
    Q_PROPERTY(quint32 sent           READ sent           NOTIFY sentChanged                   )
    Q_PROPERTY(quint32 received       READ received       NOTIFY receivedChanged               )
    Q_PROPERTY(quint32 seqLost        READ seqLost        NOTIFY seqLostChanged                )
    Q_PROPERTY(quint32 timeoutLost    READ timeoutLost    NOTIFY timeoutLostChanged            )

    quint32 id() const;
    QString name() const;
    quint32 sent() const;
    quint32 received() const;
    quint32 seqLost() const;
    quint32 timeoutLost() const;

    void ResetStatictics();
    void IncrementSent();
    void IncrementReceived();
    void IncrementSeqLost();
    void IncrementTimeoutLost();

    void PackMessage(mavlink_message_t& msg);

signals:
    void sentChanged();
    void receivedChanged();
    void seqLostChanged();
    void timeoutLostChanged();

private:
    quint32 _id = -1;
    QString _name = "UNDEFINED";
    quint32 _sent = 0;
    quint32 _received = 0;
    quint32 _seqLost = 0;
    quint32 _timeoutLost = 0;

    std::function<void(mavlink_message_t&)> _packCallback = nullptr;
};

class DataLossTester : public QGCTool {
    Q_OBJECT

public:
    DataLossTester(QGCApplication* app, QGCToolbox* toolbox);
    virtual ~DataLossTester();

    virtual void setToolbox(QGCToolbox *toolbox);

    Q_PROPERTY(QString       address                  READ address                  WRITE setAddress           NOTIFY addressChanged)
    Q_PROPERTY(quint16       port                     READ port                     WRITE setPort              NOTIFY portChanged)
    Q_PROPERTY(quint32       packetsPerTest           READ packetsPerTest           WRITE setPacketsPerTest    NOTIFY packetsPerTestChanged)
    Q_PROPERTY(quint32       sendInterval             READ sendInterval             WRITE setSendInterval      NOTIFY sendIntervalChanged)
    Q_PROPERTY(quint32       waitInterval             READ waitInterval             WRITE setWaitInterval      NOTIFY waitIntervalChanged)
    Q_PROPERTY(bool          shouldWait               READ shouldWait               WRITE setShouldWait        NOTIFY shouldWaitChanged)
    Q_PROPERTY(qint32        packetsSent              READ packetsSent                                         NOTIFY packetsSentChanged)
    Q_PROPERTY(qint32        packetsReceived          READ packetsReceived                                     NOTIFY packetsReceivedChanged)
    Q_PROPERTY(qint32        packetsLost              READ packetsLost                                         NOTIFY packetsLostChanged)
    Q_PROPERTY(qint32        waitTimeouts             READ waitTimeouts                                        NOTIFY waitTimeoutsChanged)
    Q_PROPERTY(qint32        packetsLostBySequence    READ packetsLostBySequence                               NOTIFY packetsLostBySequenceChanged)
    Q_PROPERTY(bool          connected                READ connected                                           NOTIFY connectedChanged)
    Q_PROPERTY(QVariantList  messagesModel            READ messagesModel                                       NOTIFY messagesModelChanged)

    QString      address() const;
    quint16      port() const;
    quint32      packetsPerTest() const;
    quint32      sendInterval() const;
    quint32      waitInterval() const;
    bool         shouldWait() const;
    qint32       packetsSent() const;
    qint32       packetsReceived() const;
    qint32       packetsLost() const;
    qint32       waitTimeouts() const;
    qint32       packetsLostBySequence() const;
    bool         connected() const;
    QVariantList messagesModel() const;

    void setAddress(QString address);
    void setPort(quint16 port);
    void setPacketsPerTest(quint32 count);
    void setSendInterval(quint32 ms);
    void setWaitInterval(quint32 ms);
    void setShouldWait(bool state);

    Q_INVOKABLE void startSendingMessages();
    Q_INVOKABLE void stopSendingMessages();

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
    void waitTimeoutsChanged();
    void packetsLostBySequenceChanged();
    void connectedChanged();
    void messagesModelChanged();

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
    quint32 _sendInterval = 10;
    quint32 _waitResponseInterval = 200;
    quint32 _disconnectInterval = 500;
    quint32 _packetsPerTest = 1000;
    quint32 _testPacketsSent = 0;

    QTcpSocket* _socket        = nullptr;
    QByteArray  _rxBuffer;
    bool        _connected     = false;

    QString _address           = "127.0.0.1";
    quint16 _port              = 9000;
    qint32  _packetsSent       = 0;
    qint32  _packetsReceived   = 0;
    qint32  _packetsLost       = 0;
    qint32  _waitTimeouts      = 0;
    qint32  _packetsLostBySequence = 0;

    // --- sequence tracking ---
    quint8  _lastReceivedSeq       = 0;
    bool    _hasLastReceivedSeq    = false;

    quint8  _expectedResponseSeq = 0;
    quint8  _lastSentSeq         = 0;
    quint32 _testPacketsReceived = 0;
    bool    _endReconciled       = false;

    uint32_t _msgTypeIndex = 0;

    QVariantList _messagesModel;

    void _connectToServer();
    void _disconnectFromServer();

    void _sendNextMessage();
    void _stopSendingMessages();
    void _reconcileSequenceAtEnd();
};

#endif // C_DATA_LOSS_TESTER_H