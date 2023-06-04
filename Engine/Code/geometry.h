#ifndef GEOMETRY_H
#define GEOMETRY_H

#include <vector>
#include <glad/glad.h>
#include "platform.h"

struct App;
typedef unsigned int u32;
typedef glm::vec2  vec2; 
typedef glm::vec3  vec3;
typedef glm::mat4  mat4;

/* GEOMETRY INFO */
struct VertexV3V2
{
	glm::vec3 pos;
	glm::vec2 uv;

};

const VertexV3V2 vertices[] = {
	{ glm::vec3(-0.5, -0.5, 0.0), glm::vec2(0.0, 0.0) }, // Bottom - Left Vertex
	{ glm::vec3(0.5, -0.5, 0.0), glm::vec2(0.0, 0.0) }, // Bottom - Left Right
	{ glm::vec3(0.5, 0.5, 0.0), glm::vec2(0.0, 0.0) }, // Top - Right Vertex
	{ glm::vec3(-0.5, 0.5, 0.0), glm::vec2(0.0, 0.0) }, // Top - Left Vertex

};


const u16 indices[] = {
	0,1,2,
	0,2,3
};


struct VertexBufferAttribute
{
	u8 location;
	u8 componentCount;
	u8 offset;
};

struct VertexShaderAtrribute
{
	u8 location;
	u8 componentCount;
};

struct VertexBufferLayout {
	std::vector<VertexBufferAttribute> attributes;
	u8 stride;
};

struct VertexShaderLayout {
	std::vector<VertexShaderAtrribute> attributes;
};

struct Vao
{
	GLuint handle;
	GLuint programHandle;
};


/* GEOMETRY */
struct Submesh
{
	VertexBufferLayout vertexBufferLayout;
	std::vector<float> vertices;
	std::vector<u32> indices;
	u32 vertexOffset;
	u32 indexOffset;

	std::vector<Vao> vaos;
};

struct Mesh
{
	std::vector<Submesh> submeshes;
	GLuint vertexBufferHandle;
	GLuint indexBufferHandle;
};


struct Material
{
	std::string name;
	glm::vec3 albedo;
	glm::vec3 emissive;
	f32 smoothness;
	u32 albedoTextureIdx;
	u32 normalsTextureIdx;
	u32 metallicTextureIdx;
	u32 roughnessTextureIdx;
	u32 aoTextureIdx;
};


struct Model {
	u32 meshIdx;
	std::vector<u32> materialIdx;
};

struct Entity
{
	String name;
	mat4 worldMatrix;
	mat4 worldViewProjection;

	float scale = 1.0f;

	u32 modelIndex;
	u32 localParamsOffset;
	u32 localParamsSize;

	void setPosition(const glm::vec3& newPosition) {
		mat4 translationMatrix = glm::translate(glm::mat4(1.0f), newPosition);
		worldMatrix = translationMatrix;
	}

	vec3 getPosition() const {
		return glm::vec3(worldMatrix[3]);
	}

	vec3 getScale() const {
		return glm::vec3(scale);
	}

};

struct Quad
{
	GLuint vao;

	GLuint embeddedVertices;
	GLuint embeddedElements;

	VertexV3V2 vertices[4] = { vec3(-1.0f, -1.0f, 0.0f),  vec2(0.0f, 0.0f),
							   vec3(1.0f, -1.0f, 0.0f),  vec2(1.0f, 0.0f),
							   vec3(1.0f,  1.0f, 0.0f),  vec2(1.0f, 1.0f),
							   vec3(-1.0f,  1.0f, 0.0f),  vec2(0.0f, 1.0f)
	};

	u16 indices[6] = { 0, 1, 2, 0, 2, 3 };
};

struct Cube
{
	GLuint vao;
	GLuint vbo;

	GLuint embeddedVertices;
	GLuint embeddedElements;

	float vertices[108] = {
		-1.0f,  1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		-1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f
	};
};

struct Cubemap
{
	u32 texture[6];
};

// LIGHTING
enum class LightType
{
	LightType_Directional,
	LightType_Point
};

struct Light
{
	LightType  type;
	vec3	   color;
	vec3       direction;
	vec3	   position;
	float	   intensity;
	Entity	   entity;
};

#endif