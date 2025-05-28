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
GO_EXECUTABLE_NAME="AirlinkStreamBridge$$GO_EXT"
GO_OUT_FULL = $$GO_OUT_BASE$$GO_EXT

go_target.name = AirlinkStreamBridgeCompiler
go_target.input = GO_FILES

go_target.CONFIG += combine ignore_no_exist no_link no_clean target_predeps

android {
    GOOS = android
    NDK_API = 21
    NDK = $$getenv("ANDROID_NDK_ROOT")
    GOARM = 7
    contains(ANDROID_TARGET_ARCH, x86) {
        GOARCH = 386
        ARCH = x86
        CC = $$NDK/toolchains/llvm/prebuilt/linux-x86_64/bin/i686-linux-android$$NDK_API-clang
        RESOURCES_DIR = $$MAIN_PWD/resources/$$ARCH
        GO_OUT_FULL = $$RESOURCES_DIR/$${GO_EXECUTABLE_NAME}_$${ARCH}

        go_target.commands = \
            cd $$GO_SRC_DIR && \
            CC=$$CC CGO_ENABLED=1 GOOS=$$GOOS GOARCH=$$GOARCH \
            mkdir -p $$RESOURCES_DIR && \
            go build $$quote(-ldflags=\"-s -w -checklinkname=0\") -o $$GO_OUT_FULL main.go
    } else:contains(ANDROID_TARGET_ARCH, arm64-v8a) {
        GOARCH = arm64
        ARCH = arm64-v8a
        CC = $$NDK/toolchains/llvm/prebuilt/linux-x86_64/bin/aarch64-linux-android$$NDK_API-clang
        RESOURCES_DIR = $$MAIN_PWD/resources/$$ARCH
        GO_OUT_FULL = $$RESOURCES_DIR/$${GO_EXECUTABLE_NAME}_$${ARCH}

        go_target.commands = \
            cd $$GO_SRC_DIR && \
            CC=$$CC CGO_ENABLED=1 GOOS=$$GOOS GOARCH=$$GOARCH \
            mkdir -p $$RESOURCES_DIR && \
            go build $$quote(-ldflags=\"-s -w -checklinkname=0\") -o $$GO_OUT_FULL main.go

    } else:contains(ANDROID_TARGET_ARCH, armeabi-v7a) {
        GOARCH = arm
        ARCH = armeabi-v7a
        CC = $$NDK/toolchains/llvm/prebuilt/linux-x86_64/bin/armv7a-linux-androideabi$$NDK_API-clang
        RESOURCES_DIR = $$MAIN_PWD/resources/$$ARCH
        GO_OUT_FULL = $$RESOURCES_DIR/$${GO_EXECUTABLE_NAME}_$${ARCH}

        go_target.commands = \
            cd $$GO_SRC_DIR && \
            CC=$$CC CGO_ENABLED=1 GOOS=$$GOOS GOARCH=$$GOARCH GOARM=$$GOARM \
            mkdir -p $$RESOURCES_DIR && \
            go build $$quote(-ldflags=\"-s -w -checklinkname=0\") -o $$GO_OUT_FULL main.go
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

go_target.output = $$GO_OUT_FULL
#PRE_TARGETDEPS += $$GO_OUT_FULL

export(GO_OUT_FULL)
message([DEBUG] GO_OUT_FULL: $$GO_OUT_FULL)
export(GO_SRC_DIR)
export(go_target)



HEADERS += \
    $$PWD/Airlink/Airlink.h

SOURCES += \
    $$PWD/Airlink/Airlink.cc
