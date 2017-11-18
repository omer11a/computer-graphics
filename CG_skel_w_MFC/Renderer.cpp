#include "StdAfx.h"
#include "Renderer.h"
#include "CG_skel_w_MFC.h"
#include "InitShader.h"
#include "GL\freeglut.h"

#define INDEX(width,x,y,c) (x+y*width)*3+c

Renderer::Renderer() : m_width(512), m_height(512), m_zbuffer(NULL),
	m_cTransform(), m_projection(), m_oTransform(), m_nTransform()
{
	InitOpenGLRendering();
	CreateBuffers(512, 512);
}
Renderer::Renderer(int width, int height) : m_width(width), m_height(height), m_zbuffer(NULL),
	m_cTransform(), m_projection(), m_oTransform(), m_nTransform()
{
	InitOpenGLRendering();
	CreateBuffers(width, height);
}

Renderer::~Renderer(void)
{
	DestroyBuffers();
}

void Renderer::CreateBuffers(int width, int height)
{
	m_width = width;
	m_height = height;
	CreateOpenGLBuffer(); //Do not remove this line.
	m_outBuffer = new float[3 * m_width*m_height];
}


void Renderer::DestroyBuffers()
{
	delete[] m_outBuffer;
	if (m_zbuffer != NULL) {
		delete[] m_zbuffer;
	}
}

vec3 Renderer::PointToScreen(const vec3& p, const bool is_normal) const
{
	vec4 result;
	float min_size = min(m_height, m_width) * 0.5;
	if (is_normal) {
		result = m_projection * m_cTransform * (m_nTransform * p);
	} else {
		result = m_projection * m_cTransform * m_oTransform * p;
	}
	//return vec3(round(0.5 * this->m_width * (result.x + 1)), round(0.5 * m_height * (result.y + 1)), result.z);
	return vec3(round(m_width * 0.5 + min_size * (result.x)), m_height - round(m_height * 0.5 + min_size * (result.y)), result.z);
}

void Renderer::PlotPixel(const int x, const int y, const vec3& color)
{
	if ((x >= m_width) || (x < 0) || (y >= m_height) || (y < 0)) {
		return;
	}

	m_outBuffer[INDEX(m_width, x, y, 0)] = color.x;
	m_outBuffer[INDEX(m_width, x, y, 1)] = color.y;
	m_outBuffer[INDEX(m_width, x, y, 2)] = color.z;
}

vec3 Renderer::GetCenterMass(const vec3& p1, const vec3& p2, const vec3& p3) const {
	return vec3((p1.x + p2.x + p3.x) / 3, (p1.y + p2.y + p3.y) / 3, (p1.z + p2.z + p3.z) / 3);
}

void Renderer::DrawLine(const vec3& p1, const vec3& p2, const vec3& color)
{
	if (abs(p1.y - p2.y) > abs(p1.x - p2.x)) {
		this->DrawSteepLine(p1, p2, color);
	} else {
		this->DrawModerateLine(p1, p2, color);
	}
}

void Renderer::DrawSteepLine(const vec3& p1, const vec3& p2, const vec3& color) {
	vec3 start, end;

	if (p1.y > p2.y) {
		start = p2;
		end = p1;
	} else {
		start = p1;
		end = p2;
	}

	int x = start.x;
	int dx = abs(end.x - start.x);
	int dy = end.y - start.y;
	int de = 2 * dx;
	int d = 2 * dx - dy;
	int dne = 2 * dx - 2 * dy;
	int x_change = (end.x > start.x) ? 1 : -1;

	PlotPixel(x, start.y, color);
	for (int y = start.y; y < end.y; ++y) {
		if (d < 0) {
			d += de;
		}
		else {
			x += x_change;
			d += dne;
		}
		PlotPixel(x, y, color);
	}
}

