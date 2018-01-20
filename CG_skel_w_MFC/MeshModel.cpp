#include "vec.h"
#include "StdAfx.h"
#include "MeshModel.h"
#include "lodepng.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <random>
#include <map>

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

void MeshModel::loadFile(string fileName)
{
	ifstream ifile(fileName.c_str());
	vector<FaceIdcs> faces;
	vector<vec3> vertices;
	vector<vec3> normals;
	vector<vec2> coordinates;
	map<int, vec3> smoothNormals;

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
		else if (lineType == "vt")
			coordinates.push_back(vec2fFromStream(issLine));
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
	for (vector<FaceIdcs>::iterator it = faces.begin(); it != faces.end(); ++it)
	{
		for (int i = 0; i < 3; i++)
		{
			vertexPositions.push_back(getVecByIndex(vertices, it->v[i]));
			
			if (normals.size() != 0) {
				vec3 normal = getVecByIndex(normals, it->vn[i]);
				vertexNormals.push_back(normal);

				auto mapIt = smoothNormals.find(it->v[i]);
				if (mapIt != smoothNormals.end()) {
					normal += mapIt->second;
				}
				
				smoothNormals[it->v[i]] = normal;
			}

			if (coordinates.size() != 0) {
				textureCoordinates.push_back(getVecByIndex(coordinates, it->vt[i]));
			}
		}
	}

	if (smoothNormals.size() != 0) {
		for (vector<FaceIdcs>::iterator it = faces.begin(); it != faces.end(); ++it) {
			for (int i = 0; i < 3; i++) {
				auto mapIt = smoothNormals.find(it->v[i]);
				smoothVertexNormals.push_back(normalize(mapIt->second));
			}
		}
	}
}

mat3 MeshModel::convertToNormalTransform(const mat4 & transform) const {
	mat3 transform_in_3d = convert4dTo3d(transform);
	if (glm::determinant(transform_in_3d) == 0) {
		throw invalid_argument("Can't apply matrix to normals");
	}

	return transpose(inverse(transform_in_3d));
}

void MeshModel::computeFaceNormals() {
	for (unsigned int i = 0; i < vertexPositions.size(); i += 3) {
		vec3 v1 = vertexPositions.at(i + 1) - vertexPositions.at(i);
		vec3 v2 = vertexPositions.at(i + 2) - vertexPositions.at(i);
		vec3 fn = normalize(cross(v1, v2));
		faceNormals.push_back(fn);
		faceNormals.push_back(fn);
		faceNormals.push_back(fn);
	}
}

void MeshModel::computeCenterPositions()
{
	for (int i = 0; i < vertexPositions.size(); i += 3) {
		vec3 cm = (vertexPositions[i] + vertexPositions[i + 1] + vertexPositions[i + 2]) / 3;
		centerPositions.push_back(cm);
		centerPositions.push_back(cm);
		centerPositions.push_back(cm);
	}

	for (int i = 0; i < textureCoordinates.size(); i += 3) {
		vec2 cm = (textureCoordinates[i] + textureCoordinates[i + 1] + textureCoordinates[i + 2]) / 3;
		textureCenters.push_back(cm);
		textureCenters.push_back(cm);
		textureCenters.push_back(cm);
	}
	
}

void MeshModel::clearTexture()
{
	if (hasTexture) {
		glDeleteTextures(1, &textureID);
	}
	hasTexture = false;
}

bool MeshModel::readPng(
	const string fileName,
	int element_size,
	unsigned int * width,
	unsigned int * height,
	unsigned char ** pixel_array,
	bool containsNormals
) {
	vector<unsigned char> pixels;
	LodePNGColorType colorType = containsNormals ? LCT_RGB : LCT_RGBA;
	unsigned error = lodepng::decode(pixels, *width, *height, fileName, colorType);

	// If there's an error, display it.
	if (error != 0) {
		std::cout << "error " << error << ": " << lodepng_error_text(error) << std::endl;
		return false;
	}
	*pixel_array = new unsigned char[pixels.size()];

	int wInc = (*width) * element_size;//width in char
	for (int i = 0; i < (*height) / 2; i++) {
		int top = i*wInc;
		int bot = (*height - i - 1) * wInc;
		for (int j = 0; j < wInc; j++) {
			// Swap the chars around.
			(*pixel_array)[top + j] = pixels[bot + j];
			(*pixel_array)[bot + j] = pixels[top + j];
		}
	}

	return true;
}

