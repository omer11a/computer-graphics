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

void ShaderProgram::SetUniformParameter(const GLuint ui, const char * const var_name)
{
	GLuint id = glGetUniformLocation(program_id, var_name);
	glUniform1ui(id, ui);
}

void ShaderProgram::Activate() {
	glUseProgram(program_id);
}

void ShaderProgram::ClearAttributes() {
	for (int i = 0; i < attribute_num; ++i) {
		glDisableVertexAttribArray(i);
	}
}
