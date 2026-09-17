#include "DataLossTester.h"

#include <QQmlContext>
#include <QQmlEngine>
#include <QSettings>
#include <QQuickWindow>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include <QHostAddress>
#include <QElapsedTimer>
#include <QCoreApplication>
#include <QEventLoop>

#include "ScreenToolsController.h"
#include "QGCToolbox.h"
#include "QGCCorePlugin.h"
#include "QGCOptions.h"
#include "Settings/SettingsManager.h"

QGC_LOGGING_CATEGORY(DataLossTesterLog, "DataLossTesterLog")

TestMessageInfo::TestMessageInfo(quint32 id, QString name, std::function<void(mavlink_message_t&)> packCallback):
    _id(id), _name(name), _packCallback(packCallback) {}

quint32 TestMessageInfo::id() const {
    return _id;
}

QString TestMessageInfo::name() const {
    return _name;
}

quint32 TestMessageInfo::sent() const {
    return _sent;
}

quint32 TestMessageInfo::received() const {
    return _received;
}

quint32 TestMessageInfo::seqLost() const {
    return _seqLost;
}

quint32 TestMessageInfo::timeoutLost() const {
    return _timeoutLost;
}

void TestMessageInfo::ResetStatictics() {
    _sent = 0;
    _received = 0;
    _seqLost = 0;
    _timeoutLost = 0;

    emit sentChanged();
    emit receivedChanged();
    emit seqLostChanged();
    emit timeoutLostChanged();
}

void TestMessageInfo::IncrementSent() {
    _sent++;
    emit sentChanged();
}

void TestMessageInfo::IncrementReceived() {
    _received++;
    emit receivedChanged();
}

void TestMessageInfo::IncrementSeqLost() {
    _seqLost++;
    emit seqLostChanged();
}

void TestMessageInfo::IncrementTimeoutLost() {
    _timeoutLost++;
    emit timeoutLostChanged();
}

void TestMessageInfo::PackMessage(mavlink_message_t& msg) {
    if(!_packCallback) {
        return;
    }
    _packCallback(msg);
}

DataLossTester::DataLossTester(QGCApplication* app, QGCToolbox* toolbox) :
    QGCTool(app, toolbox) {}

DataLossTester::~DataLossTester() {}

