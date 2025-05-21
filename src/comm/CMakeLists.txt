
set(EXTRA_SRC)
if(BUILD_TESTING)
	list(APPEND EXTRA_SRC
		MockLink.cc
		MockLink.h
		MockLinkFTP.cc
		MockLinkFTP.h
		MockLinkMissionItemHandler.cc
		MockLinkMissionItemHandler.h
	)
endif()

file(GLOB_RECURSE CS_AIRLINK_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/CSKY/Airlink/*.cc)
file(GLOB_RECURSE CS_AIRLINK_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/CSKY/Airlink/*.h)

add_library(comm
	#BluetoothLink.cc
	#BluetoothLink.h
	LinkConfiguration.cc
	LinkConfiguration.h
	LinkInterface.cc
	LinkInterface.h
	LinkManager.cc
	LinkManager.h
	LogReplayLink.cc
	LogReplayLink.h
	MavlinkMessagesTimer.cc
	MavlinkMessagesTimer.h
	MAVLinkProtocol.cc
	MAVLinkProtocol.h
	QGCMAVLink.cc
	QGCMAVLink.h
	QGCSerialPortInfo.cc
	QGCSerialPortInfo.h
	SerialLink.cc
	SerialLink.h
	TCPLink.cc
	TCPLink.h
	UdpIODevice.cc
	UdpIODevice.h
	UDPLink.cc
	UDPLink.h

	${EXTRA_SRC}

	${CS_AIRLINK_SOURCES}
	${CS_AIRLINK_HEADERS}
)

add_subdirectory(CSKY/AirlinkStreamBridge)

target_link_libraries(comm
	PUBLIC
		qgc
		Qt5::Location
		Qt5::SerialPort
		Qt5::Test
		Qt5::TextToSpeech
		Qt5::Widgets
		qmdnsengine
)

target_include_directories(comm INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
