cbuffer VertexUniforms : register(b0, space1) {
    float2 screen_size;
};

struct VSInput {
    float2 position : TEXCOORD0;
    uint   color    : TEXCOORD1;
    float2 texcoord : TEXCOORD2;
};

struct VSOutput {
    float4 position : SV_Position;
    float4 color    : TEXCOORD0;
    float2 texcoord : TEXCOORD1;
};

VSOutput main(VSInput input) {
    VSOutput output;
    output.position = float4(
        (input.position.x / screen_size.x) * 2.0 - 1.0,
        1.0 - (input.position.y / screen_size.y) * 2.0,
        0.0, 1.0);
    output.color = float4(
        (float)((input.color >> 24) & 0xFF) / 255.0,
        (float)((input.color >> 16) & 0xFF) / 255.0,
        (float)((input.color >>  8) & 0xFF) / 255.0,
        (float)( input.color        & 0xFF) / 255.0);
    output.texcoord = input.texcoord;
    return output;
}