void DataLossTester::setToolbox(QGCToolbox *toolbox) {
    QGCTool::setToolbox(toolbox);
    QQmlEngine::setObjectOwnership(this, QQmlEngine::CppOwnership);
    qmlRegisterUncreatableType<DataLossTester>("QGroundControl.dataLossTester", 1, 0,
                                               "DataLossTester", "Reference only");

    _sendMessagesTimer.setSingleShot(false);
    _waitResponseTimer.setSingleShot(true);
    _disconnectTimer.setSingleShot(true);

    connect(&_sendMessagesTimer, &QTimer::timeout, this, &DataLossTester::_onSendMessageTimeout);
    connect(&_waitResponseTimer, &QTimer::timeout, this, &DataLossTester::_onWaitResponseTimeout);
    connect(&_disconnectTimer,   &QTimer::timeout, this, &DataLossTester::_disconnectFromServer);

    static const uint8_t systemId    = 1;
    static const uint8_t componentId = 1;

    _messagesModel.append(QVariant::fromValue(TestMessageInfo(0, "HEARTBEAT", [](mavlink_message_t& msg) {
        mavlink_msg_heartbeat_pack(
            systemId, componentId, &msg,
            MAV_TYPE_QUADROTOR,
            MAV_AUTOPILOT_GENERIC,
            MAV_MODE_FLAG_MANUAL_INPUT_ENABLED | MAV_MODE_FLAG_CUSTOM_MODE_ENABLED,
            0x01020304u,
            MAV_STATE_ACTIVE
            );
    })));

    _messagesModel.append(QVariant::fromValue(TestMessageInfo(0, "COMMAND_LONG — ARM/DISARM", [](mavlink_message_t& msg) {
        mavlink_msg_command_long_pack(
            systemId, componentId, &msg,
            1, 1,
            MAV_CMD_COMPONENT_ARM_DISARM,
            0,
            1.0f, 0, 0, 0, 0, 0, 0
        );
    })));

    _messagesModel.append(QVariant::fromValue(TestMessageInfo(0, "COMMAND_LONG — TAKEOFF", [](mavlink_message_t& msg) {
        mavlink_msg_command_long_pack(
            systemId, componentId, &msg,
            1, 1,
            MAV_CMD_NAV_TAKEOFF,
            0,
            0.0f, 0, 0, 0, 0, 0, 10.0f
        );
    })));

    _messagesModel.append(QVariant::fromValue(TestMessageInfo(0, "COMMAND_INT — WAYPOINT", [](mavlink_message_t& msg) {
        mavlink_msg_command_int_pack(
            systemId, componentId, &msg,
            1, 1,
            MAV_FRAME_GLOBAL_RELATIVE_ALT_INT,
            MAV_CMD_NAV_WAYPOINT,
            0, 1,
            0, 0, 0, 0,
            473977000,
            85450000,
            100.0f
        );
    })));

    _messagesModel.append(QVariant::fromValue(TestMessageInfo(0, "SET_MODE", [](mavlink_message_t& msg) {
        mavlink_msg_set_mode_pack(
            systemId, componentId, &msg,
            1,
            MAV_MODE_FLAG_CUSTOM_MODE_ENABLED,
            4
        );
    })));

    _messagesModel.append(QVariant::fromValue(TestMessageInfo(0, "BATTERY_STATUS", [](mavlink_message_t& msg) {
        uint16_t voltages[10] = {12400, 12390, 12410, 12395,
                                 12405, 12398, 12402, 12393,
                                 12407, 12401};
        uint16_t voltages_ext[4] = {0, 0, 0, 0};
        mavlink_msg_battery_status_pack(
            systemId, componentId, &msg,
            0,
            MAV_BATTERY_FUNCTION_ALL,
            MAV_BATTERY_TYPE_LIPO,
            2500,
            voltages,
            1500,
            1234,
            5678,
            87,
            0,
            MAV_BATTERY_CHARGE_STATE_OK,
            voltages_ext,
            0,
            0
        );
    })));

    _messagesModel.append(QVariant::fromValue(TestMessageInfo(0, "SYSTEM_TIME", [](mavlink_message_t& msg) {
        mavlink_msg_system_time_pack(
            systemId, componentId, &msg,
            1234567890123456ULL,
            987654321u
        );
    })));

    _messagesModel.append(QVariant::fromValue(TestMessageInfo(0, "STATUSTEXT", [](mavlink_message_t& msg) {
        const char text[] = "DataLossTester payload ABCDEFG 0123456789";
        mavlink_msg_statustext_pack(
            systemId, componentId, &msg,
            MAV_SEVERITY_INFO,
            text,
            1,
            0
        );
    })));

    _messagesModel.append(QVariant::fromValue(TestMessageInfo(0, "ATTITUDE", [](mavlink_message_t& msg) {
        mavlink_msg_attitude_pack(
            systemId, componentId, &msg,
            1000,
            0.1f, -0.2f, 1.57f,
            0.01f, -0.02f, 0.5f
        );
    })));

    _messagesModel.append(QVariant::fromValue(TestMessageInfo(0, "VFR_HUD", [](mavlink_message_t& msg) {
        mavlink_msg_vfr_hud_pack(
            systemId, componentId, &msg,
            12.5f,
            13.7f,
            270,
            55,
            120.5f,
            1.2f
        );
    })));

    _messagesModel.append(QVariant::fromValue(TestMessageInfo(0, "PARAM_VALUE", [](mavlink_message_t& msg) {
        char paramId[16] = "TEST_PARAM_1234";
        mavlink_msg_param_value_pack(
            systemId, componentId, &msg,
            paramId,
            42.42f,
            MAV_PARAM_TYPE_REAL32,
            128,
            5
        );
    })));

    _messagesModel.append(QVariant::fromValue(TestMessageInfo(0, "ENCAPSULATED_DATA", [](mavlink_message_t& msg) {
        uint8_t data[253];
        for (int i = 0; i < 253; ++i) {
            data[i] = static_cast<uint8_t>((i * 7 + 1) & 0xFF);
        }
        if (data[252] == 0) data[252] = 0xA5;
        mavlink_msg_encapsulated_data_pack(
            systemId, componentId, &msg,
            0,
            data
        );
    })));
}

