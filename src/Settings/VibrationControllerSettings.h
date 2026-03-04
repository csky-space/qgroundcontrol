/****************************************************************************
 *
 * (c) 2009-2020 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/
#pragma once

#include "SettingsGroup.h"

class VibrationControllerSettings : public SettingsGroup
{
    Q_OBJECT

public:
    VibrationControllerSettings(QObject* parent = nullptr);
    DEFINE_SETTING_NAME_GROUP()

    DEFINE_SETTINGFACT(VibrationX)
    DEFINE_SETTINGFACT(VibrationY)
    DEFINE_SETTINGFACT(VibrationZ)
    DEFINE_SETTINGFACT(ClippingP)
    DEFINE_SETTINGFACT(ClippingS)
    DEFINE_SETTINGFACT(ClippingT)
};
