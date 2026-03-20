#version 450

layout(location = 0) in vec2  in_position;
layout(location = 1) in vec4  in_color;
layout(location = 2) in vec2  in_texcoord;

layout(location = 0) out vec4 frag_color;
layout(location = 1) out vec2 frag_texcoord;

layout(set = 1, binding = 0) uniform VertexUniforms {
    vec2 screen_size;
};

void main() {
    gl_Position = vec4(
        (in_position.x / screen_size.x) * 2.0 - 1.0,
        1.0 - (in_position.y / screen_size.y) * 2.0,
        0.0, 1.0);
    frag_color    = in_color;
    frag_texcoord = in_texcoord;
}
