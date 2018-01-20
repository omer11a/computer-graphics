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
m_cTransform(1), m_projection(1), m_oTransform(1), m_nTransform(1), m_cnTransform(1), is_wire_mode(false), mv(1), mvp(1), vp(1)
{
	InitOpenGLRendering();
	anti_factor = false;
	UpdateBuffers(512, 512);
}
Renderer::Renderer(int width, int height) : BaseRenderer(width, height),
m_cTransform(1), m_projection(1), m_oTransform(1), m_nTransform(1), m_cnTransform(1), is_wire_mode(false), mv(1), mvp(1), vp(1)
{
	InitOpenGLRendering();
	anti_factor = false;
	UpdateBuffers(width, height);
}

Renderer::~Renderer(void)
{
}

void Renderer::UpdateBuffers(int width, int height)
{
	m_width = (anti_factor ? 2 : 1) * width;
	m_height = (anti_factor ? 2 : 1) * height;
	m_screen_width = width;
	m_screen_height = height;
	min_size = min(m_width, m_height) / 2;
	CreateOpenGLBuffer(); //Do not remove this line.
}

void Renderer::DrawToonShadow(
	const vector<vec3>* vertices, 
	const vector<vec3>* vertexNormals,
	const float silhouetteThickness,
	const vec3& silhouetteColor)
{
	if (is_wire_mode) {
		return;
	}
	toonProgram.Activate();
	glCullFace(GL_FRONT);

	toonProgram.SetUniformParameter(m_oTransform, "modelMatrix");
	toonProgram.SetUniformParameter(m_nTransform, "normalMatrix");
	toonProgram.SetUniformParameter(vp, "viewProjectionMatrix");
	toonProgram.SetUniformParameter(silhouetteThickness, "silhouetteThickness");
	toonProgram.SetUniformParameter(silhouetteColor, "color");

	GLuint v_buffer = toonProgram.SetInParameter(*vertices, 0, 3);				//in vec3 vertexPosition;
	GLuint n_buffer = toonProgram.SetInParameter(*vertexNormals, 1, 3);			//in vec3 vertexNormal;
	
	glDrawArrays(GL_TRIANGLES, 0, vertices->size());
	// cleanup
	toonProgram.ClearAttributes();
	glDeleteBuffers(1, &v_buffer);
	glDeleteBuffers(1, &n_buffer);
	glCullFace(GL_BACK);
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
	const vector<vec3>* tangents,
	const bool hasColorAnimation,
	const int colorAnimationRepresentation,
	const float colorAnimationDelta,
	const bool hasVertexAnimation,
	const float vertexAnimationDelta,
	const bool hasToonShading,
	const int colorQuantizationCoefficient,
	const bool hasWoodTexture,
	const vec3& woodTextureColor1,
	const vec3& woodTextureColor2,
	const vec2& modelResolution,
	const vector<vec3>* vertexNormals,
	const vector<vec3>* faceNormals)
{
	// Use object shader
	objectsProgram.Activate();

	// uniform parameters
	objectsProgram.SetUniformParameter(int(hasTexture), "hasTexture");
	if (hasTexture) {
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, textureID);
		objectsProgram.SetUniformParameter(0, "textureSampler");
	}
	objectsProgram.SetUniformParameter(int(hasNormalMap), "hasNormalMapping");
	if (hasNormalMap) {
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, normalMapID);
		objectsProgram.SetUniformParameter(1, "normalSampler");
	}
	objectsProgram.SetUniformParameter(int(hasColorAnimation), "hasColorAnimation");
	if (hasColorAnimation) {
		objectsProgram.SetUniformParameter(colorAnimationRepresentation, "colorAnimationRepresentation");
		objectsProgram.SetUniformParameter(colorAnimationDelta, "colorAnimationDelta");
	}
	objectsProgram.SetUniformParameter(int(hasVertexAnimation), "hasVertexAnimation");
	if (hasVertexAnimation) {
		objectsProgram.SetUniformParameter(vertexAnimationDelta, "vertexAnimationDelta");
	}
	if (!is_wire_mode) {
		objectsProgram.SetUniformParameter(int(hasToonShading), "hasToonShading");
		if (hasToonShading) {
			objectsProgram.SetUniformParameter(colorQuantizationCoefficient, "colorQuantizationCoefficient");
		}
	} else {
		objectsProgram.SetUniformParameter(int(false), "hasToonShading");
	}
	objectsProgram.SetUniformParameter(int(hasWoodTexture), "hasWoodTexture");
	if (hasWoodTexture) {
		objectsProgram.SetUniformParameter(woodTextureColor1, "woodTextureColor1");
		objectsProgram.SetUniformParameter(woodTextureColor2, "woodTextureColor2");
		objectsProgram.SetUniformParameter(modelResolution, "modelResolution");
	}

	objectsProgram.SetUniformParameter(m_oTransform, "modelMatrix");
	objectsProgram.SetUniformParameter(m_nTransform, "normalMatrix");
	objectsProgram.SetUniformParameter(m_cTransform, "viewMatrix");
	objectsProgram.SetUniformParameter(vp, "viewProjectionMatrix");

	// if there are no normals, use the flat shader
	ShaderType temp = shader;
	if ((vertexNormals == NULL) || (vertexNormals->size() == 0)) {
		SetBaseShader(ShaderType::Flat);
	} else if ((hasVertexAnimation) && (shader == ShaderType::Flat)) {
		// flat shader can't be used during vertex animation
		SetBaseShader(ShaderType::Gouraud);
	}

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
	if ((vertexNormals != NULL) && (vertexNormals->size() != 0)) {
		buffers.push_back(objectsProgram.SetInParameter(*vertexNormals, 2, 3));			//in vec3 vertexNormal;
	}
	buffers.push_back(objectsProgram.SetInParameter(*faceNormals, 3, 3));			//in vec3 faceNormal;
	buffers.push_back(objectsProgram.SetInParameter(ambients, 4, 3));				//in vec3 ambientReflectance;
	buffers.push_back(objectsProgram.SetInParameter(speculars, 5, 3));				//in vec3 specularReflectance;
	buffers.push_back(objectsProgram.SetInParameter(diffuses, 6, 3));				//in vec3 diffuseReflectance;
	buffers.push_back(objectsProgram.SetInParameter(shininess, 7, 1));				//in float shininess;
	
	if (hasTexture) {
		buffers.push_back(objectsProgram.SetInParameter(*textureCoordinates, 8, 2));	//in vec2 uv;
		buffers.push_back(objectsProgram.SetInParameter(*textureCenters, 9, 2));		//in vec2 centerUv;
	}
	if (hasNormalMap) {
		buffers.push_back(objectsProgram.SetInParameter(*tangents, 10, 3));		//in vec3 tangent;
	}
	
	// Draw the triangle !
	glDrawArrays(GL_TRIANGLES, 0, vertices->size());

	// cleanup
	objectsProgram.ClearAttributes();
	for (int i = 0; i < buffers.size(); ++i) {
		glDeleteBuffers(1, &buffers[i]);
	}

	if (temp != shader) {
		SetBaseShader(temp);
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
	//DrawLine(p1, p2);
	//DrawLine(p2, p3);
	//DrawLine(p3, p4);
	//DrawLine(p4, p1);
}

