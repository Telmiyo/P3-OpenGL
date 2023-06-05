//
// engine.cpp : Put all your graphics stuff in this file. This is kind of the graphics module.
// In here, you should type all your OpenGL commands, and you can also type code to handle
// input platform events (e.g to move the camera or react to certain shortcuts), writing some
// graphics related GUI options, and so on.
//

#include "engine.h"
#include "assimp_model_loading.h"

#ifdef _DEBUG
#include <imgui.h>
#include <stb_image.h>
#include <stb_image_write.h>
#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <glm/glm.hpp>
#endif // _DEBUG

#ifndef _DEBUG
#include "../ThirdParty/imgui-docking/imgui.h"
#include "../ThirdParty/stb/stb_image.h"
#include "../ThirdParty/stb/stb_image_write.h"
#include "../ThirdParty/Assimp/include/assimp/cimport.h"
#include "../ThirdParty/Assimp/include/assimp/scene.h"
#include "../ThirdParty/Assimp/include/assimp/postprocess.h"
#include "../ThirdParty/glm/include/glm/glm.hpp"
#endif // !_DEBUG


GLuint CreateProgramFromSource(String programSource, const char* shaderName)
{
	GLenum err;

	if ((err = glGetError()) != GL_NO_ERROR)
		ELOG("OpenGL error %d\n", err);

	GLchar  infoLogBuffer[1024] = {};
	GLsizei infoLogBufferSize = sizeof(infoLogBuffer);
	GLsizei infoLogSize;
	GLint   success;

	char versionString[] = "#version 430\n";
	char shaderNameDefine[128];
	sprintf(shaderNameDefine, "#define %s\n", shaderName);
	char vertexShaderDefine[] = "#define VERTEX\n";
	char fragmentShaderDefine[] = "#define FRAGMENT\n";

	const GLchar* vertexShaderSource[] = {
		versionString,
		shaderNameDefine,
		vertexShaderDefine,
		programSource.str
	};
	const GLint vertexShaderLengths[] = {
		(GLint)strlen(versionString),
		(GLint)strlen(shaderNameDefine),
		(GLint)strlen(vertexShaderDefine),
		(GLint)programSource.len
	};
	const GLchar* fragmentShaderSource[] = {
		versionString,
		shaderNameDefine,
		fragmentShaderDefine,
		programSource.str
	};

	const GLint fragmentShaderLengths[] = {
		(GLint)strlen(versionString),
		(GLint)strlen(shaderNameDefine),
		(GLint)strlen(fragmentShaderDefine),
		(GLint)programSource.len
	};

	GLuint vshader = glCreateShader(GL_VERTEX_SHADER);
	if ((err = glGetError()) != GL_NO_ERROR)
		ELOG("OpenGL error %d\n", err);
	glShaderSource(vshader, ARRAY_COUNT(vertexShaderSource), vertexShaderSource, vertexShaderLengths);
	glCompileShader(vshader);
	glGetShaderiv(vshader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(vshader, infoLogBufferSize, &infoLogSize, infoLogBuffer);
		ELOG("glCompileShader() failed with vertex shader %s\nReported message:\n%s\n", shaderName, infoLogBuffer);
	}

	GLuint fshader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fshader, ARRAY_COUNT(fragmentShaderSource), fragmentShaderSource, fragmentShaderLengths);
	glCompileShader(fshader);
	glGetShaderiv(fshader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(fshader, infoLogBufferSize, &infoLogSize, infoLogBuffer);
		ELOG("glCompileShader() failed with fragment shader %s\nReported message:\n%s\n", shaderName, infoLogBuffer);
	}

	GLuint programHandle = glCreateProgram();
	glAttachShader(programHandle, vshader);
	if ((err = glGetError()) != GL_NO_ERROR)
		ELOG("OpenGL error %d\n", err);
	glAttachShader(programHandle, fshader);
	if ((err = glGetError()) != GL_NO_ERROR)
		ELOG("OpenGL error %d\n", err);
	glLinkProgram(programHandle);
	if ((err = glGetError()) != GL_NO_ERROR)
		ELOG("OpenGL error %d\n", err);
	glGetProgramiv(programHandle, GL_LINK_STATUS, &success);
	if (!success)
	{
		glGetProgramInfoLog(programHandle, infoLogBufferSize, &infoLogSize, infoLogBuffer);
		ELOG("glLinkProgram() failed with program %s\nReported message:\n%s\n", shaderName, infoLogBuffer);
	}

	glUseProgram(0);
	if ((err = glGetError()) != GL_NO_ERROR)
		ELOG("OpenGL error %d\n", err);

	glDetachShader(programHandle, vshader);
	if ((err = glGetError()) != GL_NO_ERROR)
		ELOG("OpenGL error %d\n", err);
	glDetachShader(programHandle, fshader);
	if ((err = glGetError()) != GL_NO_ERROR)
		ELOG("OpenGL error %d\n", err);
	glDeleteShader(vshader);
	if ((err = glGetError()) != GL_NO_ERROR)
		ELOG("OpenGL error %d\n", err);
	glDeleteShader(fshader);
	if ((err = glGetError()) != GL_NO_ERROR)
		ELOG("OpenGL error %d\n", err);

	if ((err = glGetError()) != GL_NO_ERROR)
		ELOG("OpenGL error %d\n", err);

	return programHandle;
}

