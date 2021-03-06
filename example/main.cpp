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
#include <memory>
#include "../src/3rd_party/handmade_math.h"
#include "../src/3rd_party/stb/stb_image.h"
#include "../src/compare.h"
#include "../src/imaging.h"
#include "../src/mesh.h"
#include "../src/types/bitmap.h"
#include "../src/types/color.h"
#include "../src/types/image.h"
#include "../src/types/rect.h"
#include "../src/types/vec2.h"

#include "3rd_party/sokol/sokol_app.h"
#include "3rd_party/sokol/sokol_gfx.h"
#include "3rd_party/sokol/sokol_gl.h"
#include "3rd_party/sokol/sokol_glue.h"
#include "3rd_party/sokol/sokol_time.h"
#include "3rd_party/sokol/sokol_audio.h"
#include "3rd_party/sokol/sokol_fetch.h"
#include "3rd_party/fontstash.h"
#include "3rd_party/sokol/sokol_fontstash.h"
#ifndef __EMSCRIPTEN__
    #include "3rd_party/wai/whereami.c"
#else
    #include <emscripten/emscripten.h>
    #include <emscripten/html5.h>
#endif

#include "input.h"
#include "shader.glsl.h"


//################################################################################
//##    Local Structs / Defines
//################################################################################
#define MAX_FILE_SIZE (2048 * 2048)

enum loadstate_t {
    LOADSTATE_UNKNOWN = 0,
    LOADSTATE_SUCCESS,
    LOADSTATE_FAILED,
    LOADSTATE_FILE_TOO_BIG,
};

struct item_t {
    sapp_event event = { };
};

struct state_t {
    // Gfx
    sg_pass_action pass_action;
    sg_pipeline pip;
    sg_bindings bind;

    // Fetch / Drop
    uint8_t file_buffer[MAX_FILE_SIZE];
    loadstate_t load_state;

    // Events
    item_t items[_SAPP_EVENTTYPE_NUM];

    // Font
    FONScontext* fons;
    float   dpi_scale;
    int     font_normal;
    uint8_t font_normal_data[MAX_FILE_SIZE];
};



//################################################################################
//##    Globals
//################################################################################
// Sokol Variables
sapp_desc   sokol_app;
state_t     state;

// Holds generated meshes
std::vector<DrMesh> meshes { };

// Image Variables
DrBitmap    bitmap              { };
DrImage     image               { "None", bitmap };
long        image_size          { 0 };
bool        initialized_image   { false };
bool        recalculate         { false };
std::string load_status         { "" };
int         triangles           { 0 };

// FPS Variables
uint64_t time_start     { 0 };
long ticks              { 0 };
long fps                { 0 };

// Model Rotation
int         mesh_quality        { 5 };
float       level_of_detail     { 100.f };
float       depth_multiplier    { 1.f };
DrVec2      total_rotation      { 0.f,  0.f };
DrVec2      add_rotation        { 25.f, 25.f };
hmm_mat4    model               { Dr::IdentityMatrix() };
DrVec2      mouse_down          { 0, 0 };
float       rotate_speed        { 1.f };
bool        is_mouse_down       { false };
float       zoom                { 1.5f };
bool        wireframe           { true };


//################################################################################
//##    Sokol-fetch load callbacks 
//################################################################################
static void image_loaded(const sfetch_response_t*);
static void font_normal_loaded(const sfetch_response_t* response) {
    if (response->fetched) {
        state.font_normal = fonsAddFontMem(state.fons, "sans", (unsigned char*)response->buffer_ptr, (int)response->fetched_size,  false);
    } 
}


