#version 400

uniform mat4 modelMatrix;
uniform mat3 normalMatrix;
uniform mat4 modelViewProjectionMatrix;

layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec3 vertexNormal;
layout(location = 2) in vec3 ambientReflectance;
layout(location = 3) in vec3 specularReflectance;
layout(location = 4) in vec3 diffuseReflectance;
layout(location = 5) in float shininess;

layout(location = 0) out vec3 outVertexPosition;
layout(location = 1) out vec3 outVertexNormal;
layout(location = 2) out vec3 outAmbientReflectance;
layout(location = 3) out vec3 outSpecularReflectance;
layout(location = 4) out vec3 outDiffuseReflectance;
layout(location = 5) out float outShininess;

void main() {
	// interpolated output
	outVertexPosition = (modelMatrix * vec4(vertexPosition, 1)).xyz;
	outVertexNormal = normalMatrix * vertexNormal;
	outAmbientReflectance = ambientReflectance;
	outSpecularReflectance = specularReflectance;
	outDiffuseReflectance = diffuseReflectance;
	outShininess = shininess;

	gl_Position = modelViewProjectionMatrix * vec4(vertexPosition, 1);
}
