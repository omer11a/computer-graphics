#include "vec.h"
#include "mat.h"
#include "stdafx.h"
#include "Renderer.h"
#include "Light.h"
#include "CG_skel_w_MFC.h"
#include "GL/freeglut.h"

#include <iostream>
#include <sstream>
#include <functional>

Renderer::Renderer() : BaseRenderer(512, 512), 
m_cTransform(1), m_projection(1), m_oTransform(1), m_nTransform(1), m_cnTransform(1), is_wire_mode(false)
{
	InitOpenGLRendering();
	anti_factor = 1;
	UpdateBuffers(512, 512);
}
Renderer::Renderer(int width, int height) : BaseRenderer(width, height),
m_cTransform(1), m_projection(1), m_oTransform(1), m_nTransform(1), m_cnTransform(1), is_wire_mode(false)
{
	InitOpenGLRendering();
	anti_factor = 1;
	UpdateBuffers(width, height);
}

Renderer::~Renderer(void)
{
}

void Renderer::UpdateBuffers(int width, int height)
{
	m_width = anti_factor * width;
	m_height = anti_factor * height;
	m_screen_width = width;
	m_screen_height = height;
	min_size = min(m_width, m_height) / 2;
	CreateOpenGLBuffer(); //Do not remove this line.
}

void Renderer::DrawTriangles(
	const vector<vec3>* vertices,
	const vector<Material>* materials,
	const vector<vec3>* centerPositions,
	const bool hasTexture,
	const GLuint textureID,
	const bool hasNormalMap,
	const GLuint normalMapID,
	const vector<vec2>* textureCoordinates,
	const vector<vec2>* textureCenters,
	const vector<vec3>* vertexNormals,
	const vector<vec3>* faceNormals)
{

	// Use object shader
	objectsProgram.Activate();

	// uniform parameters
	objectsProgram.SetUniformParameter(int(hasTexture), "hasTexture");
	objectsProgram.SetUniformParameter(textureID, "textureSampler");
	objectsProgram.SetUniformParameter(m_oTransform, "modelMatrix");
	objectsProgram.SetUniformParameter(m_nTransform, "normalMatrix");
	objectsProgram.SetUniformParameter(mv, "modelViewMatrix");
	objectsProgram.SetUniformParameter(mvp, "modelViewProjectionMatrix");

	// TODO: if there are no normals, use the flat shader

	// split material parameters
	vector<vec3> ambients, diffuses, speculars;
	vector<float> shininess;
	for (auto i = materials->begin(); i != materials->end(); ++i) {
		ambients.push_back((*i).ambientReflectance);
		diffuses.push_back((*i).diffuseReflectance);
		speculars.push_back((*i).specularReflectance);
		shininess.push_back((*i).shininess);
	}

	vector<GLuint> buffers;
	buffers.push_back(objectsProgram.SetInParameter(*vertices, 0 , 3));				//in vec3 vertexPosition;
	buffers.push_back(objectsProgram.SetInParameter(*centerPositions, 1 , 3));		//in vec3 centerPosition;
	buffers.push_back(objectsProgram.SetInParameter(*vertexNormals, 2, 3));			//in vec3 vertexNormal;
	buffers.push_back(objectsProgram.SetInParameter(*faceNormals, 3, 3));		//in vec3 faceNormal;
	buffers.push_back(objectsProgram.SetInParameter(ambients, 4, 3));				//in vec3 ambientReflectance;
	buffers.push_back(objectsProgram.SetInParameter(speculars, 5, 3));				//in vec3 specularReflectance;
	buffers.push_back(objectsProgram.SetInParameter(diffuses, 6, 3));				//in vec3 diffuseReflectance;
	buffers.push_back(objectsProgram.SetInParameter(shininess, 7, 1));				//in float shininess;
	
	if (hasTexture) {
		buffers.push_back(objectsProgram.SetInParameter(*textureCoordinates, 8, 2));	//in vec2 uv;
		buffers.push_back(objectsProgram.SetInParameter(*textureCenters, 9, 2));		//in vec2 centerUv;
	}
	
	// Draw the triangle !
	if (is_wire_mode) {
		glDrawArrays(GL_LINES, 0, vertices->size());
	} else {
		glDrawArrays(GL_TRIANGLES, 0, vertices->size());
	}

	objectsProgram.ClearAttributes();
	for (int i = 0; i < buffers.size(); ++i) {
		glDeleteBuffers(1, &buffers[i]);
	}
}