// ---------------------------------------------------------------------------
// Getters
// ---------------------------------------------------------------------------
QString      DataLossTester::address() const { return _address; }
quint16      DataLossTester::port() const { return _port; }
quint32      DataLossTester::packetsPerTest() const { return _packetsPerTest; }
quint32      DataLossTester::sendInterval() const { return _sendInterval; }
quint32      DataLossTester::waitInterval() const { return _waitResponseInterval; }
bool         DataLossTester::shouldWait() const { return _shouldWaitForResponse; }
qint32       DataLossTester::packetsSent() const { return _packetsSent; }
qint32       DataLossTester::packetsReceived() const { return _packetsReceived; }
qint32       DataLossTester::packetsLost() const { return _packetsLost; }
qint32       DataLossTester::waitTimeouts() const { return _waitTimeouts; }
qint32       DataLossTester::packetsLostBySequence() const { return _packetsLostBySequence; }
bool         DataLossTester::connected() const { return _connected; }
QVariantList DataLossTester::messagesModel() const {return _messagesModel; }

// ---------------------------------------------------------------------------
// Setters
// ---------------------------------------------------------------------------
void DataLossTester::setAddress(QString address) {
    if (_address == address) return;
    _address = address;
    emit addressChanged();
}

void DataLossTester::setPort(quint16 port) {
    if (_port == port) return;
    _port = port;
    emit portChanged();
}

void DataLossTester::setPacketsPerTest(quint32 count) {
    if (_packetsPerTest == count) return;
    _packetsPerTest = count;
    emit packetsPerTestChanged();
}

void DataLossTester::setSendInterval(quint32 ms) {
    if (_sendInterval == ms) return;
    _sendInterval = ms;
    emit sendIntervalChanged();
}

void DataLossTester::setWaitInterval(quint32 ms) {
    if (_waitResponseInterval == ms) return;
    _waitResponseInterval = ms;
    emit waitIntervalChanged();
}

void DataLossTester::setShouldWait(bool state) {
    if (_shouldWaitForResponse == state) return;
    _shouldWaitForResponse = state;
    emit shouldWaitChanged();
}

// ---------------------------------------------------------------------------
// Public API
// ---------------------------------------------------------------------------
void DataLossTester::startSendingMessages() {
    if (_connected) {
        qCWarning(DataLossTesterLog) << "Is already sending messages";
        return;
    }

    _connectToServer();
    if (!_connected) return;

    // Сбрасываем счётчики теста
    _packetsSent            = 0;
    _packetsReceived        = 0;
    _testPacketsSent        = 0;
    _packetsLost            = 0;
    _waitTimeouts           = 0;
    _packetsLostBySequence  = 0;
    _lastReceivedSeq        = 0;
    _hasLastReceivedSeq     = false;
    _isWaitingForResponse   = false;
    _expectedResponseSeq    = 0;
    _testPacketsReceived    = 0;
    _endReconciled          = false;
    _msgTypeIndex           = 0;
    _lastSentSeq            = 0;

    // Ключевой момент: сбрасываем счётчик seq у канала — MAVLink начнёт с 0.
    mavlink_get_channel_status(MAVLINK_COMM_0)->current_tx_seq = 0;

    emit packetsSentChanged();
    emit packetsReceivedChanged();
    emit packetsLostChanged();
    emit waitTimeoutsChanged();
    emit packetsLostBySequenceChanged();

    _sendMessagesTimer.setInterval(_sendInterval);
    _waitResponseTimer.setInterval(_waitResponseInterval);
    _disconnectTimer.setInterval(_disconnectInterval);

    _disconnectTimer.stop();
    _sendMessagesTimer.start();
}

