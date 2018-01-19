#version 400

#define MAX_NUMBER_OF_LIGHTS 10

struct Light {
	vec4 position;
	vec3 intensity;
};

uniform bool isFlat;
uniform bool isGouraud;
uniform bool isPhong;
uniform bool hasTexture;
uniform bool hasNormalMapping;
uniform bool hasFog;
uniform mat4 modelMatrix;
uniform mat3 normalMatrix;
uniform mat4 modelViewMatrix;
uniform mat4 modelViewProjectionMatrix;
uniform vec3 cameraPosition;
uniform vec3 ambientLightColor;
uniform int numberOfLights;
uniform Light lights[MAX_NUMBER_OF_LIGHTS];
uniform sampler2D textureSampler;

layout (location = 0) in vec3 vertexPosition;
layout (location = 1) in vec3 centerPosition;
layout (location = 2) in vec3 vertexNormal;
layout (location = 3) in vec3 faceNormal;
layout (location = 4) in vec3 ambientReflectance;
layout (location = 5) in vec3 specularReflectance;
layout (location = 6) in vec3 diffuseReflectance;
layout (location = 7) in float shininess;
layout (location = 8) in vec2 uv;
layout (location = 9) in vec2 centerUv;
layout (location = 10) in vec3 tangent;

layout (location = 0) out vec3 outVertexPosition;
layout (location = 1) out vec3 outVertexNormal;
layout (location = 2) out vec3 outAmbientReflectance;
layout (location = 3) out vec3 outSpecularReflectance;
layout (location = 4) out vec3 outDiffuseReflectance;
layout (location = 5) out float outShininess;
layout (location = 6) out vec2 outUv;
layout (location = 7) out vec3 outTangent;
layout (location = 8) out vec3 outColor;
layout (location = 9) out vec3 outViewVertexPosition;

vec3 applyLight(
	Light light,
	vec3 specularReflectance,
	vec3 diffuseReflectance,
	float shininess,
	vec3 normal,
	vec3 vertexPosition,
	vec3 modelToCamera
) {
    vec3 modelToLight;
    if (light.position.w == 0.0) {
        modelToLight = normalize(light.position.xyz);
    } else {
        modelToLight = normalize(light.position.xyz - vertexPosition);
    }

    float diffuseCoefficient = max(0.0, dot(normal, modelToLight));
    vec3 diffuseColor = diffuseCoefficient * light.intensity * diffuseReflectance;

    float specularCoefficient = 0.0;
    if (diffuseCoefficient > 0.0) {
        specularCoefficient = pow(max(0.0, dot(modelToCamera, reflect(-modelToLight, normal))), shininess);
	}

    vec3 specularColor = specularCoefficient * light.intensity * specularReflectance;

    return diffuseColor + specularColor;
}

void main() {
	outVertexPosition = (modelMatrix * vec4(vertexPosition, 1)).xyz;
	outVertexNormal = normalMatrix * vertexNormal;
	outAmbientReflectance = ambientReflectance;
	outSpecularReflectance = specularReflectance;
	outDiffuseReflectance = diffuseReflectance;
	outShininess = shininess;
	outUv = uv;

	vec3 color = vec3(0);
	if ((isFlat) || (isGouraud)) {
		vec3 worldVertexPosition = vec3(0);
		vec3 normal = vec3(0);
		vec3 ambientColor = ambientReflectance;
		vec3 specularColor = specularReflectance;
		vec3 diffuseColor = diffuseReflectance;
		if (isFlat) {
			worldVertexPosition = (modelMatrix * vec4(centerPosition, 1)).xyz;
			normal = normalize(normalMatrix * faceNormal);
			if (hasTexture) {
				ambientColor = texture(textureSampler, centerUv).rgb;
				specularColor = ambientColor;
				diffuseColor = ambientColor;
			}
		} else {
			worldVertexPosition = outVertexPosition;
			normal = normalize(outVertexNormal);
			if (hasTexture) {
				ambientColor = texture(textureSampler, uv).rgb;
				specularColor = ambientColor;
				diffuseColor = ambientColor;
			}
		}
		
		vec3 modelToCamera = normalize(cameraPosition - worldVertexPosition);
		color = ambientLightColor * ambientColor;
		for (int i = 0; i < numberOfLights; ++i) {
			color += applyLight(
				lights[i],
				specularColor,
				diffuseColor,
				shininess,
				normal,
				worldVertexPosition,
				modelToCamera
			);
		}
	}

	outColor = clamp(color, 0, 1);

	if (hasNormalMapping) {
		outTangent = normalMatrix * tangent;
	}
	
	outViewVertexPosition = vec3(0);
	if (hasFog) {
		outViewVertexPosition = (modelViewMatrix * vec4(vertexPosition, 1)).xyz;
	}
	
	gl_Position = modelViewProjectionMatrix * vec4(vertexPosition, 1);
}