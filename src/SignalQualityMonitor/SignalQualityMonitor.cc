#include "SignalQualityMonitor.h"

#include <QQmlContext>
#include <QQmlEngine>
#include <QSettings>
#include <QQuickWindow>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>

#include "ScreenToolsController.h"
#include "QGCToolbox.h"
#include "QGCCorePlugin.h"
#include "QGCOptions.h"
#include "Settings/SettingsManager.h"

QGC_LOGGING_CATEGORY(SignalQualityMonitorLog, "SignalQualityMonitorLog")


float SignalQuality::snr() const {
    return _snr;
}

float SignalQuality::gain() const {
    return _gain;
}

int SignalQuality::snrStatus() const {
    return static_cast<int>(_snrStatus);
}

int SignalQuality::gainStatus() const {
    return static_cast<int>(_gainStatus);
}

void SignalQuality::setSNR(float value) {
    _snr = value;
    _snrStatus = SignalStatus::STATUS_OK;
    emit snrChanged();
    emit snrStatusChanged();
}

void SignalQuality::setGain(float value) {
    _gain = value;
    _gainStatus = SignalStatus::STATUS_OK;
    emit gainChanged();
    emit gainStatusChanged();
}


SignalQualityMonitor::SignalQualityMonitor(QGCApplication* app, QGCToolbox* toolbox) :
    QGCTool(app, toolbox) {}

SignalQualityMonitor::~SignalQualityMonitor() {
    if (_monitoredSocket) {
        delete _monitoredSocket;
    }
}

void SignalQualityMonitor::setToolbox(QGCToolbox *toolbox) {
    QGCTool::setToolbox(toolbox);
    QQmlEngine::setObjectOwnership(this, QQmlEngine::CppOwnership);
    qmlRegisterUncreatableType<SignalQualityMonitor>("QGroundControl.SignalQualityMonitor", 1, 0, "SignalQualityMonitor", "Reference only");

    _monitoredSocket = new QUdpSocket();
    _monitoredSocket->bind(9000);
    connect(_monitoredSocket, &QUdpSocket::readyRead, this, &SignalQualityMonitor::_onMonitoredSocketReadyRead);

    _dataRecieveTimer.setInterval(5000);
    connect(&_dataRecieveTimer, &QTimer::timeout, this, &SignalQualityMonitor::_onDataRecieveTimeout);
    
    SignalQuality* signalA = new SignalQuality();
    QQmlEngine::setObjectOwnership(signalA, QQmlEngine::CppOwnership);
    _signalsModel.append(QVariant::fromValue(signalA));

    SignalQuality* signalB = new SignalQuality();
    QQmlEngine::setObjectOwnership(signalB, QQmlEngine::CppOwnership);
    _signalsModel.append(QVariant::fromValue(signalB));

    emit signalsModelChanged();
}

int SignalQualityMonitor::status() const {
    return static_cast<int>(_status);
}

QVariantList SignalQualityMonitor::signalsModel() const {
    return _signalsModel;
}

QStringList SignalQualityMonitor::dataKeys() const {
    return _dataKeys;
}

QStringList SignalQualityMonitor::dataValues() const {
    return _dataValues;
}

void SignalQualityMonitor::_parseMessageJSON(const QByteArray& data) {
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(data, &error);

    if (error.error != QJsonParseError::NoError) {
        qCDebug(SignalQualityMonitorLog) << "_parseMessageJSON: parse error:" << error.errorString();
        return;
    }

    if (!doc.isObject()) {
        qCDebug(SignalQualityMonitorLog) << "_parseMessageJSON: recieved data is not and object";
        return;
    }

    QJsonObject rootObject = doc.object();

    if (rootObject.contains("State")) {
        _status = (rootObject["State"] == "ok") ? SignalMonitorStatus::STATUS_OK : SignalMonitorStatus::STATUS_ERROR;
        emit statusChanged();
    }

    _dataKeys.clear();
    _dataValues.clear();
    for (QString key : rootObject.keys()) {
        if (rootObject[key].isObject() || rootObject[key].isArray()) {
            continue;
        }
        _dataKeys.push_back(key);
        _dataValues.push_back(rootObject[key].toString());
    }
    emit dataKeysChanged();
    emit dataValuesChanged();


    if (_signalsModel.size() < 2) {
        qCDebug(SignalQualityMonitorLog) << "_parseMessageJSON: SignalQualityMonitor is not initialized properly";
        return;
    }


    SignalQuality* signalA = _signalsModel[0].value<SignalQuality*>();

    if (rootObject.contains("snr_A")) {
        float snr = rootObject["snr_A"].toString().toFloat();
        signalA->setSNR(snr);
    }

    if (rootObject.contains("Gain_A")) {
        float gain = rootObject["Gain_A"].toString().toFloat();
        signalA->setGain(gain);
    }


    SignalQuality* signalB = _signalsModel[1].value<SignalQuality*>();

    if (rootObject.contains("snr_B")) {
        float snr = rootObject["snr_B"].toString().toFloat();
        signalB->setSNR(snr);
    }

    if (rootObject.contains("Gain_B")) {
        float gain = rootObject["Gain_B"].toString().toFloat();
        signalB->setGain(gain);
    }
}

void SignalQualityMonitor::_onMonitoredSocketReadyRead() {
    while (_monitoredSocket->hasPendingDatagrams()) {
        if (!_dataRecieveTimer.isActive()) {
            qCDebug(SignalQualityMonitorLog) << "_onMonitoredSocketReadyRead: started receiving data";
        }
        _dataRecieveTimer.start();

        qCDebug(SignalQualityMonitorLog) << "_onMonitoredSocketReadyRead: datagram received";

        QNetworkDatagram datagram = _monitoredSocket->receiveDatagram();
        QByteArray data = datagram.data();
        _parseMessageJSON(data);
    }
}

void SignalQualityMonitor::_onDataRecieveTimeout() {
    qCDebug(SignalQualityMonitorLog) << "_onDataRecieveTimeout: stopped receiving data";
    _dataRecieveTimer.stop();
    _status = SignalMonitorStatus::STATUS_CONNECTION_TIMEOUT;
    emit statusChanged();
}
