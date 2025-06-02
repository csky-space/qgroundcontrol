/****************************************************************************
 *
 * Copyright (C) 2018 Pinecone Inc. All rights reserved.
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/

#include <QtAndroidExtras/QtAndroidExtras>
#include <QtAndroidExtras/QAndroidJniObject>
#include "QGCApplication.h"
#include "AndroidInterface.h"
#include <QAndroidJniObject>
#include <QtAndroid>
#include <QDebug>

QString AndroidInterface::getSDCardPath()
{
    QAndroidJniObject value = QAndroidJniObject::callStaticObjectMethod("org/mavlink/qgroundcontrol/QGCActivity", "getSDCardPath",
                                                                        "()Ljava/lang/String;");
    QString sdCardPath = value.toString();

    if (QtAndroid::androidSdkVersion() >= 30) {
        if (!checkStoragePermission()) {
            qWarning() << "Storage permission denied";
            return QString();
        }
    } else {
        QString readPermission("android.permission.READ_EXTERNAL_STORAGE");
        QString writePermission("android.permission.WRITE_EXTERNAL_STORAGE");

        if (QtAndroid::checkPermission(readPermission) == QtAndroid::PermissionResult::Denied ||
            QtAndroid::checkPermission(writePermission) == QtAndroid::PermissionResult::Denied) {
            QtAndroid::PermissionResultMap resultHash = QtAndroid::requestPermissionsSync(QStringList({ readPermission, writePermission }));
            if (resultHash[readPermission] == QtAndroid::PermissionResult::Denied ||
                resultHash[writePermission] == QtAndroid::PermissionResult::Denied) {
                qWarning() << "Storage permissions denied for Android < 10";
                return QString();
            }
        }
    }

    return sdCardPath;
}

bool AndroidInterface::checkStoragePermission()
{
    if (QtAndroid::androidSdkVersion() < 30) {
        return true;
    }

    QtAndroid::PermissionResult permissionCheck = QtAndroid::checkPermission("android.permission.MANAGE_EXTERNAL_STORAGE");
    if (permissionCheck == QtAndroid::PermissionResult::Granted) {
        return true;
    }

    QtAndroid::requestPermissionsSync(QStringList() << "android.permission.MANAGE_EXTERNAL_STORAGE");
    permissionCheck = QtAndroid::checkPermission("android.permission.MANAGE_EXTERNAL_STORAGE");
    if (permissionCheck == QtAndroid::PermissionResult::Granted) {
        return true;
    }

    QAndroidJniObject activity = QtAndroid::androidActivity();
    QAndroidJniObject packageName = activity.callObjectMethod("getPackageName", "()Ljava/lang/String;");

    try {
        QAndroidJniObject intent(
            "android/content/Intent",
            "(Ljava/lang/String;)V",
            QAndroidJniObject::getStaticObjectField(
                "android/provider/Settings",
                "ACTION_MANAGE_ALL_FILES_ACCESS_PERMISSION",
                "Ljava/lang/String;"
                ).object()
            );

        QAndroidJniObject uri = QAndroidJniObject::callStaticObjectMethod(
            "android/net/Uri",
            "parse",
            "(Ljava/lang/String;)Landroid/net/Uri;",
            QAndroidJniObject::fromString("package:" + packageName.toString()).object()
            );

        intent.callObjectMethod(
            "setData",
            "(Landroid/net/Uri;)Landroid/content/Intent;",
            uri.object()
            );

        QAndroidJniObject manager = activity.callObjectMethod(
            "getPackageManager",
            "()Landroid/content/pm/PackageManager;"
            );

        bool isIntentSafe = intent.callMethod<jboolean>(
            "resolveActivity",
            "(Landroid/content/pm/PackageManager;)Z",
            manager.object()
            );

        if (isIntentSafe) {
            QtAndroid::startActivity(intent, 0);
        } else {
            qWarning() << "No activity found to handle manage storage permission intent";
        }
    } catch (...) {
        qCritical() << "Exception occurred while trying to request storage permission";
        return false;
    }

    return false;
}