//################################################################################
//##    Initialize
//################################################################################
void init(void) {
    // ***** Setup sokol-gfx, call sokol_glue function to obtain values from sokol_app
    sg_desc sokol_gfx { };
        sokol_gfx.context = sapp_sgcontext(); 
    sg_setup(&sokol_gfx);

    // ***** Setup sokol-gl
    sgl_desc_t (sokol_gl) { 0 };
    sgl_setup(&sokol_gl);

    // ***** Setup sokol-time
    stm_setup();
    time_start = stm_now();

    // ***** Setup sokol-fetch (for loading files) with the minimal "resource limits"
    sfetch_desc_t sokol_fetch { };
        sokol_fetch.max_requests =  4;
        sokol_fetch.num_channels =  2;
        sokol_fetch.num_lanes =     2;
    sfetch_setup(&sokol_fetch);

    // ***** Font Setup, make sure the fontstash atlas width/height is pow-2 
    state.dpi_scale = sapp_dpi_scale();
    const int atlas_dim = Dr::RoundPowerOf2(512.0f * state.dpi_scale);
    FONScontext* fons_context = sfons_create(atlas_dim, atlas_dim, FONS_ZERO_TOPLEFT);
    state.fons = fons_context;
    state.font_normal = FONS_INVALID;          


    // ***** Pass action for clearing the framebuffer to some color
    state.pass_action.colors[0].action = SG_ACTION_CLEAR;
    state.pass_action.colors[0].value = { 0.125f, 0.25f, 0.35f, 1.0f };


    // ***** Starter triangle
    // Vertex buffer
    const Vertex vertices[] = {
        // pos                  normals                uvs          barycentric (wireframe)
        { -1.0f, -1.0f, -1.0f,  1.0f, 1.0f, 1.0f,      0,   0,      1.0f, 1.0f, 1.0f },
        {  1.0f, -1.0f, -1.0f,  1.0f, 1.0f, 1.0f,      1,   0,      1.0f, 1.0f, 1.0f },
        {  1.0f,  1.0f, -1.0f,  1.0f, 1.0f, 1.0f,      1,   1,      1.0f, 1.0f, 1.0f },      
        { -1.0f,  1.0f, -1.0f,  1.0f, 1.0f, 1.0f,      0,   1,      1.0f, 1.0f, 1.0f },      
    };
    sg_buffer_desc sokol_buffer_vertex { };
        sokol_buffer_vertex.data = SG_RANGE(vertices);
        sokol_buffer_vertex.label = "temp-vertices";
    state.bind.vertex_buffers[0] = sg_make_buffer(&sokol_buffer_vertex);

    // Index buffer
    const uint16_t indices[] = { 0, 1, 2, 0, 2, 3 };
    sg_buffer_desc sokol_buffer_index { };
        sokol_buffer_index.type = SG_BUFFERTYPE_INDEXBUFFER;
        sokol_buffer_index.data = SG_RANGE(indices);
        sokol_buffer_index.label = "temp-indices";
    state.bind.index_buffer = sg_make_buffer(&(sokol_buffer_index));

    // ***** Blend mode
    sg_blend_state sokol_blend_alpha { };
        sokol_blend_alpha.enabled =              true;
        sokol_blend_alpha.src_factor_rgb =       SG_BLENDFACTOR_SRC_ALPHA;
        sokol_blend_alpha.dst_factor_rgb =       SG_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
        sokol_blend_alpha.op_rgb =               SG_BLENDOP_ADD;
        sokol_blend_alpha.src_factor_alpha =     SG_BLENDFACTOR_SRC_ALPHA;
        sokol_blend_alpha.dst_factor_alpha =     SG_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
        sokol_blend_alpha.op_alpha =             SG_BLENDOP_ADD;

    // ***** Pipeline State Object, sets 3D device parameters
    sg_pipeline_desc sokol_pipleine { };
        sokol_pipleine.shader = sg_make_shader(extrude3D_shader_desc(sg_query_backend()));
        sokol_pipleine.layout.attrs[ATTR_vs_pos].format =       SG_VERTEXFORMAT_FLOAT3;
        sokol_pipleine.layout.attrs[ATTR_vs_norm].format =      SG_VERTEXFORMAT_FLOAT3;
        sokol_pipleine.layout.attrs[ATTR_vs_texcoord0].format = SG_VERTEXFORMAT_FLOAT2; //SG_VERTEXFORMAT_SHORT2N;
        sokol_pipleine.layout.attrs[ATTR_vs_bary].format =      SG_VERTEXFORMAT_FLOAT3;
        sokol_pipleine.primitive_type = SG_PRIMITIVETYPE_TRIANGLES;
        //sokol_pipleine.index_type =   SG_INDEXTYPE_NONE;
        sokol_pipleine.index_type =     SG_INDEXTYPE_UINT16;
        //sokol_pipleine.cull_mode =    SG_CULLMODE_NONE; 
        sokol_pipleine.cull_mode =      SG_CULLMODE_FRONT;
        sokol_pipleine.depth.compare =  SG_COMPAREFUNC_LESS_EQUAL;
        sokol_pipleine.depth.write_enabled = true;
        sokol_pipleine.label = "extrude-pipeline";
        sokol_pipleine.colors[0].blend = sokol_blend_alpha;
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
    std::string image_file = "", font_file = "";

    #ifndef __EMSCRIPTEN__
        length = wai_getExecutablePath(NULL, 0, &dirname_length);
        if (length > 0) {
            path = (char*)malloc(length + 1);
            wai_getExecutablePath(path, length, &dirname_length);
            //path[length] = '\0';
            //printf("executable path: %s\n", path);
            path[dirname_length] = '\0';
            //printf("  dirname: %s\n", path);
            //printf("  basename: %s\n", path + dirname_length + 1);
            std::string base = path;
            image_file = base + "/assets/shapes.png";
            font_file  = base + "/assets/aileron-regular.otf";
            free(path);
        }
    #else        
        // ********** NOTE: About loading images with Emscripten **********
        //  When running html on local machine, must disable CORS in broswer
        //  On Safari, with 'Develop' menu enabled select "Disable Cross-Origin Restrictions"
        //image_file = "http://github.com/stevinz/extrude/blob/master/assets/shapes.png?raw=true";
        image_file = "assets/shapes.png";
        font_file  = "assets/aileron-regular.otf";
    #endif

    // Load inital "shapes.png" image in background
    sfetch_request_t sokol_fetch_image { };
        sokol_fetch_image.path = image_file.c_str();
        sokol_fetch_image.callback = image_loaded;
        sokol_fetch_image.buffer_ptr = state.file_buffer;
        sokol_fetch_image.buffer_size = sizeof(state.file_buffer);
    sfetch_send(&sokol_fetch_image);

    // Load font in background
    sfetch_request_t sokol_fetch_font { };
        sokol_fetch_font.path = font_file.c_str();
        sokol_fetch_font.callback = font_normal_loaded;
        sokol_fetch_font.buffer_ptr = state.font_normal_data;
        sokol_fetch_font.buffer_size = sizeof(state.font_normal_data);
    sfetch_send(&sokol_fetch_font);
}