void Renderer::DrawModerateLine(const vec3& p1, const vec3& p2, const vec3& color) {
	vec3 start, end;
	if (p1.x > p2.x) {
		start = p2;
		end = p1;
	}
	else {
		start = p1;
		end = p2;
	}

	int y = start.y;
	int dx = end.x - start.x;
	int dy = abs(end.y - start.y);
	int de = 2 * dy;
	int d = 2 * dy - dx;
	int dne = 2 * dy - 2 * dx;
	int y_change = (end.y > start.y) ? 1 : -1;

	PlotPixel(start.x, y, color);
	for (int x = start.x; x < end.x; ++x) {
		if (d < 0) {
			d += de;
		}
		else {
			y += y_change;
			d += dne;
		}
		PlotPixel(x, y, color);
	}
}

void Renderer::DrawTriangles(const vector<vec3>* vertices, const vector<vec3>* normals, const vector<vec3>* faceNormals) {
	vec3 white(1, 1, 1);
	
	// assuming that vertices size is a multiplication of 3

	auto i = vertices->begin();
	while (i != vertices->end()) {
		vec3 a = PointToScreen(*(i++));
		vec3 b = PointToScreen(*(i++));
		vec3 c = PointToScreen(*(i++));

		/*
		per p, do:
		projection * cTrans * oTrans{/nTrans} * p;
		*/

		this->DrawLine(a, b, white);
		this->DrawLine(b, c, white);
		this->DrawLine(c, a, white);
	}
}

void Renderer::DrawSquare(const vec3& p1, const vec3& p2, const vec3& p3, const vec3& p4, const vec3& color)
{
	vec3 screen1 = PointToScreen(p1);
	vec3 screen2 = PointToScreen(p2);
	vec3 screen3 = PointToScreen(p3);
	vec3 screen4 = PointToScreen(p4);

	DrawLine(screen1, screen2, color);
	DrawLine(screen2, screen3, color);
	DrawLine(screen3, screen4, color);
	DrawLine(screen4, screen1, color);
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
	vector<vec3> vertices;
	vertices.push_back(vec3(-0.1f, 0.0f, 20));
	vertices.push_back(vec3(-0.1f, 0.0f, 30));
	vertices.push_back(vec3(0.0f, 0.2f, 0));
	this->DrawTriangles(&vertices);
}

void Renderer::SetCameraTransform(const mat4 & cTransform)
{
	m_cTransform = cTransform;
}

void Renderer::SetProjection(const mat4 & projection)
{
	m_projection = projection;
}

void Renderer::SetObjectMatrices(const mat4 & oTransform, const mat3 & nTransform)
{
	m_oTransform = oTransform;
	m_nTransform = nTransform;
}

void Renderer::SetDemoBuffer()
{
	/*for (int i = 50; i <= 150; i += 10) {
		DrawLine(vec3(100, 100, 0), vec3(i, 150, 0), vec3(1, 0, 0));
	}
	for (int i = 50; i <= 150; i += 10) {
		DrawLine(vec3(100, 100, 0), vec3(150, i, 0), vec3(0, 1, 0));
	}
	for (int i = 50; i <= 150; i += 10) {
		DrawLine(vec3(100, 100, 0), vec3(i, 50, 0), vec3(0, 0, 1));
	}
	for (int i = 50; i <= 150; i += 10) {
		DrawLine(vec3(100, 100, 0), vec3(50, i, 0), vec3(1, 1, 1));
	}
	;
	vec3 t = this->PointToScreen(vec3(0, 0, 0));

	for (float i = -1.0f; i <= 1; i += 0.1f) {
		vec3 t2 = PointToScreen(vec3(i, 1, 0));
		DrawLine(t, t2, vec3(0, abs(i), abs(i*i)));
	}*/

	vector<vec3> vertices;
	vertices.push_back(vec3(0.2f, 0.2f, 20));
	vertices.push_back(vec3(0.5f, 0.6f, 30));
	vertices.push_back(vec3(0.8f, 0.7f, 0));

	vertices.push_back(vec3(-0.05f, -0.4f, 0));
	vertices.push_back(vec3(-0.15f, -0.4f, 0));
	vertices.push_back(vec3(-0.1f, -0.3f, 0));

	vertices.push_back(vec3(-0.05f, -0.32f, 0));
	vertices.push_back(vec3(-0.15f, -0.32f, 0));
	vertices.push_back(vec3(-0.1f, -0.42f, 0));
	
	vertices.push_back(vec3(-1.0f, -1.0f, 0));
	vertices.push_back(vec3(-1.0f, 1.0f, 0));
	vertices.push_back(vec3(1.0f, -1.0f, 0));
	
	vertices.push_back(vec3(1.0f, 1.0f, 0));
	vertices.push_back(vec3(-1.0f, 1.0f, 0));
	vertices.push_back(vec3(1.0f, -1.0f, 0));
	this->DrawTriangles(&vertices);

	//DrawBox(vec3(1), vec3(3));

	//vertical line
	for (int i = 0; i < m_height; i++) {

		m_outBuffer[INDEX(m_width, m_width / 2, i, 0)] = 1;
		m_outBuffer[INDEX(m_width, m_width / 2, i, 1)] = 0;
		m_outBuffer[INDEX(m_width, m_width / 2, i, 2)] = 0;
	}
	//horizontal line
	for (int i = 0; i < m_width; i++) {
		m_outBuffer[INDEX(m_width, i, 256, 0)] = 1;
		m_outBuffer[INDEX(m_width, i, 256, 1)] = 0;
		m_outBuffer[INDEX(m_width, i, 256, 2)] = 1;
	}
}





