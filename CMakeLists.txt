
cmake_minimum_required(VERSION 3.10)

project(SFMLDemo LANGUAGES CXX)

# Let find_package(SFML) honor SFML_ROOT (CMake 3.12+).
if(POLICY CMP0074)
	cmake_policy(SET CMP0074 NEW)
endif()

# Default build type for single-config generators (Ninja/Make).
if(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build (Debug or Release)" FORCE)
endif()

# For multi-config generators (Visual Studio), limit to Debug and Release.
if(CMAKE_CONFIGURATION_TYPES)
	set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "Limited configurations" FORCE)
	mark_as_advanced(CMAKE_CONFIGURATION_TYPES)
endif()

if(CMAKE_COMPILER_IS_GNUCXX)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
	
	if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
	endif()
endif()

set(SFML_ROOT "" CACHE PATH "Path to SFML root (contains include/ and lib/, not bin/)")
set(SFML_STATIC_LIBRARIES FALSE CACHE BOOL "Use static SFML libraries")
if(SFML_STATIC_LIBRARIES)
	add_definitions(-DSFML_STATIC)
endif()

# Prefer SFML's official CMake package (lib/cmake/SFML/SFMLConfig.cmake).
if(SFML_ROOT)
	list(PREPEND CMAKE_PREFIX_PATH "${SFML_ROOT}")
endif()

find_package(SFML 2 CONFIG REQUIRED COMPONENTS graphics window system audio)

add_subdirectory(Source)