u32 LoadProgram(App* app, const char* filepath, const char* programName)
{
	String programSource = ReadTextFile(filepath);

	Program program = {};
	program.handle = CreateProgramFromSource(programSource, programName);
	program.filepath = filepath;
	program.programName = programName;
	program.lastWriteTimestamp = GetFileLastWriteTimestamp(filepath);
	app->programs.push_back(program);

	return app->programs.size() - 1;
}

u8 LoadProgramAttributes(Program& program)
{
	GLsizei attributeCount;
	glGetProgramiv(program.handle, GL_ACTIVE_ATTRIBUTES, &attributeCount);

	for (u32 i = 0; i < attributeCount; ++i)
	{
		GLchar attributeName[128];
		GLsizei attributeNameLenght;
		GLint attributeSize;
		GLenum attributeType;
		glGetActiveAttrib(program.handle, i, ARRAY_COUNT(attributeName), &attributeNameLenght, &attributeSize, &attributeType, attributeName);

		u8 attributeLoacation = glGetAttribLocation(program.handle, attributeName);
		program.vertexInputLayout.attributes.push_back({ attributeLoacation, (u8)attributeSize });
	}

	return attributeCount;
}

Image LoadImage(const char* filename)
{
	Image img = {};
	stbi_set_flip_vertically_on_load(true);
	img.pixels = stbi_load(filename, &img.size.x, &img.size.y, &img.nchannels, 0);
	if (img.pixels)
	{
		img.stride = img.size.x * img.nchannels;
	}
	else
	{
		ELOG("Could not open file %s", filename);
	}
	return img;
}

void FreeImage(Image image)
{
	stbi_image_free(image.pixels);
}

GLuint CreateTexture2DFromImage(Image image)
{
	GLenum err;

	GLenum internalFormat = GL_RGB8;
	GLenum dataFormat = GL_RGB;
	GLenum dataType = GL_UNSIGNED_BYTE;

	switch (image.nchannels)
	{
	case 1: dataFormat = GL_RED; internalFormat = GL_R8; break;
	case 3: dataFormat = GL_RGB; internalFormat = GL_RGB8; break;
	case 4: dataFormat = GL_RGBA; internalFormat = GL_RGBA8; break;
	default: ELOG("LoadTexture2D() - Unsupported number of channels");
	}

	GLuint texHandle;
	glGenTextures(1, &texHandle);
	if ((err = glGetError()) != GL_NO_ERROR)
		ELOG("OpenGL error %d\n", err);
	glBindTexture(GL_TEXTURE_2D, texHandle);
	if ((err = glGetError()) != GL_NO_ERROR)
		ELOG("OpenGL error %d\n", err);
	glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, image.size.x, image.size.y, 0, dataFormat, dataType, image.pixels);
	if ((err = glGetError()) != GL_NO_ERROR)
		ELOG("OpenGL error %d\n", err);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	if ((err = glGetError()) != GL_NO_ERROR)
		ELOG("OpenGL error %d\n", err);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	if ((err = glGetError()) != GL_NO_ERROR)
		ELOG("OpenGL error %d\n", err);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	if ((err = glGetError()) != GL_NO_ERROR)
		ELOG("OpenGL error %d\n", err);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	if ((err = glGetError()) != GL_NO_ERROR)
		ELOG("OpenGL error %d\n", err);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	if ((err = glGetError()) != GL_NO_ERROR)
		ELOG("OpenGL error %d\n", err);
	glGenerateMipmap(GL_TEXTURE_2D);
	if ((err = glGetError()) != GL_NO_ERROR)
		ELOG("OpenGL error %d\n", err);
	glBindTexture(GL_TEXTURE_2D, 0);
	if ((err = glGetError()) != GL_NO_ERROR)
		ELOG("OpenGL error %d\n", err);

	return texHandle;
}

u32 LoadTexture2D(App* app, const char* filepath)
{
	GLenum err;

	for (u32 texIdx = 0; texIdx < app->textures.size(); ++texIdx)
		if (app->textures[texIdx].filepath == filepath)
			return texIdx;

	Image image = LoadImage(filepath);
	if ((err = glGetError()) != GL_NO_ERROR)
		ELOG("OpenGL error %d\n", err);

	if (image.pixels)
	{
		Texture tex = {};
		tex.handle = CreateTexture2DFromImage(image);
		if ((err = glGetError()) != GL_NO_ERROR)
			ELOG("OpenGL error %d\n", err);
		tex.filepath = filepath;
		tex.size = image.size;

		u32 texIdx = app->textures.size();
		app->textures.push_back(tex);

		FreeImage(image);
		return texIdx;
	}
	else
	{
		return UINT32_MAX;
	}
}

mat4 TransformScale(const vec3& scaleFactors)
{
	mat4 transform = glm::scale(scaleFactors);
	return transform;
}

mat4 TransformPositionScale(const vec3& pos, const vec3& scaleFactors)
{
	mat4 transform = glm::translate(pos);
	transform = glm::scale(transform, scaleFactors);
	return transform;
}

mat4 TransformPositionRotationScale(const vec3& pos, const vec3& rotation, const vec3& scaleFactors)
{
	mat4 transform = glm::translate(pos);
	transform = glm::rotate(transform, glm::radians(90.0f), rotation);
	transform = glm::scale(transform, scaleFactors);
	return transform;
}