void Renderer::DrawBox(const vec3& minValues, const vec3& maxValues)
{
	vec3 color(0.9f);
	mat4 mvp = m_projection * m_cTransform * m_oTransform;
	vector<vec3> vertices;
	vertices.push_back(convert4dTo3d(mvp * vec4(minValues.x, minValues.y, minValues.z, 1))); // 1
	vertices.push_back(convert4dTo3d(mvp * vec4(maxValues.x, minValues.y, minValues.z, 1))); // 2
	vertices.push_back(convert4dTo3d(mvp * vec4(maxValues.x, maxValues.y, minValues.z, 1))); // 3
	vertices.push_back(convert4dTo3d(mvp * vec4(minValues.x, maxValues.y, minValues.z, 1))); // 4
	vertices.push_back(convert4dTo3d(mvp * vec4(minValues.x, minValues.y, maxValues.z, 1))); // 1
	vertices.push_back(convert4dTo3d(mvp * vec4(maxValues.x, minValues.y, maxValues.z, 1))); // 2
	vertices.push_back(convert4dTo3d(mvp * vec4(maxValues.x, maxValues.y, maxValues.z, 1))); // 3
	vertices.push_back(convert4dTo3d(mvp * vec4(minValues.x, maxValues.y, maxValues.z, 1))); // 4
	vector<vec3> lines;
	// front
	lines.push_back(vertices[0]);
	lines.push_back(vertices[1]);
	lines.push_back(vertices[1]);
	lines.push_back(vertices[2]);
	lines.push_back(vertices[2]);
	lines.push_back(vertices[3]);
	lines.push_back(vertices[3]);
	lines.push_back(vertices[0]);
	// back
	lines.push_back(vertices[4]);
	lines.push_back(vertices[5]);
	lines.push_back(vertices[5]);
	lines.push_back(vertices[6]);
	lines.push_back(vertices[6]);
	lines.push_back(vertices[7]);
	lines.push_back(vertices[7]);
	lines.push_back(vertices[4]);
	// sides
	lines.push_back(vertices[0]);
	lines.push_back(vertices[4]);
	lines.push_back(vertices[1]);
	lines.push_back(vertices[5]);
	lines.push_back(vertices[2]);
	lines.push_back(vertices[6]);
	lines.push_back(vertices[3]);
	lines.push_back(vertices[7]);

	// Use our shader
	basicProgram.Activate();
	basicProgram.SetUniformParameter(color, "color");
	GLuint buffer = basicProgram.SetInParameter(lines, 0, 3);			//in vec3 vertexPosition;
	glDrawArrays(GL_LINES, 0, lines.size());
	basicProgram.ClearAttributes();
	glDeleteBuffers(1, &buffer);
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
	GLfloat offset = 5.0f * (anti_factor ? 2 : 1) / min_size;
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
	GLfloat offset = 5.0f * (anti_factor ? 2 : 1) / min_size;
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
	vp = m_projection * m_cTransform;
	mvp = m_projection * mv;
	objectsProgram.Activate();
	objectsProgram.SetUniformParameter(camera_position, "cameraPosition");
}

