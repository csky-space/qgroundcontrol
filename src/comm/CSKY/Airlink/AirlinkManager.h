/****************************************************************************
 *
 * (c) 2009-2020 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/

#pragma once

#include <QMutex>
#include <QNetworkReply>
#include <QTime>
#include <QTimer>
#include <QProcess>
#include <QNetworkAccessManager>

#include <Fact.h>
#include <QGCToolbox.h>

#include "airlinkstreambridgemanager.h"

class QThread;
class AppSettings;
class QGCApplication;
class LinkInterface;

class VideoManager;

namespace CSKY {
class AirlinkConfiguration;
class Airlink;
//-----------------------------------------------------------------------------
class AirlinkManager : public QGCTool {
  Q_OBJECT

public:
    Q_PROPERTY(QString airlinkHost READ getAirlinkHost CONSTANT)

	Q_PROPERTY(QStringList droneList READ droneList NOTIFY droneListChanged)
    Q_PROPERTY(QList<bool> droneOnlineList READ droneOnlineList NOTIFY droneOnlineListChanged)

    Q_PROPERTY(bool fullBlock READ fullBlock WRITE setFullBlock NOTIFY fullBlockChanged)

	Q_INVOKABLE void updateDroneList(const QString &login, const QString &pass);
	Q_INVOKABLE bool isOnline(const QString &drone);
	Q_INVOKABLE void connectToAirLinkServer(const QString &login, const QString &pass);
	Q_INVOKABLE void updateCredentials(const QString &login, const QString &pass);
    Q_INVOKABLE QString getAirlinkHost() const {return airlinkHost;}

    Q_INVOKABLE bool fullBlock() const {return _fullBlockUI;}
    Q_INVOKABLE void setFullBlock(bool block) {_fullBlockUI = block;}

	explicit AirlinkManager(QGCApplication *app, QGCToolbox *toolbox);
	~AirlinkManager() override;

    void restartASBProcess();
    void checkAirlinkService();

	void setToolbox(QGCToolbox *toolbox) override;
	QStringList droneList() const;
    QList<bool> droneOnlineList() const;

    static const QString airlinkHostPrefix;
	static const QString airlinkHost;
	static constexpr int airlinkPort = 10000;

    AirlinkStreamBridgeManager& getASBManager();
    QProcess& getAsbProcess();
    Fact* getAsbEnabled() const;
    Fact* getPort() const;
signals:
    void createWebrtcDefault(QString hostName, QString modemName, QString password, quint16 port);
    void enableVideoTransmit();
    void isWebrtcReceiverConnected();
    void openPeer();
    void closePeer();

    void asbEnabledTrue(Airlink* airlink);
    void asbEnabledFalse(Airlink* airlink);
    void asbClosed(Airlink* airlink);
    void fullBlockChanged(bool blocked);

	void droneListChanged();
    void droneOnlineListChanged();

    void sendAsbServicePort(quint16 port);
    void checkAlive();
    void onConnectedAirlinkAdded(Airlink* link = nullptr);
    void onDisconnectedAirlinkRemoved(Airlink* link = nullptr);
private:
    void _setConnects();
	void _parseAnswer(const QByteArray &ba);
	void _processReplyAirlinkServer(QNetworkReply &reply);
	void _findAirlinkConfiguration();
private:
    QMutex modemsModifyMutex;
    QNetworkAccessManager connectTelemetryManager;
    bool _fullBlockUI;
    QString asbPath;
    AirlinkStreamBridgeManager manager;
    QTimer* watchdogTimer = nullptr;
    QMutex processMutex;
    bool isRestarting = false;
	QMap<QString, bool> _vehiclesFromServer;
    QNetworkReply *_reply = nullptr;
	QString _login;
	QString _pass;
	QString droneModem = "";
	std::shared_ptr<AirlinkConfiguration> config = nullptr;
  
	QProcess asbProcess;
    VideoManager* qgcVideoManager = nullptr;

	QMetaObject::Connection setupVideoConn;
    QMetaObject::Connection portConstraintConn;
	QMetaObject::Connection holdPortConn;
	
	Fact* asbEnabled = nullptr;
	Fact* asbAutotune = nullptr;
	Fact* asbPort = nullptr;
	
	Fact* videoUDPPort = nullptr;
	Fact* videoSource = nullptr;
    QThread* requestsThread = nullptr;
    QMap<QString, Airlink*> modems;
    Airlink* lastConnectedModem = nullptr;
    Airlink* prevConnectedModem = nullptr;
public slots:
    void startWatchdog();
    Q_INVOKABLE void stopWatchdog();
    Q_INVOKABLE void terminateASB();

  	void setupVideoStream(QVariant value);
  	void setupPort(QVariant value);
    void portConstraint(QVariant value);

    void blockUI(QByteArray replyData = {}, QNetworkReply::NetworkError err = QNetworkReply::NoError);
    void unblockUI(QByteArray replyData = {}, QNetworkReply::NetworkError err = QNetworkReply::NoError);

    void addAirlink(Airlink* airlink);
    void removeAirlink(Airlink* airlink);
private slots:

    void checkAndRestartASB();
    void asbEnabledChanged(QVariant value);
    void asbAutotuneChanged(QVariant value);
};

}
