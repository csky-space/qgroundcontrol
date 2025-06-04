#include "AirlinkTelemetry.h"

#include <QThread>
#include <QTimer>
#include <MAVLinkProtocol.h>
#include <LinkInterface.h>
#include <QGCApplication.h>
#include <QGCToolbox.h>

QGC_LOGGING_CATEGORY(AirlinkTelemetryLog, "AirlinkTelemetryLog")

namespace CSKY {

AirlinkTelemetry::AirlinkTelemetry(QObject* parent)
    : QObject(parent)
    , telemetryThread(new QThread)
{
    moveToThread(telemetryThread);
}

// void AirlinkTelemetry::_configureUdpSettings()
// {
//     static quint16 availablePort = 14550;
//     QUdpSocket udpSocket;
//     while (!udpSocket.bind(QHostAddress::LocalHost, availablePort))
//         availablePort++;
//     UDPConfiguration* udpConfig = dynamic_cast<UDPConfiguration*>(UDPLink::_config.get());
//     udpConfig->addHost(AirlinkManager::airlinkHost, AirlinkManager::airlinkPort);
//     udpConfig->setLocalPort(availablePort);
//     udpConfig->setDynamic(false);
//     availablePort += 1;
// }

// void AirlinkTelemetry::_sendLoginMsgToAirLink()
// {
//     __mavlink_airlink_auth_t auth;
//     uint8_t buffer[MAVLINK_MAX_PACKET_LEN];
//     mavlink_message_t mavmsg;
//     AirlinkConfiguration* config = dynamic_cast<AirlinkConfiguration*>(_config.get());
//     qCDebug(AirlinkLog) << "before conf gets";
//     QString login = config->modemName();
//     QString pass = config->password();

//     std::fill(std::begin(auth.login), std::end(auth.login), 0);
//     std::fill(std::begin(auth.password), std::end(auth.password), 0);
//     qCDebug(AirlinkLog) << "before print authorization";
//     snprintf(auth.login, sizeof(auth.login), "%s", login.toUtf8().constData());
//     snprintf(auth.password, sizeof(auth.password), "%s", pass.toUtf8().constData());

//     qCDebug(AirlinkLog) << "before auth";
//     mavlink_msg_airlink_auth_pack(0, 0, &mavmsg, auth.login, auth.password);
//     uint16_t len = mavlink_msg_to_send_buffer(buffer, &mavmsg);

//     if (!_stillConnecting()) {
//         qCDebug(AirlinkLog) << "Force exit from connection";
//         return;
//     }
//     qCDebug(AirlinkLog) << "before write bytes";
//     this->writeBytesThreadSafe((const char*)buffer, len);
//     qCDebug(AirlinkLog) << "after write bytes";
// }

// bool AirlinkTelemetry::_stillConnecting()
// {
//     return _needToConnect.load(std::memory_order_acquire);
// }

// void AirlinkTelemetry::_setConnectFlag(bool connect)
// {
//     _needToConnect.store(connect, std::memory_order_release);
// }

// void AirlinkTelemetry::connect() {
//     telemetryThread->start(QThread::NormalPriority);
//     QTimer *pendingTimer = new QTimer(this);
//     connect(pendingTimer, &QTimer::timeout, this, [this, pendingTimer] {
//         pendingTimer->setInterval(3000);
//         if (_stillConnecting()) {
//             qCDebug(AirlinkTelemetryLog) << "Connecting...";
//             _sendLoginMsgToAirLink();
//         } else {
//             qCDebug(AirlinkTelemetryLog) << "Stopping...";
//             pendingTimer->stop();
//             pendingTimer->deleteLater();
//         }
//     });
//     MAVLinkProtocol *mavlink = qgcApp()->toolbox()->mavlinkProtocol();
//     auto conn = std::make_shared<QMetaObject::Connection>();
//     *conn = connect(mavlink, &MAVLinkProtocol::messageReceived, this, [this, conn] (LinkInterface* linkSrc, mavlink_message_t message) {
//         if ((this != linkSrc) || (message.msgid != MAVLINK_MSG_ID_AIRLINK_AUTH_RESPONSE)) {
//             return;
//         }
//         mavlink_airlink_auth_response_t responseMsg;
//         mavlink_msg_airlink_auth_response_decode(&message, &responseMsg);
//         int answer = responseMsg.resp_type;
//         if (answer != AIRLINK_AUTH_RESPONSE_TYPE::AIRLINK_AUTH_OK) {
//             qCDebug(AirlinkTelemetryLog) << "Airlink auth failed";
//             return;
//         }
//         qCDebug(AirlinkTelemetryLog) << "Connected successfully";
//         QObject::disconnect(*conn);
//         _setConnectFlag(false);
//     });
//     _setConnectFlag(true);
//     pendingTimer->start(0);
// }

}
