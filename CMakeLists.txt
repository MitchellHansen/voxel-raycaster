# Check versions
message(STATUS "CMake version: ${CMAKE_VERSION}")
cmake_minimum_required(VERSION 3.1)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Set the project name
set(PNAME VoxelRaycaster)
project(${PNAME})

# You're probably going to need to change these paths
if (UNIX)
    set(Vulkan_INCLUDE_DIR /home/mrh/vulkan-sdk/x86_64/include)
    set(Vulkan_LIBRARY /home/mrh/vulkan-sdk/source/lib/libvulkan.so)
endif (UNIX)


set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR})

find_package(Vulkan)
message(STATUS "Vulkan found: ${VULKAN_FOUND}")

# Windows requires us to use GLEW to link the GL shit
if (WIN32)
	find_package(GLEW REQUIRED)
	message(STATUS "GLEW found: ${GLEW_FOUND}")
endif()

# beignet only supports EGL and not GLX for the cl_khr_gl_sharing extension
if (UNIX)
	find_package(X11 REQUIRED)
	message(STATUS "XSS found: ${X11_FOUND}")
endif (UNIX)

# Include the directories for the main program
include_directories(${Vulkan_INCLUDE_DIR})

if (UNIX)
	include_directories(${X11_INCLUDE_DIR})
endif()
include_directories(include)

# Glob all thr sources into their values
file(GLOB_RECURSE SOURCES "src/*.cpp")
file(GLOB_RECURSE HEADERS "include/*.h" "include/*.hpp")
file(GLOB_RECURSE KERNELS "kernels/*.cl")
file(GLOB_RECURSE SHADERS "shaders/*.vert" "shaders/*.tesc" "shaders/*.tese" "shaders/*.geom" "shaders/*.frag" "shaders/*.comp")


add_executable(${PNAME} ${SOURCES} ${HEADERS} ${KERNELS} ${SHADERS})

# Follow the sub directory structure to add sub-filters in VS
# Gotta do it one by one unfortunately

foreach (source IN ITEMS ${SOURCES})
	if (IS_ABSOLUTE "${source}")

		get_filename_component(filename ${source} DIRECTORY)

		STRING(REGEX REPLACE "/" "\\\\" filename ${filename})
		
		string(REGEX MATCHALL "src(.*)" substrings ${filename})
		list(GET substrings 0 substring)
		
		SOURCE_GROUP(${substring} FILES ${source}) 
		
	endif()
endforeach()

foreach (source IN ITEMS ${HEADERS})
	if (IS_ABSOLUTE "${source}")

		get_filename_component(filename ${source} DIRECTORY)

		STRING(REGEX REPLACE "/" "\\\\" filename ${filename})
		
		string(REGEX MATCHALL "include(.*)" substrings ${filename})
		list(GET substrings 0 substring)
		
		SOURCE_GROUP(${substring} FILES ${source}) 
		
	endif()
endforeach()

foreach (source IN ITEMS ${KERNELS})
	if (IS_ABSOLUTE "${source}")

		get_filename_component(filename ${source} DIRECTORY)

		STRING(REGEX REPLACE "/" "\\\\" filename ${filename})
		
		string(REGEX MATCHALL "kernels(.*)" substrings ${filename})
		list(GET substrings 0 substring)
		
		SOURCE_GROUP(${substring} FILES ${source}) 
		
	endif()
endforeach()

foreach (source IN ITEMS ${SHADERS})
	if (IS_ABSOLUTE "${source}")

		get_filename_component(filename ${source} DIRECTORY)

		STRING(REGEX REPLACE "/" "\\\\" filename ${filename})
		
		string(REGEX MATCHALL "shaders(.*)" substrings ${filename})
		list(GET substrings 0 substring)
		
		SOURCE_GROUP(${substring} FILES ${source}) 
		
	endif()
endforeach()

target_link_libraries (${PNAME} ${Vulkan_LIBRARY})

if (UNIX)
	target_link_libraries (${PNAME} ${X11_LIBRARIES})
endif()

if (WIN32)
	target_link_libraries (${PNAME} ${GLEW_LIBRARIES})
endif()


if (NOT WIN32)
	target_link_libraries (${PNAME} -lpthread)
endif()

# Setup to use C++14
set_property(TARGET ${PNAME} PROPERTY CXX_STANDARD 14)

