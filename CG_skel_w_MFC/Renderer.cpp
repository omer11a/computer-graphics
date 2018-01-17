
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

#define INDEX(width,x,y,c) ((x)+(y)*(width))*3+(c)

Renderer::Renderer() : BaseRenderer(512, 512), m_zBuffer(NULL),
m_cTransform(1), m_projection(1), m_oTransform(1), m_nTransform(1), m_cnTransform(1), is_wire_mode(false)
{
	InitOpenGLRendering();
	anti_factor = 1;
	CreateBuffers(512, 512);
}
Renderer::Renderer(int width, int height) : BaseRenderer(width, height), m_zBuffer(NULL),
m_cTransform(1), m_projection(1), m_oTransform(1), m_nTransform(1), m_cnTransform(1), is_wire_mode(false)
{
	InitOpenGLRendering();
	anti_factor = 1;
	CreateBuffers(width, height);
}

Renderer::~Renderer(void)
{
	DestroyBuffers();
}

void Renderer::CreateBuffers(int width, int height)
{
	m_width = anti_factor * width;
	m_height = anti_factor * height;
	m_screen_width = width;
	m_screen_height = height;
	min_size = min(m_width, m_height) / 2;
	CreateOpenGLBuffer(); //Do not remove this line.
	m_outBuffer = new float[3 * m_width * m_height];
	m_screenBuffer = new float[3 * m_screen_width * m_screen_height];
	m_zBuffer = new float[m_width * m_height];
	m_paintBuffer = NULL;
}

void Renderer::CreateLocalBuffer()
{
	if (m_paintBuffer != NULL) {
		delete[] m_paintBuffer;
	}
	m_paintBuffer = new bool[m_width * m_height];
	for (int i = 0; i < m_width * m_height; ++i) {
		m_paintBuffer[i] = false;
	}
}

void Renderer::DestroyBuffers()
{
	delete[] m_outBuffer;
	delete[] m_screenBuffer;
	if (m_zBuffer != NULL) {
		delete[] m_zBuffer;
	}
}

vec4 Renderer::applyCameraTransformation(const vec3& p, const vec3& n) const {
	// TODO: do we need this?!
	//vec4 pTransformed;
	//vec4 nTransformed;

	//pTransformed = m_oTransform * p;
	//if (length(n) != 0) {
	//	nTransformed = normalize(m_nTransform * n);
	//	nTransformed.w = 0;
	//}

	//return m_cTransform * (pTransformed + nTransformed);
	return vec4();
}

vec3 Renderer::applyProjection(const vec4& p) const {
	vec4 result = m_projection * p;
	result = result / result.w;
	return vec3(result.x, result.y, result.z);
}

void Renderer::clip(float x0, float x1, float xmin, float xmax, float& t1, float& t2) const {
	float p1 = x0 - x1;
	float p2 = -p1;
	float q1 = x0 - xmin;
	float q2 = xmax - x0;

	if (p1 == 0) {
		// line is parallel to clipping window
		if ((q1 < 0) || (q2 < 0)) {
			// line is outside of the clipping window
			t1 = 0;
			t2 = 0;
		}

		return;
	}

	float u1 = 0;
	float u2 = 0;
	float r1 = q1 / p1;
	float r2 = q2 / p2;
	if (p1 < 0) {
		u1 = r1;
		u2 = r2;
	} else {
		u1 = r2;
		u2 = r1;
	}

	t1 = max(t1, u1);
	t2 = min(t2, u2);
}

bool Renderer::clipLine(
	const vec3& v1,
	const vec3& n1,
	const vec3& v2,
	const vec3& n2,
	vec3& start,
	vec3& end
) const {
	vec4 p1 = applyCameraTransformation(v1, n1);
	vec4 p2 = applyCameraTransformation(v2, n2);

	float t1 = 0;
	float t2 = 1;
	clip(p1.z, p2.z, -zFar, -zNear, t1, t2);
	if (t1 >= t2) {
		return false;
	}

	vec4 dp = p2 - p1;
	vec3 q1 = applyProjection(p1 + t1 * dp);
	vec3 q2 = applyProjection(p1 + t2 * dp);

	t1 = 0;
	t2 = 1;
	clip(q1.x, q2.x, -1, 1, t1, t2);
	clip(q1.y, q2.y, -1, 1, t1, t2);
	if (t1 >= t2) {
		return false;
	}

	vec3 dq = q2 - q1;
	start = q1 + t1 * dq;
	end = q1 + t2 * dq;
	return true;
}

bool Renderer::clipLine(
	const vec3& v1,
	const vec3& v2,
	vec3& start,
	vec3& end
) const {
	float t1 = 0;
	float t2 = 1;
	clip(v1.x, v2.x, -1, 1, t1, t2);
	clip(v1.y, v2.y, -1, 1, t1, t2);
	clip(v1.z, v2.z, -1, 1, t1, t2);
	if (t1 >= t2) {
		return false;
	}

	vec3 dv = v2 - v1;
	start = v1 + t1 * dv;
	end = v1 + t2 * dv;
	return true;
}