void Init(App* app)
{
	GLenum err;

	InitGuiStyle();
	if (GLVersion.major > 4 || (GLVersion.major == 4 && GLVersion.minor >= 3))
	{
		glDebugMessageCallback(OnGlError, app);
	}

	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

	app->camera = Camera(vec3(-1.0f, 1.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f), 0.0f, 0.0f);
	app->camera.pitch = 0.0f;
	app->camera.aspectRatio = (float)app->displaySize.x / (float)app->displaySize.y;
	app->camera.Update();

	GLint maxUniformBufferSize;

	glGetIntegerv(GL_MAX_UNIFORM_BLOCK_SIZE, &maxUniformBufferSize);
	glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &app->uniformBufferAlignment);

	glGenBuffers(1, &app->bufferHandle);
	glBindBuffer(GL_UNIFORM_BUFFER, app->bufferHandle);
	glBufferData(GL_UNIFORM_BUFFER, maxUniformBufferSize, NULL, GL_STREAM_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	app->cbuffer = CreateBuffer(maxUniformBufferSize, GL_UNIFORM_BUFFER, GL_DYNAMIC_DRAW);
	if ((err = glGetError()) != GL_NO_ERROR)
	{
		ELOG("ERROR: Could not create constant buffer: %s\n", err);
	}

	app->whiteTexIdx = LoadTexture2D(app, "color_white.png");

	// Rendering
	app->currentRenderTargetMode = RenderTargetsMode::FINAL_RENDER;

	// Load Entities & Light
	InitEntities(app);
	if ((err = glGetError()) != GL_NO_ERROR)
		ELOG("OpenGL error %d\n", err);
	InitLight(app);

	std::vector<String> skyboxFaces;
	skyboxFaces.push_back(MakeString("Assets/skybox/east.png"));
	skyboxFaces.push_back(MakeString("Assets/skybox/west.png"));
	skyboxFaces.push_back(MakeString("Assets/skybox/top.png"));
	skyboxFaces.push_back(MakeString("Assets/skybox/bottom.png"));
	skyboxFaces.push_back(MakeString("Assets/skybox/north.png"));
	skyboxFaces.push_back(MakeString("Assets/skybox/south.png"));
	app->skyboxTexture = InitSkybox(app, skyboxFaces);
	app->skyboxVAO = InitSkyboxVAO(app);

	InitPrograms(app);

	OnResize(app);
}

void InitEntities(App* app)
{
	Entity orc;
	orc.name = MakeString("orc"); // Name
	orc.worldMatrix = glm::mat4(1.0f); // worldMatrix
	orc.worldViewProjection = glm::mat4(1.0f); // worldViewProjection
	orc.scale = 1.f; // scale
	orc.modelIndex = LoadModel(app, "Assets/orc/untitled.obj"); // modelIndex

	// Positions
	orc.setPosition(vec3(0.0f, 0.0f, 0.0f));

	Entity plane;
	plane.name = MakeString("Plane"); // Name
	plane.worldMatrix = glm::mat4(1.0f); // worldMatrix
	plane.worldViewProjection = glm::mat4(1.0f); // worldViewProjection
	plane.scale = 15.f; // scale
	plane.modelIndex = LoadModel(app, "Assets/Primitives/Plane/plane.obj"); // modelIndex

	plane.setPosition(vec3(0.0f, 0.0f, 0.0f));

	// Push Entities
	app->entities.push_back(orc);
	// app->entities.push_back(plane);
}

void InitLight(App* app)
{
	//Point
	app->lights.push_back(CreateLight(app, LightType::LightType_Point, vec3(2.5f, 3.0f, 2.5f), vec3(-2.5f, -3.0f, -2.5f), vec3(1.0f, 1.0f, 1.0f)));
	app->lights.push_back(CreateLight(app, LightType::LightType_Point, vec3(-2.5f, 3.0f, 2.5f), vec3(2.5f, -3.0f, -2.5f), vec3(1.0f, 1.0f, 1.0f)));
	app->lights.push_back(CreateLight(app, LightType::LightType_Point, vec3(2.5f, 3.0f, -2.5f), vec3(-2.5f, -3.0f, 2.5f), vec3(1.0f, 1.0f, 1.0f)));
	app->lights.push_back(CreateLight(app, LightType::LightType_Point, vec3(-2.5f, 3.0f, -2.5f), vec3(2.5f, -3.0f, 2.5f), vec3(1.0f, 1.0f, 1.0f)));
}

unsigned int InitSkybox(App* app, std::vector<String> faces)
{
	GLenum err;

	unsigned int textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

	for (unsigned int i = 0; i < faces.size(); i++)
	{
		Image image = LoadImage(faces[i].str);
		if (image.nchannels == 3) {
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
				0,
				GL_RGB,
				image.size.x,
				image.size.y,
				0,
				GL_RGB,
				GL_UNSIGNED_BYTE,
				image.pixels);
		}
		else if (image.nchannels == 4) {
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
				0,
				GL_RGBA,
				image.size.x,
				image.size.y,
				0,
				GL_RGBA,
				GL_UNSIGNED_BYTE,
				image.pixels);
		}
		else {
			ELOG("ERROR: Texture format not supported\n");
		}
		if ((err = glGetError()) != GL_NO_ERROR)
			ELOG("OpenGL error %d\n", err);
		FreeImage(image);
	}

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	return textureID;
}

