Texture2D<float4> inputTexture : register(t0, space2);
SamplerState samplerState : register(s0, space2);

struct VSOutput {
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
};

float4 main(VSOutput input) : SV_TARGET {
    return inputTexture.Sample(samplerState, input.uv);
}