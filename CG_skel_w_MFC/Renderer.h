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
	float *m_zBuffer; // width*height
	bool *m_paintBuffer;
	bool is_wire_mode;
	int m_width, m_height;
	mat4 m_cTransform, m_projection, m_oTransform;
	mat3 m_nTransform;
	mat4 m_camera_multiply;
	float zNear;
	float zFar;

	void CreateBuffers(int width, int height);
	void CreateLocalBuffer();
	void DestroyBuffers();

	vec4 applyCameraTransformation(const vec3& p, const vec3& n) const;
	vec3 applyProjection(const vec4& p) const;
	void clip(float x0, float x1, float xmin, float xmax, float& t1, float& t2) const;
	bool clipLine(const vec3& v1, const vec3& n1, const vec3& v2, const vec3& n2, vec3& start, vec3& end) const;
	vec3 convertToScreen(const vec3& p) const;
	bool PixelToPoint(const vec3& p1, const vec3& p2, const vec3& p3, const vec3& p, vec3& newP) const;
	bool pointToScreen(const vec3& p, const vec3& n, vec3& q, bool should_screen=true) const;
	bool lineToScreen(const vec3& p1, const vec3& n1, const vec3& p2, const vec3& n2, vec3& q1, vec3& q2) const;

	bool PlotPixel(const int x, const int y, const float z, const vec3& color);
	vec3 GetCenterMass(const vec3& p1, const vec3& p2, const vec3& p3) const;
	vec3 GetCenterMass(const vec3 * vertices, const int length) const;
	//void UpdateBCPoint(const vec3& p1, const vec3& p2, const vec3& p3, vec3& p) const;
	bool DrawLine(const vec3& p1, const vec3& n1, const vec3& p2, const vec3& n2, const vec3& c1, const vec3& c2);
	void DrawSteepLine(const vec3& p1, const vec3& p2, const vec3& c1, const vec3& c2);
	void DrawModerateLine(const vec3& p1, const vec3& p2, const vec3& c1, const vec3& c2);
	void PaintTriangle(const vec3& p1, const vec3& p2, const vec3& p3, const vec3& c1);
	void PaintTriangleRecursive(const vec3& p1, const vec3& p2, const vec3& p3, const vec3& p, const vec3& c1);
	void PaintTriangle2(const vec3& p1, const vec3& p2, const vec3& p3, const vec3& c1);
	//void fill_tri(const vec3& p1, const vec3& p2, const vec3& p3, const vec3& c1, const vec3& c2, const vec3& c3);

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

	void DrawTriangles(const vector<vec3>* vertices, const vector<vec3>* vertexNormals = NULL, const vector<vec3>* faceNormals = NULL);
	void switchWire(); 
	void DrawSquare(const vec3& p1, const vec3& p2, const vec3& p3, const vec3& p4, const vec3& color);
	void DrawBox(const vec3& minValues, const vec3& maxValues);
	void DrawCamera();
	void SetCameraTransform(const mat4& cTransform);
	void SetProjection(const mat4& projection);
	void SetZRange(float zNear, float zFar);
	void SetObjectMatrices(const mat4& oTransform, const mat3& nTransform = mat3());
	void UpdateBuffers(int width, int height);
	void SwapBuffers();
	void ClearColorBuffer();
	void ClearDepthBuffer();
	void SetDemoBuffer();
};
