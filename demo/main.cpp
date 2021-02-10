//
// Description:     3D Extrusion
// Author:          Stephens Nunnally and Scidian Software
// License:         Distributed under the MIT License
// Source(s):       https://github.com/stevinz/extrude
//
// Copyright (c) 2021 Stephens Nunnally and Scidian Software
//
//
//################################################################################
//##    Includes
//################################################################################
#include "../src/3rd_party/handmade_math.h"
#include "../src/3rd_party/stb/stb_image.h"
#include "../src/imaging.h"
#include "../src/random.h"
#include "../src/types/bitmap.h"
#include "../src/types/color.h"
#include "../src/types/image.h"
#include "../src/types/rect.h"

#include <iostream>

#include "3rd_party/sokol/sokol_app.h"
#include "3rd_party/sokol/sokol_gfx.h"
#include "3rd_party/sokol/sokol_glue.h"
#include "3rd_party/sokol/sokol_time.h"
#include "3rd_party/sokol/sokol_audio.h"
#include "3rd_party/sokol/sokol_fetch.h"
#ifndef __EMSCRIPTEN__
    #include "3rd_party/wai/whereami.c"
#endif

#include "shader.glsl.h"


//################################################################################
//##    Local Structs / Defines
//################################################################################
#define MAX_FILE_SIZE (1024 * 1024)

typedef enum {
    LOADSTATE_UNKNOWN = 0,
    LOADSTATE_SUCCESS,
    LOADSTATE_FAILED,
    LOADSTATE_FILE_TOO_BIG,
} loadstate_t;

static struct {
    float rx, ry;
    sg_pass_action pass_action;
    sg_pipeline pip;
    sg_bindings bind;
    loadstate_t load_state;
    uint32_t size;
    uint8_t file_buffer[MAX_FILE_SIZE];
} state;

typedef struct {
    float x, y, z;
    int16_t u, v;
} vertex_t;

static void fetch_callback(const sfetch_response_t*);

size_t stringLength(const char *strPtr) {
    size_t n = 0;
    while (strPtr[n]) ++n;
    return n;
}

char* stringAdd(char *strPtr, const char *strPtr2) {
    char *p = strPtr + stringLength( strPtr );
    while ((*p++ = *strPtr2++));
    return strPtr;
}

bool initialized_image = false;


//################################################################################
//##    Blend Functions
//################################################################################
// Normal
sg_blend_state (sokol_blend_normal) {
    .enabled =              true,
    .src_factor_rgb =       SG_BLENDFACTOR_ONE,
    .dst_factor_rgb =       SG_BLENDFACTOR_ZERO,
    .op_rgb =               SG_BLENDOP_ADD,
    .src_factor_alpha =     SG_BLENDFACTOR_ONE,
    .dst_factor_alpha =     SG_BLENDFACTOR_ZERO,
    .op_alpha =             SG_BLENDOP_ADD,
    .color_write_mask =     SG_COLORMASK_RGBA
};
// Alpha Enabled
sg_blend_state (sokol_blend_alpha) {
    .enabled =              true,
    .src_factor_rgb =       SG_BLENDFACTOR_SRC_ALPHA,
    .dst_factor_rgb =       SG_BLENDFACTOR_ONE_MINUS_SRC_ALPHA,
    .op_rgb =               SG_BLENDOP_ADD,
    .src_factor_alpha =     SG_BLENDFACTOR_SRC_ALPHA,
    .dst_factor_alpha =     SG_BLENDFACTOR_ONE_MINUS_SRC_ALPHA,
    .op_alpha =             SG_BLENDOP_ADD,
    .color_write_mask =     SG_COLORMASK_RGBA
};

///glBlendFuncSeparate(GL_ONE, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);        // Premultiplied alpha blend
// typedef enum sg_blend_factor {
//     _SG_BLENDFACTOR_DEFAULT,    /* value 0 reserved for default-init */
//     SG_BLENDFACTOR_ZERO,
//     SG_BLENDFACTOR_ONE,
//     SG_BLENDFACTOR_SRC_COLOR,
//     SG_BLENDFACTOR_ONE_MINUS_SRC_COLOR,
//     SG_BLENDFACTOR_SRC_ALPHA,
//     SG_BLENDFACTOR_ONE_MINUS_SRC_ALPHA,
//     SG_BLENDFACTOR_DST_COLOR,
//     SG_BLENDFACTOR_ONE_MINUS_DST_COLOR,
//     SG_BLENDFACTOR_DST_ALPHA,
//     SG_BLENDFACTOR_ONE_MINUS_DST_ALPHA,
//     SG_BLENDFACTOR_SRC_ALPHA_SATURATED,
//     SG_BLENDFACTOR_BLEND_COLOR,
//     SG_BLENDFACTOR_ONE_MINUS_BLEND_COLOR,
//     SG_BLENDFACTOR_BLEND_ALPHA,
//     SG_BLENDFACTOR_ONE_MINUS_BLEND_ALPHA,
//     _SG_BLENDFACTOR_NUM,
//     _SG_BLENDFACTOR_FORCE_U32 = 0x7FFFFFFF
// } sg_blend_factor;



