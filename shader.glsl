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
in vec2 texcoord0;
out vec2 uv;

void main() {
    gl_Position = mvp * pos;
    uv = texcoord0;
}
@end


//########## Fragment Shader ##########
@fs fs
uniform sampler2D tex;

in vec2 uv;
out vec4 frag_color;

void main() {
    frag_color = texture(tex, uv);
}
@end


//########## Shader Name ##########
@program extrude3D vs fs


