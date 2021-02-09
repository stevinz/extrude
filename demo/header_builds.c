//
// Description:     3D Extrusion
// Author:          Stephens Nunnally and Scidian Software
// License:         Distributed under the MIT License
// Source(s):       https://github.com/stevinz/extrude
//
// Copyright (c) 2021 Stephens Nunnally and Scidian Software
//
//
//############################################################
//##
//##    Single Header Library Initialization
//##
//############################################################
#if defined(_WIN32)
#include <Windows.h>
#endif

#define SOKOL_IMPL
//############################################################
//##    Sokol Libraries... ADJUST GRAPHICS PLATFORM HERE!!
//############################################################
//#define SOKOL_GLES2               // Android, WebAssembly
//#define SOKOL_GLES3               // Android, WebAssembly
//#define SOKOL_WGPU                // Next Gen WebAssembly
//#define SOKOL_D3D11               // Windows, XBox
//#define SOKOL_METAL               // MacOS, iOS, tvOS
#define SOKOL_GLCORE33            // MacOS, Windows, Linux, Switch, Playstation

#include "3rd_party/sokol/sokol_app.h"
#include "3rd_party/sokol/sokol_gfx.h"
#include "3rd_party/sokol/sokol_fetch.h"
#include "3rd_party/sokol/sokol_glue.h"
#include "3rd_party/sokol/sokol_time.h"
#include "3rd_party/sokol/sokol_audio.h"






