#include "stdafx.h"
#include "lodepng_wrapper.h"
#include "Scene.h"
#include "GL\glew.h"
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
	viewTransform(1), worldTransform(1), inverseViewTransform(1), inverseWorldTransform(1)
{
	frustum(-5, 5, -5, 5, 5, 50);
}

void Camera::transformInView(const mat4& transform) {
	
	if (determinant(transform) == 0) {
		throw invalid_argument("Singular matrices cannot be performed as transformations on cameras.");
	}

	viewTransform = transform * viewTransform;
	inverseViewTransform = inverseViewTransform * inverse(transform);
}

void Camera::transformInWorld(const mat4& transform) {
	if (determinant(transform) == 0) {
			throw invalid_argument("Singular matrices cannot be performed as transformations on cameras.");
	}

	worldTransform = transform * worldTransform;
	inverseWorldTransform = inverseWorldTransform * inverse(transform);
}

void Camera::lookAt(const vec3 & eye, const vec3 & at, const vec3 & up) {
	viewTransform = mat4(1);
	inverseViewTransform = viewTransform;

	inverseWorldTransform = glm::lookAt(eye, at, up);
	worldTransform = inverse(worldTransform);
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

	//mat4 t = Translate(-(right + left) / 2, -(bottom + top) / 2, (zNear + zFar) / 2);
	//mat4 s = Scale(2 / (right - left), 2 / (top - bottom), 2 / (zNear - zFar));
	//mat4 m = Scale(1, 1, 0);
	//projection = m * s * t;
	projection = glm::ortho(left, right, bottom, top, zNear, zFar);
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

	//projection = mat4();
	//projection[0][0] = 2 * zNear / (right - left);
	//projection[0][2] = (right + left) / (right - left);
	//projection[1][1] = 2 * zNear / (top - bottom);
	//projection[1][2] = (top + bottom) / (top - bottom);
	//projection[2][2] = -(zFar + zNear) / (zNear - zFar);
	//projection[2][3] = -2 * zFar * zNear / (zNear - zFar);
	//projection[3][2] = -1;
	projection = glm::frustum(left, right, bottom, top, zNear, zFar);
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
	//projection = glm::perspective(fovy, aspect, zNear, zFar);
}

void Camera::zoom(const float z) {
	if ((zNear + z > 0) && (zNear + z < zFar)) {
		zNear += z;
		updateProjection();
	}
}

void Camera::switchVisibility()
{
	isVisible = !isVisible;
}

mat4 Camera::getInverseTransform() const {
	return inverseViewTransform * inverseWorldTransform;
}

mat4 Camera::getTransform() const {
	return worldTransform * viewTransform;
}

mat4 Camera::getProjection() const {
	return projection;
}

float Camera::getNear() const {
	return zNear;
}

