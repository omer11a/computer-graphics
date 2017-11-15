#include "StdAfx.h"
#include "MeshModel.h"
#include "vec.h"
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>

using namespace std;

struct FaceIdcs
{
	int v[4];
	int vn[4];
	int vt[4];

	FaceIdcs()
	{
		for (int i=0; i<4; i++)
			v[i] = vn[i] = vt[i] = 0;
	}

	FaceIdcs(std::istream & aStream)
	{
		for (int i=0; i<4; i++)
			v[i] = vn[i] = vt[i] = 0;

		char c;
		for(int i = 0; i < 3; i++)
		{
			aStream >> std::ws >> v[i] >> std::ws;
			if (aStream.peek() != '/')
				continue;
			aStream >> c >> std::ws;
			if (aStream.peek() == '/')
			{
				aStream >> c >> std::ws >> vn[i];
				continue;
			}
			else
				aStream >> vt[i];
			if (aStream.peek() != '/')
				continue;
			aStream >> c >> vn[i];
		}
	}
};

vec3 vec3fFromStream(std::istream & aStream)
{
	float x, y, z;
	aStream >> x >> std::ws >> y >> std::ws >> z;
	return vec3(x, y, z);
}

vec2 vec2fFromStream(std::istream & aStream)
{
	float x, y;
	aStream >> x >> std::ws >> y;
	return vec2(x, y);
}

vec3 & MeshModel::getVecByIndex(vector<vec3> & vecs, int i)
{
	if (i >= 1) {
		return vecs.at(i - 1);
	}

	if (i <= -1) {
		return vecs.at(vecs.size() + i);
	}

	throw out_of_range("Invalid index in obj file");
}

void MeshModel::loadFile(string fileName)
{
	ifstream ifile(fileName.c_str());
	vector<FaceIdcs> faces;
	vector<vec3> vertices;
	vector<vec3> normals;
	// while not end of file
	while (!ifile.eof())
	{
		// get line
		string curLine;
		getline(ifile, curLine);

		// read type of the line
		istringstream issLine(curLine);
		string lineType;

		issLine >> std::ws >> lineType;

		// based on the type parse data
		if (lineType == "v")
			vertices.push_back(vec3fFromStream(issLine));
		else if (lineType == "vn")
			normals.push_back(vec3fFromStream(issLine));
		else if (lineType == "f")
			faces.push_back(issLine);
		else if (lineType == "#" || lineType == "")
		{
			// comment / empty line
		}
		else
		{
			cout << "Found unknown line Type \"" << lineType << "\"";
		}
	}
	//Vertex_positions is an array of vec3. Every three elements define a triangle in 3D.
	//If the face part of the obj is
	//f 1 2 3
	//f 1 3 4
	//Then vertex_positions should contain:
	//vertex_positions={v1,v2,v3,v1,v3,v4}

	// iterate through all stored faces and create triangles
	int k = 0;
	for (vector<FaceIdcs>::iterator it = faces.begin(); it != faces.end(); ++it)
	{
		for (int i = 0; i < 3; i++)
		{
			vertex_positions->push_back(getVecByIndex(vertices, it->v[i]));
			vertex_positions->push_back(getVecByIndex(normals, it->vn[i]));
		}
	}
}

void MeshModel::applyTransformToNormals(const mat4 & transform) {
	mat3 transform_in_3d = convert4dTo3d(transform);
	if (!transform_in_3d.isInvertible()) {
		throw invalid_argument("Can't apply matrix to normals");
	}

	normal_transform = normal_transform * transpose(inverse(transform_in_3d));
}

void MeshModel::computeFaceNormals() {
	if ((vertex_positions == NULL) || (face_normals != NULL)) {
		return;
	}

	for (unsigned int i = 0; i < vertex_positions->size(); i += 3) {
		vec3 v1 = vertex_positions->at(i + 1) - vertex_positions->at(i);
		vec3 v2 = vertex_positions->at(i + 2) - vertex_positions->at(i);
		face_normals->push_back(normalize(cross(v1, v2)));
	}
}

void MeshModel::computeBoundingBox() {
	if ((vertex_positions == NULL) || (vertex_positions->empty())) {
		return;
	}

	min_values = max_values = vertex_positions->at(0);
	for (unsigned int i = 1; i < vertex_positions->size(); ++i) {
		vec3 vertex = vertex_positions->at(i);

		min_values.x = min(min_values.x, vertex.x);
		min_values.y = min(min_values.y, vertex.y);
		min_values.z = min(min_values.z, vertex.z);

		max_values.x = max(max_values.x, vertex.x);
		max_values.y = max(max_values.y, vertex.y);
		max_values.z = max(max_values.z, vertex.z);
	}
}

MeshModel::MeshModel() :
	vertex_positions(NULL), vertex_normals(NULL), face_normals(NULL),
	world_transform(), model_transform(), normal_transform(),
	allow_vertex_normals(false), allow_face_normals(false)
{}

MeshModel::MeshModel(string fileName) : MeshModel()
{
	try {
		vertex_positions = new vector<vec3>();
		vertex_normals = new vector<vec3>();
		loadFile(fileName);
	} catch (...) {
		this->~MeshModel();
		throw;
	}
}

MeshModel::~MeshModel(void)
{
	if (face_normals != NULL) {
		delete face_normals;
	}

	if (vertex_normals != NULL) {
		delete vertex_normals;
	}

	if (vertex_positions != NULL) {
		delete vertex_positions;
	}
}

void MeshModel::transformInModel(const mat4 & transform) {
	applyTransformToNormals(transform);
	model_transform = model_transform * transform;
}

void MeshModel::transformInWorld(const mat4 & transform) {
	applyTransformToNormals(transform);
	world_transform = world_transform * transform;
}

void MeshModel::setVertexNormalsVisibility(bool should_be_visible) {
	allow_vertex_normals = should_be_visible;
}

void MeshModel::setFaceNormalsVisibility(bool should_be_visible) {
	allow_face_normals = should_be_visible;
	if ((face_normals == NULL) && (allow_face_normals)) {
		computeFaceNormals();
	}
}

void MeshModel::setBoundingBoxVisibility(bool should_be_visible) {
	allow_bounding_box = should_be_visible;
	if (allow_bounding_box) {
		computeBoundingBox();
	}
}

void MeshModel::draw()
{
	
}