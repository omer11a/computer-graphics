#pragma once

#include "MeshModel.h"
using namespace std;

class Light {
protected:
	vec3 intensity;

public:
	explicit Light(const vec3& intensity);
	Light(const Light& light);
	virtual Light * clone() const = 0;

	virtual vec3 computeColor(
		const mat4& transform,
		const vec3& modelPosition,
		const vec3& cameraPosition,
		const vec3& normal,
		const Material& material
	) const = 0;
};

class AmbientLight : public Light {
public:
	explicit AmbientLight(const vec3& intensity);
	AmbientLight(const AmbientLight& light);
	Light * clone() const override;

	vec3 computeColor(
		const mat4& transform,
		const vec3& modelPosition,
		const vec3& cameraPosition,
		const vec3& normal,
		const Material& material
	) const override;
};

class DirectionalLightSource : public Light {
private:
	vec3 computeSpecularColor(
		const vec3& direction,
		const vec3& modelPosition,
		const vec3& cameraPosition,
		const vec3& normal,
		const Material& material
	) const;

	vec3 computeDiffuseColor(
		const vec3& direction,
		const vec3& normal,
		const Material& material
	) const;

protected:
	mat4 modelTransform;
	mat4 worldTransform;

	virtual vec3 getDirection(const mat4& transform, const vec3& modelPosition) const = 0;

public:
	explicit DirectionalLightSource(const vec3& intensity);
	DirectionalLightSource(const DirectionalLightSource& light);
	void transformInModel(const mat4 & transform);
	void transformInWorld(const mat4 & transform);

	virtual vec3 computeColor(
		const mat4& transform,
		const vec3& modelPosition,
		const vec3& cameraPosition,
		const vec3& normal,
		const Material& material
	) const override;
};

class PointLightSource : public DirectionalLightSource {
	vec3 position;

protected:
	vec3 getDirection(const mat4& transform, const vec3& modelPosition) const override;

public:
	PointLightSource(const vec3& intensity, const vec3& position);
	PointLightSource(const PointLightSource& light);
	Light * clone() const override;
};

class ParallelLightSource : public DirectionalLightSource {
	vec3 direction;

protected:
	vec3 getDirection(const mat4& transform, const vec3& modelPosition) const override;

public:
	ParallelLightSource(const vec3& intensity, const vec3& direction);
	ParallelLightSource(const ParallelLightSource& light);
	Light * clone() const override;
};