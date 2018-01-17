#pragma once
#include <vector>
#include "vec.h"
#include "mat.h"
#include "GL\glew.h"

using namespace std;
class ShaderProgram {
	GLuint program_id;
	int attribute_num;
public:
	ShaderProgram();
	ShaderProgram(const char * const vshader, const char * const fshader, int attribute_num);

	void SetUniformParameter(const mat3& m, const char * const var_name);
	void SetUniformParameter(const mat4& m, const char * const var_name);
	void SetUniformParameter(const vec4& v, const char * const var_name);
	void SetUniformParameter(const vec3& v, const char * const var_name);
	void SetUniformParameter(const int i, const char * const var_name);
	void SetUniformParameter(const GLfloat f, const char * const var_name);

	GLuint SetInParameter(const vector<vec3>& v, const int attribute_id);
	GLuint SetInParameter(const vector<GLfloat>& v, const int attribute_id);
	void Activate();
	void ClearAttributes();
};