//################################################################################
//##    Create 3D extrusion
//################################################################################
void calculateMesh(bool reset_position) {
    //##    Level of Detail:
    //##        0.075 = Detailed
    //##        0.250 = Nice
    //##        1.000 = Low poly
    //##       10.000 = Really low poly
    float quality_check = level_of_detail;
    switch (mesh_quality) {
        case 0: level_of_detail = 19.200f;  break;
        case 1: level_of_detail =  9.600f;  break;
        case 2: level_of_detail =  4.800f;  break;
        case 3: level_of_detail =  2.400f;  break;
        case 4: level_of_detail =  1.200f;  break;
        case 5: level_of_detail =  0.600f;  break;
        case 6: level_of_detail =  0.300f;  break;
        case 7: level_of_detail =  0.150f;  break;
        case 8: level_of_detail =  0.075f;  break;
    }

    // Recalculate image polygons if necessary
    if (level_of_detail != quality_check) {
        image.outlinePoints(level_of_detail);
    }

    // Get max image dimension
    image_size = Dr::Max(image.getBitmap().width, image.getBitmap().height);

    // Form new meshes
    meshes.clear();
    unsigned int total_vertices = 0;
    unsigned int total_indices =  0;
    for (int object = 0; object < image.m_poly_list.size(); object++) {
        DrMesh mesh {};    
        mesh.extrudeObjectFromPolygon(&image, object, mesh_quality, (static_cast<float>(image_size) * depth_multiplier));
        //mesh->initializeTextureQuad(image_size);
        //mesh->initializeTextureCube(image_size);   
        total_vertices += mesh.vertices.size();
        total_indices  += mesh.indices.size();
        meshes.push_back(mesh);
    }
    triangles = total_vertices / 3;
             
    // ***** Copy vertex data and set into state buffer
    if (meshes.size() > 0) {
        std::vector<Vertex>     vertices(total_vertices);
        std::vector<uint16_t>   indices(total_indices);
                
        // ***** Vertex Buffer
        unsigned long vertex_count = 0;
        for (int m = 0; m < meshes.size(); m++) {
            for (unsigned long i = 0; i < meshes[m].vertices.size(); i++) {
                vertices[vertex_count] = meshes[m].vertices[i];
                ++vertex_count;
            }
        }        
        sg_buffer_desc sokol_buffer_vertex { };
            sokol_buffer_vertex.data = sg_range{ &vertices[0], vertices.size() * sizeof(Vertex) };
            sokol_buffer_vertex.label = "extruded-vertices";
        sg_destroy_buffer(state.bind.vertex_buffers[0]);
        state.bind.vertex_buffers[0] = sg_make_buffer(&sokol_buffer_vertex);

        // ***** Index Buffer
        unsigned long index_count = 0;
        unsigned long index_offset = 0;
        for (int m = 0; m < meshes.size(); m++) {
            for (unsigned long i = 0; i < meshes[m].indices.size(); i++) {
                indices[index_count] = index_offset + meshes[m].indices[i];
                ++index_count;
            }
            index_offset += meshes[m].vertices.size();
        }
        sg_buffer_desc sokol_buffer_index { };
            sokol_buffer_index.type = SG_BUFFERTYPE_INDEXBUFFER;
            sokol_buffer_index.data = sg_range{ &indices[0], indices.size() * sizeof(uint16_t) };
            sokol_buffer_index.label = "extruded-indices";
        sg_destroy_buffer(state.bind.index_buffer);
        state.bind.index_buffer = sg_make_buffer(&(sokol_buffer_index));


        // ***** Reset rotation
        if (reset_position) {
            total_rotation.set(0.f, 0.f);
            add_rotation.set(25.f, 25.f);
            model = Dr::IdentityMatrix();
        }
    }
}

