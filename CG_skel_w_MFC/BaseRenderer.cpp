#pragma once
#include "stdafx.h"
#include "BaseRenderer.h"

using namespace std;
BaseRenderer::BaseRenderer() : m_width(512), m_height(512) {
	min_size = min(m_width, m_height) / 2;
}

BaseRenderer::BaseRenderer(int width, int height) : m_width(width), m_height(height)
{
	min_size = min(m_width, m_height) / 2;
}

void BaseRenderer::SetObjectMatrices(const mat4 & oTransform, const mat3 & nTransform) { }

void BaseRenderer::UpdateBuffers(int width, int height) { }

void BaseRenderer::SwapBuffers() { }

void BaseRenderer::ClearColorBuffer() { }

void BaseRenderer::SetDemoBuffer() { }

void BaseRenderer::DrawTriangles(
	const vector<vec3>* vertices,
	const vector<Material>* materials,
	const vector<vec3>* centerPositions,
	const bool hasTexture,
	const GLuint textureID,
	const bool hasNormalMap,
	const GLuint normalMapID,
	const vector<vec2>* textureCoordinates,
	const vector<vec2>* textureCenters,
	const vector<vec3>* vertexNormals,
	const vector<vec3>* faceNormals) { }

void BaseRenderer::DrawModelNormals(
	const vector<vec3>* vertices,
	const vector<vec3>* centerPositions,
	const vector<vec3>* vertexNormals,
	const vector<vec3>* faceNormals) { }

void BaseRenderer::DrawBox(const vec3 & minValues, const vec3 & maxValues) { }

void BaseRenderer::DrawCamera() { }

void BaseRenderer::DrawLight(const vec3& color, const vec3 & position) { }

void BaseRenderer::SetCameraTransform(const mat4& cInverseTransform, const mat4& cTransform) { }

void BaseRenderer::SetProjection(const mat4 & projection) { }

void BaseRenderer::SetZRange(float zNear, float zFar) { }