void Renderer::DrawModelNormals(
	const vector<vec3>* vertices,
	const vector<vec3>* centerPositions,
	const vector<vec3>* vertexNormals,
	const vector<vec3>* faceNormals) {
	vec3 pink(1, 140 / 255.0f, 1);
	vec3 yellow(1, 1, 0);
	vector<vec3> vertex_copy, center_copy, vnormal_copy, fnormal_copy;
	vector<int> is_normal;

	if ((vertexNormals == NULL) && (faceNormals == NULL)) {
		return;
	}

	for (int i = 0; i < vertices->size(); ++i) {
		if (vertexNormals != NULL) {
			vertex_copy.push_back((*vertices)[i]);
			vertex_copy.push_back((*vertices)[i]);
			vnormal_copy.push_back((*vertexNormals)[i]);
			vnormal_copy.push_back((*vertexNormals)[i]);
		}
		if (faceNormals != NULL) {
			center_copy.push_back((*centerPositions)[i]);
			center_copy.push_back((*centerPositions)[i]);
			fnormal_copy.push_back((*faceNormals)[i]);
			fnormal_copy.push_back((*faceNormals)[i]);
			
		}
		is_normal.push_back(0);
		is_normal.push_back(1);
	}

	// Use our shader
	normalsProgram.Activate();
	mat4 vp = m_projection * m_cTransform;
	// uniform parameters
	normalsProgram.SetUniformParameter(m_oTransform, "modelMatrix");
	normalsProgram.SetUniformParameter(m_nTransform, "normalModelMatrix");
	normalsProgram.SetUniformParameter(vp, "viewProjectionMatrix");
	normalsProgram.SetUniformParameter(mvp, "modelViewProjectionMatrix");

	vector<GLuint> buffers;
	buffers.push_back(normalsProgram.SetInParameter(is_normal, 2, 1));

	if (vertexNormals != NULL) {
		normalsProgram.SetUniformParameter(pink, "color");
		buffers.push_back(normalsProgram.SetInParameter(vertex_copy, 0, 3));
		buffers.push_back(normalsProgram.SetInParameter(vnormal_copy, 1, 3));
		glDrawArrays(GL_LINES, 0, vertex_copy.size());
	}

	if (faceNormals != NULL) {
		normalsProgram.SetUniformParameter(yellow, "color");
		buffers.push_back(normalsProgram.SetInParameter(center_copy, 0, 3));
		buffers.push_back(normalsProgram.SetInParameter(fnormal_copy, 1, 3));
		glDrawArrays(GL_LINES, 0, center_copy.size());
	}

	normalsProgram.ClearAttributes();
	for (int i = 0; i < buffers.size(); ++i) {
		glDeleteBuffers(1, &buffers[i]);
	}
}

void Renderer::DrawSquare(const vec3& p1, const vec3& p2, const vec3& p3, const vec3& p4, const vec3& color)
{
	//DrawLine(p1, vec3(0), p2, vec3(0), color);
	//DrawLine(p2, vec3(0), p3, vec3(0), color);
	//DrawLine(p3, vec3(0), p4, vec3(0), color);
	//DrawLine(p4, vec3(0), p1, vec3(0), color);
}

void Renderer::DrawBox(const vec3& minValues, const vec3& maxValues)
{
	vec3 color(0.9);
	// front
	DrawSquare(vec3(minValues.x, minValues.y, minValues.z), vec3(maxValues.x, minValues.y, minValues.z),
		vec3(maxValues.x, maxValues.y, minValues.z), vec3(minValues.x, maxValues.y, minValues.z), color);
	// back
	DrawSquare(vec3(minValues.x, minValues.y, maxValues.z), vec3(maxValues.x, minValues.y, maxValues.z),
		vec3(maxValues.x, maxValues.y, maxValues.z), vec3(minValues.x, maxValues.y, maxValues.z), color);
	// top
	DrawSquare(vec3(minValues.x, maxValues.y, minValues.z), vec3(maxValues.x, maxValues.y, minValues.z),
		vec3(maxValues.x, maxValues.y, maxValues.z), vec3(minValues.x, maxValues.y, maxValues.z), color);
	// bottom
	DrawSquare(vec3(minValues.x, minValues.y, minValues.z), vec3(maxValues.x, minValues.y, minValues.z),
		vec3(maxValues.x, minValues.y, maxValues.z), vec3(minValues.x, minValues.y, maxValues.z), color);
	// left
	DrawSquare(vec3(minValues.x, minValues.y, minValues.z), vec3(minValues.x, minValues.y, maxValues.z),
		vec3(minValues.x, maxValues.y, maxValues.z), vec3(minValues.x, maxValues.y, minValues.z), color);
	// right
	DrawSquare(vec3(maxValues.x, minValues.y, minValues.z), vec3(maxValues.x, minValues.y, maxValues.z),
		vec3(maxValues.x, maxValues.y, maxValues.z), vec3(maxValues.x, maxValues.y, minValues.z), color);
}

