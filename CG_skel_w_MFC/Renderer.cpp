#include "StdAfx.h"
#include "Renderer.h"
#include "CG_skel_w_MFC.h"
#include "InitShader.h"
#include "GL\freeglut.h"

#define INDEX(width,x,y,c) (x+y*width)*3+c

Renderer::Renderer() : m_width(512), m_height(512), m_zbuffer(NULL),
m_cTransform(), m_projection(), m_oTransform(), m_nTransform(), m_camera_multiply()
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

vec4 Renderer::TransformPoint(const vec3& p, const vec3& n) const
{
	vec4 pTransformed;
	vec4 nTransformed;
	float min_size = min(m_height, m_width) * 0.5;

	pTransformed = m_camera_multiply * m_oTransform * p;
	if (length(n) != 0) {
		nTransformed = m_camera_multiply * normalize(m_nTransform * n);
	}

	vec4 result = pTransformed + nTransformed;
	return result;
	//return vec3(result.x / result.w, result.y / result.w, result.z / result.w);
	//return vec3(result.x, result.y, result.z);
}

vec3 Renderer::PointToScreen(vec4 p) const
{
	float min_size = min(m_height, m_width) * 0.5;
	return vec3(round(m_width * 0.5 + min_size * (p.x / p.w)),
		m_height - round(m_height * 0.5 + min_size * (p.y / p.w)),
		p.z / p.w);
}

void Renderer::PlotPixel(const int x, const int y, const vec3& color)
{
	int min_size = min(m_height, m_width);
	int validation_x = x - (m_width / 2);
	int validation_y = y + (m_width / 2);
	//if ((validation_x >= min_size) || (validation_x < 0) || (validation_y >= min_size) || (validation_y < 0) || ()) {
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

void Renderer::DrawLine(const vec4& p1, const vec4& p2, const vec3& color)
{
	float dx = p2.x - p1.x;
	float dy = p2.y - p1.y;
	float dz = p2.z - p1.z;
	float dw = p2.w - p1.w;
	float lower_t[3], upper_t[3];
	int l_i = 0, u_i = 0;

	if (dx == 0) {
		// in case of entire line is out side of boundry
		if ((p1.x > 1) || (p1.x < -1)) return;
	} else {
		lower_t[l_i++] = min((-1 - p1.x) / dx, (1 - p1.x) / dx);
		upper_t[u_i++] = max((-1 - p1.x) / dx, (1 - p1.x) / dx);
	}
	if (dy == 0) {
		if ((p1.y > 1) || (p1.y < -1)) return;
	} else {
		lower_t[l_i++] = min((-1 - p1.y) / dy, (1 - p1.y) / dy);
		upper_t[u_i++] = max((-1 - p1.y) / dy, (1 - p1.y) / dy);
	}
	if (dz == 0) {
		if ((p1.z > 1) || (p1.z < -1)) return;
	} else {
		lower_t[l_i++] = min((-1 - p1.z) / dz, (1 - p1.z) / dz);
		upper_t[u_i++] = max((-1 - p1.z) / dz, (1 - p1.z) / dz);
	}

	float max_lower = lower_t[0];
	for (int i = 1; i < l_i; ++i) {
		max_lower = max(max_lower, lower_t[i]);
	}
	float min_upper = upper_t[0];
	for (int i = 1; i < l_i; ++i) {
		min_upper = max(min_upper, upper_t[i]);
	}

	vec4 delta(dx, dy, dz, dw);
	vec4 new_p1 = ((max_lower < 1) && (max_lower > 0)) ? p1 + max_lower * delta : p1;
	vec4 new_p2 = ((min_upper < 1) && (min_upper > 0)) ? p1 + min_upper * delta : p2;

	if (max_lower <= min_upper) {
		if (abs(dy) > abs(dx)) {
			DrawSteepLine(PointToScreen(new_p1), PointToScreen(new_p2), color);
			//DrawSteepLine(PointToScreen(p1), PointToScreen(p2), color);
		} else {
			DrawModerateLine(PointToScreen(new_p1), PointToScreen(new_p2), color);
			//DrawModerateLine(PointToScreen(p1), PointToScreen(p2), color);
		}
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
		} else {
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
	} else {
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
		} else {
			y += y_change;
			d += dne;
		}
		PlotPixel(x, y, color);
	}
}

void Renderer::DrawTriangles(const vector<vec3>* vertices, const vector<vec3>* vertexNormals, const vector<vec3>* faceNormals) {
	vec3 white(1);
	vec3 yellow(1, 1, 0);
	vec3 pink(1, 140 / 255., 1);
	int fn_index = 0;
	int vn_index = 0;
	vec3 v_normal, f_normal;

	// assuming that vertices size is a multiplication of 3
	auto i = vertices->begin();
	while (i != vertices->end()) {
		vec3 a = *(i++);
		vec3 b = *(i++);
		vec3 c = *(i++);
		vec3 cm = GetCenterMass(a, b, c);
		DrawLine(TransformPoint(a), TransformPoint(b), white);
		DrawLine(TransformPoint(b), TransformPoint(c), white);
		DrawLine(TransformPoint(c), TransformPoint(a), white);

		if (faceNormals != NULL) {
			f_normal = faceNormals->at(fn_index);
			DrawLine(TransformPoint(cm), TransformPoint(cm, f_normal), pink);
			++fn_index;
		}

		if (vertexNormals != NULL) {
			v_normal = vertexNormals->at(vn_index++);
			DrawLine(TransformPoint(a), TransformPoint(a, v_normal), yellow);
			v_normal = vertexNormals->at(vn_index++);
			DrawLine(TransformPoint(b), TransformPoint(b, v_normal), yellow);
			v_normal = vertexNormals->at(vn_index++);
			DrawLine(TransformPoint(c), TransformPoint(c, v_normal), yellow);
		}
	}
}

void Renderer::DrawSquare(const vec3& p1, const vec3& p2, const vec3& p3, const vec3& p4, const vec3& color)
{
	vec4 screen1 = TransformPoint(p1);
	vec4 screen2 = TransformPoint(p2);
	vec4 screen3 = TransformPoint(p3);
	vec4 screen4 = TransformPoint(p4);

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
	//vec3 color(1, 140 / 255, 1);
	//vec4 camera_location = TransformPoint(vec3());
	//vector<vec4> vertices;
	//vertices.push_back(camera_location + vec4(-10, 0.0f, 20,0));
	//vertices.push_back(camera_location + vec4(10, 0, 30));
	//vertices.push_back(camera_location + vec4(0, 25, 0));

	//DrawLine(vertices[0], vertices[1], color);
	//DrawLine(vertices[1], vertices[2], color);
	//DrawLine(vertices[2], vertices[0], color);
}

void Renderer::SetCameraTransform(const mat4 & cTransform)
{
	m_cTransform = cTransform;
	m_camera_multiply = m_projection * m_cTransform;
}

void Renderer::SetProjection(const mat4 & projection)
{
	m_projection = projection;
	m_camera_multiply = m_projection * m_cTransform;
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
	vec3 t = this->TransformPoint(vec3(0, 0, 0));

	for (float i = -1.0f; i <= 1; i += 0.1f) {
	vec3 t2 = TransformPoint(vec3(i, 1, 0));
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
