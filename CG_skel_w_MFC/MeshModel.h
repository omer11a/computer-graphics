#pragma once
#include "vec.h"
#include "mat.h"
#include "Renderer.h"
#include <string>
#include <vector>

using namespace std;

class MeshModel
{
	vector<vec3> vertexPositions;
	vector<vec3> vertexNormals;
	vector<vec3> faceNormals;
	mat4 modelTransform;
	mat4 worldTransform;
	mat3 normalModelTransform;
	mat3 normalWorldTransform;
	vec3 minValues;
	vec3 maxValues;
	bool allowVertexNormals;
	bool allowFaceNormals;
	bool allowBoundingBox;

	MeshModel() = delete;
	vec3 & getVecByIndex(vector<vec3> & vecs, int i);
	void loadFile(string fileName);
	mat3 convertToNormalTransform(const mat4 & transform) const;
	void computeFaceNormals();
	void computeBoundingBox();

public:
	MeshModel(string fileName);
	~MeshModel(void) = default;
	void transformInModel(const mat4 & transform);
	void transformInWorld(const mat4 & transform);
	void switchVertexNormalsVisibility();
	void switchFaceNormalsVisibility();
	void switchBoundingBoxVisibility();
	void draw(Renderer * renderer) const;
};
