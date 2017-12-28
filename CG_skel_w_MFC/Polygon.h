#pragma once
#include <vector>
#include "vec.h"
#include "MeshModel.h"
using namespace std;

class ConvexPolygon {
	vector<vec4> vertices;
	vector<Material> materials;
	vector<vec3> normals;
	vector<vec4> interpolatedVertices;
	vector<vec3> interpolatedNormals;
	vector<Material> interpolatedMaterials;

	template<class T>
	T interpolate(T v1, T v2, float t) const;

	bool verifyTwin(ConvexPolygon * twin) const;

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

	void clearInterpolatedFeatures();
	void addInterpolatedFeatures(int i, int j, float t);
	void setInterpolatedFeatures();

public:
	ConvexPolygon(
		const vector<vec4> & vertices,
		const vector<Material>& materials,
		const vector<vec3>& normals
	);

	void transform(const mat4& transform);
	void transform(const mat4& transform, const mat3& normalTransform = mat3());
	void divide();

	template<class Compare>
	void clip(
		int coordinate,
		float max,
		Compare comp,
		ConvexPolygon * twin
	);

	void getTriangles(vector<ConvexPolygon *>& triangles) const;
	const vector<vec4>& getVertices() const;
	const vector<vec3>& getNormals() const;
	const vector<Material>& getMaterials() const;
};