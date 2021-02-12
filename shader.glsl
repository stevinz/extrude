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

    frag_color = texture(tex, uv) + norm + bary;
}
@end


//########## Shader Name ##########
@program extrude3D vs fs