/////////////////////////////////////////////////////
//OpenGL stuff. Don't touch.

void Renderer::InitOpenGLRendering()
{
	int a = glGetError();
	a = glGetError();
	glGenTextures(1, &gScreenTex);
	a = glGetError();
	glGenVertexArrays(1, &gScreenVtc);
	GLuint buffer;
	glBindVertexArray(gScreenVtc);
	glGenBuffers(1, &buffer);
	const GLfloat vtc[] = {
		-1, -1,
		1, -1,
		-1, 1,
		-1, 1,
		1, -1,
		1, 1
	};
	const GLfloat tex[] = {
		0,0,
		1,0,
		0,1,
		0,1,
		1,0,
		1,1 };
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vtc) + sizeof(tex), NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vtc), vtc);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(vtc), sizeof(tex), tex);

	GLuint program = InitShader("vshader.glsl", "fshader.glsl");
	glUseProgram(program);
	GLint  vPosition = glGetAttribLocation(program, "vPosition");

	glEnableVertexAttribArray(vPosition);
	glVertexAttribPointer(vPosition, 2, GL_FLOAT, GL_FALSE, 0,
		0);

	GLint  vTexCoord = glGetAttribLocation(program, "vTexCoord");
	glEnableVertexAttribArray(vTexCoord);
	glVertexAttribPointer(vTexCoord, 2, GL_FLOAT, GL_FALSE, 0,
		(GLvoid *) sizeof(vtc));
	glProgramUniform1i(program, glGetUniformLocation(program, "texture"), 0);
	a = glGetError();
}

void Renderer::CreateOpenGLBuffer()
{
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gScreenTex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, m_width, m_height, 0, GL_RGB, GL_FLOAT, NULL);
	glViewport(0, 0, m_width, m_height);
}

void Renderer::UpdateBuffers(int width, int height)
{
	DestroyBuffers();
	CreateBuffers(width, height);
}

void Renderer::SwapBuffers()
{

	int a = glGetError();
	glActiveTexture(GL_TEXTURE0);
	a = glGetError();
	glBindTexture(GL_TEXTURE_2D, gScreenTex);
	a = glGetError();
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_width, m_height, GL_RGB, GL_FLOAT, m_outBuffer);
	glGenerateMipmap(GL_TEXTURE_2D);
	a = glGetError();

	glBindVertexArray(gScreenVtc);
	a = glGetError();
	glDrawArrays(GL_TRIANGLES, 0, 6);
	a = glGetError();
	glutSwapBuffers();
	a = glGetError();
}

void Renderer::ClearColorBuffer()
{
	vec3 black(0);

	for (int x = 0; x < m_width; ++x) {
		for (int y = 0; y < m_height; ++y) {
			PlotPixel(x, y, black);
		}
	}
}

void Renderer::ClearDepthBuffer()
{
	if (m_zbuffer != NULL) {
		for (int x = 0; x < m_width; ++x) {
			for (int y = 0; y < m_height; ++y) {
				m_zbuffer[m_width * y + x] = -INFINITY;
			}
		}
	}
}
