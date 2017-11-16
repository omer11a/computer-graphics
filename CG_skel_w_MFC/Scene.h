#pragma once

#include "gl/glew.h"
#include <vector>
#include <string>
#include "Renderer.h"
#include "MeshModel.h"
using namespace std;

class Light {

};

class Camera {
	bool isVisible;
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

public:
	Camera();
	void transformInView(const mat4 & transform);
	void transformInWorld(const mat4 & transform);
	void lookAt(const vec4& eye, const vec4& at, const vec4& up);
	void ortho( const float left, const float right,
		const float bottom, const float top,
		const float zNear, const float zFar );
	void frustum( const float left, const float right,
		const float bottom, const float top,
		const float zNear, const float zFar );
	void perspectiveVertical( const float fovy, const float aspect,
		const float zNear, const float zFar);
	void perspectiveHorizontal(const float fovx, const float aspect,
		const float zNear, const float zFar);
	void zoomIn(const float z = 1);
	void zoomOut(const float z = 1);
	void setVisibility(bool shouldBeVisible);
	mat4 getInverseTransform() const;
	mat4 getProjection() const;
	void draw(Renderer * renderer) const;
};

class Scene {
	int activeModel;
	int activeLight;
	int activeCamera;
	Renderer * renderer;
	vector<MeshModel *> models;
	vector<Light *> lights;
	vector<Camera *> cameras;

public:
	Scene() = delete;
	explicit Scene(Renderer * renderer, const vec3 & eye = vec3());
	~Scene();
	void loadOBJModel(string fileName);
	void addCamera();
	MeshModel * getActiveModel();
	Camera * getActiveCamera();
	void setActiveModel(int i);
	void setActiveCamera(int i);
	void removeActiveModel();
	void removeActiveCamera();
	void draw() const;
	void drawDemo() const;
};