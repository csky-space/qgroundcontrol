/****************************************************************************
 *
 * (c) 2009-2020 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/
//#ifdef QGC_AIRLINK_ENABLED
 #pragma once

 #include "SettingsGroup.h"
 
 class AirlinkStreamBridgeSettings : public SettingsGroup
 {
    Q_OBJECT
 public:
    AirlinkStreamBridgeSettings(QObject* parent = nullptr);
    DEFINE_SETTING_NAME_GROUP()
 
    DEFINE_SETTINGFACT(asbEnabled)
    DEFINE_SETTINGFACT(asbAutotune)
    DEFINE_SETTINGFACT(asbPort)
 };
//#endif