void Renderer::DrawCamera()
{
	vec3 color(1, 140 / 255, 1);
	mat4 mvp = m_projection * m_cTransform * m_oTransform;
	vec3 camera_location = convert4dTo3d(mvp * vec4(0, 0, 0, 1));

	// Use our shader
	basicProgram.Activate();
	basicProgram.SetUniformParameter(color, "color");

	vector<vec3> plus;
	GLfloat offset = 5.0f * anti_factor / min_size;
	plus.push_back(camera_location + vec3(0, -offset, 0)); // |
	plus.push_back(camera_location + vec3(0, offset, 0));
	plus.push_back(camera_location + vec3(-offset, 0, 0)); // -
	plus.push_back(camera_location + vec3(offset, 0, 0));

	GLuint buffer = basicProgram.SetInParameter(plus, 0, 3);			//in vec3 vertexPosition;
	glDrawArrays(GL_LINES, 0, plus.size());
	basicProgram.ClearAttributes();
	glDeleteBuffers(1, &buffer);
}

void Renderer::DrawLight(const vec3& color, const vec3& position)
{
	mat4 mvp = m_projection * m_cTransform * m_oTransform;
	vec3 light_location = convert4dTo3d(mvp * vec4(position, 1));

	// Use our shader
	basicProgram.Activate();
	basicProgram.SetUniformParameter(color, "color");

	vector<vec3> star;
	GLfloat offset = 5.0f * anti_factor / min_size;
	star.push_back(light_location + vec3(-offset, -offset, 0)); // \ 
	star.push_back(light_location + vec3(offset, offset, 0));
	star.push_back(light_location + vec3(offset, -offset, 0)); // /
	star.push_back(light_location + vec3(-offset, offset, 0));
	star.push_back(light_location + vec3(0, -offset, 0)); // |
	star.push_back(light_location + vec3(0, offset, 0));
	star.push_back(light_location + vec3(-offset, 0, 0)); // -
	star.push_back(light_location + vec3(offset, 0, 0));

	GLuint buffer = basicProgram.SetInParameter(star, 0, 3);			//in vec3 vertexPosition;
	glDrawArrays(GL_LINES, 0, star.size());
	basicProgram.ClearAttributes();
	glDeleteBuffers(1, &buffer);
}

void Renderer::SetShaderLights(const AmbientLight& amb_light, const vector<DirectionalLightSource *>& lights) {
	objectsProgram.Activate();
	for (int i = 0; i < lights.size(); i++) {
		std::ostringstream position_ss, intensity_ss;
		position_ss << "lights[" << i << "].position";
		intensity_ss << "lights[" << i << "].intensity";
		objectsProgram.SetUniformParameter(lights[i]->GetPositionForShader(), position_ss.str().c_str());
		objectsProgram.SetUniformParameter(lights[i]->getIntensity(), intensity_ss.str().c_str());
	}

	objectsProgram.SetUniformParameter(amb_light.getIntensity(), "ambientLightColor");
	objectsProgram.SetUniformParameter(int(lights.size()), "numberOfLights");
}

void Renderer::SetCameraTransform(const mat4& cInverseTransform, const mat4 & cTransform)
{
	m_cTransform = cInverseTransform;
	m_cnTransform = convert4dTo3d(m_cTransform);
	
	vec3 camera_position = convert4dTo3d(cTransform * vec4(0, 0, 0, 1));
	mv = m_cTransform * m_oTransform;
	mvp = m_projection * mv;
	objectsProgram.Activate();
	objectsProgram.SetUniformParameter(camera_position, "cameraPosition");
}

void Renderer::SetProjection(const mat4 & projection)
{
	m_projection = projection;
	mvp = m_projection * m_cTransform * m_oTransform;
}

void Renderer::SetObjectMatrices(const mat4 & oTransform, const mat3 & nTransform)
{
	m_oTransform = oTransform;
	m_nTransform = nTransform;
	mv = m_cTransform * m_oTransform;
	mvp = m_projection * mv;
}

