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
uniform bool hasColorAnimation;
uniform bool hasVertexAnimation;
uniform vec3 cameraPosition;
uniform vec3 ambientLightColor;
uniform int numberOfLights;
uniform Light lights[MAX_NUMBER_OF_LIGHTS];
uniform sampler2D textureSampler;
uniform sampler2D normalSampler;
uniform vec3 fogColor;
uniform float extinctionCoefficient;
uniform float inScatteringCoefficient;
uniform int colorAnimationRepresentation;
uniform float colorAnimationDelta;
uniform float vertexAnimationDelta;

layout (location = 0) in vec3 vertexPosition;
layout (location = 1) in vec3 vertexNormal;
layout (location = 2) in vec3 ambientReflectance;
layout (location = 3) in vec3 specularReflectance;
layout (location = 4) in vec3 diffuseReflectance;
layout (location = 5) in float shininess;
layout (location = 6) in vec2 uv;
layout (location = 7) in vec3 tangent;
layout (location = 8) in vec3 color;
layout (location = 9) in vec3 viewVertexPosition;

layout (location = 0) out vec4 outColor;

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
	vec3 finalColor = color;
	if (isPhong) {
		vec3 normal = normalize(vertexNormal);
		vec3 modelToCamera = normalize(cameraPosition - vertexPosition);
		vec3 ambientColor = ambientReflectance;
		vec3 specularColor = specularReflectance;
		vec3 diffuseColor = diffuseReflectance;
		if (hasTexture) {
			ambientColor = texture(textureSampler, uv).rgb;
			specularColor = ambientColor;
			diffuseColor = ambientColor;
		}

		if (hasColorAnimation) {
			ambientColor = applyColorAnimation(ambientColor, colorAnimationRepresentation, colorAnimationDelta);
			specularColor = applyColorAnimation(specularColor, colorAnimationRepresentation, colorAnimationDelta);
			diffuseColor = applyColorAnimation(diffuseColor, colorAnimationRepresentation, colorAnimationDelta);
		}

		if (hasNormalMapping) {
			vec3 orthogonalTangent = normalize(tangent - dot(tangent, normal) * normal);
			vec3 bitangent = cross(normal, orthogonalTangent);
			vec3 mapNormal = texture(normalSampler, uv).xyz;
			mapNormal = 2.0 * mapNormal - vec3(1.0, 1.0, 1.0);
			mat3 tbn = mat3(orthogonalTangent, bitangent, normal);
			normal = normalize(tbn * mapNormal);
		}
		
		finalColor = ambientLightColor * ambientColor;
		for (int i = 0; i < numberOfLights; ++i) {
			finalColor += applyLight(
				lights[i],
				specularColor,
				diffuseColor,
				shininess,
				normal,
				vertexPosition,
				modelToCamera
			);
		}
	}
	
	if (hasFog) {
		float dist = length(viewVertexPosition);
		float be = abs(viewVertexPosition.y) * extinctionCoefficient;
		float bi = abs(viewVertexPosition.y) * inScatteringCoefficient;
		float ext = exp(-dist * be);
		float insc = exp(-dist * bi);
		finalColor = finalColor * ext + fogColor * (1 - insc);
	}
    
    outColor = vec4(clamp(finalColor, 0, 1), 1);
}