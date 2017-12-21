#include "stdafx.h"
#include "Light.h"

Light::Light(const vec3& intensity) : intensity(intensity) {}

Light::Light(const Light& light) : intensity(light.intensity) {}

AmbientLight::AmbientLight(const vec3& intensity) : Light(intensity) {}

AmbientLight::AmbientLight(const AmbientLight& light) : Light(light) {}

Light * AmbientLight::clone() const {
	return new AmbientLight(*this);
}

vec3 AmbientLight::computeColor(
	const mat4& transform,
	const vec3& modelPosition,
	const vec3& cameraPosition,
	const vec3& normal,
	const Material& material
) const {
	return material.ambientReflectance * intensity;
}

vec3 DirectionalLightSource::computeSpecularColor(
	const vec3& direction,
	const vec3& modelPosition,
	const vec3& cameraPosition,
	const vec3& normal,
	const Material& material
) const {
	vec3 R = 2 * dot(direction, normal) * normal - direction;
	vec3 V = normalize(cameraPosition - modelPosition);
	float product = dot(R, V);
	if (product <= 0) {
		return 0;
	}

	return material.specularReflectance * pow(product, material.shininess) * intensity;
}

vec3 DirectionalLightSource::computeDiffuseColor(
	const vec3& direction,
	const vec3& normal,
	const Material& material
) const {
	float product = dot(direction, normal);
	if (product <= 0) {
		return 0;
	}

	return material.diffuseReflectance * product * intensity;
}

DirectionalLightSource::DirectionalLightSource(const vec3& intensity) :
	Light(intensity),
	modelTransform(), worldTransform()
{}

DirectionalLightSource::DirectionalLightSource(const DirectionalLightSource& light) :
	Light(light),
	modelTransform(light.modelTransform), worldTransform(light.worldTransform)
{}

void DirectionalLightSource::transformInModel(const mat4 & transform) {
	if (!transform.isInvertible()) {
		throw invalid_argument("Singular matrices cannot be performed as transformations on lights.");
	}

	modelTransform = transform * modelTransform;
}

void DirectionalLightSource::transformInWorld(const mat4 & transform) {
	if (!transform.isInvertible()) {
		throw invalid_argument("Singular matrices cannot be performed as transformations on lights.");
	}

	worldTransform = transform * worldTransform;
}

vec3 DirectionalLightSource::computeColor(
	const mat4& transform,
	const vec3& modelPosition,
	const vec3& cameraPosition,
	const vec3& normal,
	const Material& material
) const {
	vec3 direction = getDirection(transform, modelPosition);
	vec3 specularColor = computeSpecularColor(direction, modelPosition, cameraPosition, normal, material);
	vec3 diffuseColor = computeDiffuseColor(direction, normal, material);
	return specularColor + diffuseColor;
}

vec3 PointLightSource::getDirection(
	const mat4& transform,
	const vec3& modelPosition
) const {
	return convert4dTo3d(transform * worldTransform * modelTransform * position) - modelPosition;
}

PointLightSource::PointLightSource(const vec3& intensity, const vec3& position)
	: DirectionalLightSource(intensity), position(position)
{}

PointLightSource::PointLightSource(const PointLightSource& light)
	: DirectionalLightSource(light), position(light.position)
{}

Light * PointLightSource::clone() const {
	return new PointLightSource(*this);
}

vec3 ParallelLightSource::getDirection(
	const mat4& transform,
	const vec3& modelPosition
) const {
	return convert4dTo3d(transform * worldTransform * modelTransform * direction);
}

ParallelLightSource::ParallelLightSource(const vec3& intensity, const vec3& direction)
	: DirectionalLightSource(intensity), direction(direction)
{}

ParallelLightSource::ParallelLightSource(const ParallelLightSource& light) :
	DirectionalLightSource(light), direction(light.direction)
{}

Light * ParallelLightSource::clone() const {
	return new ParallelLightSource(*this);
}