void DataLossTester::stopSendingMessages() {
    _stopSendingMessages();
    _disconnectFromServer();
}

// ---------------------------------------------------------------------------
// Connection
// ---------------------------------------------------------------------------
void DataLossTester::_connectToServer() {
    if (_connected) return;

    if (_address.isEmpty()) {
        qCWarning(DataLossTesterLog) << "Empty address:" << _address;
        return;
    }

    if (!_socket) {
        _socket = new QTcpSocket(this);
        connect(_socket, &QTcpSocket::readyRead,    this, &DataLossTester::_onReadyRead);
        connect(_socket, &QTcpSocket::disconnected, this, &DataLossTester::_onSocketDisconnected);
        connect(_socket, &QTcpSocket::errorOccurred,this, &DataLossTester::_onSocketError);
    }

    _socket->connectToHost(_address, _port);
    if (!_socket->waitForConnected(5000)) {
        qCWarning(DataLossTesterLog) << "Failed to connect to" << _address << _port
                                     << ":" << _socket->errorString();
        return;
    }

    _rxBuffer.clear();
    mavlink_reset_channel_status(MAVLINK_COMM_0);

    _connected = true;
    emit connectedChanged();

    qCDebug(DataLossTesterLog) << "Connected (bidirectional) to"
                               << _address << _port
                               << "local port =" << _socket->localPort();
}

void DataLossTester::_disconnectFromServer() {
    if (!_socket) return;

    _stopSendingMessages();

    if (!_endReconciled) {
        _reconcileSequenceAtEnd();
        _endReconciled = true;
    }

    _socket->flush();
    _socket->disconnectFromHost();
    if (_socket->state() != QAbstractSocket::UnconnectedState) {
        _socket->waitForDisconnected(2000);
    }
}

void DataLossTester::_onSocketDisconnected() {
    qCDebug(DataLossTesterLog) << "Socket disconnected";
    if (_connected) {
        _connected = false;
        emit connectedChanged();
    }
    _stopSendingMessages();
    _disconnectTimer.stop();
}

void DataLossTester::_onSocketError(QAbstractSocket::SocketError error) {
    Q_UNUSED(error)
    qCWarning(DataLossTesterLog) << "Socket error:" << _socket->errorString();
}

// ---------------------------------------------------------------------------
// Sending
// ---------------------------------------------------------------------------
void DataLossTester::_onSendMessageTimeout() {
    if (!_connected) {
        _stopSendingMessages();
        return;
    }
    _sendNextMessage();
}

void DataLossTester::_sendNextMessage() {
    if (!_connected) {
        qCDebug(DataLossTesterLog) << "Failed to send next message — no connection.";
        return;
    }

    if (_messagesModel.size() == 0) {
        qCDebug(DataLossTesterLog) << "No messages registered.";
        _stopSendingMessages();
        return;
    }

    if (_msgTypeIndex > _messagesModel.size()) {
        _msgTypeIndex = _msgTypeIndex % _messagesModel.size();
    }

    TestMessageInfo* info = _messagesModel[_msgTypeIndex].value<TestMessageInfo*>();
    _msgTypeIndex = (_msgTypeIndex + 1) % _messagesModel.size();

    if (!info) {
        qCDebug(DataLossTesterLog) << "Message info is not filled.";
        _stopSendingMessages();
        return;
    }

    mavlink_message_t msg{};
    info->PackMessage(msg);

    const quint8 sentSeq = msg.seq;

    if (_shouldWaitForResponse) {
        _sendMessagesTimer.stop();
        _expectedResponseSeq  = sentSeq;
        _isWaitingForResponse = true;
        _waitResponseTimer.start();
    }

    uint8_t buf[MAVLINK_MAX_PACKET_LEN];
    const uint16_t len = mavlink_msg_to_send_buffer(buf, &msg);

    const qint64 written = _socket->write(reinterpret_cast<const char*>(buf), len);
    if (written != len) {
        qCWarning(DataLossTesterLog) << "Write size error: written=" << written
                                     << "expected=" << len;
    }

    _lastSentSeq = sentSeq;

    _packetsSent++;
    _testPacketsSent++;
    emit packetsSentChanged();

    if (_testPacketsSent >= _packetsPerTest && !_shouldWaitForResponse) {
        qCDebug(DataLossTesterLog) << "Last message sent.";
        _sendMessagesTimer.stop();
        _disconnectTimer.start();
    }
}

