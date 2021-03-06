##################################################################
#
#   To compile, set target manually below:
#
#       "auto"      Attempt to autodetect
#       "osx"       MacOS
#       "web"       Emscripten
#       "windows"   Windows
#       "linux"     Linux
#
##################################################################
##################################################################


set(EXPORT_TARGET "auto")


##################################################################
##################################################################
# set cmake version
cmake_minimum_required(VERSION 3.10)

# set the project name
project(extrude)

# glob the files, add the executable
file(GLOB SOURCE_CODE_FILES 
    "example/*.c**"
    "example/3rd_party/*.c**"
    "example/3rd_party/sokol/*.c**"
    "example/3rd_party/whereami/*.c**"
    "src/*.c**"
    "src/3rd_party/*.c**"
    "src/3rd_party/mesh_optimizer/*.c**"
    "src/types/*.c**"
)
add_executable(${PROJECT_NAME} ${SOURCE_CODE_FILES})

# determine target
if (EXPORT_TARGET MATCHES "auto")
    if     (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
        set(EXPORT_TARGET "osx")
    elseif (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
        set(EXPORT_TARGET "windows")
    elseif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
        set(EXPORT_TARGET "linux")
    endif()
endif()
if     (EXPORT_TARGET MATCHES "osx") 
    add_compile_definitions(TARGET_MAC)
elseif (EXPORT_TARGET MATCHES "web") 
    add_compile_definitions(TARGET_WEB)
elseif (EXPORT_TARGET MATCHES "windows") 
    add_compile_definitions(TARGET_WIN)
elseif (EXPORT_TARGET MATCHES "linux") 
    add_compile_definitions(TARGET_LIN)
endif()

# compile Shaders
if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    message("Compiling shader")  
    execute_process (
       COMMAND ./sokol-shdc -i shader.glsl -o example/shader.glsl.h -l glsl330:glsl100:metal_macos:hlsl4
       WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )
endif()


# copy 'assets' directory to 'build' directory
add_custom_command(TARGET ${PROJECT_NAME} PRE_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/assets/ $<TARGET_FILE_DIR:${PROJECT_NAME}>/assets/)


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
if (EXPORT_TARGET MATCHES "osx")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 -xobjective-c -fobjc-arc")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -fobjc-arc")
    set(CMAKE_C_COMPILER "gcc")
    set(CMAKE_CXX_COMPILER "g++")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -framework Cocoa -framework OpenGL -framework IOKit -framework Foundation")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -framework Metal -framework MetalKit -framework AudioToolbox")
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
    target_link_libraries(${PROJECT_NAME} ${COCOA_LIBRARY})
    target_link_libraries(${PROJECT_NAME} ${IOKIT_LIBRARY})
    target_link_libraries(${PROJECT_NAME} ${OPENGL_LIBRARY})
    target_link_libraries(${PROJECT_NAME} ${FOUNDATION_LIBRARY})
    target_link_libraries(${PROJECT_NAME} ${METAL_LIBRARY})
    target_link_libraries(${PROJECT_NAME} ${METALKIT_LIBRARY})
    target_link_libraries(${PROJECT_NAME} ${AUDIOKIT_LIBRARY})

elseif (EXPORT_TARGET MATCHES "web")
    # To optimze file size in order: -O1, -O2, -O3, -Os, -Oz
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Oz -std=c++11 --shell-file ../shell.html") 
    set(CMAKE_C_COMPILER "emcc")
    set(CMAKE_CXX_COMPILER "emcc")
    SET(CMAKE_EXECUTABLE_SUFFIX ".html")

    #   --preload-file assets/shapes.png"       # To allow access to local filesystem
    #   --embed-file assets/shapes.png"         # To embed in js file, not compatible yet with sokol_app
    #   -s ALLOW_MEMORY_GROWTH=1                # To allow for dynamic memory access
    set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "-s TOTAL_MEMORY=33554432 -s DEMANGLE_SUPPORT=1 -s WASM=0 -std=c++11 --bind ")

    # -s TOTAL_MEMORY=X  with X higher than the current value 16777216

    # -s WASM=0                         force javascript or
    # -s WASM=1                         use webassembly
    # --shell-file <path>               create a html file without emscripten logo and debug shell

elseif (EXPORT_TARGET MATCHES "windows")

    ### Don't need to do anything for MSVC... ###

elseif (EXPORT_TARGET MATCHES "linux")

    ### TODO ###

endif()