unsigned int InitSkyboxVAO(App* app)
{
	float skyboxVertices[] = {
		// positions          
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

	unsigned int skyboxVAO, skyboxVBO;
	glGenVertexArrays(1, &skyboxVAO);
	glGenBuffers(1, &skyboxVBO);

	glBindVertexArray(skyboxVAO);

	glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

	glBindVertexArray(0);

	return skyboxVAO;
}

void InitPrograms(App* app) {
	app->directPBRProgramIdx = LoadProgram(app, "shaders/pbr_direct.glsl", "PBR_DIRECT");
	Program& directPBRProgram = app->programs[app->directPBRProgramIdx];
	LoadProgramAttributes(directPBRProgram);

	app->skyboxProgramIdx = LoadProgram(app, "shaders/skybox.glsl", "PBR_DIRECT");
	Program& skyboxProgram = app->programs[app->skyboxProgramIdx];
	LoadProgramAttributes(skyboxProgram);
}

void InitGuiStyle() {
	ImGuiStyle& style = ImGui::GetStyle();

	ImVec4 lightGray = ImVec4(0.85f, 0.85f, 0.85f, 1.0f);
	ImVec4 softPurple = ImVec4(0.8f, 0.7f, 0.9f, 1.0f);
	ImVec4 darkPurple = ImVec4(0.4f, 0.3f, 0.5f, 1.0f);

	style.Colors[ImGuiCol_Text] = darkPurple;
	style.Colors[ImGuiCol_WindowBg] = lightGray;
	style.Colors[ImGuiCol_Button] = softPurple;
	style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.9f, 0.8f, 1.0f, 1.0f);

	style.WindowRounding = 10.0f;
	style.FrameRounding = 8.0f;
	style.ScrollbarRounding = 8.0f;
	style.GrabRounding = 8.0f;

	style.FramePadding = ImVec2(6, 4);
	style.WindowPadding = ImVec2(10, 10);

	style.WindowBorderSize = 1.0f;
	style.FrameBorderSize = 0.0f;

	style.AntiAliasedFill = true;
	style.AntiAliasedLines = true;

}

void Gui(App* app)
{
	//Info window
	ImGui::Begin("Info");
	ImGui::Text("FPS: %f", 1.0f / app->deltaTime);
	ImGui::Text("OpenGL version: %s", glGetString(GL_VERSION));
	ImGui::Text("OpenGL Renderer: %s", glGetString(GL_RENDERER));
	ImGui::Text("OpenGL Vendor: %s", glGetString(GL_VENDOR));
	ImGui::Text("OpenGL GLSL version: %s", glGetString(GL_SHADING_LANGUAGE_VERSION));
	if (ImGui::TreeNode("OpenGL extensions:"))
	{
		int numExtensions = 0;
		glGetIntegerv(GL_NUM_EXTENSIONS, &numExtensions);
		for (int i = 0; i < numExtensions; ++i)
		{
			const u8* str = glGetStringi(GL_EXTENSIONS, GLuint(i));
			ImGui::Text("%s", str);
		}

		ImGui::TreePop();
	}

	ImGui::End();

	ImGui::Begin("Editor");

	ImGui::Dummy(ImVec2(0.0f, 5.0f));

	RenderTargetsMode selectedMode = app->currentRenderTargetMode;  // Initialize with a default value

	const char* modeNames[] = { "ALBEDO", "NORMALS", "POSITION", "DEPTH", "METALLIC", "ROUGHNESS", "FINAL_RENDER" };

	if (ImGui::BeginCombo("Render Target", modeNames[static_cast<int>(selectedMode)]))
	{
		for (int i = 0; i < IM_ARRAYSIZE(modeNames); i++)
		{
			if (i != static_cast<int>(RenderTargetsMode::METALLIC) && i != static_cast<int>(RenderTargetsMode::ROUGHNESS))
			{
				bool isSelected = (selectedMode == static_cast<RenderTargetsMode>(i));
				if (ImGui::Selectable(modeNames[i], isSelected))
				{
					selectedMode = static_cast<RenderTargetsMode>(i);
					app->currentRenderTargetMode = selectedMode;
				}
				if (isSelected)
				{
					ImGui::SetItemDefaultFocus();
				}
			}
		}
		ImGui::EndCombo();
	}


	ImGui::Dummy(ImVec2(0.0f, 10.0f));
	ImGui::Separator();
	ImGui::Dummy(ImVec2(0.0f, 10.0f));

	if (ImGui::TreeNode("Entities"))
	{
		for (u64 i = 0; i < app->entities.size(); ++i)
		{
			ImGui::PushID(i);

			Entity& entity = app->entities[i];

			ImGui::Text(entity.name.str);

			vec3 tmpPos = entity.getPosition();
			float position[3] = { tmpPos.x, tmpPos.y, tmpPos.z };

			ImGui::DragFloat3("Position", position, 0.1f, -20000000000000000.0f, 200000000000000000000.0f);
			entity.setPosition(vec3(position[0], position[1], position[2]));

			ImGui::PopID();
		}
		ImGui::TreePop();
	}

	ImGui::Dummy(ImVec2(0.0f, 5.0f));

	ImGui::Dummy(ImVec2(0.0f, 7.5f));
	ImGui::Separator();
	ImGui::Dummy(ImVec2(0.0f, 7.5f));

	if (ImGui::TreeNode("Lights"))
	{
		for (int i = 0; i < app->lights.size(); i++)
		{
			Light& light = app->lights[i];

			ImGui::PushID(i * 1000);

			std::string lightName = "Light: " + std::to_string(i);
			lightName += light.type == LightType::LightType_Directional ? " - Directional Light" : " - Point Light";
			ImGui::Text(lightName.c_str());

			ImGui::Dummy(ImVec2(0.0f, 10.0f));

			float position[3] = { light.position.x, light.position.y, light.position.z };
			ImGui::DragFloat3("Position", position, 0.1f, -20000000000000000.0f, 200000000000000000000.0f);
			light.position = vec3(position[0], position[1], position[2]);

			if (light.type == LightType::LightType_Directional)
			{
				float direction[3] = { light.direction.x, light.direction.y, light.direction.z };
				ImGui::DragFloat3("Direction", direction, 0.1f, -20000000000000000.0f, 200000000000000000000.0f);
				light.direction = vec3(direction[0], direction[1], direction[2]);
			}

			float color[3] = { light.color.r, light.color.g, light.color.b };
			ImGui::ColorPicker3("Color", color);
			light.color = vec3(color[0], color[1], color[2]);

			float intensity = light.intensity;
			ImGui::DragFloat("Intensity", &intensity, 0.1f, 0.0f, 100000000000000000000.0f);
			light.intensity = intensity;

			ImGui::PopID();

			ImGui::Dummy(ImVec2(0.0f, 10.0f));
			ImGui::Separator();
			ImGui::Dummy(ImVec2(0.0f, 15.0f));
		}
		ImGui::TreePop();
	}
	ImGui::End();
}