void Renderer::SetDemoBuffer()
{
	basicProgram.SetUniformParameter(vec3(1, 0, 1), "color");

	static const GLfloat g_vertex_buffer_data[] = {
		-1.0f, -1.0f, 0.0f,
		1.0f, -1.0f, 0.0f,
		0.0f,  1.0f, 0.0f,
	};

	GLuint vertexbuffer;
	glGenBuffers(1, &vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);

	// first attribute buffer : vertices
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glVertexAttribPointer(
		0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
		3,                  // size
		GL_FLOAT,           // type
		GL_FALSE,           // dont normalized
		0,                  // stride
		(void*)0            // array buffer offset
	);

	glDrawArrays(GL_TRIANGLES, 0, 3); // 3 indices starting at 0 -> 1 triangle
	glDisableVertexAttribArray(0);
}

void Renderer::SwitchWire()
{
	is_wire_mode = !is_wire_mode;
}

void Renderer::SetAntiAliasing(int new_factor)
{
	if (new_factor < 1) return;

	//if (new_factor == 1) {
	//	glDisable(GL_MULTISAMPLE_BIT);
	//} else {
	//	glEnable(GL_MULTISAMPLE_BIT);
	//}
	anti_factor = new_factor;
	UpdateBuffers(m_screen_width, m_screen_height);
}

void Renderer::SetBaseShader(Renderer::ShaderType s) {
	shader = s;
	objectsProgram.Activate();
	objectsProgram.SetUniformParameter(int(s == ShaderType::Flat), "isFlat");
	objectsProgram.SetUniformParameter(int(s == ShaderType::Gouraud), "isGouraud");
	objectsProgram.SetUniformParameter(int(s == ShaderType::Phong), "isPhong");
}

void Renderer::SetFog(const vec3& color, const float extinction, const float scattering)
{
	objectsProgram.Activate();
	objectsProgram.SetUniformParameter(int(true), "hasFog");
	objectsProgram.SetUniformParameter(color, "fogColor");
	objectsProgram.SetUniformParameter(extinction, "extinctionCoefficient");
	objectsProgram.SetUniformParameter(scattering, "inScatteringCoefficient");
}

void Renderer::DisableFog()
{
	objectsProgram.Activate();
	objectsProgram.SetUniformParameter(int(false), "hasFog");
}

/////////////////////////////////////////////////////
//OpenGL stuff. Don't touch.

void Renderer::InitOpenGLRendering()
{
	int a = glGetError();
	glGenVertexArrays(1, &gScreenVtc);
	glBindVertexArray(gScreenVtc);
	glGenTextures(1, &gScreenTex);
	a = glGetError();

	// Create and compile our GLSL program from the shaders
	basicProgram = ShaderProgram("vshader_basic.glsl", "fshader_basic.glsl", 1);
	objectsProgram = ShaderProgram("vshader_texture.glsl", "fshader_texture.glsl", 10);
	normalsProgram = ShaderProgram("vshader_normal.glsl", "fshader_normal.glsl", 3);
	objectsProgram.Activate();
	SetBaseShader(ShaderType::Flat);
	DisableFog();

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glEnable(GL_CULL_FACE);
}

void Renderer::CreateOpenGLBuffer()
{
	int view_size = min_size * 2;
	//glActiveTexture(GL_TEXTURE0);
	//glBindTexture(GL_TEXTURE_2D, gScreenTex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, view_size, view_size, 0, GL_RGB, GL_FLOAT, NULL);
	glViewport((m_screen_width - view_size) / 2, (m_screen_height - view_size) / 2, view_size, view_size);
}

void Renderer::SwapBuffers()
{
	//FillAntiAliasingBuffer();
	int a = glGetError();
	//glActiveTexture(GL_TEXTURE0);
	//glBindTexture(GL_TEXTURE_2D, gScreenTex);
	//a = glGetError();
	//glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_screen_width, m_screen_height, GL_RGB, GL_FLOAT, m_screenBuffer);
	//glGenerateMipmap(GL_TEXTURE_2D);
	//a = glGetError();

	//glBindVertexArray(gScreenVtc);
	//a = glGetError();
	//glDrawArrays(GL_TRIANGLES, 0, 6);
	//a = glGetError();
	glutSwapBuffers();
	a = glGetError();
}

void Renderer::ClearColorBuffer()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}