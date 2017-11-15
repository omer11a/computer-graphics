#pragma once
#include <vector>
#include "CG_skel_w_MFC.h"
#include "vec.h"
#include "mat.h"
#include "GL/glew.h"

using namespace std;
class Renderer
{
	float *m_outBuffer; // 3*width*height
	float *m_zbuffer; // width*height
	int m_width, m_height;

	void CreateBuffers(int width, int height);
	void CreateLocalBuffer();

	void PlotPixel(const int x, const int y, const vec3 color);
	void DrawLine(const vec3 p1, const vec3 p2, const vec3 color);
	void DrawLineShape1(const vec3 p1, const vec3 p2, const vec3 color);
	void DrawLineShape2(const vec3 p1, const vec3 p2, const vec3 color);
	void DrawLineShape3(const vec3 p1, const vec3 p2, const vec3 color);
	void DrawLineShape4(const vec3 p1, const vec3 p2, const vec3 color);
	void DrawSteepLine(const vec3 p1, const vec3 p2, const vec3 color);
	void DrawModerateLine(const vec3 p1, const vec3 p2, const vec3 color);

	//////////////////////////////
	// openGL stuff. Don't touch.

	GLuint gScreenTex;
	GLuint gScreenVtc;
	void CreateOpenGLBuffer();
	void InitOpenGLRendering();
	//////////////////////////////
public:
	Renderer();
	Renderer(int width, int height);
	~Renderer(void);
	void Init();
	void DrawTriangles(const vector<vec3>* vertices, const vector<vec3>* normals = NULL);
	void SetCameraTransform(const mat4& cTransform);
	void SetProjection(const mat4& projection);
	void SetObjectMatrices(const mat4& oTransform, const mat3& nTransform);
	void SwapBuffers();
	void ClearColorBuffer();
	void ClearDepthBuffer();
	void SetDemoBuffer();
};
