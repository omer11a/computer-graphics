#include "StdAfx.h"
#include "Polygon.h"
#include <functional>

bool ConvexPolygon::verifyTwin(ConvexPolygon * twin) const {
	return (
		(twin == NULL) ||
		((twin->vertices.size() == vertices.size()) && (twin->normals.empty() == normals.empty()))
	);
}

void ConvexPolygon::clearInterpolatedFeatures() {
	interpolatedVertices.clear();
	interpolatedMaterials.clear();
	interpolatedNormals.clear();
}

void ConvexPolygon::addInterpolatedFeatures(int i, int j, float t) {
	interpolatedVertices.push_back(interpolate(vertices[i], vertices[j], t));

	interpolatedMaterials.push_back({
		interpolate(materials[i].ambientReflectance, materials[j].ambientReflectance, t),
		interpolate(materials[i].specularReflectance, materials[j].specularReflectance, t),
		interpolate(materials[i].diffuseReflectance, materials[j].diffuseReflectance, t),
		interpolate(materials[i].shininess, materials[j].shininess, t)
	});

	if (!normals.empty()) {
		interpolatedNormals.push_back(normalize(interpolate(normals[i], normals[j], t)));
	}
}

void ConvexPolygon::setInterpolatedFeatures() {
	vertices = interpolatedVertices;
	materials = interpolatedMaterials;
	normals = interpolatedNormals;

	clearInterpolatedFeatures();
}

ConvexPolygon::ConvexPolygon(
	const vector<vec4> & vertices,
	const vector<Material>& materials,
	const vector<vec3>& normals
) :
	vertices(vertices), materials(materials), normals(normals),
	interpolatedVertices(), interpolatedMaterials(), interpolatedNormals()
{
	int numberOfVertices = vertices.size();

	if (numberOfVertices < 3) {
		throw invalid_argument("ConvexPolygon must have at least 3 vertices");
	}

	if (materials.size() != numberOfVertices) {
		throw invalid_argument("Missing materials");
	}

	if ((!normals.empty()) && (normals.size() != numberOfVertices)) {
		throw invalid_argument("Missing normals");
	}
}

ConvexPolygon::ConvexPolygon(
	const vector<vec3> & vertices,
	const vector<Material>& materials,
	const vector<vec3>& normals
) {
	vector<vec4> vertices4d;
	for (auto i = vertices.begin(); i != vertices.end(); ++i) {
		vertices4d.insert(vertices4d.begin(), vec4(*i));
	}

	ConvexPolygon(vertices4d, materials, normals);
}

void ConvexPolygon::transform(
	const mat4& transform,
	const mat3& normalTransform
) {
	for (vector<vec4>::iterator it = vertices.begin(); it != vertices.end(); ++it) {
		*it = transform * (*it);
	}

	for (vector<vec3>::iterator it = normals.begin(); it != normals.end(); ++it) {
		*it = normalize(normalTransform * (*it));
	}
}

void ConvexPolygon::divide() {
	for (vector<vec4>::iterator it = vertices.begin(); it != vertices.end(); ++it) {
		*it = convert4dTo3d(*it);
	}
}

void ConvexPolygon::getTriangles(vector<ConvexPolygon *>& triangles) const {
	int count = 0;

	if (vertices.empty()) {
		return;
	}

	try {
		for (unsigned int i = 1; i < vertices.size() - 1; ++i) {
			vector<vec4> triangleVertices;
			vector<Material> triangleMaterials;
			vector<vec3> triangleNormals;

			triangleVertices.push_back(vertices[0]);
			triangleVertices.push_back(vertices[i]);
			triangleVertices.push_back(vertices[i + 1]);
			triangleMaterials.push_back(materials[0]);
			triangleMaterials.push_back(materials[i]);
			triangleMaterials.push_back(materials[i + 1]);
			if (!normals.empty()) {
				triangleNormals.push_back(normals[0]);
				triangleNormals.push_back(normals[i]);
				triangleNormals.push_back(normals[i + 1]);
			}

			ConvexPolygon * triangle = new ConvexPolygon(triangleVertices, triangleMaterials, triangleNormals);
			try {
				triangles.push_back(triangle);
				++count;
			} catch (...) {
				delete triangle;
			}
		}
	} catch (...) {
		for (int i = 0; i < count; ++i) {
			ConvexPolygon * triangle = triangles.back();
			triangles.pop_back();
			delete triangle;
		}
	}
}

const vector<vec4>& ConvexPolygon::getVertices() const {
	return vertices;
}

const vector<vec3>& ConvexPolygon::getNormals() const {
	return normals;
}

const vector<Material>& ConvexPolygon::getMaterials() const {
	return materials;
}