//################################################################################
//##    Load Image
//################################################################################
static void load_image(stbi_uc *buffer_ptr, int fetched_size) {
    int png_width, png_height, num_channels;
    const int desired_channels = 4;
    stbi_uc* pixels = stbi_load_from_memory(buffer_ptr, fetched_size, &png_width, &png_height, &num_channels, desired_channels);

    // Stb Load Succeeded
    if (pixels && (png_width <= 2048) && (png_height <= 2048)) {

        // ********** Copy data into our custom bitmap class, create image and trace outline
        DrBitmap bitmap = DrBitmap(pixels, static_cast<int>(png_width * png_height * 4), false, png_width, png_height);

        // ********** Ensure bitmap is power of 2
        int max_side = Dr::Max(png_width, png_height);
        int pow2 = 2;
        int new_size = 2;
        while (new_size < max_side) {
            new_size = std::pow(2.0, pow2);
            pow2++;
        }
        DrBitmap square = DrBitmap(new_size, new_size);
        for (int x = 0; x < png_width; x++) {
            for (int y = 0; y < png_height; y++) {
                square.setPixel(x, y, bitmap.getPixel(x, y));
            }
        }
        //square = Dr::ApplySinglePixelFilter(Image_Filter_Type::Hue, square, Dr::RandomInt(-100, 100));
        image = DrImage("shapes", square, 0.25f);

        // ********** Calculate 3D Mesh
        calculateMesh(true);        

        // ********** Initialze the sokol-gfx texture
        sg_image_desc sokol_image { };
            sokol_image.width =  image.getBitmap().width;
            sokol_image.height = image.getBitmap().height;
            sokol_image.pixel_format = SG_PIXELFORMAT_RGBA8;
            sokol_image.min_filter = SG_FILTER_LINEAR;
            sokol_image.mag_filter = SG_FILTER_LINEAR;
            sokol_image.data.subimage[0][0].ptr =  &(image.getBitmap().data[0]);
            sokol_image.data.subimage[0][0].size = (size_t)image.getBitmap().size();
    
        // If we already have an image in the state buffer, uninit before initializing new image
        if (initialized_image == true) { sg_uninit_image(state.bind.fs_images[SLOT_tex]); }

        // Initialize new image into state buffer
        sg_init_image(state.bind.fs_images[SLOT_tex], &sokol_image);
        initialized_image = true;
        stbi_image_free(pixels);
    
        load_status = "";
    } else if (pixels) {
        load_status = "Image size too big! Maximum width and height of 2048 pixels!";    
    } else {
        load_status = "Error loading image!";
    }
}

//################################################################################
//##    Callback: File Loading
//################################################################################
/* The fetch-callback is called by sokol_fetch.h when the data is loaded,
   or when an error has occurred. */
