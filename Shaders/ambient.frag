#version 460 core

layout (location = 0) out vec4 FragColour;
in vec2 texCoords;

uniform sampler2D screenDepth;
uniform sampler2D screenAlbedo;
uniform sampler2D screenNormal;
uniform sampler2D screenEmission;
uniform sampler2D screenSSAO;

uniform samplerCube skybox;
uniform samplerCube irradianceMap;

uniform mat4 invP;
uniform mat4 invV;

uniform vec3 lightDirection;
uniform vec3 lightColour;
uniform vec3 camPos;
uniform vec3 cameraDelta;


vec3 trueAlbedo;
float roughness;
float metallic;
float ao;
vec3 screenPos;
vec3 fragPos;

vec3 viewDirection;
vec3 F0;

float DistributionGGX(vec3 N, vec3 H);
float GeometrySchlickGGX(float NdotV);
float GeometrySmith(vec3 N, vec3 V, vec3 L);
vec3 fresnelSchlick(float cosTheta, vec3 F0);
vec3 Radiance(
    vec3 normal
);
vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness);
#define PI 3.1415926535
const float MAX_REFLECTION_LOD = 4.0;

void main()
{
	float depthValue = texture(screenDepth, texCoords).r;
    vec4 NDC = vec4(texCoords * 2.0 - 1.0, depthValue * 2.0 - 1.0, 1.0);
    vec4 clipPos = invP * NDC;
    screenPos = clipPos.xyz / clipPos.w;
    fragPos = (invV * vec4(screenPos, 1.0)).xyz;

    viewDirection = normalize(camPos - fragPos);

    vec4 emission = texture(screenEmission, texCoords);
    vec4 albedo = texture(screenAlbedo, texCoords);
    vec4 normal = texture(screenNormal, texCoords);
    vec3 trueNormal = normal.rgb;
    trueAlbedo = albedo.rgb;
    roughness = normal.a;
    metallic = albedo.a;
    ao = emission.a;
    if(ao == 0) ao = 1.0;

    vec3 Lo = vec3(0.0);
	F0 = vec3(0.04); 
    F0 = mix(F0, trueAlbedo, metallic);

    Lo = max(Radiance(trueNormal), 0);

    vec3 result = Lo;
    float SSAOvalue = texture(screenSSAO, texCoords).r;
    result *= SSAOvalue;
    result += emission.rgb;
    FragColour = vec4(result, 1.0);
}


vec3 Radiance(
    vec3 normal
)
{
    // calculate per-light radiance
    vec3 H = normalize(viewDirection - lightDirection);
    float attenuation = 1.0;
    vec3 radiance  = lightColour * attenuation;        
        
    // cook-torrance brdf
    float NDF = DistributionGGX(normal, H);        
    float G   = GeometrySmith(normal, viewDirection, -lightDirection);      
    vec3 F    = fresnelSchlick(max(dot(H, viewDirection), 0.0), F0);       
    
    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metallic;
   
    vec3 numerator    = NDF * G * F;
    float denominator = 4.0 * max(dot(normal, viewDirection), 0.0) * max(dot(normal, -lightDirection), 0.0) + 0.0001;
    vec3 specular     = numerator / denominator;  
            
    // add to outgoing radiance Lo
    float NdotL = max(dot(normal, -lightDirection), 0.0);                
    return (kD * trueAlbedo / PI + specular) * radiance * NdotL; 
}

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

float DistributionGGX(vec3 N, vec3 H)
{
    float a      = roughness*roughness;
    float a2     = a*a;
    float NdotH  = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;
	
    float num   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
	
    return num / denom;
}

float GeometrySchlickGGX(float NdotV)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float num   = NdotV;
    float denom = NdotV * (1.0 - k) + k;
	
    return num / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2  = GeometrySchlickGGX(NdotV);
    float ggx1  = GeometrySchlickGGX(NdotL);
	
    return ggx1 * ggx2;
}