void MeshModel::computeBoundingBox() {
	minValues = maxValues = vertexPositions.at(0);
	for (unsigned int i = 1; i < vertexPositions.size(); ++i) {
		vec3 vertex = vertexPositions.at(i);

		minValues.x = std::min(minValues.x, vertex.x);
		minValues.y = std::min(minValues.y, vertex.y);
		minValues.z = std::min(minValues.z, vertex.z);

		maxValues.x = std::max(maxValues.x, vertex.x);
		maxValues.y = std::max(maxValues.y, vertex.y);
		maxValues.z = std::max(maxValues.z, vertex.z);
	}
}

void MeshModel::computeTangents() {
	//if (faceNormals.empty()) {
	//	computeFaceNormals();
	//}
	if (textureCoordinates.empty()) {
		return;
	}

	tangents.clear();
	//bitangents.clear();

	for (int i = 0; i < vertexPositions.size(); i += 3) {
		vec3 & v1 = vertexPositions[i];
		vec3 & v2 = vertexPositions[i + 1];
		vec3 & v3 = vertexPositions[i + 2];

		vec2 & uv1 = textureCoordinates[i];
		vec2 & uv2 = textureCoordinates[i + 1];
		vec2 & uv3 = textureCoordinates[i + 2];

		vec3 deltaV12 = v2 - v1;
		vec3 deltaV13 = v3 - v1;
		vec2 deltaUv12 = uv2 - uv1;
		vec2 deltaUv13 = uv3 - uv1;

		float r = 1.0f / (deltaUv12.x * deltaUv13.y - deltaUv12.y * deltaUv13.x);
		vec3 t = (deltaV12 * deltaUv13.y - deltaV13 * deltaUv12.y) * r;
		//vec3 b = (deltaV13 * deltaUv12.x - deltaV12 * deltaUv13.x) * r;

		//vec3 n = faceNormals[i];
		//t = normalize(t - n * dot(n, t));
		//if (dot(cross(n, t), b) < 0) {
		//	t = -1.0f * t;
		//}

		tangents.push_back(t);
		tangents.push_back(t);
		tangents.push_back(t);
		//bitangents.push_back(b);
		//bitangents.push_back(b);
		//bitangents.push_back(b);
	}
}

MeshModel::MeshModel() :
	vertexPositions(), vertexNormals(), smoothVertexNormals(), textureCoordinates(), textureCenters(),
	faceNormals(), materials(),
	worldTransform(1), modelTransform(1), normalModelTransform(1), normalWorldTransform(1),
	allowVertexNormals(false), allowFaceNormals(false), allowBoundingBox(false),
	textureID(0), hasTexture(false), normalMapID(0), hasNormalMap(false),
	hasColorAnimation(false), colorAnimationRepresentation(0), colorAnimationSpeed(0), colorAnimationDuration(0), colorAnimationProgress(0),
	hasVertexAnimation(false), vertexAnimationSpeed(0), vertexAnimationDuration(0), vertexAnimationProgress(0),
	colorAnimationDirection(0), vertexAnimationDirection(0), hasToonShading(false), colorQuantizationCoefficient(0), silhouetteThickness(0), silhouetteColor(0)
{ }

