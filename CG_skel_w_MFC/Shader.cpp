#include "stdafx.h"
#include "Shader.h"

vec3 Shader::computeColor(
	const vec3& modelPosition,
	const vec3& normal,
	const Material& material
) const {
	vec3 color;
	if (lights != NULL) {
		for (Light * light : *lights) {
			color += light->computeColor(transform, modelPosition, cameraPosition, normal, material);
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
}

void Shader::setTransform(const mat4& transform) {
	if (!transform.isInvertible()) {
		throw invalid_argument("Invertible transformation");
	}

	this->transform = transform;
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

GouraudShader::GouraudShader() : Shader(), positionToColorMatrix()
{}

void GouraudShader::setPolygon(
	const mat3& vertices,
	const PolygonMaterial& materials,
	const mat3& vertexNormals,
	const vec3& faceNormal
) {
	if (!vertices.isInvertible()) {
		throw invalid_argument("Vertices do not span a triangle");
	}

	mat3 barycentricMatrix = inverse(transpose(vertices));
	mat3 colors;
	for (int i = 0; i < 3; ++i) {
		colors[i] = computeColor(vertices[i], vertexNormals[i], materials[i]);
	}

	positionToColorMatrix = transpose(colors) * barycentricMatrix;
}

vec3 GouraudShader::getColor(const vec3& position) const {
	return positionToColorMatrix * position;
}

PhongShader::PhongShader() :
	Shader(),
	barycentricMatrix(),
	ambientMatrix(), specularMatrix(), diffuseMatrix(), shininessVector(),
	normalMatrix()
{}

void PhongShader::setPolygon(
	const mat3& vertices,
	const PolygonMaterial& materials,
	const mat3& vertexNormals,
	const vec3& faceNormal
) {
	if (!vertices.isInvertible()) {
		throw invalid_argument("Vertices do not span a triangle");
	}

	barycentricMatrix = inverse(transpose(vertices));
	ambientMatrix = transpose(mat3(materials[0].ambientReflectance, materials[1].ambientReflectance, materials[2].ambientReflectance));
	specularMatrix = transpose(mat3(materials[0].specularReflectance, materials[1].specularReflectance, materials[2].specularReflectance));
	diffuseMatrix = transpose(mat3(materials[0].diffuseReflectance, materials[1].diffuseReflectance, materials[2].diffuseReflectance));
	shininessVector = vec3(materials[0].shininess, materials[1].shininess, materials[2].shininess);
	normalMatrix = transpose(vertexNormals);
}

vec3 PhongShader::getColor(const vec3& position) const {
	vec3 barycentricCoordinates = barycentricMatrix * position;
	Material material = {
		ambientMatrix * barycentricCoordinates,
		specularMatrix * barycentricCoordinates,
		diffuseMatrix * barycentricCoordinates,
		dot(shininessVector, barycentricCoordinates)
	};

	vec3 normal = normalMatrix * barycentricCoordinates;
	return computeColor(position, normal, material);
}