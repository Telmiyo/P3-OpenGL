#ifndef BUFFER
#define BUFFER_H

#include "platform.h"

struct App;
typedef unsigned int u32;
typedef glm::vec2  vec2;
typedef glm::vec3  vec3;
typedef glm::mat4  mat4;

typedef unsigned int GLuint;
typedef unsigned int GLenum;

struct Buffer
{
	GLuint handle;
	GLenum type;

	u32 size;
	u32 head;

	void* data;
};

u32 Align(u32 value, u32 alignment);
Buffer CreateBuffer(u32 size, GLenum type, GLenum usage);

void BindBuffer(const Buffer& buffer);
void MapBuffer(Buffer& buffer, GLenum access);
void UnmapBuffer(Buffer& buffer);
void AlignHead(Buffer& buffer, u32 alignment);
void PushAlignedData(Buffer& buffer, const void* data, u32 size, u32 alignment);

#endif