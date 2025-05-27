message([DEBUG] PWD: $$PWD)

MAIN_PWD = $$PWD/../../..
GO_SRC_DIR = $$PWD/AirlinkStreamBridge
GO_SRC = $$GO_SRC_DIR/main.go
GO_OUT_BASE = $$OUT_PWD/AirlinkStreamBridge
GO_FILES = $$files($$GO_SRC_DIR/*.go, true)

win32 {
    GO_EXT = .exe
} else {
    GO_EXT =
}

GO_OUT_FULL = $$GO_OUT_BASE$$GO_EXT

go_target.target = $$GO_OUT_FULL
go_target.depends = $$GO_FILES
go_target.depends += $$GO_OUT_FULL

android {
    GOOS = android
    NDK_API = 21
    NDK = $$getenv("ANDROID_NDK_ROOT")
    GOARM = 7

    contains(ANDROID_TARGET_ARCH, x86) {
        GOARCH = 386
        CC = $$NDK/toolchains/llvm/prebuilt/linux-x86_64/bin/i686-linux-android$$NDK_API-clang
        RESOURCES_DIR = $$MAIN_PWD/resources/x86
        RESOURCE_TARGET = $$RESOURCES_DIR/AirlinkStreamBridge_x86

        go_target.commands = \
            cd $$GO_SRC_DIR && \
            CC=$$CC CGO_ENABLED=1 GOOS=$$GOOS GOARCH=$$GOARCH \
            go build $$quote(-ldflags=\"-s -w -checklinkname=0\") -o $$GO_OUT_FULL main.go && \
            mkdir -p $$RESOURCES_DIR && \
            cp -f $$GO_OUT_FULL $$RESOURCE_TARGET

    } else:contains(ANDROID_TARGET_ARCH, arm64-v8a) {
        GOARCH = arm64
        CC = $$NDK/toolchains/llvm/prebuilt/linux-x86_64/bin/aarch64-linux-android$$NDK_API-clang
        RESOURCES_DIR = $$MAIN_PWD/resources/arm64-v8a
        RESOURCE_TARGET = $$RESOURCES_DIR/AirlinkStreamBridge_arm64-v8a

        go_target.commands = \
            cd $$GO_SRC_DIR && \
            CC=$$CC CGO_ENABLED=1 GOOS=$$GOOS GOARCH=$$GOARCH \
            go build $$quote(-ldflags=\"-s -w -checklinkname=0\") -o $$GO_OUT_FULL main.go && \
            mkdir -p $$RESOURCES_DIR && \
            cp -f $$GO_OUT_FULL $$RESOURCE_TARGET

    } else:contains(ANDROID_TARGET_ARCH, armeabi-v7a) {
        GOARCH = arm
        CC = $$NDK/toolchains/llvm/prebuilt/linux-x86_64/bin/armv7a-linux-androideabi$$NDK_API-clang
        RESOURCES_DIR = $$MAIN_PWD/resources/armeabi-v7a
        RESOURCE_TARGET = $$RESOURCES_DIR/AirlinkStreamBridge_armeabi-v7a

        go_target.commands = \
            cd $$GO_SRC_DIR && \
            CC=$$CC CGO_ENABLED=1 GOOS=$$GOOS GOARCH=$$GOARCH GOARM=$$GOARM \
            go build $$quote(-ldflags=\"-s -w -checklinkname=0\") -o $$GO_OUT_FULL main.go && \
            mkdir -p $$RESOURCES_DIR && \
            cp -f $$GO_OUT_FULL $$RESOURCE_TARGET
    }

} else:win32 {
    go_target.commands = \
        cd $$GO_SRC_DIR && \
        go build -o $$GO_OUT_FULL main.go && \
        cmd /c "if not exist \"$$OUT_PWD\\staging\" mkdir \"$$OUT_PWD\\staging\" & copy /Y \"$$GO_OUT_FULL\" \"$$OUT_PWD\\staging\\\""
} else {
    go_target.commands = \
        cd $$GO_SRC_DIR && \
        go build -o $$GO_OUT_FULL main.go && \
        mkdir -p $$OUT_PWD/staging && \
        cp $$GO_OUT_FULL $$MAIN_PWD/libs/OpenSSL/linux/*.so* $$OUT_PWD/staging/ && \
        cd $$OUT_PWD/staging/ && \
        ln -sf libssl.so.1.1 libssl.so && ln -sf libcrypto.so.1.1 libcrypto.so
}

export(GO_OUT_FULL)
export(GO_SRC_DIR)

QMAKE_EXTRA_TARGETS += go_target
PRE_TARGETDEPS += $$GO_OUT_FULL

HEADERS += \
    $$PWD/Airlink/Airlink.h

SOURCES += \
    $$PWD/Airlink/Airlink.cpp
