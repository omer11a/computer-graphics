#pragma once
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
using namespace glm;
#include <vector>
#include "vec.h"
#include "mat.h"
#include "Material.h"
#include "BaseRenderer.h"
#include "Polygon.h"
#include "Shader.h"
#include "GL/glew.h"
#include "CG_skel_w_MFC.h"

using namespace std;
class Renderer : public BaseRenderer
{
	float *m_outBuffer;		// width * height * 3
	float *m_screenBuffer;		// screen_width * screen_height * 3
	float *m_zBuffer;		// width * height
	bool *m_paintBuffer;	// width * height
	int m_screen_width, m_screen_height, anti_factor;

	mat4 m_cTransform, m_projection, m_oTransform, mvp;
	mat3 m_cnTransform, m_nTransform;
	Shader * shader, * default_shader;
	float zNear;
	float zFar;

	bool is_wire_mode;

	void CreateBuffers(int width, int height);
	void CreateLocalBuffer();
	void SetPolygonToShader(const ConvexPolygon * shader_polygon, const vec3& f_normal);
	vec3 CalculatePointColor(const vec3& p1, const vec3& p2, const vec3& p3, const float abc_area, const vec3& p);
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
	void DrawLine(const vec3& p1, const vec3& p2, const vec3& c=vec3(-1), const int p1_idx=0, const int p2_idx=0);
	void DrawSteepLine(const vec3& p1, const vec3& p2, const vec3& c, const int p1_idx = 0, const int p2_idx = 0);
	void DrawModerateLine(const vec3& p1, const vec3& p2, const vec3& c, const int p1_idx = 0, const int p2_idx = 0);
	
	void PaintTriangle(const vector<vec3> * vertices, const vector<Material> * materials, const vector<vec3> * v_normals, const vec3& f_normal);
	void PaintTriangleFloodFill(const vec3& p1, const vec3& p2, const vec3& p3, const vec3& p);
	void PaintTriangleScanLines(const vec3& p1, const vec3& p2, const vec3& p3);

	void FillAntiAliasingBuffer();
	//////////////////////////////
	// openGL stuff. Don't touch.

	GLuint gScreenTex;
	GLuint gScreenVtc;	// VertexArrayID in tutorials
	GLuint programID;
	void CreateOpenGLBuffer();
	void InitOpenGLRendering();
	//////////////////////////////
public:
	Renderer();
	Renderer(int width, int height, Shader * shader);
	~Renderer(void);

	void DrawTriangles(
		const vector<vec3>* vertices,
		const vector<Material>* materials,
		const vector<vec3>* vertexNormals = NULL,
		const vector<vec3>* faceNormals = NULL,
		const bool allowVertexNormals = false,
		const bool allowFaceNormals = false) override;
	void SwitchWire(); 
	void SetAntiAliasing(int new_factor);
	void SetBaseShader(Shader * s);
	void SetFog(const vec3& color, const float extinction, const float scattering);

	void DrawSquare(const vec3& p1, const vec3& p2, const vec3& p3, const vec3& p4, const vec3& color);
	void DrawBox(const vec3& minValues, const vec3& maxValues) override;
	void DrawCamera() override;
	void DrawLight(const vec3& color, const vec3& position) override;
	void SetLights(const vector<Light *> * lights);
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