static void image_loaded(const sfetch_response_t* response) {
    if (response->fetched) {
        // File data has been fetched
        //  Since we provided a big-enough buffer we can be sure that all data has been loaded here
        load_image((stbi_uc *)response->buffer_ptr, (int)response->fetched_size);
    }
    else if (response->finished) {
        // If loading the file failed, set clear color to signal reason
        if (response->failed) {
            switch (response->error_code) {
                case SFETCH_ERROR_NO_ERROR:             state.pass_action.colors[0].value = { 1.0f, 0.0f, 0.0f, 1.0f }; break;
                case SFETCH_ERROR_FILE_NOT_FOUND:       state.pass_action.colors[0].value = { 0.0f, 1.0f, 0.0f, 1.0f }; break;
                case SFETCH_ERROR_NO_BUFFER:            state.pass_action.colors[0].value = { 0.0f, 0.0f, 1.0f, 1.0f }; break;
                case SFETCH_ERROR_BUFFER_TOO_SMALL:     state.pass_action.colors[0].value = { 1.0f, 1.0f, 0.0f, 1.0f }; break;
                case SFETCH_ERROR_UNEXPECTED_EOF:       state.pass_action.colors[0].value = { 0.0f, 1.0f, 1.0f, 1.0f }; break;
                case SFETCH_ERROR_CANCELLED:            state.pass_action.colors[0].value = { 1.0f, 0.0f, 1.0f, 1.0f }; break;
                case SFETCH_ERROR_INVALID_HTTP_STATUS:  state.pass_action.colors[0].value = { 0.3f, 0.3f, 0.3f, 1.0f }; break;
                default:                                state.pass_action.colors[0].value = { 0.6f, 0.6f, 0.6f, 1.0f }; 
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
static void input(const sapp_event* event) {
    state.items[event->type].event = *event;

    if ((event->type == SAPP_EVENTTYPE_KEY_DOWN) && !event->key_repeat) {
        switch (event->key_code) {
            case SAPP_KEYCODE_1: 
            case SAPP_KEYCODE_2:
            case SAPP_KEYCODE_3:
            case SAPP_KEYCODE_4:
            case SAPP_KEYCODE_5:
            case SAPP_KEYCODE_6:
            case SAPP_KEYCODE_7:
            case SAPP_KEYCODE_8:
            case SAPP_KEYCODE_9:
                mesh_quality = event->key_code - SAPP_KEYCODE_1;
                recalculate = true;
                break;
            case SAPP_KEYCODE_R:
                total_rotation.set(0.f, 0.f);
                add_rotation.set(25.f, 25.f);
                model = Dr::IdentityMatrix();
                break;
            case SAPP_KEYCODE_W:
                wireframe = !wireframe;
                break;
            case SAPP_KEYCODE_MINUS:
                depth_multiplier -= 0.1f;
                recalculate = true;
                break;
            case SAPP_KEYCODE_EQUAL:
                depth_multiplier += 0.1f;
                recalculate = true;
            default: ;
        }
                
    } else if (event->type == SAPP_EVENTTYPE_MOUSE_SCROLL) {
        zoom -= (event->scroll_y * 0.1f);
        zoom = Dr::Clamp(zoom, 0.5f, 5.0f);

    } else if (event->type == SAPP_EVENTTYPE_MOUSE_DOWN) {
        if (event->mouse_button == SAPP_MOUSEBUTTON_LEFT) {
            mouse_down.set(event->mouse_y, event->mouse_x);
            is_mouse_down = true;
        }
    } else if (event->type == SAPP_EVENTTYPE_TOUCHES_BEGAN) {
        if (event->num_touches >= 0) {
            mouse_down.set(event->touches[0].pos_y, event->touches[0].pos_x);
            is_mouse_down = true;
        }

    } else if (event->type == SAPP_EVENTTYPE_MOUSE_UP) {
        if (event->mouse_button == SAPP_MOUSEBUTTON_LEFT) is_mouse_down = false;
    } else if (event->type == SAPP_EVENTTYPE_TOUCHES_ENDED) {
        if (event->num_touches == 0) is_mouse_down = false;
            
    } else if (event->type == SAPP_EVENTTYPE_MOUSE_MOVE || event->type == SAPP_EVENTTYPE_TOUCHES_MOVED) {
        if (is_mouse_down) {
            float x_movement = 0.f;
            float y_movement = 0.f;

            if (event->type == SAPP_EVENTTYPE_MOUSE_MOVE) {
                x_movement = event->mouse_y;
                y_movement = event->mouse_x;
            } else if (event->type == SAPP_EVENTTYPE_TOUCHES_MOVED) {
                x_movement = event->touches[0].pos_y;
                y_movement = event->touches[0].pos_x;
            }

            if (mouse_down.x < x_movement) {
                add_rotation.x = rotate_speed * (x_movement - mouse_down.x);
            } else if (mouse_down.x > x_movement) {
                add_rotation.x = 360 - (rotate_speed * (mouse_down.x - x_movement));
            }
            
            if (mouse_down.y > y_movement) {
                add_rotation.y = 360 - (rotate_speed * (mouse_down.y - y_movement));
            } else if (mouse_down.y < y_movement) {
                add_rotation.y = rotate_speed * (y_movement - mouse_down.y);
            }

            mouse_down.x = x_movement;
            mouse_down.y = y_movement;
            add_rotation.x = Dr::EqualizeAngle0to360(add_rotation.x);
            add_rotation.y = Dr::EqualizeAngle0to360(add_rotation.y);
        }

    } else if (event->type == SAPP_EVENTTYPE_FILES_DROPPED) {
        #if defined(__EMSCRIPTEN__)
            // on emscripten need to use the sokol-app helper function to load the file data
            sapp_html5_fetch_request sokol_fetch_request { };
                sokol_fetch_request.dropped_file_index = 0;
                sokol_fetch_request.callback = emsc_load_callback;
                sokol_fetch_request.buffer_ptr = state.file_buffer;
                sokol_fetch_request.buffer_size = sizeof(state.file_buffer);
            sapp_html5_fetch_dropped_file(&sokol_fetch_request);
        #else
            // native platform: use sokol-fetch to load file content
            sfetch_request_t sokol_fetch_request { };
                sokol_fetch_request.path = sapp_get_dropped_file_path(0);
                sokol_fetch_request.callback = native_load_callback;
                sokol_fetch_request.buffer_ptr = state.file_buffer;
                sokol_fetch_request.buffer_size = sizeof(state.file_buffer);
            sfetch_send(&sokol_fetch_request);
        #endif
    }
}

//################################################################################
//##    Render
//################################################################################
// Was playing with getting canvas size from html file to resize framebuffer when
// canvas is resized. Went around adding to sokol_app by invoking 'resize' event
// directly from javascript in the webpage on canvas resize.
#if defined(__EMSCRIPTEN__)
    EM_JS(int, get_canvas_width,  (), { return canvas.width; });
    EM_JS(int, get_canvas_height, (), { return canvas.height; });
#endif

/* The frame-function is fairly boring, note that no special handling is
   needed for the case where the texture isn't loaded yet.
   Also note the sfetch_dowork() function, this is usually called once a
   frame to pump the sokol-fetch message queues.
*/
static void frame(void) {
    // ***** Pump the sokol-fetch message queues, and invoke response callbacks
    sfetch_dowork();

    // ***** Compute model-view-projection matrix for vertex shader
    hmm_mat4 proj = HMM_Perspective(52.5f, (float)sapp_width()/(float)sapp_height(), 5.f, 20000.0f);
    hmm_mat4 view = HMM_LookAt(HMM_Vec3(0.0f, 1.5f, static_cast<float>(image_size) * zoom), HMM_Vec3(0.0f, 0.0f, 0.0f), HMM_Vec3(0.0f, 1.0f, 0.0f));
    hmm_mat4 view_proj = HMM_MultiplyMat4(proj, view);

    hmm_mat4 rxm = HMM_Rotate(add_rotation.x, HMM_Vec3(1.0f, 0.0f, 0.0f));
    hmm_mat4 rym = HMM_Rotate(add_rotation.y, HMM_Vec3(0.0f, 1.0f, 0.0f));
    hmm_mat4 rotate = HMM_MultiplyMat4(rxm, rym); 
             model =  HMM_MultiplyMat4(rotate, model);
    total_rotation.x = Dr::EqualizeAngle0to360(total_rotation.x + add_rotation.x);
    total_rotation.y = Dr::EqualizeAngle0to360(total_rotation.y + add_rotation.y);
    add_rotation.set(0.f, 0.f);


    // Uniforms for vertex shader
    vs_params_t vs_params;
    vs_params.m =   model;
    vs_params.mvp = HMM_MultiplyMat4(view_proj, model);
    
    // Uniforms for fragment shader
    fs_params_t fs_params;
    fs_params.u_wireframe = (wireframe) ? 1.0f : 0.0f;


    // Check if user requested new model quality, if so recalculate
    if (recalculate) {
        calculateMesh(false);
        recalculate = false;
    }


    // ***** Render pass
    sg_begin_default_pass(&state.pass_action, sapp_width(), sapp_height());
    sg_apply_pipeline(state.pip);
    sg_apply_bindings(&state.bind);
    sg_apply_uniforms(SG_SHADERSTAGE_VS, SLOT_vs_params, SG_RANGE(vs_params));
    sg_apply_uniforms(SG_SHADERSTAGE_FS, SLOT_fs_params, SG_RANGE(fs_params));
    
    // Draw Triangles
    unsigned long index_count = 0;
    for (auto mesh : meshes) {
        index_count += mesh.indices.size();
    }
    sg_draw(0, index_count, 1);

    // ***** Text
    fonsClearState(state.fons);    
    sgl_defaults();
    sgl_matrix_mode_projection();
    sgl_ortho(0.0f, sapp_widthf(), sapp_heightf(), 0.0f, -1.0f, +1.0f);

    const float dpis = state.dpi_scale;
    FONScontext* fs = state.fons;

    if (state.font_normal != FONS_INVALID) {
        fonsSetAlign(fs, FONS_ALIGN_LEFT | FONS_ALIGN_BASELINE);
        fonsSetFont(fs, state.font_normal);
        fonsSetSize(fs, 18.0f * dpis);
        fonsSetColor(fs, sfons_rgba(255, 255, 255, 255));
        fonsSetBlur(fs, 0);
        fonsSetSpacing(fs, 0.0f); 
        fonsDrawText(fs, 10 * dpis,  20 * dpis, ("FPS: " +  std::to_string(fps)).c_str(), NULL);
        fonsDrawText(fs, 10 * dpis,  40 * dpis, ("Quality: " + std::to_string(mesh_quality+1)).c_str(), NULL);
        fonsDrawText(fs, 10 * dpis,  60 * dpis, ("Triangles: " + std::to_string(triangles)).c_str(), NULL);
        fonsDrawText(fs, 10 * dpis,  80 * dpis, ("Depth: " + std::to_string((int)(image_size * depth_multiplier))).c_str(), NULL);
        //fonsDrawText(fs, 10 * dpis, 100 * dpis, ("ZOOM: " + std::to_string(zoom)).c_str(), NULL);

        if (load_status != "") {
            fonsSetAlign(fs, FONS_ALIGN_CENTER | FONS_ALIGN_MIDDLE);
            fonsSetSize(fs, 22.0f * dpis);
            fonsDrawText(fs, 0, (sapp_heightf() / 2.f) * dpis, load_status.c_str(), NULL);
        }
    }
    sfons_flush(fs);            // Flush fontstash's font atlas to sokol-gfx texture
    sgl_draw();


    // ***** End Rendering
    sg_end_pass();
    sg_commit();


    // ***** Update frames per second
    ticks++;
    uint64_t elapsed = stm_since(time_start);
    double seconds = stm_sec(elapsed);
    if (seconds >= 1.0) {
        fps = ticks;
        time_start = stm_now();
        ticks = 0;
    }
}


//################################################################################
//##    Clean Up
//################################################################################
void cleanup(void) {
    sfetch_shutdown();
    sfons_destroy(state.fons);
    sgl_shutdown();
    sg_shutdown();
}

//################################################################################
//##    App Entry
//################################################################################
sapp_desc sokol_main(int argc, char* argv[]) {
    (void)argc; 
    (void)argv;

    sokol_app.window_title = "3D Extrusion";
    sokol_app.init_cb = init;
    sokol_app.frame_cb = frame;
    sokol_app.event_cb = input;
    sokol_app.cleanup_cb = cleanup;
    sokol_app.width = 800;
    sokol_app.height = 600;
    sokol_app.enable_clipboard = true;
    sokol_app.enable_dragndrop = true;
    sokol_app.max_dropped_files = 1;
    return sokol_app;
}






