#include "StdAfx.h"
#include "Polygon.h"
#include <functional>

template<class T>
T ConvexPolygon::interpolate(T v1, T v2, float t) const {
	return (1 - t) * v1 + t * v2;
}

template<class Compare>
bool ConvexPolygon::clip(
	const vec4& v1,
	const vec4& v2,
	int coordinate,
	float max,
	Compare comp,
	vec4& start,
	vec4& end,
	float t
) const {
	start = v1;
	end = v2;
	float y1 = v1[coordinate];
	float y2 = v2[coordinate];
	float dy = y2 - y1;
	if (dy == 0) {
		return comp(y1, max);
	}

	t = (max - y1) / dy;
	vec4& p = v1 + (v2 - v1) * t;
	float y3 = p[coordinate];
	bool y12 = comp(y1, y2);
	bool y13 = comp(y1, y3);
	bool y23 = comp(y2, y3);
	if ((comp(y1, y3)) && (comp(y2, y3))) {
		return true;
	}

	if (comp(y1, y2)) {
		end = p;
	} else {
		start = p;
	}

	return true;
}

void ConvexPolygon::addInterpolatedFeatures(
	vector<Material>& interpolatedMaterials,
	vector<vec3>& interpolatedNormals,
	int i,
	int j,
	float t
) const {
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

ConvexPolygon::ConvexPolygon(
	const vector<vec4> & vertices,
	const vector<Material>& materials,
	const vector<vec3>& normals
) : vertices(vertices), materials(materials), normals(normals) {
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

template<class Compare>
void ConvexPolygon::clip(
	int coordinate,
	float max,
	Compare comp
) {
	vector<vec4> clippedVertices;
	vector<vec3> interpolatedNormals;
	vector<Material> interpolatedMaterials;

	for (unsigned int i = 0; i < vertices.size(); ++i) {
		int j = (i + 1) % vertices.size();

		vec4 start;
		vec4 end;
		float t;
		clip(vertices[i], vertices[j], coordinate, max, comp, start, end, t);

		if (start[coordinate] != vertices[i][coordinate]) {
			clippedVertices.push_back(start);
			addInterpolatedFeatures(interpolatedMaterials, interpolatedNormals, i, j, t);
		}

		clippedVertices.push_back(end);
		if (end[coordinate] == vertices[j][coordinate]) {
			interpolatedMaterials.push_back(materials[j]);
			interpolatedNormals.push_back(normals[j]);
		}
		else {
			addInterpolatedFeatures(interpolatedMaterials, interpolatedNormals, i, j, t);
		}
	}

	vertices = clippedVertices;
	materials = interpolatedMaterials;
	normals = interpolatedNormals;
}

void ConvexPolygon::getTriangles(vector<ConvexPolygon *>& triangles) const {
	int count = 0;

	try {
		for (int i = 1; i < vertices.size() - 1; ++i) {
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