MeshModel::MeshModel(string fileName) :
	vertexPositions(), vertexNormals(), smoothVertexNormals(), textureCoordinates(), textureCenters(),
	faceNormals(), materials(),
	worldTransform(1), modelTransform(1), normalModelTransform(1), normalWorldTransform(1),
	allowVertexNormals(false), allowFaceNormals(false), allowBoundingBox(false),
	textureID(0), hasTexture(false), normalMapID(0), hasNormalMap(false),
	hasColorAnimation(false), colorAnimationRepresentation(0), colorAnimationSpeed(0), colorAnimationDuration(0), colorAnimationProgress(0),
	hasVertexAnimation(false), vertexAnimationSpeed(0), vertexAnimationDuration(0), vertexAnimationProgress(0),
	colorAnimationDirection(0), vertexAnimationDirection(0), hasToonShading(false), colorQuantizationCoefficient(0), silhouetteThickness(0), silhouetteColor(0)
{
	loadFile(fileName);
	setUniformMaterial({ vec3(1), vec3(1), vec3(1), 1 });
	computeFaceNormals();
	computeCenterPositions();
}

MeshModel::~MeshModel()
{
	clearTexture();
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

	return worldTransform * modelTransform * vec4(cm / vertexPositions.size(), 1);
}

void MeshModel::switchVertexNormalsVisibility() {
	if ((hasColorAnimation)) {
		cout << "can't show normals while animation in progress" << endl;
		return;
	}
	allowVertexNormals = !allowVertexNormals;
}

void MeshModel::switchFaceNormalsVisibility() {
	if ((hasColorAnimation)) {
		cout << "can't show normals while animation in progress" << endl;
		return;
	}
	allowFaceNormals = !allowFaceNormals;
}

void MeshModel::switchBoundingBoxVisibility() {
	if ((hasColorAnimation)) {
		cout << "can't show bounding box while animation in progress" << endl;
		return;
	}
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
	clearTexture();
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
	clearTexture();
}

void MeshModel::setTextures(const vec3& ambient, const vec3& specular, const string fileName, const float shininess)
{
	unsigned int width, height;
	unsigned char * pixel_array;
	if (!readPng(fileName, 4, &width, &height, &pixel_array)) {
		return;
	}

	hasTexture = true;
	glGenTextures(1, &textureID);
	try {
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixel_array);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	catch (...) {
		clearTexture();
		delete[] pixel_array;
		return;
	}
	delete[] pixel_array;

	materials.clear();
	Material m = {
		ambient,
		specular,
		vec3(0),
		shininess
	};
	for (int i = 0; i < vertexPositions.size(); ++i) {
		materials.push_back(m);
	}
}

void MeshModel::enableNormalMap(const string fileName)
{
	unsigned int width, height;
	unsigned char * pixel_array;
	computeTangents();
	if (tangents.size() == 0) {
		return;
	}

	if (!readPng(fileName, 3, &width, &height, &pixel_array, true)) {
		return;
	}

	hasNormalMap = true;
	glGenTextures(1, &normalMapID);
	try {
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, normalMapID);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, pixel_array);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	catch (...) {
		disableNormalMap();
		delete[] pixel_array;
		return;
	}

	delete[] pixel_array;
}

void MeshModel::disableNormalMap()
{
	if (hasNormalMap) {
		glDeleteTextures(1, &normalMapID);
	}
	hasNormalMap = false;
}

void MeshModel::startColorAnimation(const int animationType, const float speed, const float duration)
{
	colorAnimationRepresentation = animationType;
	colorAnimationSpeed = speed;
	colorAnimationDuration = duration;
	colorAnimationProgress = 0;
	colorAnimationDirection = 1;
	hasColorAnimation = true;
}

void MeshModel::stepAnimation(const float timeDelta)
{
	if (hasColorAnimation) {
		// bumerang
		if ((colorAnimationProgress > colorAnimationDuration) || (colorAnimationProgress < 0)) {
			colorAnimationDirection *= -1;
		}
		colorAnimationProgress += colorAnimationDirection;// *0.001;
	}

	if (hasVertexAnimation) {
		// bumerang
		if ((vertexAnimationProgress > vertexAnimationDuration) || (vertexAnimationProgress < 0)) {
			vertexAnimationDirection *= -1;
		}
		vertexAnimationProgress += vertexAnimationDirection;// *0.001;
	}
}

