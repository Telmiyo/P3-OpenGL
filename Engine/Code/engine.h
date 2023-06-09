//
// engine.h: This file contains the types and functions relative to the engine.
//

#pragma once

#include "platform.h"
#include "geometry.h"
#include "camera.h"
#include "buffer.h"

#ifdef _DEBUG
#include <glad/glad.h>
#endif // _DEBUG

#ifndef _DEBUG
#include "../ThirdParty/glad/include/glad/glad.h";
#endif // !DEBUG

#define PushData(buffer, data, size) PushAlignedData(buffer, data, size, 1)
#define PushUInt(buffer, value)  { u32   v = value; PushAlignedData(buffer, &v, sizeof(v), 4); }
#define PushFloat(buffer, value) { f32   v = value; PushAlignedData(buffer, &v, sizeof(v), 4); }
#define PushVec3(buffer, value) PushAlignedData(buffer, value_ptr(value), sizeof(value), sizeof(vec4))
#define PushVec4(buffer, value) PushAlignedData(buffer, value_ptr(value), sizeof(value), sizeof(vec4))
#define PushMat3(buffer, value) PushAlignedData(buffer, value_ptr(value), sizeof(value), sizeof(vec4))
#define PushMat4(buffer, value) PushAlignedData(buffer, value_ptr(value), sizeof(value), sizeof(vec4))

#define CreateConstantBuffer(size) CreateBuffer(size, GL_UNIFORM_BUFFER, GL_STREAM_DRAW)
#define CreateStaticVertexBuffer(size) CreateBuffer(size, GL_ARRAY_BUFFER, GL_STATIC_DRAW)
#define CreateStaticIndexBuffer(size) CreateBuffer(size, GL_ELEMENT_ARRAY_BUFFER, GL_STATIC_DRAW)

struct aiScene;
struct aiNode;
struct aiMesh;
struct aiMaterial;
struct Image;

typedef glm::vec2  vec2;
typedef glm::vec3  vec3;
typedef glm::vec4  vec4;
typedef glm::ivec2 ivec2;
typedef glm::ivec3 ivec3;
typedef glm::ivec4 ivec4;
typedef glm::mat4  mat4;

struct Image
{
	void* pixels;

	ivec2 size;

	i32   nchannels;
	i32   stride;
};

struct Texture
{
	GLuint      handle;
	std::string filepath;
	ivec2		size;
};

struct Program
{
	GLuint             handle;
	std::string        filepath;
	std::string        programName;
	u64                lastWriteTimestamp; // What is this for?
	VertexShaderLayout vertexInputLayout;
};

enum class RenderMode
{
	FORWARD,
	DEFERRED
};

enum class RenderTargetsMode
{
	ALBEDO,
	NORMALS,
	POSITION,
	DEPTH,
	METALLIC, 
	ROUGHNESS,
	FINAL_RENDER,
};

enum Mode
{
	Mode_TexturedQuad,
	Mode_Count
};

struct App
{
	// Loop
	f32  deltaTime;
	bool isRunning;
	bool skyBox = false;
	// Input
	Input input;

	// Graphics
	char gpuName[64];
	char openGlVersion[64];

	ivec2 displaySize;
	
	// Entities
	Camera camera;
	Buffer cbuffer;

	u32 directionalLightModel;
	u32 sphereModel;

	u32 directPBRIBLProgramIdx;
	u32 deferredGeometryProgramIdx;
	u32 skyboxProgramIdx;
	u32 equirectangularToCubemapProgramIdx;
	u32 irradianceConvolutionProgramIdx;
	u32 prefilterProgramIdx;
	u32 brdfProgramIdx;

	u32 whiteTexIdx;
	u32 greyTexIdx;
	u32 blackTexIdx;
	u32 defaultNormalTexIdx;

	// Mode
	Mode mode;

	GLint uniformBufferAlignment;
	
	u32 globalParamsOffset;
	u32 globalParamsSize;

	// Location of the texture uniform in the textured quad shader
	GLuint programUniformTexture;
	GLuint texturedMeshProgram_uTexture;

	u32 bufferHandle;

	GLuint framebufferHandle;	

	RenderTargetsMode currentRenderTargetMode;
	RenderMode currentRenderMode;

	// GLuint finalRenderAttachmentHandle;

	// Lists
	std::vector<Texture>  textures;
	std::vector<Material> materials;
	std::vector<Mesh>	  meshes;
	std::vector<Model>	  models;
	std::vector<Program>  programs;
	std::vector<Light>    lights;

	std::vector<Entity> entities;

	unsigned int cubeVAO = 0;
	unsigned int cubeVBO = 0;

	unsigned int quadVAO = 0;
	unsigned int quadVBO;

	unsigned int irradianceMap;
	unsigned int prefilterMap;
	unsigned int brdfLUTTexture;
	unsigned int envCubemap;
};

// Functions
void Init(App* app);
void InitEntities(App* app);
void InitLight(App* app);
unsigned int InitSkybox(App* app, std::string filename, unsigned int& captureFBO, unsigned int& captureRBO, unsigned int& envCubemap, unsigned int& irradianceMap, unsigned int& prefilterMap, unsigned int& brdfLUTTexture);
void InitPrograms(App* app);
void InitGuiStyle();

void Gui(App* app);

void Update(App* app);
void UpdateInput(App* app);

void Render(App* app);
void RenderModel(App* app, Entity entity, Program program);
void RenderLight(App* app, Light light, Program program);

void OnGlError(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam);

GLuint FindVAO(Mesh& mesh, u32 submeshIndex, const Program& program);

void OnResize(App* app);
void GenerateColorTexture(GLuint& colorAttachmentHandle, vec2 displaySize, GLint internalFormat);

GLuint CreateTexture2DFromImage(Image image);
u32 LoadTexture2D(App* app, std::string filepath, unsigned int* width = nullptr, unsigned int* height = nullptr);

mat4 TransformScale(const vec3& scaleFactors);
mat4 TransformPositionScale(const vec3& pos, const vec3& scaleFactors);
mat4 TransformPositionRotationScale(const vec3& pos, const vec3& rotation, const vec3& scaleFactors);

Light CreateLight(App* app, LightType lightType, vec3 position, vec3 direction, vec3 color = vec3(1.0f, 1.0f, 1.0f), float intensity = 60000.0f);

void RenderCube(App* app);
void RenderQuad(App* app);