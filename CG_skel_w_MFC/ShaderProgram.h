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

	//GLuint SetInParameter(const vector<vec3>& v, const int attribute_id);
	//GLuint SetInParameter(const vector<GLfloat>& v, const int attribute_id);

	template <class T>
	GLuint SetInParameter(const vector<T>& v, const int attribute_id, const int size) {
		GLuint buffer;
		glGenBuffers(1, &buffer);
		glBindBuffer(GL_ARRAY_BUFFER, buffer);
		glBufferData(GL_ARRAY_BUFFER, v.size() * sizeof(T), &v[0], GL_STATIC_DRAW);

		// attribute buffer
		glEnableVertexAttribArray(attribute_id);
		glBindBuffer(GL_ARRAY_BUFFER, buffer);
		glVertexAttribPointer(
			attribute_id,       // attribute
			size,               // size
			GL_FLOAT,           // type
			GL_FALSE,           // normalized
			0,                  // stride
			(void*)0            // array buffer offset
		);
		return buffer;
	}



	void Activate();
	void ClearAttributes();
};