// A constant buffer that stores the three basic column-major matrices for composing geometry.
cbuffer ModelViewProjectionConstantBuffer : register(b0)
{
	matrix model;
	matrix view;
	matrix projection;
};

// Per-vertex data used as input to the vertex shader.
struct VertexShaderInput
{
	float3 pos : POSITION;
	float3 uv : UV;
	float3 norm : NORMAL;
};

// Per-pixel color data passed through the pixel shader.
struct PixelShaderInput
{
	float4 pos : SV_POSITION;
	float3 WorldPos : W_POSITION;
	float3 uv : UV;
	float3 norm : NORAML;
};

// Simple shader to do vertex processing on the GPU.
PixelShaderInput main(VertexShaderInput input)
{
	PixelShaderInput output;
	float4 pos = float4(input.pos, 1.0f);
	matrix view2 = view;
	view2[3][0] = 0;
	view2[3][1] = 0;
	view2[3][2] = 0;

	// Transform the vertex position into projected space.
	//pos = mul(pos, model);
	output.WorldPos = pos.xyz;
	pos = mul(pos, view2);
	pos = mul(pos, projection);
	output.pos = pos;

	// Pass the color through without modification.
	output.uv = input.uv;
	output.norm = input.norm;//mul(input.normal, (float3x3)model);
	
	return output;
}
