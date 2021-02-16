//################################################################################
//##    Shader code
//################################################################################
@ctype mat4 hmm_mat4


//########## Vertex Shader ##########
@vs vs
uniform vs_params {
    mat4 mvp;
    mat4 m;
};

in vec4 pos;
in vec3 norm;
in vec2 texcoord0;
in vec3 bary;

out vec2 uv;
out vec3 vert;
out vec3 vert_normal;
out vec3 vert_bary;

void main() {
    gl_Position = mvp * pos;
    uv = texcoord0;
    vert =          (m * vec4(pos.xyz, 1.0)).xyz;
    vert_normal =   (m * vec4(norm, 0.0)).xyz;
    vert_bary = bary;
}
@end


//########## Fragment Shader ##########
@fs fs
uniform sampler2D tex;

uniform fs_params {
    float u_wireframe;
};

in vec2 uv;
in vec3 vert;
in vec3 vert_normal;
in vec3 vert_bary;

out vec4 frag_color;

void main() {
    vec4 norm = vec4(0.0, vert_normal * 0.0);
    vec4 bary = vec4(0.0, vert_bary * 0.0);
       

    // ***** Color from texture
    vec4  color_in  = texture(tex, uv);
    vec3  rgb_in    = color_in.xyz;
    float alpha_in  = 1.0;//color_in.a;
    vec3  rgb_out   = rgb_in;
    float alpha_out = 1.0;


    // ***** Wireframe
    if (u_wireframe == 1.0) {
        float width = 1.0;

        vec3  d = fwidth(vert_bary);
        vec3  a3 = smoothstep(vec3(0.0), d * width, vert_bary);
        float wire = min(min(a3.x, a3.y), a3.z);
        rgb_out = rgb_in * (1.0 - wire);

        // If not on edge, draw texture faded
        if (rgb_out.x < 0.02 && rgb_out.y < 0.02 && rgb_out.z < 0.02) {
            // Texture is slightly there
            rgb_out = rgb_in * 0.8;
            alpha_out = alpha_in * 0.8;
        }
    }


    // ***** Shade Away
    // Calculate angle between camera vector and vertex normal for triangle shading
    float shade_away = 1.0;
    if (shade_away == 1.0) {
        vec3 eye = vec3(0.0, 1.5, 500.0);
        float dp = dot(normalize(vert_normal), normalize(vert - eye)) + 0.15;
              dp = clamp(dp, 0.0, 1.0);
        rgb_out = mix(vec3(0.0), rgb_out, dp);
    }
    

    // ***** Set Final Color
    frag_color = vec4(rgb_out, alpha_out);
}
@end


//########## Shader Name ##########
@program extrude3D vs fs