void MeshModel::stopColorAnimation()
{
	hasColorAnimation = false;
}

void MeshModel::startVertexAnimation(const float speed, const float duration)
{
	if (vertexNormals.size() == 0) {
		cout << "can't add animation without vertex normals" << endl;
		return;
	}
	if (allowFaceNormals || allowVertexNormals || allowBoundingBox) {
		cout << "can't add animation while normals or bouning box is visible" << endl;
		return;
	}
	vertexAnimationSpeed = speed;
	vertexAnimationDuration = duration;
	vertexAnimationProgress = 0;
	vertexAnimationDirection = 1;
	hasVertexAnimation = true;
}

void MeshModel::stopVertexAnimation()
{
	hasVertexAnimation = false;
}

void MeshModel::enableToonShading(const int cqc, const float st, const vec3& color)
{
	if (vertexNormals.size() == 0) {
		cout << "can't add toon shading without vertex normals" << endl;
		return;
	}
	colorQuantizationCoefficient = cqc;
	silhouetteThickness = st;
	silhouetteColor = color;
	hasToonShading = true;
}

void MeshModel::disableToonShading()
{
	hasToonShading = false;
}

void MeshModel::draw(BaseRenderer * renderer) const {
	if (renderer == NULL) {
		throw invalid_argument("Renderer is null");
	}
	renderer->SetObjectMatrices(worldTransform * modelTransform, normalWorldTransform * normalModelTransform);
	if (hasToonShading) {
		renderer->DrawToonShadow(&vertexPositions, &vertexNormals, silhouetteThickness, silhouetteColor);
	}
	renderer->DrawTriangles(&vertexPositions, &materials, &centerPositions, hasTexture, textureID, hasNormalMap, normalMapID,
		&textureCoordinates, &textureCenters, &tangents, 
		hasColorAnimation, colorAnimationRepresentation, colorAnimationProgress * colorAnimationSpeed,
		hasVertexAnimation, vertexAnimationProgress * vertexAnimationSpeed, hasToonShading, colorQuantizationCoefficient,
		(hasVertexAnimation) ? &smoothVertexNormals : &vertexNormals, &faceNormals);

	if (allowBoundingBox) {
		renderer->DrawBox(minValues, maxValues);
	}
}

void MeshModel::drawNormals(BaseRenderer * renderer) const {
	if (renderer == NULL) {
		throw invalid_argument("Renderer is null");
	}

	if (!allowVertexNormals && !allowFaceNormals) {
		return;
	}

	renderer->SetObjectMatrices(worldTransform * modelTransform, normalWorldTransform * normalModelTransform);
	renderer->DrawModelNormals(&vertexPositions, &centerPositions, 
		allowVertexNormals ? &vertexNormals : NULL,
		allowFaceNormals ? &faceNormals : NULL);
}

PrimMeshModel::PrimMeshModel() : MeshModel()
{
	vector<FaceIdcs> faces;
	vector<vec3> vertices;
	vector<vec3> normals;

	// cube with center mass at (0,0,0), edge legnth is 2
	vertices.push_back(vec3(-1, -1, -1));	// 1
	vertices.push_back(vec3(-1, -1, 1));	// 2
	vertices.push_back(vec3(-1, 1, -1));	// 3
	vertices.push_back(vec3(-1, 1, 1));		// 4
	vertices.push_back(vec3(1, -1, -1));	// 5
	vertices.push_back(vec3(1, -1, 1));		// 6
	vertices.push_back(vec3(1, 1, -1));		// 7
	vertices.push_back(vec3(1, 1, 1));		// 8

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

	setUniformMaterial({ vec3(1), vec3(1), vec3(1), 1 });
	computeFaceNormals();
	computeCenterPositions();
}