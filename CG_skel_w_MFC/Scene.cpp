#include "stdafx.h"
#include "Scene.h"
#include <string>

using namespace std;

void Camera::verifyProjectionParameters(
	const float left, const float right,
	const float bottom, const float top,
	const float zNear, const float zFar
) {
	if ((left >= right) || (bottom >= top) || (zNear >= zFar)) {
		throw invalid_argument("Invalid projection arguments");
	}
}

void Camera::verifyProjectionParameters(
	const float fov, const float aspect,
	const float zNear, const float zFar
) {
	if ((fov <= 0) || (fov >= 180) || (aspect <= 0) || (zNear >= zFar)) {
		throw invalid_argument("Invalid projection arguments");
	}
}

void Camera::updateProjection() {
	if (isPerspective) {
		frustum(left, right, bottom, top, zNear, zFar);
	} else {
		ortho(left, right, bottom, top, zNear, zFar);
	}
}

Camera::Camera() :
	isVisible(true), isPerspective(true),
	viewTransform(), worldTransform(), inverseViewTransform(), inverseWorldTransform()
{
	frustum(-5, 5, -5, 5, 1, 5);
}

void Camera::transformInView(const mat4& transform) {
	if (!transform.isInvertible()) {
		throw invalid_argument("Singular matrices cannot be performed as transformations on cameras.");
	}

	viewTransform = transform * viewTransform;
	inverseViewTransform = inverseViewTransform * inverse(transform);
}

void Camera::transformInWorld(const mat4& transform) {
	if (!transform.isInvertible()) {
			throw invalid_argument("Singular matrices cannot be performed as transformations on cameras.");
	}

	worldTransform = transform * worldTransform;
	inverseWorldTransform = inverseWorldTransform * inverse(transform);
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
	verifyProjectionParameters(left, right, bottom, top, zNear, zFar);

	this->left = left;
	this->right = right;
	this->bottom = bottom;
	this->top = top;
	this->zNear = zNear;
	this->zFar = zFar;

	mat4 t = Translate(-(right + left) / 2, -(bottom + top) / 2, (zNear + zFar) / 2);
	mat4 s = Scale(2 / (right - left), 2 / (top - bottom), 2 / (zNear - zFar));
	projection = s * t;
	isPerspective = false;
}

void Camera::frustum(
	const float left, const float right,
	const float bottom, const float top,
	const float zNear, const float zFar
) {
	verifyProjectionParameters(left, right, bottom, top, zNear, zFar);

	this->left = left;
	this->right = right;
	this->bottom = bottom;
	this->top = top;
	this->zNear = zNear;
	this->zFar = zFar;

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
	isPerspective = true;
}

void Camera::perspectiveVertical(
	const float fovy, const float aspect,
	const float zNear, const float zFar
) {
	verifyProjectionParameters(fovy, aspect, zNear, zFar);

	float fov = (M_PI / 180.0f) * fovy;
	float tangent = tanf(fov / 2.0f);
	float height = zNear * tangent;
	float width = height * aspect;
	frustum(-width, width, -height, height, zNear, zFar);
}

void Camera::perspectiveHorizontal(
	const float fovx, const float aspect,
	const float zNear, const float zFar
) {
	verifyProjectionParameters(fovx, aspect, zNear, zFar);

	float fov = (M_PI / 180.0f) * fovx;
	float fovy = 2.0f * atanf(tanf(fov * 0.5f) / aspect);
	perspectiveVertical(fovy, aspect, zNear, zFar);
}

void Camera::zoomIn(const float z) {
	zNear += z;
	updateProjection();
}

void Camera::zoomOut(const float z) {
	zNear -= z;
	updateProjection();
}

void Camera::setVisibility(bool shouldBeVisible) {
	isVisible = shouldBeVisible;
}

mat4 Camera::getInverseTransform() const {
	return inverseViewTransform * inverseWorldTransform;
}

mat4 Camera::getProjection() const {
	return projection;
}

void Camera::draw(Renderer * renderer) const {
	if (renderer == NULL) {
		throw invalid_argument("Renderer is null");
	}

	if (isVisible) {
		renderer->SetObjectMatrices(worldTransform * viewTransform);
		renderer->DrawCamera();
	}
}

