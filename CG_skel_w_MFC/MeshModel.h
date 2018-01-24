#pragma once
#include "vec.h"
#include "mat.h"
#include "Material.h"
#include "BaseRenderer.h"
#include <string>
#include <vector>

using namespace std;

class MeshModel
{
	mat4 modelTransform;
	mat4 worldTransform;
	mat3 normalModelTransform;
	mat3 normalWorldTransform;
	vec3 minValues;
	vec3 maxValues;
	bool allowVertexNormals;
	bool allowFaceNormals;
	bool allowBoundingBox;

	void loadFile(string fileName);
	mat3 convertToNormalTransform(const mat4 & transform) const;
	void computeBoundingBox();
	void computeTangents();

protected:
	vector<vec3> vertexPositions;
	vector<vec3> centerPositions;
	vector<vec3> vertexNormals;
	vector<vec3> smoothVertexNormals;
	vector<vec2> textureCoordinates;
	vector<vec2> textureCenters;
	vector<vec3> faceNormals;
	vector<vec3> tangents;
	vector<vec3> ambients, diffuses, speculars;
	vector<float> shininess;

	GLuint textureID, normalMapID;
	bool hasTexture, hasNormalMap, hasColorAnimation, hasVertexAnimation, hasToonShading, hasWoodTexture, hasEnvironmentMapping;
	int colorAnimationRepresentation, colorAnimationDirection, vertexAnimationDirection;
	float colorAnimationSpeed, colorAnimationDuration, colorAnimationProgress;
	float vertexAnimationSpeed, vertexAnimationDuration, vertexAnimationProgress;
	int colorQuantizationCoefficient;
	float silhouetteThickness, refractionRatio;
	vec3 silhouetteColor, woodTextureColor1, woodTextureColor2;

	template<class T>
	T & getVecByIndex(vector<T> & vecs, int i)
	{
		if (i >= 1) {
			return vecs.at(i - 1);
		}

		if (i <= -1) {
			return vecs.at(vecs.size() + i);
		}

		throw out_of_range("Invalid index in obj file");
	}
	
	void computeFaceNormals();
	void computeCenterPositions();
	void clearTexture();
	MeshModel();

public:
	MeshModel(string fileName);
	~MeshModel();
	void transformInModel(const mat4 & transform);
	void transformInWorld(const mat4 & transform);
	vec4 getLocation();
	void switchVertexNormalsVisibility();
	void switchFaceNormalsVisibility();
	void switchBoundingBoxVisibility();
	void setUniformMaterial(Material material);
	void setRandomMaterial();
	void setTextures(const string fileName, const float shininess);
	void enableNormalMap(const string fileName);
	void disableNormalMap();
	void startColorAnimation(const int animationType, const float speed, const float duration);
	void stepAnimation(const float timeDelta);
	void stopColorAnimation();
	void startVertexAnimation(const float speed, const float duration);
	void stopVertexAnimation();
	void enableToonShading(const int cqc, const float st, const vec3& color);
	void disableToonShading();
	void enableWoodTexture(const vec3& color1, const vec3& color2);
	void disableWoodTexture();
	void enableEnviromentMapping(const float refRatio);
	void disableEnviromentMapping();
	void draw(BaseRenderer * renderer) const;
	void drawNormals(BaseRenderer * renderer) const;
};

class PrimMeshModel : public MeshModel
{
public:
	PrimMeshModel();
};

class PlaneMeshModel : public MeshModel
{
public:
	PlaneMeshModel();
};

class CylinderMeshModel : public MeshModel
{
public:
	CylinderMeshModel();
};