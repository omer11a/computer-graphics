#pragma once


#include "vec.h"
#include "mat.h"
#include "Material.h"
#include <vector>

using namespace std;
class BaseRenderer {
protected:
	int m_width, m_height, min_size;

public:
	explicit BaseRenderer();
	explicit BaseRenderer(int width, int height);
	
	virtual void DrawSkyBox(const vector<vec3>* vertices);
	virtual void DrawToonShadow(
		const vector<vec3>* vertices,
		const vector<vec3>* vertexNormals,
		const float silhouetteThickness,
		const vec3& silhouetteColor);
	virtual void DrawTriangles(
		const vector<vec3>* vertices,
		const vector<vec3>* ambients,
		const vector<vec3>* diffuses,
		const vector<vec3>* speculars,
		const vector<float>* shininess,
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
		const bool hasEnvironmentMapping,
		const float refractionRatio,
		const vector<vec3>* vertexNormals = NULL,
		const vector<vec3>* faceNormals = NULL);
	virtual void DrawModelNormals(
		const vector<vec3>* vertices,
		const vector<vec3>* centerPositions,
		const vector<vec3>* vertexNormals,
		const vector<vec3>* faceNormals);
	virtual void DrawBox(const vec3& minValues, const vec3& maxValues);
	virtual void DrawCamera();
	virtual void DrawLight(const vec3& color, const vec3& position);

	virtual void SetCameraTransform(const mat4& cInverseTransform, const mat4& cTransform);
	virtual void SetProjection(const mat4& projection);
	virtual void SetZRange(float zNear, float zFar);
	virtual void SetObjectMatrices(const mat4& oTransform, const mat3& nTransform = mat3());
	virtual void EnableSkyBox(const GLuint skyBox);
	virtual void DisableSkyBox();

	virtual void UpdateBuffers(int width, int height);
	virtual void SwapBuffers();
	virtual void ClearColorBuffer();
	virtual void SetDemoBuffer();
};
