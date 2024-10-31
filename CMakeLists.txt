cmake_minimum_required(VERSION 3.10)

project(stereo_imu_srv VERSION 0.1 DESCRIPTION "Server for stereo_imu.")

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(srv "src/main.cpp")