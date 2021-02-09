cmake_minimum_required(VERSION 3.10)

# set the project name
project(Extrude)

# glob the files, add the executable
file(GLOB extrude_SRC 
    "header_builds.c" 
    "main.cpp"
    "libs/*.cpp"
    "src/*.cpp"
    "src/types/*.cpp")
add_executable(Extrude ${extrude_SRC})


##################################################################
#
#   Set Target Manually:
#
#       "Mac"   MacOS
#       "Web"   Emscripten
#
##################################################################
set(ExportTarget "Mac")


##################################################################
# Sokol Gfx Requires Libraries:
# - on macOS with Metal:    Cocoa, IOKit, QuartzCore, Metal, MetalKit, Foundation
# - on macOS with GL:       Cocoa, IOKit, QuartzCore, OpenGL, Foundation
# - on iOS with Metal:      Foundation, UIKit, Metal, MetalKit
# - on iOS with GL:         Foundation, UIKit, OpenGLES, GLKit
# - on Linux:               X11, Xi, Xcursor, GL, dl, pthread, m(?)
# - on Android:             GLESv3, EGL, log, android
# - on Windows:             no action needed, libs are defined in-source via pragma-comment-lib
#
# Sokol Audio Requires
# - Windows:                WASAPI
# - Linux:                  ALSA (link with asound)
# - macOS/iOS:              CoreAudio (link with AudioToolbox)
# - emscripten:             WebAudio with ScriptProcessorNode
# - Android:                OpenSLES (link with OpenSLES)
##################################################################
#if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
if (ExportTarget MATCHES "Mac")
    find_library(COCOA_LIBRARY Cocoa REQUIRED)
    find_library(IOKIT_LIBRARY IOKit REQUIRED)
    find_library(OPENGL_LIBRARY OpenGL REQUIRED)
    find_library(FOUNDATION_LIBRARY Foundation REQUIRED)
    find_library(METAL_LIBRARY Metal REQUIRED)
    find_library(METALKIT_LIBRARY MetalKit REQUIRED)
    find_library(AUDIOKIT_LIBRARY AudioToolbox REQUIRED)
    message(${COCOA_LIBRARY})
    message(${IOKIT_LIBRARY})
    message(${OPENGL_LIBRARY})
    message(${FOUNDATION_LIBRARY})
    message(${METAL_LIBRARY})
    message(${METALKIT_LIBRARY})
    message(${AUDIOKIT_LIBRARY})
    target_link_libraries(Extrude ${COCOA_LIBRARY})
    target_link_libraries(Extrude ${IOKIT_LIBRARY})
    target_link_libraries(Extrude ${OPENGL_LIBRARY})
    target_link_libraries(Extrude ${FOUNDATION_LIBRARY})
    target_link_libraries(Extrude ${METAL_LIBRARY})
    target_link_libraries(Extrude ${METALKIT_LIBRARY})
    target_link_libraries(Extrude ${AUDIOKIT_LIBRARY})

    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -xobjective-c -fobjc-arc")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -fobjc-arc")

    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -framework Cocoa -framework OpenGL -framework IOKit -framework Foundation")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -framework Metal -framework MetalKit -framework AudioToolbox")

    set(CMAKE_C_COMPILER "gcc")
    set(CMAKE_CXX_COMPILER "g++")


elseif (ExportTarget MATCHES "Web")
    set(CMAKE_C_COMPILER "emcc")
    set(CMAKE_CXX_COMPILER "emcc")

endif()


