file(GLOB CS_AIRLINK_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/CSKY/*.qml)

add_custom_target(UiPreferencesQml
	SOURCES
		BluetoothSettings.qml
		DebugWindow.qml
		GeneralSettings.qml
		HelpSettings.qml
		LinkSettings.qml
		LogReplaySettings.qml
		MavlinkSettings.qml
		MockLink.qml
		MockLinkSettings.qml
		SerialSettings.qml
		TcpSettings.qml
		UdpSettings.qml
		${CS_AIRLINK_SOURCES}
)
