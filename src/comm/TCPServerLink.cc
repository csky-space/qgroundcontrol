#include "TCPServerLink.h"
#include "LinkManager.h"

#include <QSettings>

// ---------------------------------------------------------------------------
// TCPServerConfiguration
// ---------------------------------------------------------------------------

TCPServerConfiguration::TCPServerConfiguration(const QString& name)
    : LinkConfiguration(name)
    , _host(QLatin1String("0.0.0.0"))
    , _port(5760)
{
}

TCPServerConfiguration::TCPServerConfiguration(TCPServerConfiguration* source)
    : LinkConfiguration(source)
{
    _host = source->host();
    _port = source->port();
}

void TCPServerConfiguration::setPort(quint16 port)
{
    if (_port != port) {
        _port = port;
        emit portChanged();
    }
}

void TCPServerConfiguration::setHost(const QString& host)
{
    if (_host != host) {
        _host = host;
        emit hostChanged();
    }
}

void TCPServerConfiguration::copyFrom(LinkConfiguration* source)
{
    LinkConfiguration::copyFrom(source);
    auto* usource = qobject_cast<TCPServerConfiguration*>(source);
    Q_ASSERT(usource);
    _host = usource->host();
    _port = usource->port();
}

void TCPServerConfiguration::saveSettings(QSettings& settings, const QString& root)
{
    settings.beginGroup(root);
    settings.setValue("port", (int)_port);
    settings.setValue("host", _host);
    settings.endGroup();
}

void TCPServerConfiguration::loadSettings(QSettings& settings, const QString& root)
{
    settings.beginGroup(root);
    _port = (quint16)settings.value("port", 5760).toUInt();
    _host = settings.value("host", _host).toString();
    settings.endGroup();
}

// ---------------------------------------------------------------------------
// TCPServerLink
// ---------------------------------------------------------------------------

TCPServerLink::TCPServerLink(SharedLinkConfigurationPtr& config)
    : LinkInterface(config)
    , _tcpConfig(qobject_cast<TCPServerConfiguration*>(config.get()))
    , _server(nullptr)
    , _socket(nullptr)
    , _socketIsConnected(false)
{
    Q_ASSERT(_tcpConfig);
}

TCPServerLink::~TCPServerLink()
{
    disconnect();
}

bool TCPServerLink::_connect(void)
{
    Q_ASSERT(_server == nullptr);

    _server = new QTcpServer(this);
    connect(_server, &QTcpServer::newConnection, this, &TCPServerLink::_onNewConnection);

    QHostAddress bindAddr(_tcpConfig->host().isEmpty()
                              ? QStringLiteral("0.0.0.0")
                              : _tcpConfig->host());

    if (!_server->listen(bindAddr, _tcpConfig->port())) {
        emit communicationError(tr("Link Error"),
                                tr("Error on link %1. Listen failed: %2")
                                    .arg(_config->name()).arg(_server->errorString()));
        delete _server;
        _server = nullptr;
        return false;
    }

    qCDebug(LinkManagerLog) << "TCPServerLink listening on"
                            << bindAddr.toString() << _tcpConfig->port();
    return true;
}

void TCPServerLink::_onNewConnection(void)
{
    if (_socket) {
        // Single client policy — reject extra peers
        QTcpSocket* extra = _server->nextPendingConnection();
        extra->disconnectFromHost();
        extra->deleteLater();
        return;
    }

    _socket = _server->nextPendingConnection();
    _socketIsConnected = true;

    connect(_socket, &QIODevice::readyRead,           this, &TCPServerLink::_readBytes);
    connect(_socket, &QAbstractSocket::errorOccurred, this, &TCPServerLink::_socketError);
    connect(_socket, &QTcpSocket::disconnected,       this, &TCPServerLink::_onSocketDisconnected);

    emit connected();
}

void TCPServerLink::_onSocketDisconnected(void)
{
    if (!_socket) return;

    _socketIsConnected = false;
    _socket->deleteLater();
    _socket = nullptr;

    emit disconnected();

    // _server stays open — next client re-triggers _onNewConnection.
}

void TCPServerLink::_readBytes(void)
{
    if (!_socket) return;

    qint64 byteCount = _socket->bytesAvailable();
    if (byteCount) {
        QByteArray buffer;
        buffer.resize(byteCount);
        _socket->read(buffer.data(), buffer.size());
        emit bytesReceived(this, buffer);
    }
}

void TCPServerLink::_writeBytes(const QByteArray data)
{
    if (_socket && _socket->state() == QAbstractSocket::ConnectedState) {
        _socket->write(data);
        emit bytesSent(this, data);
    }
}

void TCPServerLink::_socketError(QAbstractSocket::SocketError socketError)
{
    Q_UNUSED(socketError);
    if (_socket) {
        emit communicationError(tr("Link Error"),
                                tr("Error on link %1. Error on socket: %2.")
                                    .arg(_config->name()).arg(_socket->errorString()));
    }
}

bool TCPServerLink::isConnected(void) const
{
    return _socketIsConnected;
}

void TCPServerLink::disconnect(void)
{
    if (_socket) {
        QObject::disconnect(_socket, nullptr, this, nullptr);
        _socketIsConnected = false;
        _socket->disconnectFromHost();
        _socket->deleteLater();
        _socket = nullptr;
        emit disconnected();
    }

    if (_server) {
        _server->close();
        _server->deleteLater();
        _server = nullptr;
    }
}
