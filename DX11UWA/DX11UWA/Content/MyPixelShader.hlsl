texture2D base : register(t0);
SamplerState samples : register(s0);

// Per-pixel color data passed through the pixel shader.
struct PixelShaderInput
{
	float4 pos : SV_POSITION;
	float3 WorldPos : W_POSITION;
	float3 uv : UV;
	float3 normal : NORMAL;
};

float4 DirectionL(PixelShaderInput input)
{
	float3 LightDirection = { -10.0f, 10.0f, 0.0f };
	float LightRatio = clamp(dot(-normalize(LightDirection), normalize(input.normal)), 0, 1);
	float3 Color = { 1.0f, 0.0f, 0.0f };
	float4 ReturnThis = float4(Color, 1.0f) * LightRatio;
	return ReturnThis;
}

float4 PointL(PixelShaderInput input)
{
	float3 lightpos;
	lightpos.x = 100.0f - input.WorldPos.x;
	lightpos.y = 100.0f - input.WorldPos.y;
	lightpos.z = 0.0f - input.WorldPos.z;

	float3 direction = normalize(lightpos);
	float ratio = clamp(dot(-normalize(direction), normalize(input.normal)), 0.0f, 1.0f);
	float3 clr = { 0.0f,1.0f,0.0f };
	float4 ReturnThis = float4(clr, 1.0f)*ratio;
	return ReturnThis;
}

float4 SpotL(PixelShaderInput input)
{
	float3 pos;
	pos.x = 0.0f - input.WorldPos.x;
	pos.y = 10.5f - input.WorldPos.y;
	pos.z = 0.0f - input.WorldPos.z;

	float3 Cone = { 0.0f,-1.0f, 0.0f };

	float3 Direction = normalize(pos);
	float ratio = clamp(dot(-normalize(Direction), normalize(Cone)), 0.0f, 1.0f);
	float spotfactor = (ratio > 0.9f) ? 1 : 0;
	float lightratio = clamp(dot(normalize(Direction), normalize(input.normal)), 0.0f, 1.0f);

	float3 clr = { 0.0f,0.0f,1.0f };
	float4 ReturnThis = float4(clr, 1.0f)*lightratio*spotfactor;
	return ReturnThis;
}

// A pass-through function for the (interpolated) color data.
float4 main(PixelShaderInput input) : SV_TARGET
{
	float4 DirectionalLight = DirectionL(input);
	float4 PointLight = PointL(input);
	float4 SpotLight = SpotL(input);

	float4 AllLight = DirectionalLight+PointLight+SpotLight;

	//return AllLight;

	return base.Sample(samples, input.uv) * AllLight;
//	return float4(input.uv, 1.0f);
}
