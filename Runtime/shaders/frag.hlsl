Texture2D tex : register(t0, space2);
SamplerState samp : register(s0, space2);

struct PSInput {
    float4 position : SV_Position;
    float4 color    : TEXCOORD0;
    float2 texcoord : TEXCOORD1;
};

float4 main(PSInput input) : SV_Target {
    return input.color * tex.Sample(samp, input.texcoord);
}
