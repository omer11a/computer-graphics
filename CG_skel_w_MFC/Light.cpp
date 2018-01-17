#include "stdafx.h"
#include "Light.h"

Light::Light(const vec3& intensity) : intensity(intensity), transform(1) {}

Light::Light(const Light& light) : intensity(light.intensity), transform(light.transform) {}

void Light::setIntensity(const vec3& intensity) {
	this->intensity = intensity;
}

vec3 Light::getIntensity() const
{
	return intensity;
}

void Light::setTransform(const mat4 & transform) {
	this->transform = transform;
}

void Light::draw(BaseRenderer * r) const
{
	if (r == NULL) {
		throw invalid_argument("Renderer is null");
	}
}

AmbientLight::AmbientLight(const vec3& intensity) : Light(intensity) {}

AmbientLight::AmbientLight(const AmbientLight& light) : Light(light) {}

Light * AmbientLight::clone() const {
	return new AmbientLight(*this);
}

vec3 AmbientLight::computeColor(
	const vec3& modelPosition,
	const vec3& normal,
	const Material& material
) const {
	return material.ambientReflectance * intensity;
}

vec3 DirectionalLightSource::computeSpecularColor(
	const vec3& direction,
	const vec3& modelPosition,
	const vec3& normal,
	const Material& material
) const {
	float product = dot(direction, normal);
	if (product <= 0) {
		return vec3(0);
	}

	vec3 R = normalize(2 * product * normal - direction);
	vec3 V = normalize(-modelPosition);
	product = dot(R, V);
	if (product <= 0) {
		return vec3(0);
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
		return vec3(0);
	}

	return material.diffuseReflectance * product * intensity;
}

DirectionalLightSource::DirectionalLightSource(const vec3& intensity) :
	Light(intensity),
	modelTransform(1), worldTransform(1)
{}

DirectionalLightSource::DirectionalLightSource(const DirectionalLightSource& light) :
	Light(light),
	modelTransform(light.modelTransform), worldTransform(light.worldTransform)
{}

void DirectionalLightSource::transformInModel(const mat4 & transform) {
	if (glm::determinant(transform) == 0) {
		throw invalid_argument("Singular matrices cannot be performed as transformations on lights.");
	}

	modelTransform = transform * modelTransform;
}

void DirectionalLightSource::transformInWorld(const mat4 & transform) {
	if (glm::determinant(transform) == 0) {
		throw invalid_argument("Singular matrices cannot be performed as transformations on lights.");
	}

	worldTransform = transform * worldTransform;
}

vec3 DirectionalLightSource::computeColor(
	const vec3& modelPosition,
	const vec3& normal,
	const Material& material
) const {
	vec3 direction = getDirection(modelPosition);
	vec3 specularColor = computeSpecularColor(direction, modelPosition, normal, material);
	vec3 diffuseColor = computeDiffuseColor(direction, normal, material);
	return specularColor + diffuseColor;
}

void PointLightSource::updatePosition() {
	transformedPosition = convert4dTo3d(transform * worldTransform * modelTransform * vec4(position, 1));
}

vec3 PointLightSource::getDirection(const vec3& modelPosition) const {
	return normalize(transformedPosition - modelPosition);
}

PointLightSource::PointLightSource(const vec3& intensity, const vec3& position)
	: DirectionalLightSource(intensity), position(position), transformedPosition(position)
{}

PointLightSource::PointLightSource(const PointLightSource& light)
	: DirectionalLightSource(light), position(light.position), transformedPosition(light.transformedPosition)
{}

Light * PointLightSource::clone() const {
	return new PointLightSource(*this);
}

void PointLightSource::setTransform(const mat4 & transform) {
	DirectionalLightSource::setTransform(transform);
	updatePosition();
}

void PointLightSource::transformInModel(const mat4 & transform) {
	DirectionalLightSource::transformInModel(transform);
	updatePosition();
}

void PointLightSource::transformInWorld(const mat4 & transform) {
	DirectionalLightSource::transformInWorld(transform);
	updatePosition();
}

vec4 PointLightSource::GetPositionForShader() const
{
	return vec4(transformedPosition, 1);
}

void PointLightSource::draw(BaseRenderer * renderer) const
{
	if (renderer == NULL) {
		throw invalid_argument("Renderer is null");
	}

	renderer->SetObjectMatrices(worldTransform * modelTransform);
	renderer->DrawLight(intensity, position);

}

void ParallelLightSource::updateDirection() {
	transformedDirection = normalize(convert4dTo3d(transform * worldTransform * modelTransform) * direction);
}

vec3 ParallelLightSource::getDirection(const vec3& modelPosition) const {
	return -transformedDirection;
}

ParallelLightSource::ParallelLightSource(const vec3& intensity, const vec3& direction)
	: DirectionalLightSource(intensity), direction(direction), transformedDirection(direction)
{}

ParallelLightSource::ParallelLightSource(const ParallelLightSource& light) :
	DirectionalLightSource(light), direction(light.direction), transformedDirection(light.transformedDirection)
{}

Light * ParallelLightSource::clone() const {
	return new ParallelLightSource(*this);
}

void ParallelLightSource::setTransform(const mat4 & transform) {
	DirectionalLightSource::setTransform(transform);
	updateDirection();
}

void ParallelLightSource::transformInModel(const mat4 & transform) {
	DirectionalLightSource::transformInModel(transform);
	updateDirection();
}

void ParallelLightSource::transformInWorld(const mat4 & transform) {
	DirectionalLightSource::transformInWorld(transform);
	updateDirection();
}

vec4 ParallelLightSource::GetPositionForShader() const
{
	return vec4(-transformedDirection, 0);
}
