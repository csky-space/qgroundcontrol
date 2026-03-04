/****************************************************************************
 *
 * (c) 2009-2020 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/

#include "VibrationControllerSettings.h"
#include "QGCApplication.h"

#include <QQmlEngine>
#include <QtQml>
#include <QVariantList>

#ifndef QGC_DISABLE_UVC
#include <QCameraInfo>
#endif

DECLARE_SETTINGGROUP(VibrationController, "VibrationController")
{
    qmlRegisterUncreatableType<VibrationControllerSettings>("QGroundControl.SettingsManager", 1, 0, "VibrationControllerSettings", "Reference only");
}

DECLARE_SETTINGSFACT(VibrationControllerSettings, VibrationX)
DECLARE_SETTINGSFACT(VibrationControllerSettings, VibrationY)
DECLARE_SETTINGSFACT(VibrationControllerSettings, VibrationZ)
DECLARE_SETTINGSFACT(VibrationControllerSettings, ClippingP)
DECLARE_SETTINGSFACT(VibrationControllerSettings, ClippingS)
DECLARE_SETTINGSFACT(VibrationControllerSettings, ClippingT)
