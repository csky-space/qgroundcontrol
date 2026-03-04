
add_library(Vibration
	VibrationController.cc
)

target_link_libraries(Vibration
	PUBLIC
		qgc
)

target_include_directories(Vibration
	PUBLIC
		${CMAKE_CURRENT_SOURCE_DIR}
)


