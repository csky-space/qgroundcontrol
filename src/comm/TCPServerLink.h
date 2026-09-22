#pragma once

#include <QString>
#include <QHostAddress>
#include <QTcpServer>
#include <QTcpSocket>
#include <QMutex>

#include "LinkInterface.h"

class LinkManager;
class TCPServerLinkTest;

class TCPServerConfiguration : public LinkConfiguration
{
    Q_OBJECT

public:
    Q_PROPERTY(quint16 port READ port WRITE setPort NOTIFY portChanged)
    Q_PROPERTY(QString host READ host WRITE setHost NOTIFY hostChanged)   // bind address

    TCPServerConfiguration(const QString& name);
    TCPServerConfiguration(TCPServerConfiguration* source);

    quint16 port(void) const { return _port; }
    QString host(void) const { return _host; }
    void    setPort(quint16 port);
    void    setHost(const QString& host);

    // LinkConfiguration overrides
    LinkType type(void) override { return LinkConfiguration::TypeTcpServer; }
    void     copyFrom(LinkConfiguration* source) override;
    void     loadSettings(QSettings& settings, const QString& root) override;
    void     saveSettings(QSettings& settings, const QString& root) override;
    QString  settingsURL(void) override   { return "TcpServerSettings.qml"; }
    QString  settingsTitle(void) override { return tr("TCP Server Link Settings"); }

signals:
    void portChanged(void);
    void hostChanged(void);

private:
    QString _host;      // bind address; default QHostAddress::Any
    quint16 _port;
};

class TCPServerLink : public LinkInterface
{
    Q_OBJECT

public:
    TCPServerLink(SharedLinkConfigurationPtr& config);
    virtual ~TCPServerLink();

    // LinkInterface overrides
    bool isConnected(void) const override;
    void disconnect(void) override;

private slots:
    void _onNewConnection(void);
    void _onSocketDisconnected(void);
    void _readBytes(void);
    void _socketError(QAbstractSocket::SocketError socketError);

private:
    // LinkInterface overrides
    bool _connect(void) override;
    void _writeBytes(const QByteArray data) override;

    TCPServerConfiguration* _tcpConfig;
    QTcpServer*             _server;
    QTcpSocket*             _socket;
    bool                    _socketIsConnected;
};
