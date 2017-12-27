#pragma once
#include <vector>
#include "vec.h"
#include "MeshModel.h"
using namespace std;

class Polygon {
	vector<vec4> vertices;
	vector<Material> materials;
	vector<vec3> normals;

	template<class T>
	T interpolate(T v1, T v2, float t) const;

	template<class Compare>
	bool clip(
		const vec4& v1,
		const vec4& v2,
		int coordinate,
		float max,
		Compare comp,
		vec4& start,
		vec4& end,
		float t
	) const;

	void addInterpolatedFeatures(
		vector<Material>& interpolatedMaterials,
		vector<vec3>& interpolatedNormals,
		int i,
		int j,
		float t
	) const;

public:
	Polygon(
		const vector<vec4> & vertices,
		const vector<Material>& materials,
		const vector<vec3>& normals
	);

	void transform(const mat4& transform, const mat3& normalTransform);
	void divide();

	template<class Compare>
	void clip(
		int coordinate,
		float max,
		Compare comp
	);

	void getTriangles(vector<Polygon *>& triangles) const;
	const vector<vec4>& getVertices() const;
	const vector<vec3>& getNormals() const;
	const vector<Material>& getMaterials() const;
};