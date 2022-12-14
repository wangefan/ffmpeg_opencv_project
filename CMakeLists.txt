cmake_minimum_required(VERSION 2.8)

# project name
project(ffmpegOpencvProject)

# set include
set(ffmpeg_headers_DIR /usr/include/x86_64-linux-gnu)
include_directories(${CMAKE_SOURCE_DIR})
include_directories(${ffmpeg_headers_DIR}/include)
include_directories(${OpenCV_INCLUDE_DIRS})

# Libraries
find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBAV REQUIRED IMPORTED_TARGET
    libavformat
    libavcodec
    libavutil
    libswresample
    libswscale
)
find_package(OpenCV REQUIRED)

# show message of finding openCV
message(STATUS "Found OpenCV_INCLUDE_DIRS = ${OpenCV_INCLUDE_DIRS}")
message(STATUS "Found OpenCV_LIBS = ${OpenCV_LIBS}")

# set source
#set(source ${CMAKE_SOURCE_DIR}/ERT/Configuration.cpp ${CMAKE_SOURCE_DIR}/ERT/ERT.cpp ${CMAKE_SOURCE_DIR}/ERT/Regressor.cpp ${CMAKE_SOURCE_DIR}/#ERT/Ferm.cpp ${CMAKE_SOURCE_DIR}/ERT/Utils.cpp ${CMAKE_SOURCE_DIR}/ERT/SampleData.cpp)

# add exe
add_executable(${PROJECT_NAME} ${CMAKE_SOURCE_DIR}/src/main.cpp  ${source})

# link libraries
target_link_libraries(${PROJECT_NAME} ${OpenCV_LIBS} PkgConfig::LIBAV)