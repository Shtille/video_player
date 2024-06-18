#
# CMake file for ffmpeg for Windows.
# It should be built separately as a number of static libraries: avformat, avdevice, etc.
#

# MSYS_LIBRARY_PATH should be set as environment variable by user
set(link_directories
  "$ENV{MSYS_LIBRARY_PATH}"
)

# avcodec
project(avcodec_interface)
set(libs
	avcodec iconv m atomic mfuuid ole32 strmiids ole32 user32 z
)
add_library(${PROJECT_NAME} INTERFACE)
target_link_directories(${PROJECT_NAME} INTERFACE ${link_directories})
target_link_libraries(${PROJECT_NAME} INTERFACE ${libs})

# avdevice
project(avdevice_interface)
set(libs
	avdevice psapi ole32 strmiids uuid oleaut32 shlwapi gdi32 m atomic vfw32
)
add_library(${PROJECT_NAME} INTERFACE)
target_link_directories(${PROJECT_NAME} INTERFACE ${link_directories})
target_link_libraries(${PROJECT_NAME} INTERFACE ${libs})

# avfilter
project(avfilter_interface)
set(libs
	avfilter m atomic
)
add_library(${PROJECT_NAME} INTERFACE)
target_link_directories(${PROJECT_NAME} INTERFACE ${link_directories})
target_link_libraries(${PROJECT_NAME} INTERFACE ${libs})

# avformat
project(avformat_interface)
set(libs
	avformat m atomic z secur32 ws2_32
)
add_library(${PROJECT_NAME} INTERFACE)
target_link_directories(${PROJECT_NAME} INTERFACE ${link_directories})
target_link_libraries(${PROJECT_NAME} INTERFACE ${libs})

# avutil
project(avutil_interface)
set(libs
	avutil m user32 bcrypt atomic
)
add_library(${PROJECT_NAME} INTERFACE)
target_link_directories(${PROJECT_NAME} INTERFACE ${link_directories})
target_link_libraries(${PROJECT_NAME} INTERFACE ${libs})

# swresample
project(swresample_interface)
set(libs
	swresample m atomic
)
add_library(${PROJECT_NAME} INTERFACE)
target_link_directories(${PROJECT_NAME} INTERFACE ${link_directories})
target_link_libraries(${PROJECT_NAME} INTERFACE ${libs})

# swscale
project(swscale_interface)
set(libs
	swscale m atomic
)
add_library(${PROJECT_NAME} INTERFACE)
target_link_directories(${PROJECT_NAME} INTERFACE ${link_directories})
target_link_libraries(${PROJECT_NAME} INTERFACE ${libs})