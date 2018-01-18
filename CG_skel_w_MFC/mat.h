#pragma once
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
using namespace glm;

#include "vec.h"
//    //
//    //  --- Insertion and Extraction Operators ---
//    //
//	
//    friend std::ostream& operator << ( std::ostream& os, const mat2& m )
//	{ return os << std::endl << m[0] << std::endl << m[1] << std::endl; }
//
//    friend std::istream& operator >> ( std::istream& is, mat2& m )
//	{ return is >> m._m[0] >> m._m[1] ; }
//
////----------------------------------------------------------------------------
////
////  mat3 - 3D square matrix 
////
//
//    //
//    //  --- Insertion and Extraction Operators ---
//    //
//	
//    friend std::ostream& operator << ( std::ostream& os, const mat3& m ) {
//	return os << std::endl 
//		  << m[0] << std::endl
//		  << m[1] << std::endl
//		  << m[2] << std::endl;
//    }
//
//    friend std::istream& operator >> ( std::istream& is, mat3& m )
//	{ return is >> m._m[0] >> m._m[1] >> m._m[2] ; }
//
////----------------------------------------------------------------------------
////
////  mat4.h - 4D square matrix
////
//    //
//    //  --- Insertion and Extraction Operators ---
//    //
//	
//    friend std::ostream& operator << ( std::ostream& os, const mat4& m ) {
//	return os << std::endl 
//		  << m[0] << std::endl
//		  << m[1] << std::endl
//		  << m[2] << std::endl
//		  << m[3] << std::endl;
//    }
//
//    friend std::istream& operator >> ( std::istream& is, mat4& m )
//	{ return is >> m._m[0] >> m._m[1] >> m._m[2] >> m._m[3]; }

inline
mat3 convert4dTo3d(const mat4& A) {
	return mat3(A[0][0], A[0][1], A[0][2], A[1][0], A[1][1], A[1][2], A[2][0], A[2][1], A[2][2]);
}

//////////////////////////////////////////////////////////////////////////////
//
//  Helpful Matrix Methods
//
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
//
//  Rotation matrix generators
//

inline
mat4 RotateX( const GLfloat theta )
{
    GLfloat angle = (M_PI/180.0f) * theta;

    mat4 c(1);
    c[1][1] = c[2][2] = cos(angle);
    c[1][2] = sin(angle);
	c[2][1] = -c[1][2];
    return c;
}

inline
mat4 RotateY(const GLfloat theta)
{
	GLfloat angle = (M_PI / 180.0f) * theta;

	mat4 c(1);
	c[0][0] = c[2][2] = cos(angle);
	c[0][2] = sin(angle);
	c[2][0] = -c[0][2];
	return c;
}

inline
mat4 RotateZ(const GLfloat theta)
{
	GLfloat angle = (M_PI / 180.0f) * theta;

	mat4 c(1);
	c[0][0] = c[1][1] = cos(angle);
	c[0][1] = sin(angle);
	c[1][0] = -c[0][1];
	return c;
}


//----------------------------------------------------------------------------
//
//  Translation matrix generators
//

inline
mat4 Translate( const GLfloat x, const GLfloat y, const GLfloat z )
{
    mat4 c(1);
    c[3][0] = x;
	c[3][1] = y;
	c[3][2] = z;
    return c;
}

inline
mat4 Translate( const vec3& v )
{
    return Translate( v.x, v.y, v.z );
}

inline
mat4 Translate( const vec4& v )
{
    return Translate( v.x, v.y, v.z );
}

//----------------------------------------------------------------------------
//
//  Scale matrix generators
//

inline
mat4 Scale( const GLfloat x, const GLfloat y, const GLfloat z )
{
    mat4 c(1);
    c[0][0] = x;
    c[1][1] = y;
    c[2][2] = z;
    return c;
}

inline
mat4 Scale( const vec3& v )
{
    return Scale( v.x, v.y, v.z );
}

//----------------------------------------------------------------------------