void Update(App* app)
{
	// You can handle app->input keyboard/mouse here
	UpdateInput(app);

	for (u64 i = 0; i < app->programs.size(); i++)
	{
		Program& program = app->programs[i];
		u64 currentTimestamp = GetFileLastWriteTimestamp(program.filepath.c_str());
		if (currentTimestamp > program.lastWriteTimestamp)
		{
			glDeleteProgram(program.handle);
			String programSource = ReadTextFile(program.filepath.c_str());
			const char* programName = program.programName.c_str();
			program.handle = CreateProgramFromSource(programSource, programName);
			program.lastWriteTimestamp = currentTimestamp;
		}
	}

	float znear = 0.1f;
	float zfar = 1000.0f;

	mat4 projection = glm::perspective(glm::radians(app->camera.zoom), app->camera.aspectRatio, znear, zfar);
	mat4 view = app->camera.GetViewMatrix();

	MapBuffer(app->cbuffer, GL_WRITE_ONLY);

	//Global params
	app->globalParamsOffset = app->cbuffer.head;

	PushUInt(app->cbuffer, (u32)app->currentRenderTargetMode);
	PushVec3(app->cbuffer, app->camera.position);
	PushUInt(app->cbuffer, app->lights.size());

	for (u32 i = 0; i < app->lights.size(); ++i)
	{
		AlignHead(app->cbuffer, sizeof(vec4));

		Light& light = app->lights[i];
		PushUInt(app->cbuffer, (u32)light.type);
		PushVec3(app->cbuffer, light.color);
		PushVec3(app->cbuffer, light.direction);
		PushVec3(app->cbuffer, light.position);
		PushFloat(app->cbuffer, light.intensity);
	}

	app->globalParamsSize = app->cbuffer.head - app->globalParamsOffset;

	// Entities
	for (u64 i = 0; i < app->entities.size(); ++i)
	{
		AlignHead(app->cbuffer, app->uniformBufferAlignment);

		Entity& entity = app->entities[i];
		mat4 world = entity.worldMatrix;

		world = TransformPositionScale(entity.getPosition(), entity.getScale());
		mat4 worldViewProjection = projection * view * world;

		entity.localParamsOffset = app->cbuffer.head;
		PushMat4(app->cbuffer, world);
		PushMat4(app->cbuffer, worldViewProjection);
		entity.localParamsSize = app->cbuffer.head - entity.localParamsOffset;
	}

	UnmapBuffer(app->cbuffer);
}

void UpdateInput(App* app)
{
	if (app->input.keys[K_W] == BUTTON_PRESSED) {
		app->camera.UpdateKeyboard(Camera_Movement::CAMERA_FORWARD, app->deltaTime);
	}
	if (app->input.keys[K_A] == BUTTON_PRESSED) {
		app->camera.UpdateKeyboard(Camera_Movement::CAMERA_LEFT, app->deltaTime);
	}
	if (app->input.keys[K_S] == BUTTON_PRESSED) {
		app->camera.UpdateKeyboard(Camera_Movement::CAMERA_BACKWARD, app->deltaTime);
	}
	if (app->input.keys[K_D] == BUTTON_PRESSED) {
		app->camera.UpdateKeyboard(Camera_Movement::CAMERA_RIGHT, app->deltaTime);
	}

	if (app->input.mouseButtons[LEFT] == BUTTON_PRESSED || app->input.mouseButtons[RIGHT] == BUTTON_PRESSED)
	{
		app->camera.UpdateMouse(app->input.mouseDelta.x, -app->input.mouseDelta.y);
	}

}

