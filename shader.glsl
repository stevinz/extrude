//################################################################################
//##    Shader code
//################################################################################
@ctype mat4 hmm_mat4


//########## Vertex Shader ##########
@vs vs
uniform vs_params {
    mat4 mvp;
};

in vec4 pos;
in vec3 norm;
in vec2 texcoord0;
in vec3 bary;

out vec2 uv;
out vec3 vert_normal;
out vec3 vert_bary;

void main() {
    gl_Position = mvp * pos;
    uv = texcoord0;
    vert_normal = norm;
    vert_bary = bary;
}
@end


//########## Fragment Shader ##########
@fs fs
uniform sampler2D tex;

in vec2 uv;
in vec3 vert_normal;
in vec3 vert_bary;

out vec4 frag_color;

void main() {
    vec4 norm = vec4(0.0, vert_normal * 0.0);
    vec4 bary = vec4(0.0, vert_bary * 0.0);
       
    // ***** Color from texture
    frag_color = texture(tex, uv);
    vec3  frag_rgb = frag_color.rgb;
    float frag_a   = frag_color.a;


    // ***** Wireframe
    //if (wireframe) {
        float wireframe_percent = 0.2;
        float width = 1.0;

        vec3  d = fwidth(vert_bary);
        vec3  a3 = smoothstep(vec3(0.0), d * width, vert_bary);
        float wire = min(min(a3.x, a3.y), a3.z);

        frag_color *= (1.0 - wire);

        // If not on edge, draw texture faded, or just maybe just discard
        if (all(lessThan(frag_color, vec4(0.02)))) {
            // Texture is slightly there
            frag_color = vec4(frag_rgb * 0.15, 0.15) * frag_a;
        }
    // }

    
}
@end


//########## Shader Name ##########
@program extrude3D vs fs


