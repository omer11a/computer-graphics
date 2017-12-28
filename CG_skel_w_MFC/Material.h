#pragma once
#include "vec.h"

struct Material {
	vec3 ambientReflectance;
	vec3 specularReflectance;
	vec3 diffuseReflectance;
	float shininess;
};

typedef Material PolygonMaterial[3];