float Camera::getFar() const {
	return zFar;
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

bool Scene::loadEnviromentSideTexture(GLenum side, CString filePath)
{
	unsigned int width, height;
	unsigned char * pixel_array;

	if (!readPng((LPCTSTR)filePath, &width, &height, &pixel_array)) {
		return false;
	}
	glBindTexture(GL_TEXTURE_CUBE_MAP, skyCubeTexture);
	glTexImage2D(side, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixel_array);
	delete[] pixel_array;
	return true;
}

void Scene::computeSkyCube()
{
	// top
	enviromentCube.push_back(vec3(-10.0f, 10.0f, -10.0f));
	enviromentCube.push_back(vec3(10.0f, 10.0f, -10.0f));
	enviromentCube.push_back(vec3(10.0f, 10.0f, 10.0f));
	enviromentCube.push_back(vec3(10.0f, 10.0f, 10.0f));
	enviromentCube.push_back(vec3(-10.0f, 10.0f, 10.0f));
	enviromentCube.push_back(vec3(-10.0f, 10.0f, -10.0f));
	// bottom
	enviromentCube.push_back(vec3(-10.0f, -10.0f, -10.0f));
	enviromentCube.push_back(vec3(-10.0f, -10.0f, 10.0f));
	enviromentCube.push_back(vec3(10.0f, -10.0f, -10.0f));
	enviromentCube.push_back(vec3(10.0f, -10.0f, -10.0f));
	enviromentCube.push_back(vec3(-10.0f, -10.0f, 10.0f));
	enviromentCube.push_back(vec3(10.0f, -10.0f, 10.0f));
	// left
	enviromentCube.push_back(vec3(-10.0f, -10.0f, 10.0f));
	enviromentCube.push_back(vec3(-10.0f, -10.0f, -10.0f));
	enviromentCube.push_back(vec3(-10.0f, 10.0f, -10.0f));
	enviromentCube.push_back(vec3(-10.0f, 10.0f, -10.0f));
	enviromentCube.push_back(vec3(-10.0f, 10.0f, 10.0f));
	enviromentCube.push_back(vec3(-10.0f, -10.0f, 10.0f));
	// right
	enviromentCube.push_back(vec3(10.0f, -10.0f, -10.0f));
	enviromentCube.push_back(vec3(10.0f, -10.0f, 10.0f));
	enviromentCube.push_back(vec3(10.0f, 10.0f, 10.0f));
	enviromentCube.push_back(vec3(10.0f, 10.0f, 10.0f));
	enviromentCube.push_back(vec3(10.0f, 10.0f, -10.0f));
	enviromentCube.push_back(vec3(10.0f, -10.0f, -10.0f));
	// front
	enviromentCube.push_back(vec3(-10.0f, -10.0f, 10.0f));
	enviromentCube.push_back(vec3(-10.0f, 10.0f, 10.0f));
	enviromentCube.push_back(vec3(10.0f, 10.0f, 10.0f));
	enviromentCube.push_back(vec3(10.0f, 10.0f, 10.0f));
	enviromentCube.push_back(vec3(10.0f, -10.0f, 10.0f));
	enviromentCube.push_back(vec3(-10.0f, -10.0f, 10.0f));
	// back
	enviromentCube.push_back(vec3(-10.0f, 10.0f, -10.0f));
	enviromentCube.push_back(vec3(-10.0f, -10.0f, -10.0f));
	enviromentCube.push_back(vec3(10.0f, -10.0f, -10.0f));
	enviromentCube.push_back(vec3(10.0f, -10.0f, -10.0f));
	enviromentCube.push_back(vec3(10.0f, 10.0f, -10.0f));
	enviromentCube.push_back(vec3(-10.0f, 10.0f, -10.0f));
}

Scene::Scene(
	Renderer * renderer,
	const vec3& eye,
	const AmbientLight& ambientLight
) :
	activeModel(-1), activeLight(-1), activeCamera(-1),
	renderer(renderer),
	ambientLight(ambientLight),
	hasSkyBox(false),
	enviromentCube()
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

	if (hasSkyBox) {
		glDeleteTextures(1, &skyCubeTexture);
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

void Scene::addPrimitive(int id)
{
	MeshModel * model = NULL;
	int newActiveModel = models.size();

	try {
		switch (id) {
		case 4:
			model = new PrimMeshModel();
			break;
		default:
			return;
		}

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

void Scene::setAmbientLight(const AmbientLight& ambientLight) {
	this->ambientLight = ambientLight;
}

void Scene::addLight(const DirectionalLightSource& light) {
	DirectionalLightSource * newLight = NULL;
	int newActiveLight = lights.size();

	try {
		newLight = dynamic_cast<DirectionalLightSource*>(light.clone());
		lights.push_back(newLight);
		activeLight = newActiveLight;

	} catch (...) {
		if (newLight != NULL) {
			delete newLight;
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

DirectionalLightSource * Scene::getActiveLight() {
	if (activeLight < 0) {
		throw invalid_argument("No active light");
	}

	return lights.at(activeLight);
}

size_t Scene::getNumberOfModels() const {
	return models.size();
}

size_t Scene::getNumberOfCameras() const {
	return cameras.size();
}

size_t Scene::getNumberOfLights() const {
	return lights.size();
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

void Scene::setActiveLight(int i) {
	if ((i < 0) || (i >= (signed int) lights.size())) {
		throw invalid_argument("No such light");
	}

	activeLight = i;
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

void Scene::stepAnimations(float timeDelta)
{
	for (MeshModel * m : models) {
		m->stepAnimation(timeDelta);
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

	if (cameras.size() < 2) {
		throw invalid_argument("Can't remove all cameras");
	}

	Camera * camera = cameras.at(activeCamera);
	cameras.erase(cameras.begin() + activeCamera);
	activeCamera = cameras.empty() ? -1 : 0;
	delete camera;
}

void Scene::removeActiveLight() {
	if (activeLight < 0) {
		throw invalid_argument("No active camera");
	}

	Light * light = lights.at(activeLight);
	lights.erase(lights.begin() + activeLight);
	activeLight = lights.empty() ? -1 : 0;
	delete light;
}

void Scene::clear() {
	while (cameras.size() >= 2) {
		cameras.pop_back();
	}

	models.clear();

	activeModel = -1;
	activeCamera = 0;
}

void Scene::loadSkyBox(
	const CString topPath, const CString bottomPath,
	const CString leftPath, const CString rightPath,
	const CString frontPath, const CString backPath) {
	if (enviromentCube.size() == 0) {
		computeSkyCube();
	}

	glActiveTexture(GL_TEXTURE0);
	glGenTextures(1, &skyCubeTexture);
	hasSkyBox = true;

	// load each image and copy into a side of the cube-map texture
	loadEnviromentSideTexture(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, topPath);
	loadEnviromentSideTexture(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, bottomPath);
	loadEnviromentSideTexture(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, leftPath);
	loadEnviromentSideTexture(GL_TEXTURE_CUBE_MAP_POSITIVE_X, rightPath);
	loadEnviromentSideTexture(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, frontPath);
	loadEnviromentSideTexture(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, backPath);
	// format cube map texture
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

void Scene::unloadSkyBox()
{
	hasSkyBox = false;
}

void Scene::draw() const {
	if (activeCamera < 0) {
		throw invalid_argument("No active camera");
	}

	// 0. Send the renderer the current lights.
	renderer->SetShaderLights(ambientLight, lights);

	// 1. Send the renderer the current camera transform and the projection
	Camera * camera = cameras.at(activeCamera);
	renderer->SetCameraTransform(camera->getInverseTransform(), camera->getTransform());
	renderer->SetProjection(camera->getProjection());

	// 1.5 Draw the Enviroment if defined
	if (hasSkyBox) {
		renderer->EnableSkyBox(skyCubeTexture);
		renderer->DrawSkyBox(&enviromentCube);
	}

	// 2. Tell all models to draw themselves
	for (MeshModel * model : models) {
		model->draw(renderer);
	}

	// 3. Tell all models to draw their normals
	for (MeshModel * model : models) {
		model->drawNormals(renderer);
	}

	// 4. Tell all cameras to draw themselves
	for (int i = 0; i < (signed int) cameras.size(); ++i) {
		if (i != activeCamera) {
			cameras.at(i)->draw(renderer);
		}
	}

	// 5. Tell all lights to draw themselves
	for (auto i = lights.begin(); i != lights.end(); ++i) {
		(*i)->draw(renderer);
	}

	renderer->SwapBuffers();
}

void Scene::drawDemo() const {
	renderer->SetDemoBuffer();
	renderer->SwapBuffers();
}