//################################################################################
//##    Initialize
//################################################################################
void init(void) {
    // ***** Setup sokol-gfx, call sokol_glue function to obtain values from sokol_app
    sg_desc (sokol_gfx) {
        .context = sapp_sgcontext()
    };            
    sg_setup(&sokol_gfx);

    // ***** Setup sokol-fetch (for loading files) with the minimal "resource limits"
    #if !defined(__EMSCRIPTEN__)
        sfetch_desc_t (sokol_fetch) {
            .max_requests = 1,
            .num_channels = 1,
            .num_lanes = 1
        };
        sfetch_setup(&sokol_fetch);
    #endif
    
    // ***** Pass action for clearing the framebuffer to some color
    sg_pass_action (pass_action) {
        .colors[0] = { .action = SG_ACTION_CLEAR, .val = { 0.125f, 0.25f, 0.35f, 1.0f } }
    };
    state.pass_action = (pass_action);


    // ***** Cube vertex buffer with packed texcoords
    const vertex_t vertices[] = {
        /* pos                  uvs */
        { -1.0f, -1.0f, -1.0f,      0,     0 },
        {  1.0f, -1.0f, -1.0f,  32767,     0 },
        {  1.0f,  1.0f, -1.0f,  32767, 32767 },
        { -1.0f,  1.0f, -1.0f,      0, 32767 },

        { -1.0f, -1.0f,  1.0f,      0,     0 },
        {  1.0f, -1.0f,  1.0f,  32767,     0 },
        {  1.0f,  1.0f,  1.0f,  32767, 32767 },
        { -1.0f,  1.0f,  1.0f,      0, 32767 },

        { -1.0f, -1.0f, -1.0f,      0,     0 },
        { -1.0f,  1.0f, -1.0f,  32767,     0 },
        { -1.0f,  1.0f,  1.0f,  32767, 32767 },
        { -1.0f, -1.0f,  1.0f,      0, 32767 },

        {  1.0f, -1.0f, -1.0f,      0,     0 },
        {  1.0f,  1.0f, -1.0f,  32767,     0 },
        {  1.0f,  1.0f,  1.0f,  32767, 32767 },
        {  1.0f, -1.0f,  1.0f,      0, 32767 },

        { -1.0f, -1.0f, -1.0f,      0,     0 },
        { -1.0f, -1.0f,  1.0f,  32767,     0 },
        {  1.0f, -1.0f,  1.0f,  32767, 32767 },
        {  1.0f, -1.0f, -1.0f,      0, 32767 },

        { -1.0f,  1.0f, -1.0f,      0,     0 },
        { -1.0f,  1.0f,  1.0f,  32767,     0 },
        {  1.0f,  1.0f,  1.0f,  32767, 32767 },
        {  1.0f,  1.0f, -1.0f,      0, 32767 },
    };
    sg_buffer_desc (sokol_buffer_vertex) {
        .size = sizeof(vertices),
        .content = vertices,
        .label = "cube-vertices"
    };
    state.bind.vertex_buffers[0] = sg_make_buffer(&sokol_buffer_vertex);


    // ***** Create an index buffer for the cube
    const uint16_t indices[] = {
         0,  1,  2,   0,  2,  3,
         6,  5,  4,   7,  6,  4,
         8,  9, 10,   8, 10, 11,
        14, 13, 12,  15, 14, 12,
        16, 17, 18,  16, 18, 19,
        22, 21, 20,  23, 22, 20
    };
    sg_buffer_desc (sokol_buffer_index) {
        .type = SG_BUFFERTYPE_INDEXBUFFER,
        .size = sizeof(indices),
        .content = indices,
        .label = "cube-indices"
    };
    state.bind.index_buffer = sg_make_buffer(&sokol_buffer_index);


    // ***** Pipeline State Object, sets 3D device parameters
    sg_pipeline_desc (sokol_pipleine) {
        .shader = sg_make_shader(extrude3D_shader_desc()),
        .layout = {
            .attrs = {
                [ATTR_vs_pos].format = SG_VERTEXFORMAT_FLOAT3,
                [ATTR_vs_texcoord0].format = SG_VERTEXFORMAT_SHORT2N
            }
        },
        .index_type = SG_INDEXTYPE_UINT16,
        .depth_stencil = {
            .depth_compare_func = SG_COMPAREFUNC_LESS_EQUAL,
            .depth_write_enabled = true
        },
        .rasterizer = {
            .cull_mode = SG_CULLMODE_BACK,
        },
        .label = "cube-pipeline",
        .blend = sokol_blend_alpha
    };
    state.pip = sg_make_pipeline(&sokol_pipleine);


    // ***** Allocate an image handle, 
    //  but don't actually initialize the image yet, this happens later when the asynchronous file load has finished.
    //  Any draw calls containing such an "incomplete" image handle will be silently dropped.
    state.bind.fs_images[SLOT_tex] = sg_alloc_image();

    // ***** Start loading the PNG File
    //  We don't need the returned handle since we can also get that inside the fetch-callback from the response
    //  structure. NOTE: we're not using the user_data member, since all required state is in a global variable anyway
    char* path = NULL;
    int length, dirname_length;
    std::string image_file = "";

    #ifndef __EMSCRIPTEN__
        length = wai_getExecutablePath(NULL, 0, &dirname_length);
        if (length > 0) {
            path = (char*)malloc(length + 1);
            wai_getExecutablePath(path, length, &dirname_length);
            path[length] = '\0';

            printf("executable path: %s\n", path);
            path[dirname_length] = '\0';
            printf("  dirname: %s\n", path);
            printf("  basename: %s\n", path + dirname_length + 1);
            //free(path);
        }
        image_file = std::strcat(path, "/../assets/shapes.png");
        // std::cout << "full: " << image_file << std::endl << "Cube" << std::endl;

        sfetch_request_t (sokol_fetch_response) {
            .path = image_file.c_str(),
            .callback = fetch_callback,
            .buffer_ptr = state.file_buffer,
            .buffer_size = sizeof(state.file_buffer)
        };
        sfetch_send(&sokol_fetch_response);
    #else        
        image_file = "assets/shapes.png";
    #endif
    
}


