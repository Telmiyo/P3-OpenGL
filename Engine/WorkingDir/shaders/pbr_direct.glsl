#if defined(VERTEX)

struct Light
{
    unsigned int type;
    vec3 color;
    vec3 direction;
    vec3 position;
    float intensity;
};

layout(location=0) in vec3 aPosition;
layout(location=1) in vec3 aNormal;
layout(location=2) in vec2 aTexCoord;

layout(binding = 0, std140) uniform GlobalParams
{
    unsigned int uRenderMode;
    vec3         uCameraPosition;
    unsigned int uLightCount;
    Light        uLight[16];
};

layout(binding = 1, std140) uniform LocalParams
{
    mat4 uWorldMatrix;
    mat4 uWorldViewProjectionMatrix;
};

out vec2 vTexCoord;
out vec3 vPosition; //In worldspace
out vec3 vNormal;   //In worldspace
out vec3 vViewDir;  //In worldspace

void main()
{
    vTexCoord = aTexCoord;
    
    mat4 model = mat4(1.0f);

    vNormal = mat3(transpose(inverse(model))) * aNormal;

    vPosition = vec3(model * vec4(aPosition, 1.0f));

    vViewDir  = uCameraPosition - vPosition;

    gl_Position = uWorldViewProjectionMatrix * vec4(aPosition, 1.0f);
}   

#elif defined(FRAGMENT) 

// Fragment Shader

// Output fragment color
out vec4 FragColor;

// Vertex shader outputs
in vec2 vTexCoord;
in vec3 vPosition; //In worldspace
in vec3 vNormal;   //In worldspace
in vec3 vViewDir;  //In worldspace

// Material properties
uniform sampler2D albedoMap;
uniform sampler2D normalMap;
uniform sampler2D metallicMap;
uniform sampler2D roughnessMap;
uniform sampler2D aoMap;
uniform samplerCube skybox;

struct Light
{
    unsigned int type; //0: Directional, 1 Point
    vec3 color;
    vec3 direction;
    vec3 position;
    float intensity;
};

layout(binding = 0, std140) uniform GlobalParams
{
    unsigned int uRenderMode;
    vec3         uCameraPosition;
    unsigned int uLightCount;
    Light        uLight[16];
};

layout(binding = 1, std140) uniform LocalParams
{
    mat4 uWorldMatrix;
    mat4 uWorldViewProjectionMatrix;
};

const float PI = 3.14159265359;

// Function to extract normal from a normal map
vec3 getNormalFromMap()
{
    // Obtain normal from normal map
    vec3 tangentNormal = texture(normalMap, vTexCoord).xyz * 2.0 - 1.0;

    // Calculate derivative of coordinates
    vec3 Q1  = dFdx(vPosition);
    vec3 Q2  = dFdy(vPosition);
    vec2 st1 = dFdx(vTexCoord);
    vec2 st2 = dFdy(vTexCoord);

    // Calculate TBN matrix
    vec3 N   = normalize(vNormal);
    vec3 T  = normalize(Q1*st2.t - Q2*st1.t);
    vec3 B  = -normalize(cross(N, T));
    mat3 TBN = mat3(T, B, N);

    return normalize(TBN * tangentNormal);
}

// GGX distribution function
float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness*roughness;
    float a2 = a*a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;

    float nom   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / denom;
}

// Schlick approximation for GGX
float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}

// Smith's GGX geometry function
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

// Fresnel equations
vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

vec3 getEnvironmentReflection(vec3 N, vec3 V, float roughness, float metallic, vec3 F0)
{
    vec3 R = reflect(-V, N);
    vec3 envColor = textureLod(skybox, R, roughness * 8.0).rgb;
    
    vec3 kS = fresnelSchlick(max(dot(N, V), 0.0), F0);
    vec3 kD = vec3(1.0) - kS; 
    kD *= 1.0 - metallic;    
    vec3 specular = envColor * (kS + kD * envColor * (1.0 - kS));
    
    return specular;
}

// Main PBR lighting calculation
void main()
{       
    vec3 albedo     = pow(texture(albedoMap, vTexCoord).rgb, vec3(2.2));
    float metallic  = texture(metallicMap, vTexCoord).r;
    float roughness = texture(roughnessMap, vTexCoord).r;
    float ao        = texture(aoMap, vTexCoord).r;

    vec3 N = getNormalFromMap();
    vec3 V = normalize(uCameraPosition - vPosition);

    vec3 F0 = vec3(0.04); 
    F0 = mix(F0, albedo, metallic);

    vec3 Lo = vec3(0.0);
    for(int i = 0; i < uLightCount; ++i) 
    {
        vec3 L;
        float attenuation;
        if(uLight[i].type == 0) // Directional light
        {
            L = normalize(-uLight[i].direction);
            attenuation = uLight[i].intensity; // factor in light intensity
        }
        else if(uLight[i].type == 1) // Point light
        {
            vec3 lightDir = uLight[i].position - vPosition;
            L = normalize(lightDir);
            float distance = length(lightDir);
            attenuation = uLight[i].intensity / (distance * distance); // factor in light intensity
        }

        vec3 radiance = uLight[i].color * attenuation;
        // ... rest of your light loop

        // Calculate the halfway vector
        vec3 H = normalize(V + L);

        float NDF = DistributionGGX(N, H, roughness);   
        float G   = GeometrySmith(N, V, L, roughness);      
        vec3 F    = fresnelSchlick(max(dot(H, V), 0.0), F0);
           
        vec3 numerator    = NDF * G * F; 
        float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001; // Prevent divide by zero
        vec3 specular = numerator / denominator;
        
        vec3 kS = F; // Specular reflection coefficient
        vec3 kD = vec3(1.0) - kS; // Diffuse reflection coefficient
        kD *= 1.0 - metallic;	 // Adjust for metals 

        float NdotL = max(dot(N, L), 0.0);        

        Lo += (kD * albedo / PI + specular) * radiance * NdotL;
    }   

    vec3 environmentReflection = getEnvironmentReflection(N, V, roughness, metallic, F0);
    // Lo += environmentReflection;

    vec3 ambient = vec3(0.03) * albedo * ao;
    
    vec3 color = ambient + Lo;

    color = color / (color + vec3(1.0)); // HDR tonemapping
    color = pow(color, vec3(1.0/2.2)); // Gamma correction

    FragColor = vec4(color, 1.0);
}

#endif
