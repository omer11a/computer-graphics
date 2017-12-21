#pragma once

#include "Light.h"
using namespace std;

class Shader {
	void updateLightTransform() const;

protected:
	const int VERTICES_NUMBER = 3;

	const vector<Light *> * lights;
	mat4 transform;
	vec3 cameraPosition;

	vec3 computeColor(
		const vec3& modelPosition,
		const vec3& normal,
		const Material& material
	) const;

public:
	Shader();
	void setLights(const vector<Light *> * lights);
	void setTransform(const mat4 & transform);
	void setCameraPosition(const vec3 & cameraPosition);

	virtual void setPolygon(
		const mat3& vertices,
		const PolygonMaterial& materials,
		const mat3& vertexNormals = mat3(),
		const vec3& faceNormal = vec3()
	) = 0;

	virtual vec3 getColor(const vec3& pixel) const = 0;
};

class FlatShader : public Shader {
	vec3 color;

public:
	FlatShader();

	void setPolygon(
		const mat3& vertices,
		const PolygonMaterial& materials,
		const mat3& vertexNormals = mat3(),
		const vec3& faceNormal = vec3()
	) override;

	vec3 getColor(const vec3& pixel) const override;
};

class InterpolatedShader : public Shader {
	mat3 vertices;
	float area;

protected:
	vec3 calculateBarycentricCoordinates(const vec3& position) const;

public:
	InterpolatedShader();

	void setPolygon(
		const mat3& vertices,
		const PolygonMaterial& materials,
		const mat3& vertexNormals = mat3(),
		const vec3& faceNormal = vec3()
	) override;
};

class GouraudShader : public InterpolatedShader {
	mat3 colorMatrix;

public:
	GouraudShader();

	void setPolygon(
		const mat3& vertices,
		const PolygonMaterial& materials,
		const mat3& vertexNormals = mat3(),
		const vec3& faceNormal = vec3()
	) override;

	vec3 getColor(const vec3& pixel) const override;
};

class PhongShader : public InterpolatedShader {
	mat3 ambientMatrix;
	mat3 specularMatrix;
	mat3 diffuseMatrix;
	vec3 shininessVector;
	mat3 normalMatrix;

public:
	PhongShader();

	void setPolygon(
		const mat3& vertices,
		const PolygonMaterial& materials,
		const mat3& vertexNormals = mat3(),
		const vec3& faceNormal = vec3()
	) override;

	vec3 getColor(const vec3& pixel) const override;
};