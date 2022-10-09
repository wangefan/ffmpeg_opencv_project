cmake_minimum_required(VERSION 2.8)

# project name
project(ffmpegOpencvProject)

# set include
include_directories(${CMAKE_SOURCE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${OpenCV_INCLUDE_DIRS})


# set library
message(STATUS "Found CMAKE_SOURCE_DIR/libs = ${CMAKE_SOURCE_DIR}/libs")
set(ffmpeg_libs_dir ${CMAKE_SOURCE_DIR}/libs)
add_library( swscale SHARED IMPORTED)
add_library( avutil SHARED IMPORTED)
set_target_properties(swscale PROPERTIES IMPORTED_LOCATION ${ffmpeg_libs_dir}/libswscale.so)
set_target_properties(avutil PROPERTIES IMPORTED_LOCATION ${ffmpeg_libs_dir}/libavutil.so)
find_package(OpenCV REQUIRED)

# show message of finding openCV
message(STATUS "Found OpenCV_INCLUDE_DIRS = ${OpenCV_INCLUDE_DIRS}")
message(STATUS "Found OpenCV_LIBS = ${OpenCV_LIBS}")

# set source
#set(source ${CMAKE_SOURCE_DIR}/ERT/Configuration.cpp ${CMAKE_SOURCE_DIR}/ERT/ERT.cpp ${CMAKE_SOURCE_DIR}/ERT/Regressor.cpp ${CMAKE_SOURCE_DIR}/#ERT/Ferm.cpp ${CMAKE_SOURCE_DIR}/ERT/Utils.cpp ${CMAKE_SOURCE_DIR}/ERT/SampleData.cpp)

# add exe
add_executable(${PROJECT_NAME} ${CMAKE_SOURCE_DIR}/src/main.cpp  ${source})

# link libraries
target_link_libraries(${PROJECT_NAME} ${OpenCV_LIBS} swscale avutil)
