#include <vector>
#include "stdafx.h"
#include "ShaderProgram.h"
#include "InitShader.h"

using namespace std;
ShaderProgram::ShaderProgram() : attribute_num(0), program_id(0)
{
}

ShaderProgram::ShaderProgram(const char * const vshader, const char * const fshader, int attribute_num) :
	attribute_num(attribute_num)
{
	program_id = InitShader(vshader, fshader);
}

void ShaderProgram::SetUniformParameter(const mat3& m, const char * const var_name) {
	GLuint id = glGetUniformLocation(program_id, var_name);
	glUniformMatrix3fv(id, 1, GL_FALSE, &m[0][0]);
}

void ShaderProgram::SetUniformParameter(const mat4& m, const char * const var_name) {
	GLuint id = glGetUniformLocation(program_id, var_name);
	glUniformMatrix4fv(id, 1, GL_FALSE, &m[0][0]);
}

void ShaderProgram::SetUniformParameter(const vec4& v, const char * const var_name)
{
	GLuint id = glGetUniformLocation(program_id, var_name);
	glUniform4fv(id, 1, &v[0]);
}

void ShaderProgram::SetUniformParameter(const vec3& v, const char * const var_name)
{
	GLuint id = glGetUniformLocation(program_id, var_name);
	glUniform3fv(id, 1, &v[0]);
}

void ShaderProgram::SetUniformParameter(const int i, const char * const var_name)
{
	GLuint id = glGetUniformLocation(program_id, var_name);
	glUniform1i(id, i);
}

void ShaderProgram::SetUniformParameter(const GLfloat f, const char * const var_name)
{
	GLuint id = glGetUniformLocation(program_id, var_name);
	glUniform1f(id, f);
}

GLuint ShaderProgram::SetInParameter(const vector<vec3>& v, const int attribute_id) {
	GLuint buffer;
	glGenBuffers(1, &buffer);
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glBufferData(GL_ARRAY_BUFFER, v.size() * sizeof(vec3), &v[0], GL_STATIC_DRAW);

	// attribute buffer
	glEnableVertexAttribArray(attribute_id);
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glVertexAttribPointer(
		attribute_id,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
		3,                  // size
		GL_FLOAT,           // type
		GL_FALSE,           // normalized?
		0,                  // stride
		(void*)0            // array buffer offset
	);
	return buffer;
}

GLuint ShaderProgram::SetInParameter(const vector<GLfloat>& v, const int attribute_id) {
	GLuint buffer;
	glGenBuffers(1, &buffer);
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glBufferData(GL_ARRAY_BUFFER, v.size() * sizeof(GLfloat), &v[0], GL_STATIC_DRAW);

	// attribute buffer
	glEnableVertexAttribArray(attribute_id);
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glVertexAttribPointer(
		attribute_id,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
		1,                  // size
		GL_FLOAT,           // type
		GL_FALSE,           // normalized?
		0,                  // stride
		(void*)0            // array buffer offset
	);
	return buffer;
}

void ShaderProgram::Activate() {
	glUseProgram(program_id);
}

void ShaderProgram::ClearAttributes() {
	for (int i = 0; i < attribute_num; ++i) {
		glDisableVertexAttribArray(i);
	}
}