void Renderer::SetProjection(const mat4 & projection)
{
	m_projection = projection;
	vp = m_projection * m_cTransform;
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
	if (is_wire_mode) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	} else {
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
}

void Renderer::SetAntiAliasing()
{
	anti_factor = !anti_factor;
	if (anti_factor) {
		glEnable(GL_MULTISAMPLE_BIT);
	} else {
		glDisable(GL_MULTISAMPLE_BIT);
	}
	//UpdateBuffers(m_screen_width, m_screen_height);
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
	glGenTextures(1, &gScreenTex);
	glGenVertexArrays(1, &gScreenVtc);
	glBindVertexArray(gScreenVtc);
	a = glGetError();
	
	// Create and compile our GLSL program from the shaders
	basicProgram = ShaderProgram("vshader_basic.glsl", "fshader_basic.glsl", 1);
	objectsProgram = ShaderProgram("vshader_texture.glsl", "fshader_texture.glsl", 10);
	normalsProgram = ShaderProgram("vshader_normal.glsl", "fshader_normal.glsl", 3);
	toonProgram = ShaderProgram("vshader_silhouette.glsl", "fshader_silhouette.glsl", 2);
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
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gScreenTex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, view_size, view_size, 0, GL_RGB, GL_FLOAT, NULL);
	glViewport((m_screen_width - view_size) / 2, (m_screen_height - view_size) / 2, view_size, view_size);
}

void Renderer::SwapBuffers()
{
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