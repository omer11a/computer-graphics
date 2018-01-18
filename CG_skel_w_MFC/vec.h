////////////////////////////////////////////////////////////////////////////////
////
////  --- vec.h ---
////
////////////////////////////////////////////////////////////////////////////////
#pragma once
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
using namespace glm;

#include <iostream>
#include <cmath>
#include "GL/glew.h"

#define M_PI 3.14159265358979323846264338327

//////////////////////////////////////////////////////////////////////////////
//
//  vec2.h - 2D vector
//
//////////////////////////////////////////////////////////////////////////////
inline
std::ostream& operator << ( std::ostream& os, const vec2& v ) {
	return os << "( " << v.x << ", " << v.y <<  " )";
}

inline
std::istream& operator >> ( std::istream& is, vec2& v )
{ return is >> v.x >> v.y ; }

inline
vec2 operator / (const vec2& v, const GLfloat s) {
	return vec2(v.x / s, v.y / s);
}

//////////////////////////////////////////////////////////////////////////////
//
//  vec3.h - 3D vector
//
//////////////////////////////////////////////////////////////////////////////
inline
std::ostream& operator << ( std::ostream& os, const vec3& v ) {
	return os << "( " << v.x << ", " << v.y << ", " << v.z <<  " )";
}

inline
std::istream& operator >> ( std::istream& is, vec3& v )
{ return is >> v.x >> v.y >> v.z ; }

inline
vec3 operator / (const vec3& v, const GLfloat s) {
	return vec3(v.x / s, v.y / s, v.z / s);
}

inline
vec3 minvec(const vec3& v, GLfloat s) {
	return vec3(min(v.x, s), min(v.y, s), min(v.z, s));
}
//////////////////////////////////////////////////////////////////////////////
//
//  vec4 - 4D vector
//
//////////////////////////////////////////////////////////////////////////////
inline
std::ostream& operator << ( std::ostream& os, const vec4& v ) {
return os << "( " << v.x << ", " << v.y
	  << ", " << v.z << ", " << v.w << " )";
}

inline
std::istream& operator >> ( std::istream& is, vec4& v )
{ return is >> v.x >> v.y >> v.z >> v.w; }

inline
vec4 operator / (const vec4& v, const GLfloat s) {
	return vec4(v.x / s, v.y / s, v.z / s, v.w / s);
}

inline
vec3 convert4dTo3d(const vec4& v) {
	return vec3(v.x / v.w, v.y / v.w, v.z / v.w);
}
