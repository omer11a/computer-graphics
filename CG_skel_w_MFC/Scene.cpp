#include "stdafx.h"
#include "Scene.h"
#include "MeshModel.h"
#include <string>

using namespace std;

Camera::Camera() :
	viewTransform(), worldTransform(), inverseViewTransform(), inverseWorldTransform(), projection()
{}

void Camera::transformInView(const mat4& transform) {
	if (!transform.isInvertible()) {
		throw invalid_argument("Singular matrices cannot be performed as transformations on cameras.");
	}

	viewTransform = viewTransform * transform;
	inverseViewTransform = inverse(transform) * inverseViewTransform;
}

void Camera::transformInWorld(const mat4& transform) {
	if (!transform.isInvertible()) {
		throw invalid_argument("Singular matrices cannot be performed as transformations on cameras.");
	}

	worldTransform = worldTransform * transform;
	inverseWorldTransform = inverse(worldTransform) * inverseWorldTransform;
}

void Camera::lookAt(const vec4 & eye, const vec4 & at, const vec4 & up) {
	viewTransform = mat4();
	inverseViewTransform = viewTransform;

	vec4 n = normalize(eye - at);
	vec4 u = normalize(cross(up, n));
	vec4 v = normalize(cross(n, u));
	vec4 t = vec4(0, 0, 0, 1);
	mat4 c = mat4(u, v, n, t);
	worldTransform = Translate(eye) * transpose(c);
	inverseWorldTransform = c * Translate(-eye);
}

void Camera::ortho(
	const float left, const float right,
	const float bottom, const float top,
	const float zNear, const float zFar
) {
	mat4 t = Translate(-(right + left) / 2, -(bottom + top) / 2, (zNear + zFar) / 2);
	mat4 s = Scale(2 / (right - left), 2 / (top - bottom), 2 / (zNear - zFar));
	projection = s * t;
}

void Camera::frustum(
	const float left, const float right,
	const float bottom, const float top,
	const float zNear, const float zFar
) {
	mat4 h = mat4();
	h[0][2] = (left + right) / (-2 * zNear);
	h[1][2] = (top + bottom) / (-2 * zNear);

	mat4 s = Scale((-2 * zNear) / (right - left), (-2 * zNear) / (top - bottom), 1);

	mat4 n = mat4();
	n[2][2] = -((zNear + zFar) / (zNear - zFar));
	n[2][3] = -((2 * zNear * zFar) / (zNear - zFar));
	n[3][2] = -1;
	n[3][3] = 0;

	projection = n * s * h;
}

void Scene::loadOBJModel(string fileName)
{
	MeshModel *model = new MeshModel(fileName);
	models.push_back(model);
}

void Scene::draw()
{
	// 1. Send the renderer the current camera transform and the projection
	// 2. Tell all models to draw themselves

	m_renderer->SwapBuffers();
}

void Scene::drawDemo()
{
	m_renderer->SetDemoBuffer();
	m_renderer->SwapBuffers();
}
