# CMakeLists file for ffmpeg static libraries

if (WIN32)
	include(ffmpeg_win.cmake)
else (WIN32)
	message(ERROR " CMake file for Unix build has not been implemented yet")
endif (WIN32)

project(ffmpeg)

set(libraries
	avcodec_interface
	# avdevice_interface
	avfilter_interface
	avformat_interface
	avutil_interface
	swresample_interface
	swscale_interface
)
add_library(${PROJECT_NAME} INTERFACE)
target_link_libraries(${PROJECT_NAME} INTERFACE ${libraries})