void Render(App* app)
{
	GLenum err;

	glClearColor(0.1f, 0.1f, 0.1f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glViewport(0, 0, app->displaySize.x, app->displaySize.y);

	// Skybox
	glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 1, -1, "Skybox");

	glDepthFunc(GL_LEQUAL);  // Change this to GL_LEQUAL. The depth buffer will have a value of 1.0 for the skybox, so changing this allows us to still pass the depth test

	glDepthMask(GL_FALSE);
	Program skyboxProgram = app->programs[app->skyboxProgramIdx];
	glUseProgram(skyboxProgram.handle);
	if ((err = glGetError()) != GL_NO_ERROR) {
		ELOG("Error using program: %d\n", err);
	}

	float znear = 0.1f;
	float zfar = 1000.0f;

	mat4 projection = glm::perspective(glm::radians(app->camera.zoom), app->camera.aspectRatio, znear, zfar);
	mat4 view = mat4(glm::mat3(app->camera.GetViewMatrix()));

	glUniformMatrix4fv(glGetUniformLocation(skyboxProgram.handle, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
	glUniformMatrix4fv(glGetUniformLocation(skyboxProgram.handle, "view"), 1, GL_FALSE, glm::value_ptr(view));

	glBindVertexArray(app->skyboxVAO);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, app->skyboxTexture);
	glUniform1i(glGetUniformLocation(skyboxProgram.handle, "skybox"), 0); // we used GL_TEXTURE0
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glDepthMask(GL_TRUE);

	glDepthFunc(GL_LESS); // Set it back to default afterwards

	glPopDebugGroup();
	if ((err = glGetError()) != GL_NO_ERROR) { ELOG("Error popping debug group: %d\n", err); }

	// Model
	glEnable(GL_DEPTH_TEST);
	if ((err = glGetError()) != GL_NO_ERROR) { ELOG("Error enabling depth test: %d\n", err); }

	Program modelProgram = app->programs[app->directPBRProgramIdx];
	glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 1, -1, "Direct PBR Shaded Model");

	glUseProgram(modelProgram.handle);
	if ((err = glGetError()) != GL_NO_ERROR) {
		ELOG("Error using program: %d\n", err);
	}

	for (u64 i = 0; i < app->entities.size(); ++i)
	{
		Entity& entity = app->entities[i];
		RenderModel(app, entity, modelProgram);
	}

	glPopDebugGroup();
	if ((err = glGetError()) != GL_NO_ERROR) { ELOG("Error popping debug group: %d\n", err); }
}

void RenderModel(App* app, Entity entity, Program program)
{
	GLenum err;

	Model& model = app->models[entity.modelIndex];
	Mesh& mesh = app->meshes[model.meshIdx];

	glBindBufferRange(GL_UNIFORM_BUFFER, BINDING(0), app->cbuffer.handle, app->globalParamsOffset, app->globalParamsSize);
	glBindBufferRange(GL_UNIFORM_BUFFER, BINDING(1), app->cbuffer.handle, entity.localParamsOffset, entity.localParamsSize);
	if ((err = glGetError()) != GL_NO_ERROR) {
		ELOG("Error binding buffer range: %d\n", err);
	}

	for (u32 j = 0; j < mesh.submeshes.size(); ++j)
	{
		GLuint vao = FindVAO(mesh, j, program);
		glBindVertexArray(vao);
		if ((err = glGetError()) != GL_NO_ERROR) { ELOG("Error binding vertex array: %d\n", err); }

		u32 submeshMaterialIdx = model.materialIdx[j];
		Material& submeshMaterial = app->materials[submeshMaterialIdx];

		if (submeshMaterial.albedoTextureIdx < app->textures.size())
		{
			glActiveTexture(GL_TEXTURE0);
			if ((err = glGetError()) != GL_NO_ERROR)
				ELOG("Error setting textures: %d\n", err);
			glBindTexture(GL_TEXTURE_2D, app->textures[submeshMaterial.albedoTextureIdx].handle);
			if ((err = glGetError()) != GL_NO_ERROR)
				ELOG("Error setting textures: %d\n", err);
			GLuint textureLocation = glGetUniformLocation(program.handle, "albedoMap");
			if ((err = glGetError()) != GL_NO_ERROR)
				ELOG("Error setting textures: %d\n", err);
			glUniform1i(textureLocation, 0);
			if ((err = glGetError()) != GL_NO_ERROR)
				ELOG("Error setting textures: %d\n", err);
		}

		if (submeshMaterial.normalsTextureIdx < app->textures.size()) {
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, app->textures[submeshMaterial.normalsTextureIdx].handle);
			GLuint textureLocation = glGetUniformLocation(program.handle, "normalMap");
			glUniform1i(textureLocation, 1);
			if ((err = glGetError()) != GL_NO_ERROR)
				ELOG("Error setting textures: %d\n", err);
		}

		if (submeshMaterial.metallicTextureIdx < app->textures.size()) {
			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D, app->textures[submeshMaterial.metallicTextureIdx].handle);
			GLuint textureLocation = glGetUniformLocation(program.handle, "metallicMap");
			glUniform1i(textureLocation, 2);
			if ((err = glGetError()) != GL_NO_ERROR)
				ELOG("Error setting textures: %d\n", err);
		}

		if (submeshMaterial.roughnessTextureIdx < app->textures.size()) {
			glActiveTexture(GL_TEXTURE3);
			glBindTexture(GL_TEXTURE_2D, app->textures[submeshMaterial.roughnessTextureIdx].handle);
			GLuint textureLocation = glGetUniformLocation(program.handle, "roughnessMap");
			glUniform1i(textureLocation, 3);
			if ((err = glGetError()) != GL_NO_ERROR)
				ELOG("Error setting textures: %d\n", err);
		}

		if (submeshMaterial.aoTextureIdx < app->textures.size()) {
			glActiveTexture(GL_TEXTURE4);
			glBindTexture(GL_TEXTURE_2D, app->textures[submeshMaterial.aoTextureIdx].handle);
			GLuint textureLocation = glGetUniformLocation(program.handle, "aoMap");
			glUniform1i(textureLocation, 4);
			if ((err = glGetError()) != GL_NO_ERROR)
				ELOG("Error setting textures: %d\n", err);
		}

		glActiveTexture(GL_TEXTURE5);
		if ((err = glGetError()) != GL_NO_ERROR)
			ELOG("Error drawing elements: %d\n", err);
		glBindTexture(GL_TEXTURE_CUBE_MAP, app->skyboxTexture);
		if ((err = glGetError()) != GL_NO_ERROR)
			ELOG("Error drawing elements: %d\n", err);
		GLuint skyboxLocation = glGetUniformLocation(program.handle, "skybox");
		if ((err = glGetError()) != GL_NO_ERROR)
			ELOG("Error drawing elements: %d\n", err);
		glUniform1i(skyboxLocation, 5);
		if ((err = glGetError()) != GL_NO_ERROR)
			ELOG("Error drawing elements: %d\n", err);

		Submesh& submesh = mesh.submeshes[j];
		glDrawElements(GL_TRIANGLES, submesh.indices.size(), GL_UNSIGNED_INT, (void*)(u64)submesh.indexOffset);
		if ((err = glGetError()) != GL_NO_ERROR) {
			ELOG("Error drawing elements: %d\n", err);
		}
	}

	// Unbind and cleanup
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	if ((err = glGetError()) != GL_NO_ERROR) {
		ELOG("Error unbinding uniform buffer: %d\n", err);
	}

	glBindVertexArray(0);
	if ((err = glGetError()) != GL_NO_ERROR) {
		ELOG("Error unbinding vertex array: %d\n", err);
	}

	for (int i = 0; i < 5; ++i) {
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, 0);
		if ((err = glGetError()) != GL_NO_ERROR) {
			ELOG("Error unbinding texture at index %d: %d\n", i, err);
		}
	}
}

