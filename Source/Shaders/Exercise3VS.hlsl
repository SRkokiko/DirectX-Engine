cbuffer Transforms : register(b0)
{
    float4x4 mvp; // mvp = model * view * projection
};

float4 main(float3 pos : MY_POSITION) : SV_POSITION
{
    return mul(float4(pos, 1.0), mvp); // Pre-multiply vector by MVP matrix
}
