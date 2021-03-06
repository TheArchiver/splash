#
# Copyright (C) 2016 Emmanuel Durand
#
# This file is part of Splash.
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# Splash is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Splash.  If not, see <http://www.gnu.org/licenses/>.
#

cmake_minimum_required(VERSION 3.2)
project(
    splash 
    VERSION 0.6.0
    LANGUAGES C CXX
)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules")

# Set default build type to Release
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build, options are: Debug Release RelWithDebInfo MinSizeRel." FORCE)
endif(NOT CMAKE_BUILD_TYPE)

enable_testing()

#
# Package information
#
set(API_VERSION ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR})
set(PACKAGE_VERSION ${PROJECT_VERSION})

#
# Find the various libraries
#
set(ENV{PKG_CONFIG_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/external/third_parties/lib/pkgconfig:$ENV{PKG_CONFIG_PATH}")
find_package(PkgConfig REQUIRED)

# Mandatory dependencies
pkg_search_module(SNAPPY REQUIRED snappy)
pkg_search_module(GSL REQUIRED gsl)
pkg_search_module(ZMQ REQUIRED libzmq)
pkg_check_modules(FFMPEG REQUIRED libavformat libavcodec libavutil libswscale)

# Optional dependencies
if(UNIX AND NOT APPLE)
    set(Python_ADDITIONAL_VERSIONS 3.4 3.5)
    find_package(PythonLibs)
elseif(APPLE)
    pkg_check_modules(PYTHON python3)
    set(PYTHONLIBS_VERSION_STRING ${PYTHON_VERSION})
    set(PYTHONLIBS_FOUND ${PYTHON_FOUND})
endif()

find_package(Doxygen)
pkg_search_module(GPHOTO libgphoto2)
pkg_search_module(OPENCV opencv)
pkg_search_module(PORTAUDIO portaudio-2.0)
pkg_search_module(SHMDATA shmdata-1.3)

#
# Configuration
#
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DGLM_FORCE_RADIANS")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DDATADIR=\\\"${CMAKE_INSTALL_PREFIX}/share/splash/\\\"")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DSPLASHPREFIX=\\\"${CMAKE_INSTALL_PREFIX}\\\"")

set(HAVE_GPHOTO ${GPHOTO_FOUND})
set(HAVE_OPENCV ${OPENCV_FOUND})
set(HAVE_PORTAUDIO ${PORTAUDIO_FOUND})
set(HAVE_SHMDATA ${SHMDATA_FOUND})
set(HAVE_PYTHON ${PYTHONLIBS_FOUND})

if (FFMPEG_libavformat_VERSION LESS 57)
    message(WARNING "FFmpeg version is older than 3.1, support disabled")
    set(FFMPEG_FOUND 0)
endif()

if(UNIX AND NOT APPLE)
    set(HAVE_LINUX 1)
    set(HAVE_OSX 0)
elseif(APPLE)
    set(HAVE_LINUX 0)
    set(HAVE_OSX 1)
endif()

if (DATAPATH_SDK_PATH)
    if (IS_DIRECTORY "${DATAPATH_SDK_PATH}/include" AND IS_DIRECTORY "${DATAPATH_SDK_PATH}/utils")
        set(HAVE_DATAPATH 1)
    else()
        set(DATAPATH_SDK_PATH "")
        set(HAVE_DATAPATH 0)
    endif()
else()
    set(DATAPATH_SDK_PATH "")
    set(HAVE_DATAPATH 0)
endif()

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/include/config.h.cmake" "${CMAKE_CURRENT_SOURCE_DIR}/include/config.h")

#
# External sources
#
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
add_subdirectory(external/glfw)

#
# Sources
#
add_subdirectory(addons)
add_subdirectory(data)
add_subdirectory(docs)
add_subdirectory(external)
add_subdirectory(osx)
add_subdirectory(src)
add_subdirectory(tests)

#
# CPack related info
# Creates package on Linux. On OSX, it is handled separately (see ./osx/CMakeLists.txt)
#
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Splash, a modular multi-projector video-mapping software")
set(CPACK_PACKAGE_VENDOR "Emmanuel Durand")
set(CPACK_PACKAGE_NAME "${PROJECT_NAME}")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_PACKAGE_VERSION_MAJOR "${PROJECT_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${PROJECT_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${PROJECT_VERSION_PATCH}")

if(UNIX AND NOT APPLE) # Package creation on Linux
    set(CPACK_GENERATOR "DEB")
    set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
    set(CPACK_DEBIAN_PACKAGE_NAME "${CPACK_PACKAGE_NAME}-mapper")
    set(CPACK_DEBIAN_PACKAGE_VERSION "${CPACK_PACKAGE_VERSION}")
    set(CPACK_DEBIAN_PROJECT_HOMEPAGE "https://github.com/paperManu/splash/wiki")
    set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "amd64")
    set(CPACK_DEBIAN_PACKAGE_DEPENDS "libatlas3-base,
                                      libboost-system1.58.0,
                                      libgphoto2-6,
                                      libgsl2,
                                      libopencv-core2.4v5,
                                      libopencv-highgui2.4v5,
                                      libopencv-imgproc2.4v5,
                                      libopencv-video2.4v5,
                                      libxrandr2,
                                      libxi6,
                                      libxinerama1,
                                      libxcursor1,
                                      libzmq5,
                                      libportaudio2,
                                      zip")
    set(CPACK_DEBIAN_PACKAGE_MAINTAINER "${CPACK_PACKAGE_VENDOR} <emmanueldurand@gmail.com>")
    set(CPACK_DEBIAN_PACKAGE_DESCRIPTION "Splash is a free (as in GPL) modular mapping software. Provided that the user creates a 3D model with UV mapping of the projection surface, Splash will take care of calibrating the videoprojectors (intrinsic and extrinsic parameters, blending), and feed them with the input video sources. Splash can handle multiple inputs, mapped on multiple 3D models, and has been tested with up to eight outputs on two graphic cards. It currently runs on a single computer but support for multiple computers mapping together is planned.")
    set(CPACK_DEBIAN_PACKAGE_SECTION "video")
    set(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")
endif()

include(CPack)

#
# Summary
#

set(_config_msg "\nSplash Configuration\n=====================")

function(info_cfg_option
    _setting
    )
    
    set(_msg "  - ${_setting}")
    string(LENGTH "${_msg}" _len)
    while("32" GREATER "${_len}")
        set(_msg "${_msg} ")
         math(EXPR _len "${_len} + 1")
    endwhile()
    
    set(_config_msg "${_config_msg}\n${_msg}${${_setting}}" PARENT_SCOPE)
endfunction()

function(info_cfg_text
    _text
    )
    
    set(_config_msg "${_config_msg}\n\n  ${_text}" PARENT_SCOPE)
endfunction()

info_cfg_option(GPHOTO_VERSION)
info_cfg_option(FFMPEG_libavformat_VERSION)
info_cfg_option(FFMPEG_libavcodec_VERSION)
info_cfg_option(FFMPEG_libavutil_VERSION)
info_cfg_option(FFMPEG_libswscale_VERSION)
info_cfg_option(OPENCV_VERSION)
info_cfg_option(PORTAUDIO_VERSION)
info_cfg_option(SHMDATA_VERSION)
info_cfg_option(PYTHONLIBS_VERSION_STRING)
info_cfg_option(DATAPATH_SDK_PATH)
info_cfg_option(DOXYGEN_FOUND)

info_cfg_text("")
message("${_config_msg}")
