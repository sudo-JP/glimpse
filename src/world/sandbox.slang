struct MVP {
    float4x4 model;
    float4x4 view;
    float4x4 proj;
};

ConstantBuffer<MVP> mvp;

struct VSInput {
    float3 inPosition;
    float2 inTexCoord;
};

struct VSOutput {
    float4 pos: SV_Position;
    float2 fragTexCoord;
};

Sampler2D texture;

[shader("vertex")]
VSOutput vertMain(VSInput input) {
    VSOutput output;
    output.pos = mul(mvp.proj, mul(mvp.view, mul(mvp.model, float4(input.inPosition, 1.0))));
    output.fragTexCoord = input.inTexCoord;
    return output;
}


[shader("fragment")]
float4 fragMain(VSOutput vertIn) : SV_Target {
    //return float4(1.0, 1.0, 1.0, 1.0);
    return texture.Sample(vertIn.fragTexCoord);
}
