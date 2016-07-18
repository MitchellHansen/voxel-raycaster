set(PNAME Game)

set(SFML_ROOT root CACHE STRING "User specified path")

set(SFML_VERSION 2.3)

set(SFML_COMPONENTS graphics window system network audio)

cmake_minimum_required(VERSION 3.2)
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR})

project(${PNAME})
find_package(SFML ${SFML_VERSION} COMPONENTS ${SFML_COMPONENTS} REQUIRED)

include_directories(${SFML_INCLUDE_DIR}/include)
include_directories(include)
file(GLOB SOURCES "src/*.cpp")

add_executable(${PNAME} ${SOURCES})

target_link_libraries(${PNAME} ${SFML_LIBRARIES} ${SFML_DEPENDENCIES})

set_property(TARGET ${PNAME} PROPERTY CXX_STANDARD 11) # Use C++11