Scene::Scene(Renderer * renderer, const vec3 & eye) :
	activeModel(-1), activeLight(-1), activeCamera(-1),
	renderer(renderer)
{
	if (renderer == NULL) {
		throw invalid_argument("Renderer is null");
	}

	addCamera();
	
	try {
		cameras.at(0)->transformInWorld(Translate(eye));
	} catch (...) {
		this->~Scene();
	}
}

Scene::~Scene() {
	while (!models.empty()) {
		MeshModel * model = models.back();
		models.pop_back();
		delete model;
	}

	while (!lights.empty()) {
		Light * light = lights.back();
		lights.pop_back();
		delete light;
	}

	while (!cameras.empty()) {
		Camera * camera = cameras.back();
		cameras.pop_back();
		delete camera;
	}
}

void Scene::loadOBJModel(string fileName)
{
	MeshModel * model = NULL;
	int newActiveModel = models.size();

	try {
		model = new MeshModel(fileName);
		models.push_back(model);
		activeModel = newActiveModel;

	} catch (...) {
		if (model != NULL) {
			delete model;
		}

		throw;
	}
}

void Scene::addCamera() {
	Camera * camera = NULL;
	int newActiveCamera = cameras.size();

	try {
		camera = new Camera();
		cameras.push_back(camera);
		activeCamera = newActiveCamera;

	} catch (...) {
		if (camera != NULL) {
			delete camera;
		}

		throw;
	}
}

MeshModel * Scene::getActiveModel() {
	if (activeModel < 0) {
		throw invalid_argument("No active model");
	}

	return models.at(activeModel);
}

Camera * Scene::getActiveCamera() {
	if (activeCamera < 0) {
		throw invalid_argument("No active camera");
	}

	return cameras.at(activeCamera);
}

size_t Scene::getNumberOfModels()
{
	return models.size();
}

size_t Scene::getNumberOfCameras()
{
	return cameras.size();
}

void Scene::setActiveModel(int i) {
	if ((i < 0) || (i >= (signed int) models.size())) {
		throw invalid_argument("No such model");
	}

	activeModel = i;
}

void Scene::setActiveCamera(int i) {
	if ((i < 0) || (i >= (signed int) cameras.size())) {
		throw invalid_argument("No such camera");
	}

	activeCamera = i;
}

void Scene::prevCamera()
{
	if (cameras.size() != 0) {
		--activeCamera;
		if (activeCamera < 0) {
			activeCamera = cameras.size() - 1;
		}
	}
}

void Scene::nextCamera()
{
	if (cameras.size() != 0) {
		++activeCamera;
		if (activeCamera == cameras.size()) {
			activeCamera = 0;
		}
	}
}

void Scene::removeActiveModel() {
	if (activeModel < 0) {
		throw invalid_argument("No active model");
	}

	MeshModel * model = models.at(activeModel);
	models.erase(models.begin() + activeModel);
	activeModel = models.empty() ? -1 : 0;
	delete model;
}
void Scene::removeActiveCamera() {
	if (activeCamera < 0) {
		throw invalid_argument("No active camera");
	}

	if (activeCamera == 0) {
		throw invalid_argument("Can't remove main camera");
	}

	Camera * camera = cameras.at(activeCamera);
	cameras.erase(cameras.begin() + activeCamera);
	activeCamera = cameras.empty() ? -1 : 0;
	delete camera;
}

void Scene::clear() {
	while (cameras.size() >= 2) {
		cameras.pop_back();
	}

	models.clear();
}

void Scene::draw() const {
	if (activeCamera < 0) {
		throw invalid_argument("No active camera");
	}

	// 1. Send the renderer the current camera transform and the projection
	renderer->SetCameraTransform(cameras.at(activeCamera)->getInverseTransform());
	renderer->SetProjection(cameras.at(activeCamera)->getProjection());

	// 2. Tell all models to draw themselves
	for (MeshModel * model : models) {
		model->draw(renderer);
	}

	for (int i = 0; i < (signed int) cameras.size(); ++i) {
		if (i != activeCamera) {
			cameras.at(i)->draw(renderer);
		}
	}

	renderer->SwapBuffers();
}

void Scene::drawDemo() const {
	renderer->SetDemoBuffer();
	renderer->SwapBuffers();
}
