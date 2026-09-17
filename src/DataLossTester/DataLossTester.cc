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

#include "QGCMAVLink.h"

QGC_LOGGING_CATEGORY(DataLossTesterLog, "DataLossTesterLog")

DataLossTester::DataLossTester(QGCApplication* app, QGCToolbox* toolbox) :
    QGCTool(app, toolbox) {}

DataLossTester::~DataLossTester() {}

void DataLossTester::setToolbox(QGCToolbox *toolbox) {
    QGCTool::setToolbox(toolbox);
    QQmlEngine::setObjectOwnership(this, QQmlEngine::CppOwnership);
    qmlRegisterUncreatableType<DataLossTester>("QGroundControl.dataLossTester", 1, 0,
                                               "DataLossTester", "Reference only");

    _cacheMessages();

    connect(&_sendMessagesTimer, &QTimer::timeout, this, &DataLossTester::_onSendMessageTimeout);
    connect(&_waitResponseTimer, &QTimer::timeout, this, &DataLossTester::_onWaitResponseTimeout);
    connect(&_disconnectTimer, &QTimer::timeout, this, &DataLossTester::_disconnectFromServer);
}

QString DataLossTester::address() const {
    return _address;
}

quint16 DataLossTester::port() const {
    return _port;
}

quint32 DataLossTester::packetsPerTest() const {
    return _packetsPerTest;
}

quint32 DataLossTester::sendInterval() const {
    return _sendInterval;
}

quint32 DataLossTester::waitInterval() const {
    return _waitResponseInterval;
}

bool DataLossTester::shouldWait() const {
    return _shouldWaitForResponse;
}

qint32 DataLossTester::packetsSent() const {
    return _packetsSent;
}

qint32 DataLossTester::packetsReceived() const {
    return _packetsReceived;
}

qint32 DataLossTester::packetsLost() const {
    return _packetsLost;
}

bool DataLossTester::connected() const {
    return _connected;
}

void DataLossTester::setAddress(QString address) {
    if (_address == address) {
        return;
    }
    _address = address;
    emit addressChanged();
}

void DataLossTester::setPort(quint16 port) {
    if (_port == port) {
        return;
    }
    _port = port;
    emit portChanged();
}

void DataLossTester::setPacketsPerTest(quint32 count) {
    if (_packetsPerTest == count) {
        return;
    }
    _packetsPerTest = count;
    emit packetsPerTestChanged();
}

void DataLossTester::setSendInterval(quint32 ms) {
    if (_sendInterval == ms) {
        return;
    }
    _sendInterval = ms;
    emit sendIntervalChanged();
}

void DataLossTester::setWaitInterval(quint32 ms) {
    if (_waitResponseInterval == ms) {
        return;
    }
    _waitResponseInterval = ms;
    emit waitIntervalChanged();
}

void DataLossTester::setShouldWait(bool state) {
    if (_shouldWaitForResponse == state) {
        return;
    }
    _shouldWaitForResponse = state;
    emit shouldWaitChanged();
}

void DataLossTester::startSendingMessages() {
    if (_connected) {
        qCWarning(DataLossTesterLog) << "Is already sending messages";
        return;
    }

    _connectToServer();

    if (!_connected) {
        return;
    }

    _testPacketsSent = 0;

    _sendMessagesTimer.setInterval(_sendInterval);
    _waitResponseTimer.setInterval(_waitResponseInterval);
    _disconnectTimer.setInterval(_disconnectInterval);

    _disconnectTimer.stop();
    _sendMessagesTimer.start();
}

void DataLossTester::_cacheMessages() {
    const uint8_t systemId    = 1;
    const uint8_t componentId = 1;

    mavlink_message_t msg{};

    mavlink_msg_heartbeat_pack(
        systemId,
        componentId,
        &msg,
        MAV_TYPE_QUADROTOR,
        MAV_AUTOPILOT_GENERIC,
        MAV_MODE_FLAG_MANUAL_INPUT_ENABLED,
        0,
        MAV_STATE_ACTIVE);

    uint8_t buf[MAVLINK_MAX_PACKET_LEN];
    const uint16_t len = mavlink_msg_to_send_buffer(buf, &msg);

    _cachedMessages.push_back(QByteArray(reinterpret_cast<const char*>(buf), len));
}