void RenderLight(App* app, Light light, Program program)
{
	Model& model = app->models[light.entity.modelIndex];
	Mesh& mesh = app->meshes[model.meshIdx];

	glBindBufferRange(GL_UNIFORM_BUFFER, BINDING(0), app->cbuffer.handle, app->globalParamsOffset, app->globalParamsSize);
	glBindBufferRange(GL_UNIFORM_BUFFER, BINDING(1), app->cbuffer.handle, light.entity.localParamsOffset, light.entity.localParamsSize);

	for (u32 j = 0; j < mesh.submeshes.size(); ++j)
	{
		GLuint vao = FindVAO(mesh, j, program);
		glBindVertexArray(vao);

		GLuint lightColorLocation = glGetUniformLocation(program.handle, "uLightColor");
		glUniform3f(lightColorLocation, light.color.r, light.color.g, light.color.b);

		Submesh& submesh = mesh.submeshes[j];
		glDrawElements(GL_TRIANGLES, submesh.indices.size(), GL_UNSIGNED_INT, (void*)(u64)submesh.indexOffset);
	}

	glUnmapBuffer(GL_UNIFORM_BUFFER);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void OnGlError(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
{
	if (severity == GL_DEBUG_SEVERITY_NOTIFICATION) {
		return;
	}

	ELOG("OpenGL debug message: %s", message);

	switch (source)
	{
	case GL_DEBUG_SOURCE_API:				ELOG(" - source: GL_DEBUG_SOURCE_API"); break;
	case GL_DEBUG_SOURCE_WINDOW_SYSTEM:		ELOG(" - source: GL_DEBUG_SOURCE_WINDOW_SYSTEM"); break;
	case GL_DEBUG_SOURCE_SHADER_COMPILER:	ELOG(" - source: GL_DEBUG_SOURCE_SHADER_COMPILER"); break;
	case GL_DEBUG_SOURCE_THIRD_PARTY:		ELOG(" - source: GL_DEBUG_SOURCE_THIRD_PARTY"); break;
	case GL_DEBUG_SOURCE_APPLICATION:		ELOG(" - source: GL_DEBUG_SOURCE_APPLICATION"); break;
	case GL_DEBUG_SOURCE_OTHER:				ELOG(" - source: GL_DEBUG_SOURCE_OTHER");  break;
	}

	switch (source)
	{
	case GL_DEBUG_TYPE_ERROR:				ELOG(" - source: GL_DEBUG_TYPE_ERROR"); break;
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:	ELOG(" - source: GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR"); break;
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:	ELOG(" - source: GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR"); break;
	case GL_DEBUG_TYPE_PORTABILITY:			ELOG(" - source: GL_DEBUG_TYPE_PORTABILITY"); break;
	case GL_DEBUG_TYPE_PERFORMANCE:			ELOG(" - source: GL_DEBUG_TYPE_PERFORMANCE"); break;
	case GL_DEBUG_TYPE_MARKER:				ELOG(" - source: GL_DEBUG_TYPE_MARKER"); break;
	case GL_DEBUG_TYPE_PUSH_GROUP:			ELOG(" - source: GL_DEBUG_TYPE_PUSH_GROUP"); break;
	case GL_DEBUG_TYPE_POP_GROUP:			ELOG(" - source: GL_DEBUG_TYPE_POP_GROUP"); break;
	case GL_DEBUG_TYPE_OTHER:				ELOG(" - source: GL_DEBUG_TYPE_OTHER"); break;
	}

	switch (source)
	{
	case GL_DEBUG_SEVERITY_HIGH:			ELOG(" - source: GL_DEBUG_SEVERITY_HIGH"); break;
	case GL_DEBUG_SEVERITY_MEDIUM:			ELOG(" - source: GL_DEBUG_SEVERITY_MEDIUM"); break;
	case GL_DEBUG_SEVERITY_LOW:				ELOG(" - source: GL_DEBUG_SEVERITY_LOW"); break;
	case GL_DEBUG_SEVERITY_NOTIFICATION:	ELOG(" - source: GL_DEBUG_SEVERITY_NOTIFICATION"); break;
	}
}

GLuint FindVAO(Mesh& mesh, u32 submeshIndex, const Program& program)
{
	Submesh& submesh = mesh.submeshes[submeshIndex];

	//Try finding a vao for this submesh/program
	for (u32 i = 0; i < (u32)submesh.vaos.size(); ++i)
	{
		if (submesh.vaos[i].programHandle == program.handle)
		{
			return submesh.vaos[i].handle;
		}
	}

	GLuint vaoHandle = 0;

	//Create a new vao for this submesh/program
	glGenVertexArrays(1, &vaoHandle);
	glBindVertexArray(vaoHandle);

	glBindBuffer(GL_ARRAY_BUFFER, mesh.vertexBufferHandle);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.indexBufferHandle);

	for (u32 i = 0; i < program.vertexInputLayout.attributes.size(); ++i)
	{
		bool attributeWasLinked = false;

		for (u32 j = 0; j < submesh.vertexBufferLayout.attributes.size(); ++j)
		{
			if (program.vertexInputLayout.attributes[i].location == submesh.vertexBufferLayout.attributes[j].location)
			{
				const u32 index = submesh.vertexBufferLayout.attributes[j].location;
				const u32 ncomp = submesh.vertexBufferLayout.attributes[j].componentCount;
				const u32 offset = submesh.vertexBufferLayout.attributes[j].offset + submesh.vertexOffset;  //attribute offset + vertex offset
				const u32 stride = submesh.vertexBufferLayout.stride;
				glVertexAttribPointer(index, ncomp, GL_FLOAT, GL_FALSE, stride, (void*)(u64)offset);
				glEnableVertexAttribArray(index);

				attributeWasLinked = true;
				break;
			}
		}
	}

	glBindVertexArray(0);

	Vao vao = { vaoHandle, program.handle };
	submesh.vaos.push_back(vao);

	return vaoHandle;
}

void OnResize(App* app)
{
	app->camera.aspectRatio = (float)app->displaySize.x / (float)app->displaySize.y;

	GenerateColorTexture(app->finalRenderAttachmentHandle, app->displaySize, GL_RGBA16F);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, app->displaySize.x, app->displaySize.y, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_2D, 0);


	glGenFramebuffers(1, &app->framebufferHandle);
	glBindFramebuffer(GL_FRAMEBUFFER, app->framebufferHandle);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, app->finalRenderAttachmentHandle, 0);

	GLenum buffers[] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(1, buffers);

	GLenum framebufferStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (framebufferStatus != GL_FRAMEBUFFER_COMPLETE)
	{
		switch (framebufferStatus)
		{
		case GL_FRAMEBUFFER_UNDEFINED:						ELOG("GL_FRAMEBUFFER_UNDEFINED"); break;
		case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:			ELOG("GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT"); break;
		case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:	ELOG("GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT"); break;
		case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:			ELOG("GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER"); break;
		case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:			ELOG("GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER"); break;
		case GL_FRAMEBUFFER_UNSUPPORTED:					ELOG("GL_FRAMEBUFFER_UNSUPPORTED"); break;
		case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:			ELOG("GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE"); break;
		case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:		ELOG("GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS"); break;
		default:											ELOG("Unknown framebuffer status error") break;
		}
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void GenerateColorTexture(GLuint& colorAttachmentHandle, vec2 displaySize, GLint internalFormat)
{
	GLenum err;

	glGenTextures(1, &colorAttachmentHandle);
	if ((err = glGetError()) != GL_NO_ERROR) {
		ELOG("Error generating color texture");
	}
	glBindTexture(GL_TEXTURE_2D, colorAttachmentHandle);
	if ((err = glGetError()) != GL_NO_ERROR) {
		ELOG("Error generating color texture");
	}
	glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, displaySize.x, displaySize.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	if ((err = glGetError()) != GL_NO_ERROR) {
		ELOG("Error generating color texture");
	}
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_2D, 0);
	if ((err = glGetError()) != GL_NO_ERROR) {
		ELOG("Error generating color texture");
	}
}

Light CreateLight(App* app, LightType lightType, vec3 position, vec3 direction, vec3 color, float intensity)
{
	Light light;
	light.type = lightType;
	light.position = position;
	light.color = color;
	light.direction = direction;
	light.intensity = intensity;

	Entity entity;
	entity.setPosition(position);

	if (lightType == LightType::LightType_Directional) { entity.modelIndex = app->directionalLightModel; }
	else if (lightType == LightType::LightType_Point) { entity.modelIndex = app->sphereModel; }

	light.entity = entity;

	return light;
}