//################################################################################
//##    Load Image
//################################################################################
static void load_image(stbi_uc *buffer_ptr, int fetched_size) {
    int png_width, png_height, num_channels;
    const int desired_channels = 4;
    stbi_uc* pixels = stbi_load_from_memory(buffer_ptr, fetched_size, &png_width, &png_height, 
                                            &num_channels, desired_channels);
    if (pixels) {

        DrBitmap bitmap = DrBitmap(pixels, static_cast<int>(png_width * png_height * 4), false, png_width, png_height);
        //bitmap.forceAlpha();
        //bitmap = Dr::ApplySinglePixelFilter(Image_Filter_Type::Hue, bitmap, Dr::RandomInt(-100, 100));
        DrImage *dr_image = new DrImage("shapes", bitmap);

        // Initialze the sokol-gfx texture
        sg_image_desc (sokol_image) {
            .width =  dr_image->getBitmap().width,
            .height = dr_image->getBitmap().height,
            .pixel_format = SG_PIXELFORMAT_RGBA8,
            .min_filter = SG_FILTER_LINEAR,
            .mag_filter = SG_FILTER_LINEAR,
            .content.subimage[0][0] = {
                .ptr =  &(dr_image->getBitmap().data[0]),
                .size = dr_image->getBitmap().size(),
            }
        };
        if (initialized_image == true) {
            sg_uninit_image(state.bind.fs_images[SLOT_tex]);
        }
        sg_init_image(state.bind.fs_images[SLOT_tex], &sokol_image);
        initialized_image = true;
        stbi_image_free(pixels);
    }
}

//################################################################################
//##    Callback: File Loading
//################################################################################
/* The fetch-callback is called by sokol_fetch.h when the data is loaded,
   or when an error has occurred. */
static void fetch_callback(const sfetch_response_t* response) {
    if (response->fetched) {
        /* the file data has been fetched, since we provided a big-enough
           buffer we can be sure that all data has been loaded here */
        load_image((stbi_uc *)response->buffer_ptr, (int)response->fetched_size);
    }
    else if (response->finished) {
        // if loading the file failed, set clear color to red
        if (response->failed) {
            sg_pass_action (pass_action0) { .colors[0] = { .action = SG_ACTION_CLEAR, .val = { 1.0f, 0.0f, 0.0f, 1.0f } } };
            sg_pass_action (pass_action1) { .colors[0] = { .action = SG_ACTION_CLEAR, .val = { 0.0f, 1.0f, 0.0f, 1.0f } } };
            sg_pass_action (pass_action2) { .colors[0] = { .action = SG_ACTION_CLEAR, .val = { 0.0f, 0.0f, 1.0f, 1.0f } } };

            switch (response->error_code) {
                case SFETCH_ERROR_FILE_NOT_FOUND:   state.pass_action = (pass_action0);     break;
                case SFETCH_ERROR_NO_BUFFER:        state.pass_action = (pass_action1);     break;
                default:                            state.pass_action = (pass_action2);
            }            
        }
    }
}


