#pragma once

#include "gl/glew.h"
#include <vector>
#include <string>
#include "Renderer.h"
using namespace std;

class Model {
protected:
	virtual ~Model() {}
	void virtual draw()=0;
};


class Light {

};

class Camera {
	mat4 viewTransform;
	mat4 worldTransform;
	mat4 inverseViewTransform;
	mat4 inverseWorldTransform;
	mat4 projection;

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
	mat4 perspective( const float fovy, const float aspect,
		const float zNear, const float zFar);

};

class Scene {

	vector<Model*> models;
	vector<Light*> lights;
	vector<Camera*> cameras;
	Renderer *m_renderer;

public:
	Scene() {};
	Scene(Renderer *renderer) : m_renderer(renderer) {};
	void loadOBJModel(string fileName);
	void draw();
	void drawDemo();
	
	int activeModel;
	int activeLight;
	int activeCamera;
};