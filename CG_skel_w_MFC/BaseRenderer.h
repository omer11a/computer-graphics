#pragma once
#include <vector>

#include "vec.h"
#include "mat.h"
#include "Material.h"

using namespace std;
class BaseRenderer {
protected:
	int m_width, m_height, min_size;

public:
	explicit BaseRenderer();
	explicit BaseRenderer(int width, int height);
	
	virtual void DrawTriangles(const vector<vec3>* vertices, const vector<Material>* materials, const vector<vec3>* vertexNormals = NULL, const vector<vec3>* faceNormals = NULL);
	virtual void DrawBox(const vec3& minValues, const vec3& maxValues);
	virtual void DrawCamera();
	virtual void DrawLight(const vec3& color);

	virtual void SetCameraTransform(const mat4& cTransform);
	virtual void SetProjection(const mat4& projection);
	virtual void SetZRange(float zNear, float zFar);
	virtual void SetObjectMatrices(const mat4& oTransform, const mat3& nTransform = mat3());

	virtual void UpdateBuffers(int width, int height);
	virtual void SwapBuffers();
	virtual void ClearColorBuffer();
	virtual void ClearDepthBuffer();
	virtual void SetDemoBuffer();
};