//################################################################################
//##    Callback: File Dropped
//################################################################################
#if defined(__EMSCRIPTEN__)
// the async-loading callback for sapp_html5_fetch_dropped_file
static void emsc_load_callback(const sapp_html5_fetch_response* response) {
    if (response->succeeded) {
        state.load_state = LOADSTATE_SUCCESS;
        load_image((stbi_uc *)response->buffer_ptr, (int)response->fetched_size);
    } else if (SAPP_HTML5_FETCH_ERROR_BUFFER_TOO_SMALL == response->error_code) {
        state.load_state = LOADSTATE_FILE_TOO_BIG;
    } else {
        state.load_state = LOADSTATE_FAILED;
    }
}
#else
// the async-loading callback for native platforms
static void native_load_callback(const sfetch_response_t* response) {
    if (response->fetched) {
        state.load_state = LOADSTATE_SUCCESS;
        load_image((stbi_uc *)response->buffer_ptr, (int)response->fetched_size);
    } else if (response->error_code == SFETCH_ERROR_BUFFER_TOO_SMALL) {
        state.load_state = LOADSTATE_FILE_TOO_BIG;
    } else {
        state.load_state = LOADSTATE_FAILED;
    }
}
#endif


//################################################################################
//##    Input
//################################################################################
static void input(const sapp_event* ev) {
    if (ev->type == SAPP_EVENTTYPE_FILES_DROPPED) {
        #if defined(__EMSCRIPTEN__)
            // on emscripten need to use the sokol-app helper function to load the file data
            sapp_html5_fetch_request (sokol_fetch_request) {
                .dropped_file_index = 0,
                .callback = emsc_load_callback,
                .buffer_ptr = state.file_buffer,
                .buffer_size = sizeof(state.file_buffer),
            };
            sapp_html5_fetch_dropped_file(&sokol_fetch_request);
        #else
            // native platform: use sokol-fetch to load file content
            sfetch_request_t (sokol_fetch_request) {
                .path = sapp_get_dropped_file_path(0),
                .callback = native_load_callback,
                .buffer_ptr = state.file_buffer,
                .buffer_size = sizeof(state.file_buffer)
            };
            sfetch_send(&sokol_fetch_request);
        #endif
    }
}

//################################################################################
//##    Render
//################################################################################
/* The frame-function is fairly boring, note that no special handling is
   needed for the case where the texture isn't loaded yet.
   Also note the sfetch_dowork() function, this is usually called once a
   frame to pump the sokol-fetch message queues.
*/
static void frame(void) {
    /* pump the sokol-fetch message queues, and invoke response callbacks */
    #if !defined(__EMSCRIPTEN__)
        sfetch_dowork();
    #endif

    /* compute model-view-projection matrix for vertex shader */
    hmm_mat4 proj = HMM_Perspective(60.0f, (float)sapp_width()/(float)sapp_height(), 0.01f, 10.0f);
    hmm_mat4 view = HMM_LookAt(HMM_Vec3(0.0f, 1.5f, 6.0f), HMM_Vec3(0.0f, 0.0f, 0.0f), HMM_Vec3(0.0f, 1.0f, 0.0f));
    hmm_mat4 view_proj = HMM_MultiplyMat4(proj, view);
    vs_params_t vs_params;
    state.rx += 1.0f; 
    state.ry += 2.0f;
    hmm_mat4 rxm = HMM_Rotate(state.rx, HMM_Vec3(1.0f, 0.0f, 0.0f));
    hmm_mat4 rym = HMM_Rotate(state.ry, HMM_Vec3(0.0f, 1.0f, 0.0f));
    hmm_mat4 model = HMM_MultiplyMat4(rxm, rym);
    vs_params.mvp = HMM_MultiplyMat4(view_proj, model);

    sg_begin_default_pass(&state.pass_action, sapp_width(), sapp_height());
    sg_apply_pipeline(state.pip);
    sg_apply_bindings(&state.bind);
    sg_apply_uniforms(SG_SHADERSTAGE_VS, SLOT_vs_params, &vs_params, sizeof(vs_params));
    sg_draw(0, 36, 1);

    sg_end_pass();
    sg_commit();
}


//################################################################################
//##    Clean Up
//################################################################################
void cleanup(void) {
    #if !defined(__EMSCRIPTEN__)
        sfetch_shutdown();
    #endif
    sg_shutdown();
}

//################################################################################
//##    App Entry
//################################################################################
sapp_desc sokol_main(int argc, char* argv[]) {
    (void)argc; 
    (void)argv;

    sapp_desc sokol_app {
        .window_title = "3D Extrusion",
        .init_cb = init,
        .frame_cb = frame,
        .cleanup_cb = cleanup,
        .event_cb = input,
        .width = 600,
        .height = 400,
        .enable_dragndrop = true,
        .max_dropped_files = 1,
    };
    return sokol_app;
}












