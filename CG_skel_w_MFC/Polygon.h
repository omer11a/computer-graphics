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

	void verifyThis() const;

	template<class T>
	T interpolate(T v1, T v2, float t) const {
		return (1 - t) * v1 + t * v2;
	}

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
		float& t
	) const {
		start = v1;
		end = v2;
		t = 0;

		float y1 = v1[coordinate];
		float y2 = v2[coordinate];
		bool isV1InRange = comp(y1, max);
		bool isV2InRange = comp(y2, max);
		if (isV1InRange == isV2InRange) {
			return isV1InRange;
		}

		float dy = y2 - y1;
		ASSERT(dy != 0);
		t = (max - y1) / dy;
		vec4& p = v1 + (v2 - v1) * t;

		if (comp(y1, y2)) {
			end = p;
		} else {
			start = p;
		}

		return true;
	}

	void clearInterpolatedFeatures();
	void addInterpolatedFeatures(int i, int j, float t);
	void setInterpolatedFeatures();

public:
	ConvexPolygon() = delete;

	ConvexPolygon(
		const vector<vec4> & vertices,
		const vector<Material>& materials,
		const vector<vec3>& normals
	);

	ConvexPolygon(
		const vector<vec3> & vertices,
		const vector<Material>& materials,
		const vector<vec3>& normals
	);

	void transform(const mat4& transform, const mat3& normalTransform = mat3());
	void divide();

	template<class Compare>
	void clip(
		int coordinate,
		float max,
		Compare comp,
		ConvexPolygon * twin = NULL
	) {
		verifyTwin(twin);

		for (unsigned int i = 0; i < vertices.size(); ++i) {
			int j = (i + 1) % vertices.size();

			vec4 start;
			vec4 end;
			float t;
			if (!clip(vertices[i], vertices[j], coordinate, max, comp, start, end, t)) {
				continue;
			}

			if (start[coordinate] != vertices[i][coordinate]) {
				addInterpolatedFeatures(i, j, t);
				if (twin != NULL) {
					twin->addInterpolatedFeatures(i, j, t);
				}
			}

			t = (end[coordinate] == vertices[j][coordinate]) ? 1 : t;
			addInterpolatedFeatures(i, j, t);
			if (twin != NULL) {
				twin->addInterpolatedFeatures(i, j, t);
			}
		}

		setInterpolatedFeatures();
		if (twin != NULL) {
			twin->setInterpolatedFeatures();
		}
	}

	void getTriangles(vector<ConvexPolygon *>& triangles) const;
	const vector<vec4>& getVertices() const;
	const vector<vec3>& getNormals() const;
	const vector<Material>& getMaterials() const;
};