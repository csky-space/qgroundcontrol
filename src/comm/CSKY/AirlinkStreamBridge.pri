message([DEBUG] PWD: $$PWD)

MAIN_PWD = $$PWD/../../..
GO_SRC_DIR = $$PWD/AirlinkStreamBridge
GO_SRC = $$GO_SRC_DIR/main.go
!android {
    GO_OUT_BASE = $$OUT_PWD/
} else {
    GO_OUT_BASE = $$MAIN_PWD/android/libs/
}
GO_FILES = $$files($$GO_SRC_DIR/*.go, true)

win32 {
    GO_EXT = .exe
} else:android {
    GO_EXT = .aar
} else {
    GO_EXT =
}
GO_EXECUTABLE_NAME="AirlinkStreamBridge$$GO_EXT"
GO_OUT_FULL = $$GO_OUT_BASE$$GO_EXECUTABLE_NAME

go_target.name = AirlinkStreamBridgeCompiler
go_target.input = GO_FILES

go_target.CONFIG += combine ignore_no_exist no_link no_clean target_predeps

android {
    go_target.commands = \
        cd $$PWD/AirlinkStreamBridge/mobile && \
        gomobile bind -target=android -javapkg=com.csky.airlinkstreambridge -ldflags=\"-s -w -checklinkname=0\" -o \"$$GO_OUT_FULL\"
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
    src/comm/CSKY/Airlink/AirlinkConfiguration.h \
    src/comm/CSKY/Airlink/AirlinkManager.h \
    src/comm/CSKY/Airlink/airlinkstreambridgemanager.h \
    src/Settings/AirlinkStreamBridgeSettings.h \
    $$PWD/Airlink/Airlink.h \
    $$PWD/Airlink/AirlinkTelemetry.h \
    $$PWD/Airlink/AirlinkVideo.h

SOURCES += \
    src/Settings/AirlinkStreamBridgeSettings.cc \
    src/comm/CSKY/Airlink/AirlinkConfiguration.cc \
    src/comm/CSKY/Airlink/AirlinkManager.cc \
    src/comm/CSKY/Airlink/airlinkstreambridgemanager.cc \
    $$PWD/Airlink/Airlink.cc \
    $$PWD/Airlink/AirlinkTelemetry.cc \
    $$PWD/Airlink/AirlinkVideo.cc
