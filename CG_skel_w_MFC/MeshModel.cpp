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
			vertexPositions.push_back(getVecByIndex(vertices, it->v[i]));
			vertexNormals.push_back(getVecByIndex(normals, it->vn[i]));
		}
	}
}

mat3 MeshModel::convertToNormalTransform(const mat4 & transform) const {
	mat3 transform_in_3d = convert4dTo3d(transform);
	if (!transform_in_3d.isInvertible()) {
		throw invalid_argument("Can't apply matrix to normals");
	}

	return transpose(inverse(transform_in_3d));
}

void MeshModel::computeFaceNormals() {
	for (unsigned int i = 0; i < vertexPositions.size(); i += 3) {
		vec3 v1 = vertexPositions.at(i + 1) - vertexPositions.at(i);
		vec3 v2 = vertexPositions.at(i + 2) - vertexPositions.at(i);
		faceNormals.push_back(normalize(cross(v1, v2)));
	}
}

void MeshModel::computeBoundingBox() {
	minValues = maxValues = vertexPositions.at(0);
	for (unsigned int i = 1; i < vertexPositions.size(); ++i) {
		vec3 vertex = vertexPositions.at(i);

		minValues.x = min(minValues.x, vertex.x);
		minValues.y = min(minValues.y, vertex.y);
		minValues.z = min(minValues.z, vertex.z);

		maxValues.x = max(maxValues.x, vertex.x);
		maxValues.y = max(maxValues.y, vertex.y);
		maxValues.z = max(maxValues.z, vertex.z);
	}
}

MeshModel::MeshModel(string fileName) :
	vertexPositions(), vertexNormals(), faceNormals(),
	worldTransform(), modelTransform(), normalModelTransform(), normalWorldTransform(),
	allowVertexNormals(false), allowFaceNormals(false), allowBoundingBox(false)
{
	loadFile(fileName);
}

void MeshModel::transformInModel(const mat4 & transform) {
	normalModelTransform = normalModelTransform * convertToNormalTransform(transform);
	modelTransform = modelTransform * transform;
}

void MeshModel::transformInWorld(const mat4 & transform) {
	normalWorldTransform = normalWorldTransform * convertToNormalTransform(transform);
	worldTransform = worldTransform * transform;
}

void MeshModel::switchVertexNormalsVisibility() {
	allowVertexNormals = !allowVertexNormals;
}

void MeshModel::switchFaceNormalsVisibility() {
	allowFaceNormals = !allowFaceNormals;
	if (allowFaceNormals) {
		computeFaceNormals();
	}
}

void MeshModel::switchBoundingBoxVisibility() {
	allowBoundingBox = !allowBoundingBox;
	if (allowBoundingBox) {
		computeBoundingBox();
	}
}

void MeshModel::draw(Renderer * renderer) const {
	if (renderer == NULL) {
		throw invalid_argument("Renderer is null");
	}

	renderer->SetObjectMatrices(worldTransform * modelTransform, normalWorldTransform * normalModelTransform);

	const vector<vec3> * vertexNormalsToRenderer = allowVertexNormals ? &vertexNormals : NULL;
	const vector<vec3> * faceNormalsToRenderer = allowFaceNormals ? &faceNormals : NULL;
	renderer->DrawTriangles(&vertexPositions, vertexNormalsToRenderer, faceNormalsToRenderer);

	if (allowBoundingBox) {
		renderer->DrawBox(minValues, maxValues);
	}
}