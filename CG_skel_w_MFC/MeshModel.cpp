#include "StdAfx.h"
#include "MeshModel.h"
#include "vec.h"
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <random>

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

MeshModel::MeshModel() :
	vertexPositions(), vertexNormals(), faceNormals(), materials(),
	worldTransform(), modelTransform(), normalModelTransform(), normalWorldTransform(),
	allowVertexNormals(false), allowFaceNormals(false), allowBoundingBox(false)
{ }

MeshModel::MeshModel(string fileName) :
	vertexPositions(), vertexNormals(), faceNormals(), materials(),
	worldTransform(), modelTransform(), normalModelTransform(), normalWorldTransform(),
	allowVertexNormals(false), allowFaceNormals(false), allowBoundingBox(false)
{
	loadFile(fileName);
	setUniformMaterial({ 1, 1, 1, 1 });
}

void MeshModel::transformInModel(const mat4 & transform) {
	normalModelTransform = convertToNormalTransform(transform) * normalModelTransform;
	modelTransform = transform * modelTransform;
}

void MeshModel::transformInWorld(const mat4 & transform) {
	normalWorldTransform = convertToNormalTransform(transform) * normalWorldTransform;
	worldTransform = transform * worldTransform;
}

vec4 MeshModel::getLocation()
{
	vec3 cm;
	for (unsigned int i = 0; i < vertexPositions.size(); ++i) {
		vec3 vertex = vertexPositions.at(i);
		cm += vertex;
	}

	return worldTransform * modelTransform * (cm / vertexPositions.size());
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

void MeshModel::setUniformMaterial(Material material) {
	materials.clear();
	for (int i = 0; i < vertexPositions.size(); ++i) {
		materials.push_back(material);
	}
}

void MeshModel::setRandomMaterial() {
	random_device rd;
	mt19937 rng(rd());
	uniform_int_distribution<int> uni(0, 255);

	materials.clear();
	for (int i = 0; i < vertexPositions.size(); ++i) {
		materials.push_back({
			vec3(uni(rng), uni(rng), uni(rng)) / 255,
			vec3(uni(rng), uni(rng), uni(rng)) / 255,
			vec3(uni(rng), uni(rng), uni(rng)) / 255,
			(float) uni(rng)
		});
	}
}

void MeshModel::draw(BaseRenderer * renderer) const {
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

PrimMeshModel::PrimMeshModel() : MeshModel()
{
	vector<FaceIdcs> faces;
	vector<vec3> vertices;
	vector<vec3> normals;

	// cube with center mass at (0,0,0), edge legnth is 2
	vertices.push_back(vec3(-1, -1, -1));
	vertices.push_back(vec3(-1, -1, 1));
	vertices.push_back(vec3(-1, 1, -1));
	vertices.push_back(vec3(-1, 1, 1));
	vertices.push_back(vec3(1, -1, -1));
	vertices.push_back(vec3(1, -1, 1));
	vertices.push_back(vec3(1, 1, -1));
	vertices.push_back(vec3(1, 1, 1));

	normals.push_back(vec3(0, 0, 1));
	normals.push_back(vec3(0, 0, -1));
	normals.push_back(vec3(0, 1, 0));
	normals.push_back(vec3(0, -1, 0));
	normals.push_back(vec3(1, 0, 0));
	normals.push_back(vec3(-1, 0, 0));
	
	// back face
	faces.push_back(istringstream("1//2  7//2  5//2"));
	faces.push_back(istringstream("1//2  3//2  7//2"));
	// left face
	faces.push_back(istringstream("1//6  4//6  3//6"));
	faces.push_back(istringstream("1//6  2//6  4//6"));
	// top face
	faces.push_back(istringstream("3//3  8//3  7//3"));
	faces.push_back(istringstream("3//3  4//3  8//3"));
	// right face
	faces.push_back(istringstream("5//5  7//5  8//5"));
	faces.push_back(istringstream("5//5  8//5  6//5"));
	// bottom face
	faces.push_back(istringstream("1//4  5//4  6//4"));
	faces.push_back(istringstream("1//4  6//4  2//4"));
	// front face
	faces.push_back(istringstream("2//1  6//1  8//1"));
	faces.push_back(istringstream("2//1  8//1  4//1"));
	
	for (vector<FaceIdcs>::iterator it = faces.begin(); it != faces.end(); ++it) {
		for (int i = 0; i < 3; i++) {
			vertexPositions.push_back(getVecByIndex(vertices, it->v[i]));
			vertexNormals.push_back(getVecByIndex(normals, it->vn[i]));
		}
	}

	setUniformMaterial({ 1, 1, 1, 1 });
}