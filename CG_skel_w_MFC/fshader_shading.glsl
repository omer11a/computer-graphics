#version 400

#define MAX_NUMBER_OF_LIGHTS 10;

struct Light {
	vec4 position;
	vec3 intensity;
};

uniform vec4 cameraPosition;
uniform vec3 ambientLightColor;
uniform int numberOfLights;
uniform lights[MAX_NUMBER_OF_LIGHTS];

in vec3 vertexPosition;
in vec3 vertexNormal;
in vec3 ambientReflectance;
in vec3 specularReflectance;
in vec3 diffuseReflectance;
in float shininess;

out vec4 outColor;

vec3 applyLight(
	Light light,
	vec3 specularReflectance,
	vec3 diffuseReflectance,
	flot shininess,
	vec3 normal,
	vec3 vertexPosition,
	vec3 modelToCamera
) {
    vec3 modelToLight;
    if (light.position.w == 0.0) {
        modelToLight = normalize(-light.position.xyz);
    } else {
        modelToLight = normalize(light.position.xyz - vertexPosition);
    }

    float diffuseCoefficient = max(0.0, dot(normal, modelToLight));
    vec3 diffuseColor = diffuseCoefficient * light.color * diffuseReflectance;

    float specularCoefficient = 0.0;
    if (diffuseCoefficient > 0.0) {
        specularCoefficient = pow(max(0.0, dot(modelToCamera, reflect(-modelToLight, normal))), shininess);
	}

    vec3 specularColor = specularCoefficient * light.color * specularReflectance;

    return diffuseColor + specularColor;
}

void main() {
	vec3 normal = normalize(vertexNormal);
	vec3 modelToCamera = normalize(cameraPosition - vertexPosition);

    vec3 color = ambientLightColor * ambientReflectance;
    for (int i = 0; i < numberOfLights; ++i) {
        color += applyLight(
			lights[i],
			specularReflectance,
			diffuseReflectance,
			shininess,
			normal,
			vertexPosition,
			surfaceToCamera
		);
    }
    
    outColor = color;
}