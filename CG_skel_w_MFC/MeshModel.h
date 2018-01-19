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
	vector<vec2> textureCoordinates;
	vector<vec2> textureCenters;
	vector<vec3> faceNormals;
	vector<vec3> tangents;
	//vector<vec3> bitangents;
	vector<Material> materials;
	GLuint textureID, normalMapID;
	bool hasTexture, hasNormalMap;

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
	bool readPng(
		const string fileName,
		int element_size,
		unsigned int * width,
		unsigned int * height,
		unsigned char ** pixel_array,
		bool containsNormals = false
	);
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
	void setTextures(const vec3& ambient, const vec3& specular, const string fileName, const float shininess);
	void enableNormalMap(const string fileName);
	void disableNormalMap();
	void draw(BaseRenderer * renderer) const;
	void drawNormals(BaseRenderer * renderer) const;
};

class PrimMeshModel : public MeshModel
{
public:
	PrimMeshModel();
};