void DataLossTester::_stopSendingMessages() {
    _sendMessagesTimer.stop();
    _waitResponseTimer.stop();
    _isWaitingForResponse = false;
}

// ---------------------------------------------------------------------------
// Wait-response timeout
// ---------------------------------------------------------------------------
void DataLossTester::_onWaitResponseTimeout() {
    if (!_isWaitingForResponse) {
        return;
    }

    qCDebug(DataLossTesterLog) << "Wait response timeout for seq" << _expectedResponseSeq;

    _waitTimeouts++;
    emit waitTimeoutsChanged();

    _packetsLost++;
    emit packetsLostChanged();

    _isWaitingForResponse = false;

    if (_testPacketsSent >= _packetsPerTest) {
        _disconnectTimer.start();
    } else {
        _sendMessagesTimer.start();
    }
}

// ---------------------------------------------------------------------------
// Receiving + sequence check
// ---------------------------------------------------------------------------
void DataLossTester::_onReadyRead() {
    if (!_socket) return;

    _rxBuffer.append(_socket->readAll());

    mavlink_message_t msg;
    mavlink_status_t  status{};
    qint32 newMessages = 0;
    qint32 lostBySeq   = 0;

    for (int i = 0; i < _rxBuffer.size(); ++i) {
        const uint8_t c = static_cast<uint8_t>(_rxBuffer.at(i));
        if (!mavlink_parse_char(MAVLINK_COMM_0, c, &msg, &status)) {
            continue;
        }
        newMessages++;
        _testPacketsReceived++;

        if (_isWaitingForResponse && msg.seq == _expectedResponseSeq) {
            _isWaitingForResponse = false;
            _waitResponseTimer.stop();

            if (_testPacketsSent >= _packetsPerTest) {
                _disconnectTimer.start();
            } else {
                _sendMessagesTimer.start();
            }
        }

        if (_hasLastReceivedSeq) {
            const int delta = (static_cast<int>(msg.seq) - static_cast<int>(_lastReceivedSeq) + 256) % 256;
            if (delta > 1 && delta < 128) {
                lostBySeq += (delta - 1);
            }
        }
        _lastReceivedSeq    = msg.seq;
        _hasLastReceivedSeq = true;
    }
    _rxBuffer.clear();

    if (newMessages > 0) {
        _packetsReceived += newMessages;
        emit packetsReceivedChanged();
    }

    if (lostBySeq > 0) {
        _packetsLostBySequence += lostBySeq;
        emit packetsLostBySequenceChanged();
        qCDebug(DataLossTesterLog) << "Sequence gap: lost" << lostBySeq
                                   << "packets (total by seq ="
                                   << _packetsLostBySequence << ")";
    }
}

void DataLossTester::_reconcileSequenceAtEnd() {
    if (_testPacketsSent == 0 || !_hasLastReceivedSeq) {
        return;
    }

    const int tailDelta = (static_cast<int>(_lastSentSeq)
                           - static_cast<int>(_lastReceivedSeq)
                           + 256) % 256;

    if (tailDelta > 0 && tailDelta < 128) {
        _packetsLostBySequence += tailDelta;
        emit packetsLostBySequenceChanged();
        qCDebug(DataLossTesterLog)
            << "End-of-test sequence reconciliation: lastSentSeq ="
            << _lastSentSeq << "lastReceivedSeq =" << _lastReceivedSeq
            << "→ tail loss +" << tailDelta;
    }
}