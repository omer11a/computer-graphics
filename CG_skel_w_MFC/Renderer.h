#pragma once
#include <vector>
#include "BaseRenderer.h"
#include "CG_skel_w_MFC.h"
#include "Material.h"
#include "vec.h"
#include "mat.h"
#include "GL/glew.h"

using namespace std;
class Renderer : public BaseRenderer
{
	float *m_outBuffer;		// width * height * 3
	float *m_zBuffer;		// width * height
	bool *m_paintBuffer;	// width * height

	mat4 m_cTransform, m_projection, m_oTransform;
	mat3 m_nTransform;
	mat4 m_camera_multiply;
	float zNear;
	float zFar;

	bool is_wire_mode;

	void CreateBuffers(int width, int height);
	void CreateLocalBuffer();
	void DestroyBuffers();

	vec4 applyCameraTransformation(const vec3& p, const vec3& n) const;
	vec3 applyProjection(const vec4& p) const;
	void clip(float x0, float x1, float xmin, float xmax, float& t1, float& t2) const;
	bool clipLine(const vec3& v1, const vec3& n1, const vec3& v2, const vec3& n2, vec3& start, vec3& end) const;
	bool clipLine(const vec3& v1, const vec3& v2, vec3& start, vec3& end) const;
	vec3 convertToScreen(const vec3& p) const;
	bool PixelToPoint(const vec3& p1, const vec3& p2, const vec3& p3, const vec3& p, vec3& newP) const;
	bool pointToScreen(const vec3& p, const vec3& n, vec3& q, bool should_screen=true) const;
	bool lineToScreen(const vec3& p1, const vec3& n1, const vec3& p2, const vec3& n2, vec3& q1, vec3& q2) const;

	bool PlotPixel(const int x, const int y, const float z, const vec3& color);
	vec3 GetCenterMass(const vec3& p1, const vec3& p2, const vec3& p3) const;
	vec3 GetCenterMass(vector<vec3> const * const vertices) const;
	
	bool DrawLine(const vec3& p1, const vec3& n1, const vec3& p2, const vec3& n2, const vec3& c1);
	void DrawLine(const vec3& p1, const vec3& p2, const vec3& c=vec3(-1));
	void DrawSteepLine(const vec3& p1, const vec3& p2, const vec3& c);
	void DrawModerateLine(const vec3& p1, const vec3& p2, const vec3& c);
	
	void PaintTriangle(const vec3& p1, const vec3& p2, const vec3& p3, const vec3& c1);	// deprecated
	void PaintTriangle(const vector<vec3> * vertices, const vector<Material> * materials, const vector<vec3> * vertexNormals);
	void PaintTriangleFloodFill(const vec3& p1, const vec3& p2, const vec3& p3, const vec3& p);
	void PaintTriangleScanLines(const vec3& p1, const vec3& p2, const vec3& p3, const vec3& c1);	// deprecated

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

	void DrawTriangles(const vector<vec3>* vertices, const vector<Material>* materials, const vector<vec3>* vertexNormals = NULL, const vector<vec3>* faceNormals = NULL) override;
	void switchWire(); 
	void DrawSquare(const vec3& p1, const vec3& p2, const vec3& p3, const vec3& p4, const vec3& color);
	void DrawBox(const vec3& minValues, const vec3& maxValues) override;
	void DrawCamera() override;
	void DrawLight(const vec3& color) override;
	void SetCameraTransform(const mat4& cTransform) override;
	void SetProjection(const mat4& projection) override;
	void SetZRange(float zNear, float zFar) override;
	void SetObjectMatrices(const mat4& oTransform, const mat3& nTransform = mat3()) override;
	void UpdateBuffers(int width, int height) override;
	void SwapBuffers() override;
	void ClearColorBuffer() override;
	void ClearDepthBuffer() override;
	void SetDemoBuffer() override;
};
