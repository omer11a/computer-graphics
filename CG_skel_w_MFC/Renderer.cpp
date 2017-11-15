#include "StdAfx.h"
#include "Renderer.h"
#include "CG_skel_w_MFC.h"
#include "InitShader.h"
#include "GL\freeglut.h"

#define INDEX(width,x,y,c) (x+y*width)*3+c

Renderer::Renderer() :m_width(512), m_height(512)
{
	InitOpenGLRendering();
	CreateBuffers(512, 512);
}
Renderer::Renderer(int width, int height) : m_width(width), m_height(height)
{
	InitOpenGLRendering();
	CreateBuffers(width, height);
}

Renderer::~Renderer(void)
{
	delete[] m_outBuffer;
}



void Renderer::CreateBuffers(int width, int height)
{
	m_width = width;
	m_height = height;
	CreateOpenGLBuffer(); //Do not remove this line.
	m_outBuffer = new float[3 * m_width*m_height];
}

void Renderer::PlotPixel(const int x, const int y, const vec3 color)
{
	m_outBuffer[INDEX(m_width, x, y, 0)] = color.x;
	m_outBuffer[INDEX(m_width, x, y, 1)] = color.y;
	m_outBuffer[INDEX(m_width, x, y, 2)] = color.z;
}
void Renderer::DrawLine(const vec3 p1, const vec3 p2, const vec3 color)
{
	float dx = p1.x - p2.x;
	float dy = p1.y - p2.y;
	float m = dy / dx;

	if ((m >= 0) && (m <= 1)) {
		this->DrawLineShape1(p1, p2, color);
	}
	else if (m > 1) {
		this->DrawLineShape2(p1, p2, color);
	}
	else if (m < -1) {
		this->DrawLineShape3(p1, p2, color);
	}
	else {
		this->DrawLineShape4(p1, p2, color);
	}
}

void Renderer::DrawLineShape1(const vec3 p1, const vec3 p2, const vec3 color) {
	vec3 start, end;
	cout << "shape1" << endl;
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
	int dy = end.y - start.y;
	//int y_change = (dy > 1) ? 1 : -1;
	int de = 2 * dy;
	int d = 2 * dy - dx;
	int dne = 2 * dy - 2 * dx;

	PlotPixel(start.x, y, color);

	for (int x = start.x; x < end.x; ++x) {
		if (d < 0) {
			d += de;
		}
		else {
			++y;
			d += dne;
		}
		PlotPixel(x, y, color);
	}
}
void Renderer::DrawLineShape2(const vec3 p1, const vec3 p2, const vec3 color) {
	vec3 start, end;
	cout << "shape2" << endl;
	if (p1.y > p2.y) {
		start = p2;
		end = p1;
	}
	else {
		start = p1;
		end = p2;
	}

	int x = start.x;
	int dx = end.x - start.x;
	int dy = end.y - start.y;
	int de = 2 * dx;
	int d = 2 * dx - dy;
	int dne = 2 * dx - 2 * dy;

	PlotPixel(x, start.y, color);

	for (int y = start.y; y < end.y; ++y) {
		if (d < 0) {
			d += de;
		}
		else {
			++x;
			d += dne;
		}
		PlotPixel(x, y, color);
	}
}
void Renderer::DrawLineShape3(const vec3 p1, const vec3 p2, const vec3 color) {
	vec3 start, end;
	cout << "shape3 - TODO" << endl;
	if (p1.y > p2.y) {
		start = p2;
		end = p1;
	}
	else {
		start = p1;
		end = p2;
	}

	int x = start.x;
	int dx = -1 * (end.x - start.x);
	int dy = end.y - start.y;
	int de = 2 * dx;
	int d = 2 * dx - dy;
	int dne = 2 * dx - 2 * dy;

	PlotPixel(x, start.y, color);

	for (int y = start.y; y < end.y; ++y) {
		if (d < 0) {
			d += de;
		}
		else {
			--x;
			d += dne;
		}
		PlotPixel(x, y, color);
	}
}
void Renderer::DrawLineShape4(const vec3 p1, const vec3 p2, const vec3 color) {
	vec3 start, end;
	cout << "shape4 - TODO repair" << endl;
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
	int dy = -1 * (end.y - start.y);
	int de = 2 * dy;
	int d = 2 * dy - dx;
	int dne = 2 * dy - 2 * dx;

	PlotPixel(start.x, y, color);

	for (int x = start.x; x < end.x; ++x) {
		if (d < 0) {
			d += de;
		}
		else {
			--y;
			d += dne;
		}
		PlotPixel(x, y, color);
	}
}

void Renderer::DrawSteepLine(const vec3 p1, const vec3 p2, const vec3 color) {
	vec3 start, end;
	cout << "steep" << endl;
	if (p1.y > p2.y) {
		start = p2;
		end = p1;
	}
	else {
		start = p1;
		end = p2;
	}

	int x = start.x;
	int dx = end.x - start.x;
	int dy = end.y - start.y;
	//int x_change = (dx > 1) ? 1 : -1;
	/*int de = 2 * dx;
	int d = 2 * dx - dy;
	int dne = 2 * dx - 2 * dy;*/
	int de = abs(2 * dx);
	int d = abs(2 * dx - dy);
	int dne = abs(2 * dx - 2 * dy);

	PlotPixel(x, start.y, color);

	for (int y = start.y; y < end.y; ++y) {
		if (d < 0) {
			d += de;
		}
		else {
			x += 1;
			d += dne;
		}
		PlotPixel(x, y, color);
	}
}

void Renderer::DrawModerateLine(const vec3 p1, const vec3 p2, const vec3 color) {
	vec3 start, end;
	cout << "moderate" << endl;
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
	int dy = end.y - start.y;
	int y_change = (dy > 1) ? 1 : -1;
	int de = 2 * dy;
	int d = 2 * dy - dx;
	int dne = 2 * dy - 2 * dx;

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

void Renderer::SetDemoBuffer()
{
	for (int i = 50; i <= 150; i += 10) {
		DrawLine(vec3(100, 100, 0), vec3(i, 150, 0), vec3(1, 0, 0));
	}
	for (int i = 50; i <= 150; i += 10) {
		DrawLine(vec3(100, 100, 0), vec3(150, i, 0), vec3(0, 1, 0));
	}
	//vertical line
	for(int i=0; i<m_width; i++)
	{
	m_outBuffer[INDEX(m_width,256,i,0)]=1;
	m_outBuffer[INDEX(m_width,256,i,1)]=0;
	m_outBuffer[INDEX(m_width,256,i,2)]=0;
	}
	//horizontal line
	for (int i = 0; i<m_width; i++) {
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