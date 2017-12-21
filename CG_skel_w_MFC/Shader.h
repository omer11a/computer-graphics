#pragma once

#include "Light.h"
using namespace std;

class Shader {
protected:
	const int VERTICES_NUMBER = 3;

public:
	Shader() = default;
	virtual void setLights(const vector<Light *> * lights) = 0;
	virtual void setTransform(const mat4 & transform) = 0;

	virtual void setPolygon(
		const mat3& vertices,
		const PolygonMaterial& materials,
		const mat3& vertexNormals = mat3(),
		const vec3& faceNormal = vec3()
	) = 0;

	virtual vec3 getColor(const vec3& position) const = 0;
};

class DirectShader : public Shader {
	void updateLightTransform() const;

protected:
	const vector<Light *> * lights;
	mat4 transform;

	vec3 computeColor(
		const vec3& modelPosition,
		const vec3& normal,
		const Material& material
	) const;

public:
	DirectShader();
	void setLights(const vector<Light *> * lights) override;
	void setTransform(const mat4 & transform) override;
};

class FlatShader : public DirectShader {
	vec3 color;

public:
	FlatShader();

	void setPolygon(
		const mat3& vertices,
		const PolygonMaterial& materials,
		const mat3& vertexNormals = mat3(),
		const vec3& faceNormal = vec3()
	) override;

	vec3 getColor(const vec3& position) const override;
};

class InterpolatedShader : public DirectShader {
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

	vec3 getColor(const vec3& position) const override;
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

	vec3 getColor(const vec3& position) const override;
};

class LayeredShader : public Shader {
protected:
	Shader * parent;

public:
	LayeredShader() = delete;
	LayeredShader(Shader * parent);
	~LayeredShader();
	void setLights(const vector<Light *> * lights) override;
	void setTransform(const mat4 & transform) override;

	void setPolygon(
		const mat3& vertices,
		const PolygonMaterial& materials,
		const mat3& vertexNormals = mat3(),
		const vec3& faceNormal = vec3()
	) override;
};

class FogShader : public LayeredShader {
	vec3 fogColor;

public:
	FogShader(Shader * parent, const vec3& fogColor);
	~FogShader() = default;
	vec3 getColor(const vec3& position) const override;
};