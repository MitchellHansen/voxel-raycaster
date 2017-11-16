# Check versions
message(STATUS "CMake version: ${CMAKE_VERSION}")
cmake_minimum_required(VERSION 3.1)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Set the project name
set(PNAME VoxelRaycaster)
project(${PNAME})

# Set up variables, and find SFML
if (WIN32)
    set(SFML_ROOT root CACHE STRING "User specified path")
    set(SFML_INCLUDE_DIR ${SFML_ROOT}/include)
endif()

set(OPENGL_LIBRARIES "/opt/amdgpu-pro/lib/x86_64-linux-gnu")

set(SFML_COMPONENTS graphics window system network audio)
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR})
find_package(SFML 2.1 COMPONENTS ${SFML_COMPONENTS} REQUIRED)
message(STATUS "SFML found: ${SFML_FOUND}")

# Find OpenCL
find_package( OpenCL REQUIRED )
message(STATUS "OpenCL found: ${OPENCL_FOUND}")

# Find OpenGL
find_package( OpenGL REQUIRED)
message(STATUS "OpenGL found: ${OPENGL_FOUND}")

# Windows requires us to use GLEW to link the GL shit
if (WIN32)
	# Find GLEW
	find_package(GLEW REQUIRED)
	message(STATUS "GLEW found: ${GLEW_FOUND}")
endif()

# beignet only supports EGL and not GLX for the cl_khr_gl_sharing extension
if (UNIX)
#    find_package(EGL REQUIRED)
#message(STATUS "EGL found: ${EGL_FOUND}")
endif (UNIX)

# Include the directories for the main program, GL, CL and SFML's headers
include_directories(${SFML_INCLUDE_DIR})
include_directories(${OpenCL_INCLUDE_DIRS})
include_directories(${OpenGL_INCLUDE_DIRS})
if (UNIX)
    #    include_directories(${EGL_INCLUDE_DIRS})
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

# Link CL, GL, and SFML
target_link_libraries (${PNAME} ${SFML_LIBRARIES} ${SFML_DEPENDENCIES})
target_link_libraries (${PNAME} ${OpenCL_LIBRARY})
target_link_libraries (${PNAME} ${OPENGL_LIBRARIES})

if (UNIX)
    #    target_link_libraries (${PNAME} ${EGL_LIBRARIES})
endif()

if (WIN32)
	target_link_libraries (${PNAME} ${GLEW_LIBRARIES})
endif()


if (NOT WIN32)
	target_link_libraries (${PNAME} -lpthread)
endif()

# Setup to use C++14
set_property(TARGET ${PNAME} PROPERTY CXX_STANDARD 14)