vec3 Renderer::convertToScreen(const vec3& p) const {
	return vec3(round(m_width * 0.5 + min_size * (p.x)), round(m_height * 0.5 + min_size * (p.y)), p.z);
}

bool Renderer::pointToScreen(const vec3& p, const vec3& n, vec3& q, bool should_screen) const
{
	vec4 transformed = applyCameraTransformation(p, n);
	if ((transformed.z > -zNear) || (transformed.z < -zFar)) {
		return false;
	}

	vec3 result = applyProjection(transformed);
	if ((result.x < -1) || (result.x > 1) || (result.y < -1) || (result.y > 1)) {
		return false;
	}

	q = should_screen ? convertToScreen(result) : result;
	return true;
}

bool Renderer::lineToScreen(const vec3& p1, const vec3& n1, const vec3& p2, const vec3& n2, vec3& q1, vec3& q2) const {
	bool shouldDraw = clipLine(p1, n1, p2, n2, q1, q2);
	if (!shouldDraw) {
		return false;
	}

	q1 = convertToScreen(q1);
	q2 = convertToScreen(q2);
	return true;
}

bool Renderer::PlotPixel(const int x, const int y, const float z, const vec3& color)
{
	if ((x >= m_width) || (x < 0) || (y >= m_height) || (y < 0)) {
		return false;
	}

	if (z > m_zBuffer[y * m_width + x]) {
		m_outBuffer[INDEX(m_width, x, y, 0)] = color.x;
		m_outBuffer[INDEX(m_width, x, y, 1)] = color.y;
		m_outBuffer[INDEX(m_width, x, y, 2)] = color.z;
		m_zBuffer[y * m_width + x] = z;
	}
	if (m_paintBuffer != NULL) {
		m_paintBuffer[y * m_width + x] = true;
	}
	return true;
}


bool Renderer::DrawLine(const vec3& p1, const vec3& n1, const vec3& p2, const vec3& n2, const vec3& c1)
{
	vec3 newP1, newP2;
	if (!lineToScreen(p1, n1, p2, n2, newP1, newP2)) {
		std::cout << "skipped drawing lines" << std::endl;
		return false;
	}

	if (abs(newP1.y - newP2.y) > abs(newP1.x - newP2.x)) {
		this->DrawSteepLine(newP1, newP2, c1);
	} else {
		this->DrawModerateLine(newP1, newP2, c1);
	}

	return true;
}

void Renderer::DrawLine(const vec3& p1, const vec3& p2, const vec3& c, const int p1_idx, const int p2_idx)
{
	vec3 q1 = convertToScreen(p1);
	vec3 q2 = convertToScreen(p2);

	if (abs(q1.y - q2.y) > abs(q1.x - q2.x)) {
		this->DrawSteepLine(q1, q2, c, p1_idx, p2_idx);
	} else {
		this->DrawModerateLine(q1, q2, c, p1_idx, p2_idx);
	}
}

void Renderer::DrawSteepLine(const vec3& p1, const vec3& p2, const vec3& c, const int p1_idx, const int p2_idx) {
	
}

void Renderer::DrawModerateLine(const vec3& p1, const vec3& p2, const vec3& c, const int p1_idx, const int p2_idx) {
	
}


bool Renderer::PixelToPoint(const vec3& p1, const vec3& p2, const vec3& p3, const vec3& p, vec3& newP) const {

	float x = (p.x - m_width * 0.5) / min_size;
	float y = (p.y - m_height * 0.5) / min_size;
	vec2 np(x, y);

	mat2 a(np.x - p1.x, p2.x - p3.x,
		np.y - p1.y, p2.y - p3.y);
	if (glm::determinant(a) == 0) {
		return false;
	}
	vec2 tk = inverse(a) * vec2(p2.x - p1.x, p2.y - p1.y);
	// p4 is the intersection of lines: p1<->p, p2<->p3
	vec3 p4 = p2 + tk[1] * (p3 - p2);

	newP = p1 + (p4 - p1) / tk[0];
	newP.x = np.x;
	newP.y = np.y;

	if ((newP.x < -1) || (newP.x > 1) ||
		(newP.y < -1) || (newP.y > 1) ||
		(newP.z < -1) || (newP.z > 1)) {
		return false;
	} else {
		return true;
	}
}


bool GetIntersectionPoint(const vec3& p1, const vec3& p2, const float y, vec3& p) {
	float s = p2.y - p1.y;

	if (s == 0) {
		return false;
	}
	float t = (y - p1.y) / s;
	if ((t < 0) || (t > 1)) {
		return false;
	}

	p = p1 + t * (p2 - p1);
	return true;
}

