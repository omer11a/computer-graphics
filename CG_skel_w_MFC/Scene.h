#pragma once

#include "gl/glew.h"
#include <vector>
#include <string>
#include "Renderer.h"
#include "MeshModel.h"
#include "Light.h"
using namespace std;

class Camera {
	bool isVisible;
	bool isPerspective;
	float left;
	float right;
	float bottom;
	float top;
	float zNear;
	float zFar;
	mat4 viewTransform;
	mat4 worldTransform;
	mat4 inverseViewTransform;
	mat4 inverseWorldTransform;
	mat4 projection;
	
	void verifyProjectionParameters(
		const float left, const float right,
		const float bottom, const float top,
		const float zNear, const float zFar
	);

	void verifyProjectionParameters(
		const float fov, const float aspect,
		const float zNear, const float zFar
	);

	void updateProjection();

public:
	Camera();
	void transformInView(const mat4 & transform);
	void transformInWorld(const mat4 & transform);
	void lookAt(const vec3& eye, const vec3& at, const vec3& up);
	void ortho(const float left, const float right,
		const float bottom, const float top,
		const float zNear, const float zFar );
	void frustum(const float left, const float right,
		const float bottom, const float top,
		const float zNear, const float zFar );
	void perspectiveVertical(const float fovy, const float aspect,
		const float zNear, const float zFar);
	void zoom(const float z = 1);
	void switchVisibility();
	mat4 getInverseTransform() const;
	mat4 getTransform() const;
	mat4 getProjection() const;
	float getNear() const;
	float getFar() const;
	void draw(Renderer * renderer) const;
};

class Scene {
	int activeModel;
	int activeLight;
	int activeCamera;
	Renderer * renderer;
	vector<MeshModel *> models;
	vector<Camera *> cameras;
	AmbientLight ambientLight;
	vector<DirectionalLightSource *> lights;
	vector<vec3> enviromentCube;
	GLuint skyCubeTexture;
	bool hasSkyBox;

	bool loadEnviromentSideTexture(GLenum side, CString filePath);
	void computeSkyCube();
public:
	Scene() = delete;
	explicit Scene(
		Renderer * renderer,
		const vec3& eye = vec3(),
		const AmbientLight& ambientLight = AmbientLight(vec3(1))
	);
	~Scene();
	void loadOBJModel(string fileName);
	void addPrimitive(int id);
	void addCamera();
	void setAmbientLight(const AmbientLight& ambientLight);
	void addLight(const DirectionalLightSource& light);
	MeshModel * getActiveModel();
	Camera * getActiveCamera();
	DirectionalLightSource * getActiveLight();
	size_t getNumberOfModels() const;
	size_t getNumberOfCameras() const;
	size_t getNumberOfLights() const;
	void setActiveModel(int i);
	void setActiveCamera(int i);
	void setActiveLight(int i);
	void prevCamera();
	void nextCamera();
	void stepAnimations(float timeDelta);
	void removeActiveModel();
	void removeActiveCamera();
	void removeActiveLight();
	void clear();
	void Scene::loadSkyBox(
		const CString topPath, const CString bottomPath,
		const CString leftPath, const CString rightPath,
		const CString frontPath, const CString backPath);
	void unloadSkyBox();
	void draw() const;
	void drawDemo() const;
};