/****************************************************************************
 *
 * (c) 2009-2020 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/
//#ifdef QGC_AIRLINK_ENABLED
 #include "AirlinkStreamBridgeSettings.h"

 #include <QQmlEngine>
 #include <QtQml>
 
 DECLARE_SETTINGGROUP(AirlinkStreamBridge, "AirlinkStreamBridge")
 {
    qmlRegisterUncreatableType<AirlinkStreamBridgeSettings>("QGroundControl.SettingsManager", 1, 0, "AirlinkStreamBridgeSettings", "Reference only");
 }
 
 DECLARE_SETTINGSFACT(AirlinkStreamBridgeSettings, asbEnabled)
 DECLARE_SETTINGSFACT(AirlinkStreamBridgeSettings, asbAutotune)
 DECLARE_SETTINGSFACT(AirlinkStreamBridgeSettings, asbPort)
//#endif
