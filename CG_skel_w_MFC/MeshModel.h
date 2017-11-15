#pragma once
#include "scene.h"
#include "vec.h"
#include "mat.h"
#include <string>

using namespace std;

class MeshModel : public Model
{
protected :
	MeshModel();
	vector<vec3> * vertex_positions;
	vector<vec3> * vertex_normals;
	vector<vec3> * face_normals;
	mat4 model_transform;
	mat4 world_transform;
	mat3 normal_transform;
	vec3 min_values;
	vec3 max_values;
	bool allow_vertex_normals;
	bool allow_face_normals;
	bool allow_bounding_box;

	vec3 & getVecByIndex(vector<vec3> & vecs, int i);
	void loadFile(string fileName);
	void applyTransformToNormals(const mat4 & transform);
	void computeFaceNormals();
	void computeBoundingBox();

public:

	MeshModel(string fileName);
	~MeshModel(void);
	void transformInModel(const mat4 & transform);
	void transformInWorld(const mat4 & transform);
	void setVertexNormalsVisibility(bool should_be_visible);
	void setFaceNormalsVisibility(bool should_be_visible);
	void setBoundingBoxVisibility(bool should_be_visible);
	void draw();
};
