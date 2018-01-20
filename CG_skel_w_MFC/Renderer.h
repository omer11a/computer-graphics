#pragma once
#include <vector>
#include "vec.h"
#include "mat.h"
#include "ShaderProgram.h"
#include "Material.h"
#include "Light.h"
#include "BaseRenderer.h"
#include "GL/glew.h"
#include "CG_skel_w_MFC.h"

using namespace std;
class Renderer : public BaseRenderer
{
public:
	enum ShaderType {
		Flat, Gouraud, Phong
	};
private:
	int m_screen_width, m_screen_height;
	bool is_wire_mode, anti_factor;

	mat4 m_cTransform, m_projection, m_oTransform, mvp, mv, vp;
	mat3 m_cnTransform, m_nTransform;

	ShaderType shader;

	//////////////////////////////
	// openGL stuff. Don't touch.
	GLuint gScreenTex;
	GLuint gScreenVtc;	// VertexArrayID in tutorials
	ShaderProgram basicProgram, objectsProgram, normalsProgram, toonProgram;
	void CreateOpenGLBuffer();
	void InitOpenGLRendering();
	//////////////////////////////
public:
	Renderer();
	Renderer(int width, int height);
	~Renderer(void);

	void DrawToonShadow(
		const vector<vec3>* vertices,
		const vector<vec3>* vertexNormals,
		const float silhouetteThickness,
		const vec3& silhouetteColor) override;
	void DrawTriangles(
		const vector<vec3>* vertices,
		const vector<Material>* materials,
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
		const vector<vec3>* vertexNormals = NULL,
		const vector<vec3>* faceNormals = NULL) override;
	void DrawModelNormals(
		const vector<vec3>* vertices,
		const vector<vec3>* centerPositions,
		const vector<vec3>* vertexNormals,
		const vector<vec3>* faceNormals);

	void SwitchWire(); 
	void SetAntiAliasing();
	void SetBaseShader(ShaderType s);
	void SetFog(const vec3& color, const float extinction, const float scattering);
	void DisableFog();

	void DrawSquare(const vec3& p1, const vec3& p2, const vec3& p3, const vec3& p4, const vec3& color);
	void DrawBox(const vec3& minValues, const vec3& maxValues) override;
	void DrawCamera() override;
	void DrawLight(const vec3& color, const vec3& position) override;
	void SetShaderLights(const AmbientLight& amb_light, const vector<DirectionalLightSource *>& lights);
	void SetCameraTransform(const mat4& cInverseTransform, const mat4& cTransform) override;
	void SetProjection(const mat4& projection) override;
	void SetObjectMatrices(const mat4& oTransform, const mat3& nTransform = mat3()) override;
	void UpdateBuffers(int width, int height) override;
	void SwapBuffers() override;
	void ClearColorBuffer() override;
	void SetDemoBuffer() override;
};
