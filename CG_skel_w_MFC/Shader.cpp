#include "stdafx.h"
#include "Shader.h"

void Shader::updateLightTransform() const {
	if (lights != NULL) {
		for (Light * light : *lights) {
			light->setTransform(transform);
		}
	}
}

vec3 Shader::computeColor(
	const vec3& modelPosition,
	const vec3& normal,
	const Material& material
) const {
	vec3 color;
	if (lights != NULL) {
		for (Light * light : *lights) {
			color += light->computeColor(modelPosition, cameraPosition, normal, material);
		}
	}

	return color;
}

Shader::Shader() : lights(NULL), transform(), cameraPosition()
{}

void Shader::setLights(const vector<Light *> * lights) {
	if (lights == NULL) {
		throw invalid_argument("lights is NULL");
	}

	this->lights = lights;
	updateLightTransform();
}

void Shader::setTransform(const mat4& transform) {
	if (!transform.isInvertible()) {
		throw invalid_argument("Invertible transformation");
	}

	this->transform = transform;
	updateLightTransform();
}

void Shader::setCameraPosition(const vec3 & cameraPosition) {
	this->cameraPosition = cameraPosition;
}

FlatShader::FlatShader() : Shader(), color()
{}

void FlatShader::setPolygon(
	const mat3& vertices,
	const PolygonMaterial& materials,
	const mat3& vertexNormals,
	const vec3& faceNormal
) {
	vec3 sum = vec3();
	for (int i = 0; i < 3; ++i) {
		sum += computeColor(vertices[i], faceNormal, materials[i]);
	}

	color = sum / VERTICES_NUMBER;
}

vec3 FlatShader::getColor(const vec3& pixel) const {
	return color;
}

vec3 InterpolatedShader::calculateBarycentricCoordinates(const vec3& position) const {
	float areaPBC = length(cross(vertices[1] - position, vertices[2] - position));
	float areaPCA = length(cross(vertices[2] - position, vertices[0] - position));

	vec3 result;
	result.x = areaPBC / area;
	result.y = areaPCA / area;
	result.z = 1 - result.x - result.z;
	return result;
}

InterpolatedShader::InterpolatedShader() : Shader(), vertices(), area(1)
{}

void InterpolatedShader::setPolygon(
	const mat3& vertices,
	const PolygonMaterial& materials,
	const mat3& vertexNormals,
	const vec3& faceNormal
) {
	float areaABC = length(cross(vertices[1] - vertices[0], vertices[2] - vertices[0]));
	if (areaABC == 0) {
		throw invalid_argument("Vertices are on the same line");
	}

	this->vertices = vertices;
	this->area = areaABC;
}

GouraudShader::GouraudShader() : InterpolatedShader(), colorMatrix()
{}

void GouraudShader::setPolygon(
	const mat3& vertices,
	const PolygonMaterial& materials,
	const mat3& vertexNormals,
	const vec3& faceNormal
) {
	InterpolatedShader::setPolygon(vertices, materials, vertexNormals, faceNormal);

	mat3 colors;
	for (int i = 0; i < 3; ++i) {
		colors[i] = computeColor(vertices[i], vertexNormals[i], materials[i]);
	}

	colorMatrix = transpose(colors);
}

vec3 GouraudShader::getColor(const vec3& position) const {
	return colorMatrix * calculateBarycentricCoordinates(position);
}

PhongShader::PhongShader() :
	InterpolatedShader(),
	ambientMatrix(), specularMatrix(), diffuseMatrix(), shininessVector(),
	normalMatrix()
{}

void PhongShader::setPolygon(
	const mat3& vertices,
	const PolygonMaterial& materials,
	const mat3& vertexNormals,
	const vec3& faceNormal
) {
	InterpolatedShader::setPolygon(vertices, materials, vertexNormals, faceNormal);

	ambientMatrix = transpose(mat3(materials[0].ambientReflectance, materials[1].ambientReflectance, materials[2].ambientReflectance));
	specularMatrix = transpose(mat3(materials[0].specularReflectance, materials[1].specularReflectance, materials[2].specularReflectance));
	diffuseMatrix = transpose(mat3(materials[0].diffuseReflectance, materials[1].diffuseReflectance, materials[2].diffuseReflectance));
	shininessVector = vec3(materials[0].shininess, materials[1].shininess, materials[2].shininess);
	normalMatrix = transpose(vertexNormals);
}

vec3 PhongShader::getColor(const vec3& position) const {
	vec3 barycentricCoordinates = calculateBarycentricCoordinates(position);
	Material material = {
		ambientMatrix * barycentricCoordinates,
		specularMatrix * barycentricCoordinates,
		diffuseMatrix * barycentricCoordinates,
		dot(shininessVector, barycentricCoordinates)
	};

	vec3 normal = normalMatrix * barycentricCoordinates;
	return computeColor(position, normal, material);
}