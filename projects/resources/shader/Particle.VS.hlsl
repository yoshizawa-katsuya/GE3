#include "Particle.hlsli"

struct ParticleForGPU
{
    float32_t4x4 WVP;
    float32_t4x4 World;
    float32_t4 color;
};

StructuredBuffer<ParticleForGPU> gParticle : register(t0);



struct VertexShaderInput {
	float32_t4 position : POSITION0;
    float32_t2 texcord : TEXCOORD0;
    float32_t3 normal : NORMAL0;
};

VertexShaderOutput main(VertexShaderInput input, uint32_t instanceId : SV_InstanceID) {
	VertexShaderOutput output;
    output.position = mul(input.position, gParticle[instanceId].WVP);
    output.texcord = input.texcord;
    //output.normal = normalize(mul(input.normal, (float32_t3x3) gParticle[instanceId].World));
    output.color = gParticle[instanceId].color;
	return output;
}