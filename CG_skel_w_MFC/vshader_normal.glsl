#version 400

uniform mat4 modelMatrix;
uniform mat3 normalModelMatrix;
uniform mat4 viewProjectionMatrix;
uniform mat4 modelViewProjectionMatrix;

layout (location = 0) in vec3 vertexPosition;
layout (location = 1) in vec3 normal;
layout (location = 2) in int isNormal;

void main() {
	if (isNormal != 0) {
		vec3 transformedVertex = (modelMatrix * vec4(vertexPosition, 1)).xyz;
		vec3 transformedNormal = normalize(normalModelMatrix * normal);
		gl_Position = viewProjectionMatrix * vec4(transformedVertex + transformedNormal, 1);
	} else {
		gl_Position = modelViewProjectionMatrix * vec4(vertexPosition, 1);
	}
}