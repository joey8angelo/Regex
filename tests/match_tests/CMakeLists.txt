cmake_minimum_required(VERSION 3.10)
project(RegexMatchTest)

find_package(Regex REQUIRED)

add_executable(main main.cpp)
target_link_libraries(main PRIVATE Regex::Regex)
