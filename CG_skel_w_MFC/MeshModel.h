#pragma once
#include "vec.h"
#include "mat.h"
#include "BaseRenderer.h"
#include "Material.h"
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

protected:
	vector<vec3> vertexPositions;
	vector<vec3> vertexNormals;
	vector<vec3> faceNormals;
	vector<Material> materials;

	vec3 & getVecByIndex(vector<vec3> & vecs, int i);
	void computeFaceNormals();
	MeshModel();

public:
	MeshModel(string fileName);
	~MeshModel(void) = default;
	void transformInModel(const mat4 & transform);
	void transformInWorld(const mat4 & transform);
	vec4 getLocation();
	void switchVertexNormalsVisibility();
	void switchFaceNormalsVisibility();
	void switchBoundingBoxVisibility();
	void setUniformMaterial(Material material);
	void setRandomMaterial();
	void draw(BaseRenderer * renderer) const;
};

class PrimMeshModel : public MeshModel
{
public:
	PrimMeshModel();
};