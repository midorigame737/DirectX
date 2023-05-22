
float4 BasicPS(float4 pos : SV_POSITION) : SV_TARGET
{
	
	return float4((float2(0, 1)+pos.xy)*0.005f , 1, 1.);

}