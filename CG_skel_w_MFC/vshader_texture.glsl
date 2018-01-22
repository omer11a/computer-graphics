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
uniform bool hasSkyBox;
uniform bool hasEnvironmentMapping;
uniform bool hasFog;
uniform bool hasColorAnimation;
uniform bool hasVertexAnimation;
uniform mat4 modelMatrix;
uniform mat3 normalMatrix;
uniform mat4 viewMatrix;
uniform mat4 viewProjectionMatrix;
uniform vec3 cameraPosition;
uniform vec3 ambientLightColor;
uniform int numberOfLights;
uniform Light lights[MAX_NUMBER_OF_LIGHTS];
uniform sampler2D textureSampler;
uniform samplerCube cubeSampler;
uniform int colorAnimationRepresentation;
uniform float colorAnimationDelta;
uniform float vertexAnimationDelta;

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
layout (location = 10) out vec3 outModelVertexPosition;

vec3 rgb2hsv(vec3 c) {
    vec4 k = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
    vec4 p = mix(vec4(c.bg, k.wz), vec4(c.gb, k.xy), step(c.b, c.g));
    vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));

    float d = q.x - min(q.w, q.y);
    float e = 1.0e-10;
    return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
}

vec3 hsv2rgb(vec3 c) {
    vec4 k = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + k.xyz) * 6.0 - k.www);
    return c.z * mix(k.xxx, clamp(p - k.xxx, 0.0, 1.0), c.y);
}

vec3 applyColorAnimation(vec3 color, int representation, float delta) {
	vec3 hsv = rgb2hsv(color);
	
	if (representation == 0) {
		hsv.x += delta;
	}

	if (representation == 1) {
		hsv.y += delta;
	}

	if (representation == 2) {
		hsv.z += delta;
	}

	return hsv2rgb(clamp(hsv, 0, 1));
}

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
	outVertexNormal = normalMatrix * vertexNormal;
	if (hasVertexAnimation) {
	    vec3 transformedVertex = (modelMatrix * vec4(vertexPosition, 1)).xyz;
		vec3 transformedNormal = length(vertexPosition) * vertexAnimationDelta * normalize(outVertexNormal);
		outVertexPosition = transformedVertex + transformedNormal;
	} else {
		outVertexPosition = (modelMatrix * vec4(vertexPosition, 1)).xyz;
	}

	outAmbientReflectance = ambientReflectance;
	outSpecularReflectance = specularReflectance;
	outDiffuseReflectance = diffuseReflectance;
	outShininess = shininess;
	outUv = uv;
	outModelVertexPosition = vertexPosition;

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
		if ((hasEnvironmentMapping) && (hasSkyBox)) {
			vec3 reflected = reflect(-modelToCamera, normal);
			specularColor = texture(cubeSampler, reflected).rgb;
		}

		if (hasColorAnimation) {
			ambientColor = applyColorAnimation(ambientColor, colorAnimationRepresentation, colorAnimationDelta);
			specularColor = applyColorAnimation(specularColor, colorAnimationRepresentation, colorAnimationDelta);
			diffuseColor = applyColorAnimation(diffuseColor, colorAnimationRepresentation, colorAnimationDelta);
		}
		
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
		outViewVertexPosition = (viewMatrix * vec4(outVertexPosition, 1)).xyz;
	}
	
	gl_Position = viewProjectionMatrix * vec4(outVertexPosition, 1);
}