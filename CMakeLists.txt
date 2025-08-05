cmake_minimum_required(VERSION 3.10)
project(Regex VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/source")
set(PUBLIC_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include")

file(GLOB_RECURSE SOURCE_FILES "${SOURCE_DIR}/*.cpp")

add_library(Regex SHARED ${SOURCE_FILES})

if(NOT TARGET CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

target_include_directories(Regex
    PUBLIC
        "$<BUILD_INTERFACE:${PUBLIC_INCLUDE_DIR}>"
        "$<INSTALL_INTERFACE:include>"
        PRIVATE
        "${SOURCE_DIR}"
)

target_compile_options(Regex PRIVATE
    $<$<CONFIG:Debug>:-O0 -g>
    $<$<CONFIG:Release>:-O3 -Wall -Wextra -Werror>
)

install(TARGETS Regex
    EXPORT RegexTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include
)

install(DIRECTORY "${PUBLIC_INCLUDE_DIR}/" DESTINATION include)

install(EXPORT RegexTargets
    FILE RegexConfig.cmake
    NAMESPACE Regex::
    DESTINATION lib/cmake/Regex
)