void Renderer::DrawTriangles(
	const vector<vec3>* vertices,
	const vector<Material>* materials,
	const vector<vec3>* centerPositions,
	const vector<vec3>* vertexNormals,
	const vector<vec3>* faceNormals,
	const bool allowVertexNormals,
	const bool allowFaceNormals)
{
	vec3 white(1);
	int fn_index = 0;
	int vn_index = 0;
	vec3 v_normal, f_normal;

	// Use object shader
	objectsProgram.Activate();

	// uniform parameters
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

	vector<vec3> shader_faceNormals;
	for (auto i = faceNormals->begin(); i != faceNormals->end(); ++i) {
		shader_faceNormals.push_back((*i));
		shader_faceNormals.push_back((*i));
		shader_faceNormals.push_back((*i));
	}

	vector<GLuint> buffers;
	buffers.push_back(objectsProgram.SetInParameter(*vertices, 0));			//in vec3 vertexPosition;
	buffers.push_back(objectsProgram.SetInParameter(*centerPositions, 1));			//in vec3 centerPosition;
	buffers.push_back(objectsProgram.SetInParameter(*vertexNormals, 2));		//in vec3 vertexNormal;
	buffers.push_back(objectsProgram.SetInParameter(shader_faceNormals, 3));		//in vec3 faceNormal;
	buffers.push_back(objectsProgram.SetInParameter(ambients, 4));			//in vec3 ambientReflectance;
	buffers.push_back(objectsProgram.SetInParameter(speculars, 5));			//in vec3 specularReflectance;
	buffers.push_back(objectsProgram.SetInParameter(diffuses, 6));			//in vec3 diffuseReflectance;
	buffers.push_back(objectsProgram.SetInParameter(shininess, 7));			//in float shininess;
	
	// Draw the triangle !
	if (is_wire_mode) {
		glDrawArrays(GL_LINES, 0, vertices->size());
	} else {
		glDrawArrays(GL_TRIANGLES, 0, vertices->size());
	}

	objectsProgram.ClearAttributes();
}

void Renderer::DrawVertexNormals(
	const vector<vec3>* vertices,
	const vector<vec3>* vertexNormals) {
	
	vec3 pink(1, 140 / 255.0f, 1);
	// Use our shader
	normalsProgram.Activate();

	mat4 mvp = m_projection * m_cTransform * m_oTransform;
	//mat4 nvp = m_projection * m_cTransform * m_nTransform;
	vector<vec3> lines;

	//for (int i = 0; i < vertices->size(); ++i) {
	//	lines.push_back(convert4dTo3d(mvp*vec4((*vertices)[i], 1)));
	//	lines.push_back(convert4dTo3d(nvp*vec4((*vertices)[i], 0)));
	//}
	// uniform parameters
	normalsProgram.SetUniformParameter(m_oTransform, "color");

}
void Renderer::DrawFaceNormals(
	const vector<vec3>* vertices,
	const vector<vec3>* faceNormals) {
	vec3 yellow(1, 1, 0);
}

void Renderer::DrawSquare(const vec3& p1, const vec3& p2, const vec3& p3, const vec3& p4, const vec3& color)
{
	DrawLine(p1, vec3(0), p2, vec3(0), color);
	DrawLine(p2, vec3(0), p3, vec3(0), color);
	DrawLine(p3, vec3(0), p4, vec3(0), color);
	DrawLine(p4, vec3(0), p1, vec3(0), color);
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
	vec3 camera_location;
	bool in_sight = pointToScreen(vec3(), vec3(), camera_location);
	if (!in_sight) {
		return;
	}

	for (int i = -5 * anti_factor; i < 5 * anti_factor; ++i) {
		PlotPixel(camera_location.x + i, camera_location.y, camera_location.z, color);
		PlotPixel(camera_location.x, camera_location.y + i, camera_location.z, color);
	}
}

void Renderer::DrawLight(const vec3& color, const vec3& position)
{
	mat4 mvp = m_projection * m_cTransform * m_oTransform;
	vec3 light_location = convert4dTo3d(mvp * vec4(position, 1));

	// Use our shader
	normalsProgram.Activate();
	normalsProgram.SetUniformParameter(color, "color");

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

	GLuint buffer = normalsProgram.SetInParameter(star, 0);			//in vec3 vertexPosition;
	
	glDrawArrays(GL_LINES, 0, star.size());
	
	normalsProgram.ClearAttributes();
	//glDeleteBuffers(1, &buffer);
}

void Renderer::SetShaderLights(const AmbientLight& amb_light, const vector<DirectionalLightSource *>& lights) {
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
	objectsProgram.SetUniformParameter(camera_position, "cameraPosition");
}

