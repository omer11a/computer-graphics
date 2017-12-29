#include "stdafx.h"
#include "Shader.h"

void DirectShader::updateLightTransform() const {
	if (lights != NULL) {
		for (Light * light : *lights) {
			light->setTransform(transform);
		}
	}
}

vec3 DirectShader::computeColor(
	const vec3& modelPosition,
	const vec3& normal,
	const Material& material
) const {
	vec3 color;
	if (lights != NULL) {
		for (Light * light : *lights) {
			color += light->computeColor(modelPosition, normal, material);
		}
	}

	return minvec(color, 1);
}

DirectShader::DirectShader() : Shader(), lights(NULL), transform()
{}

void DirectShader::setLights(const vector<Light *> * lights) {
	if (lights == NULL) {
		throw invalid_argument("lights is NULL");
	}

	this->lights = lights;
	updateLightTransform();
}

void DirectShader::setTransform(const mat4& transform) {
	if (!transform.isInvertible()) {
		throw invalid_argument("Invertible transformation");
	}

	this->transform = transform;
	updateLightTransform();
}

FlatShader::FlatShader() : DirectShader(), color()
{}

void FlatShader::setPolygon(
	const mat3& vertices,
	const PolygonMaterial& materials,
	const mat3& vertexNormals,
	const vec3& faceNormal
) {
	vec3 sum = vec3();
	for (int i = 0; i < VERTICES_NUMBER; ++i) {
		sum += computeColor(vertices[i], faceNormal, materials[i]);
	}

	color = sum / VERTICES_NUMBER;
}

vec3 FlatShader::getColor(const vec3& barycentricCoordinates) const {
	return color;
}

GouraudShader::GouraudShader() : DirectShader(), colorMatrix()
{}

void GouraudShader::setPolygon(
	const mat3& vertices,
	const PolygonMaterial& materials,
	const mat3& vertexNormals,
	const vec3& faceNormal
) {
	mat3 colors;
	for (int i = 0; i < 3; ++i) {
		colors[i] = computeColor(vertices[i], vertexNormals[i], materials[i]);
	}

	colorMatrix = transpose(colors);
}

vec3 GouraudShader::getColor(const vec3& barycentricCoordinates) const {
	return colorMatrix * barycentricCoordinates;
}

PhongShader::PhongShader() :
	DirectShader(),
	vertexMatrix(),
	ambientMatrix(), specularMatrix(), diffuseMatrix(), shininessVector(),
	normalMatrix()
{}

void PhongShader::setPolygon(
	const mat3& vertices,
	const PolygonMaterial& materials,
	const mat3& vertexNormals,
	const vec3& faceNormal
) {
	vertexMatrix = transpose(vertices);
	ambientMatrix = transpose(mat3(materials[0].ambientReflectance, materials[1].ambientReflectance, materials[2].ambientReflectance));
	specularMatrix = transpose(mat3(materials[0].specularReflectance, materials[1].specularReflectance, materials[2].specularReflectance));
	diffuseMatrix = transpose(mat3(materials[0].diffuseReflectance, materials[1].diffuseReflectance, materials[2].diffuseReflectance));
	shininessVector = vec3(materials[0].shininess, materials[1].shininess, materials[2].shininess);
	normalMatrix = transpose(vertexNormals);
}

vec3 PhongShader::getColor(const vec3& barycentricCoordinates) const {
	vec3 position = vertexMatrix * barycentricCoordinates;

	Material material = {
		ambientMatrix * barycentricCoordinates,
		specularMatrix * barycentricCoordinates,
		diffuseMatrix * barycentricCoordinates,
		dot(shininessVector, barycentricCoordinates)
	};

	vec3 normal = normalMatrix * barycentricCoordinates;
	return computeColor(position, normal, material);
}

LayeredShader::LayeredShader(Shader * parent) : parent(parent) {
	if (parent == NULL) {
		throw invalid_argument("parent is NULL");
	}
}

LayeredShader::~LayeredShader() {
	delete parent;
}

void LayeredShader::setLights(const vector<Light *> * lights) {
	parent->setLights(lights);
}

void LayeredShader::setTransform(const mat4 & transform) {
	parent->setTransform(transform);
}

void LayeredShader::setPolygon(
	const mat3& vertices,
	const PolygonMaterial& materials,
	const mat3& vertexNormals,
	const vec3& faceNormal
) {
	parent->setPolygon(vertices, materials, vertexNormals, faceNormal);
}

FogShader::FogShader(Shader * parent, const vec3& fogColor)
	: LayeredShader(parent), fogColor(fogColor), vertexMatrix()
{}

void FogShader::setPolygon(
	const mat3& vertices,
	const PolygonMaterial& materials,
	const mat3& vertexNormals,
	const vec3& faceNormal
) {
	LayeredShader::setPolygon(vertices, materials, vertexNormals, faceNormal);
	vertexMatrix = transpose(vertices);
}

vec3 FogShader::getColor(const vec3& barycentricCoordinates) const {
	vec3 position = vertexMatrix * position;
	vec3 lightColor = parent->getColor(position);
	float dist = length(position);
	float be = fabs(position.y) * 0.004;
	float bi = fabs(position.y) * 0.001;
	float ext = exp(-dist * be);
	float insc = exp(-dist * bi);
	return lightColor * ext + fogColor * (1 - insc);
}