#version 400

uniform mat4 modelMatrix;
uniform mat3 normalMatrix;
uniform mat4 viewProjectionMatrix;
uniform float silhouetteThickness;

layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec3 vertexNormal;

void main() {
    vec3 transformedVertex = (modelMatrix * vec4(vertexPosition, 1)).xyz;
	vec3 transformedNormal = silhouetteThickness * normalize(normalMatrix * vertexNormal);
	gl_Position = viewProjectionMatrix * vec4(transformedVertex + transformedNormal, 1);
}