void DataLossTester::_connectToServer() {
    if (_connected) {
        return;
    }

    if (_address.isEmpty()) {
        qCWarning(DataLossTesterLog) << "Empty address:" << _address;
        return;
    }

    if (!_socket) {
        _socket = new QTcpSocket(this);
        connect(_socket, &QTcpSocket::readyRead, this, &DataLossTester::_onReadyRead);
        connect(_socket, &QTcpSocket::disconnected, this, &DataLossTester::_onSocketDisconnected);
        connect(_socket, &QTcpSocket::errorOccurred, this, &DataLossTester::_onSocketError);
    }

    _socket->connectToHost(_address, _port);
    if (!_socket->waitForConnected(5000)) {
        qCWarning(DataLossTesterLog) << "Failed to connect to" << _address << _port << ":" << _socket->errorString();
        return;
    }

    _rxBuffer.clear();
    mavlink_reset_channel_status(MAVLINK_COMM_0);

    _connected = true;
    emit connectedChanged();

    qCDebug(DataLossTesterLog) << "Connected (bidirectional) to" << _address << _port << "local port =" << _socket->localPort();
}

void DataLossTester::_disconnectFromServer() {
    if (!_socket) {
        return;
    }

    _stopSendingMessages();

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

void DataLossTester::_onSendMessageTimeout() {
    if (!_connected) {
        _stopSendingMessages();
        return;
    }
    _sendNextMessage();
}

void DataLossTester::_onWaitResponseTimeout() {
    qCDebug(DataLossTesterLog) << "Wait response timeout";

    _packetsLost++;
    emit packetsLostChanged();

    if (_isWaitingForResponse) {
        _isWaitingForResponse = false;
        _waitResponseTimer.stop();
        _sendMessagesTimer.start();
    } else if (_testPacketsSent >= _packetsPerTest) {
        qCDebug(DataLossTesterLog) << "Last awaited message received.";
        _stopSendingMessages();
    }
}


void DataLossTester::_onReadyRead() {
    if (!_socket) {
        return;
    }

    _rxBuffer.append(_socket->readAll());

    mavlink_message_t msg;
    mavlink_status_t  status{};
    qint32 newMessages = 0;

    for (int i = 0; i < _rxBuffer.size(); ++i) {
        const uint8_t c = static_cast<uint8_t>(_rxBuffer.at(i));
        if (mavlink_parse_char(MAVLINK_COMM_0, c, &msg, &status)) {
            newMessages++;
        }
    }
    _rxBuffer.clear();

    if (newMessages > 0) {
        _packetsReceived += newMessages;
        emit packetsReceivedChanged();
    }
}

void DataLossTester::_sendNextMessage() {
    if (!_connected) {
        qCDebug(DataLossTesterLog) << "Failed to send next message because there is no connection.";
        return;
    }

    if (_shouldWaitForResponse) {
        _sendMessagesTimer.stop();
        _waitResponseTimer.start();
        _isWaitingForResponse = true;
    }

    const qint64 written = _socket->write(_cachedMessages[_currentMessageIndex].data(), _cachedMessages[_currentMessageIndex].size());
    if (written != _cachedMessages[_currentMessageIndex].size()) {
        qCWarning(DataLossTesterLog) << "Write size error: written=" << written << "expected=" << _cachedMessages[_currentMessageIndex].size();
    }
    _currentMessageIndex = (_currentMessageIndex + 1) % _cachedMessages.size();

    _packetsSent++;
    _testPacketsSent++;
    emit packetsSentChanged();

    if (_testPacketsSent >= _packetsPerTest) {
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
