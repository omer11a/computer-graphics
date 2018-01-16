#version 400

uniform mat4 modelMatrix;
uniform mat3 normalMatrix;
uniform mat4 modelViewProjectionMatrix;

in vec3 vertexPosition;
in vec3 vertexNormal;
in vec3 ambientReflectance;
in vec3 specularReflectance;
in vec3 diffuseReflectance;
in float shininess;

out vec3 outVertexPosition;
out vec3 outVertexNormal;
out vec3 outAmbientReflectance;
out vec3 outSpecularReflectance;
out vec3 outDiffuseReflectance;
out float outShininess;

void main() {
	// interpolated output
	outVertexPosition = modelMatrix * vec4(vertexPosition, 1);
	outVertexNormal = normalMatrix * vertexNormal;
	outAmbientReflectance = ambientReflectance;
	outSpecularReflectance = specularReflectance;
	outDiffuseReflectance = diffuseReflectance;
	outShininess = shininess;

	gl_Position = modelViewProjectionMatrix * vec4(vertexPosition, 1);
}