cmake_minimum_required(VERSION 3.10)

project(stereo_imu_srv VERSION 0.1 DESCRIPTION "Server for stereo_imu.")

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(srv "src/main.cpp" "src/StereoCam.cpp")
target_include_directories(srv PUBLIC "include" "/usr/lib/opencv" "/usr/include/opencv4")
target_link_libraries(srv opencv_core opencv_highgui opencv_videoio opencv_imgcodecs opencv_imgproc)