cmake_minimum_required(VERSION 3.10)
project(RegexSpeedTest)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")

find_package(Regex REQUIRED)
add_executable(cpp_Regex_speed cpp_Regex_speed.cpp)
target_link_libraries(cpp_Regex_speed PRIVATE Regex::Regex)
