#pragma once

#include "MeshModel.h"
#include "BaseRenderer.h"
using namespace std;

class Light {
protected:
	vec3 intensity;
	mat4 transform;

public:
	explicit Light(const vec3& intensity);
	Light(const Light& light);
	virtual Light * clone() const = 0;

	void setIntensity(const vec3& intensity);
	vec3 getIntensity() const;
	virtual void setTransform(const mat4 & transform);
	virtual void draw(BaseRenderer * r) const;

	virtual vec3 computeColor(
		const vec3& modelPosition,
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
		const vec3& modelPosition,
		const vec3& normal,
		const Material& material
	) const override;
};

class DirectionalLightSource : public Light {
private:
	vec3 computeSpecularColor(
		const vec3& direction,
		const vec3& modelPosition,
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

	virtual vec3 getDirection(const vec3& modelPosition) const = 0;

public:
	explicit DirectionalLightSource(const vec3& intensity);
	DirectionalLightSource(const DirectionalLightSource& light);
	virtual void transformInModel(const mat4 & transform);
	virtual void transformInWorld(const mat4 & transform);
	virtual vec4 GetPositionForShader() const = 0;

	virtual vec3 computeColor(
		const vec3& modelPosition,
		const vec3& normal,
		const Material& material
	) const override;
};

class PointLightSource : public DirectionalLightSource {
	vec3 position;
	vec3 transformedPosition;

	void updatePosition();

protected:
	vec3 getDirection(const vec3& modelPosition) const override;

public:
	PointLightSource(const vec3& intensity, const vec3& position);
	PointLightSource(const PointLightSource& light);
	Light * clone() const override;
	void setTransform(const mat4 & transform);
	void transformInModel(const mat4 & transform);
	void transformInWorld(const mat4 & transform);
	vec4 GetPositionForShader() const override;
	void draw(BaseRenderer * renderer) const override;
};

class ParallelLightSource : public DirectionalLightSource {
	vec3 direction;
	vec3 transformedDirection;

	void updateDirection();

protected:
	vec3 getDirection(const vec3& modelPosition) const override;

public:
	ParallelLightSource(const vec3& intensity, const vec3& direction);
	ParallelLightSource(const ParallelLightSource& light);
	Light * clone() const override;
	void setTransform(const mat4 & transform);
	void transformInModel(const mat4 & transform);
	void transformInWorld(const mat4 & transform);
	vec4 GetPositionForShader() const override;

};