void Renderer::SetProjection(const mat4 & projection)
{
	m_projection = projection;
	mvp = m_projection * m_cTransform * m_oTransform;
}

void Renderer::SetZRange(float zNear, float zFar)
{
	this->zNear = zNear;
	this->zFar = zFar;
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
	normalsProgram.SetUniformParameter(vec3(1, 0, 1), "color");

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

	anti_factor = new_factor;
	DestroyBuffers();
	CreateBuffers(m_screen_width, m_screen_height);
}

void Renderer::SetBaseShader(Renderer::ShaderType s) {
	objectsProgram.SetUniformParameter(int(s == ShaderType::Flat), "isFlat");
	objectsProgram.SetUniformParameter(int(s == ShaderType::Gouraud), "isGouraud");
	objectsProgram.SetUniformParameter(int(s == ShaderType::Phong), "isPhong");
}

void Renderer::SetFog(const vec3& color, const float extinction, const float scattering)
{
	objectsProgram.SetUniformParameter(int(true), "isFog");
	objectsProgram.SetUniformParameter(color, "fogColor");
	objectsProgram.SetUniformParameter(extinction, "extinctionCoefficient");
	objectsProgram.SetUniformParameter(scattering, "inScatteringCoefficient");
	//fog_color = color;
	//fog_extinction = extinction;
	//fog_scattering = scattering;
	//has_fog = true;
}

void Renderer::DisableFog()
{
	objectsProgram.SetUniformParameter(int(false), "isFog");
}

/////////////////////////////////////////////////////
//OpenGL stuff. Don't touch.

void Renderer::InitOpenGLRendering()
{
	int a = glGetError();
	//a = glGetError();
	//glGenTextures(1, &gScreenTex);
	//a = glGetError();
	glGenVertexArrays(1, &gScreenVtc);
	glBindVertexArray(gScreenVtc);

	//const GLfloat vtc[] = {
	//	-1, -1,
	//	1, -1,
	//	-1, 1,
	//	-1, 1,
	//	1, -1,
	//	1, 1
	//};
	//const GLfloat tex[] = {
	//	0,0,
	//	1,0,
	//	0,1,
	//	0,1,
	//	1,0,
	//	1,1 };
	//
	//GLuint buffer;
	//glGenBuffers(1, &buffer);
	//glBindBuffer(GL_ARRAY_BUFFER, buffer);
	//glBufferData(GL_ARRAY_BUFFER, sizeof(vtc) + sizeof(tex), NULL, GL_STATIC_DRAW);

	//glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vtc), vtc);
	//glBufferSubData(GL_ARRAY_BUFFER, sizeof(vtc), sizeof(tex), tex);

	//GLuint program = InitShader("vshader.glsl", "fshader.glsl");
	//glUseProgram(program);
	//GLint  vPosition = glGetAttribLocation(program, "vPosition");

	//glEnableVertexAttribArray(vPosition);
	//glVertexAttribPointer(vPosition, 2, GL_FLOAT, GL_FALSE, 0,
	//	0);

	//GLint  vTexCoord = glGetAttribLocation(program, "vTexCoord");
	//glEnableVertexAttribArray(vTexCoord);
	//glVertexAttribPointer(vTexCoord, 2, GL_FLOAT, GL_FALSE, 0,
	//	(GLvoid *) sizeof(vtc));
	//glProgramUniform1i(program, glGetUniformLocation(program, "texture"), 0);
	//a = glGetError();

	// shai's code
	// Create and compile our GLSL program from the shaders
	normalsProgram = ShaderProgram("vshader_basic.glsl", "fshader_basic.glsl", 1);
	objectsProgram = ShaderProgram("vshader_fog.glsl", "fshader_fog.glsl", 8);
	SetBaseShader(ShaderType::Flat);
	DisableFog();

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glEnable(GL_CULL_FACE);
}

void Renderer::CreateOpenGLBuffer()
{
	//glActiveTexture(GL_TEXTURE0);
	//glBindTexture(GL_TEXTURE_2D, gScreenTex);
	//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, m_screen_width, m_screen_height, 0, GL_RGB, GL_FLOAT, NULL);
	//glViewport(0, 0, m_screen_width, m_screen_height);
}

void Renderer::UpdateBuffers(int width, int height)
{
	DestroyBuffers();
	CreateBuffers(width, height);
}

void Renderer::SwapBuffers()
{
	//FillAntiAliasingBuffer();
	int a = glGetError();
	//glActiveTexture(GL_TEXTURE0);
	//a = glGetError();
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
	//glClear(GL_COLOR_BUFFER_BIT);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Renderer::ClearDepthBuffer()
{
	if (m_zBuffer != NULL) {
		for (int x = 0; x < m_width; ++x) {
			for (int y = 0; y < m_height; ++y) {
				m_zBuffer[m_width * y + x] = -INFINITY;//can be zFar
			}
		}
	}
}