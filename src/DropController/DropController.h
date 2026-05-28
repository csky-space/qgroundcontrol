/// @file DropController.h

#pragma once

#include <QLoggingCategory>
#include <QTimer>

#include "Vehicle.h"
#include "QmlObjectListModel.h"

Q_DECLARE_LOGGING_CATEGORY(DropControllerLog)

class DropController : public QObject {
    Q_OBJECT

public:
    DropController (MAVLinkProtocol* mavlink, Vehicle* vehicle);
    ~DropController();

    Q_PROPERTY(bool           initialized          READ initialized          NOTIFY initializedChanged)
    Q_PROPERTY(bool           isInterfaceLocked    READ isInterfaceLocked    NOTIFY isInterfaceLockedChanged)
    Q_PROPERTY(QStringList    dropModesModel       READ dropModesModel       NOTIFY dropModesModelChanged)
    Q_PROPERTY(int            activeModeIndex      READ activeModeIndex      NOTIFY activeModeIndexChanged)

    bool        initialized         () const;
    bool        isInterfaceLocked   () const;
    QStringList dropModesModel      () const;
    int         activeModeIndex     () const;

    Q_INVOKABLE void setDropModeIndex (int index);

private slots:
    void _onParametersReadyChanged   (bool parametersReady);
    void _onDropModeParameterChanged ();
    void _onParamUpdateTimeout      ();

signals:
    void initializedChanged       ();
    void isInterfaceLockedChanged ();
    void dropModesModelChanged    ();
    void activeModeIndexChanged   ();

private:
    MAVLinkProtocol* _mavlink = nullptr;
    Vehicle*         _vehicle = nullptr;

    bool             _initialized;
    bool             _isInterfaceLocked;
    QStringList      _dropModesModel;
    int              _activeModeIndex;

    QTimer           _paramUpdateTimer;
};
