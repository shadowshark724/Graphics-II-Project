texture2D base : register(t0);
texture2D normbase : register(t1);
SamplerState samples : register(s0);

// Per-pixel color data passed through the pixel shader.
struct PixelShaderInput
{
	float4 pos : SV_POSITION;
	float3 WorldPos : W_POSITION;
	float3 uv : UV;
	float3 normal : NORMAL;
	float3  lightVal : COLOR0;
};

float4 DirectionL(PixelShaderInput input)
{
	//float hold = input.lightVal.x;
	float4 pos;
	float4x4 lightz = { cos(input.lightVal.x), -sin(input.lightVal.x),0.0f, 0.0f,
		sin(input.lightVal.x), cos(input.lightVal.x), 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f

	};
	pos.x = (-10.0f - input.WorldPos.x);
	pos.y = (10.0f - input.WorldPos.y);
	pos.z = (0.0f - input.WorldPos.z);
	pos.w = 1.0f;
	pos = mul(pos,lightz);
	float3 LightDirection = normalize(pos);
	float LightRatio = clamp(dot(LightDirection, normalize(input.normal)), 0, 1);
	float3 Color = { .20f, 0.2f, 1.0f };
	float4 ReturnThis = float4(Color*LightRatio, 1.0f);
	return ReturnThis;
}

float4 PointL(PixelShaderInput input)
{
	float3x3 lightx = { 1.0f, 0.0f,0.0f,
						0.0f, cos(input.lightVal.x), -sin(input.lightVal.x),
						0.0f, sin(input.lightVal.x), cos(input.lightVal.x),
	};

	float3x3 lighty = { cos(-input.lightVal.y),0.0f  ,sin(-input.lightVal.y),
						0.0f,                  1.0f,  0.0f,
						-sin(-input.lightVal.y), 0.0f, cos(-input.lightVal.y),
	};
	float3x3 lightz = { cos(input.lightVal.z), -sin(input.lightVal.z),0.0f, 
		sin(input.lightVal.z), cos(input.lightVal.z), 0.0f,
		0.0f, 0.0f, 1.0f,
	};
	float3 lightpos;
	float radius = 15.50f;
	float3 wrldpos = input.WorldPos.xyz;
	wrldpos = mul(wrldpos, lighty);
	lightpos.x = (-5.0f -wrldpos.x);
	lightpos.y = (-1.5f - wrldpos.y);
	lightpos.z = (0.0f - wrldpos.z);
	//lightpos.w = 1.0f;
	//float3x3 mathold = mul(lightx, lightz);
	lightpos = mul(lightpos, lighty);
	float3 direction = normalize(lightpos);
	float ratio = clamp(dot(-direction, normalize(input.normal)), 0.0f, 1.0f);
	float att = 1.0f - saturate(length(lightpos.xyz) / radius);
	float3 clr = { 1.0f,1.0f,1.0f };
	float4 ReturnThis = float4(clr*ratio, 1.0f)*att;
	return ReturnThis;
}

float4 SpotL(PixelShaderInput input)
{
	float3x3 lightx = { 1.0f, 0.0f,0.0f,
		0.0f, cos(input.lightVal.x), -sin(input.lightVal.x),
		0.0f, sin(input.lightVal.x), cos(input.lightVal.x),
	};
	float3x3 lighty = { cos(-input.lightVal.y),0.0f  ,sin(-input.lightVal.y),
		0.0f,                  1.0f,  0.0f,
		-sin(-input.lightVal.y), 0.0f, cos(-input.lightVal.y),
	};
	float3x3 lightz = { cos(input.lightVal.z), -sin(input.lightVal.z),0.0f,
		sin(input.lightVal.z), cos(input.lightVal.z), 0.0f,
		0.0f, 0.0f, 1.0f,
	};

	//float hold = input.lightVal._11;
	float3 pos;
	float3 wrldpos = input.WorldPos.xyz;
	wrldpos = mul(wrldpos, lighty); 
	pos.x = 10.0f - wrldpos.x;
	pos.y = 10.5f - wrldpos.y;
	pos.z = -3.0f - wrldpos.z;
	pos = mul(pos, lighty);
	float3 Cone = { 0.0f,5.0f, -1.0f };
	Cone = mul(Cone, lightz);
	float3 Direction = normalize(pos);
	float ratio = clamp(dot(Direction, normalize(Cone)), 0.0f, 1.0f);
	float spotfactor = (ratio > 0.9f) ? 1 : 0;
	float lightratio = clamp(dot(Direction, normalize(input.normal)), 0.0f, 1.0f);

	float3 clr = { 1.0f,1.0f,1.0f };
	float4 ReturnThis = float4(clr, 1.0f)*lightratio*spotfactor;
	return ReturnThis;
}

// A pass-through function for the (interpolated) color data.
float4 main(PixelShaderInput input) : SV_TARGET
{
	float4 baseclr = base.Sample(samples, input.uv);
	float4 baseNorm = normbase.Sample(samples, input.uv);

	float4 white = float4(1, 1, 1, 1);

	float4 DirectionalLight = DirectionL(input);
	float4 PointLight = PointL(input);
	float4 SpotLight = SpotL(input);

	float4 AllLight = PointLight;// DirectionalLight + PointLight + SpotLight;
	//return AllLight;
	//return PointLight * baseclr;          
	//return SpotLight * baseclr;
	//return DirectionalLight * baseclr;
	return white * AllLight;
//	return float4(input.uv, 1.0f);
}
