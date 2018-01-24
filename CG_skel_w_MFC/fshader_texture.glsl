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
uniform bool shouldRefract;
uniform bool hasFog;
uniform bool hasColorAnimation;
uniform bool hasToonShading;
uniform bool hasWoodTexture;
uniform vec3 cameraPosition;
uniform vec3 ambientLightColor;
uniform int numberOfLights;
uniform Light lights[MAX_NUMBER_OF_LIGHTS];
uniform sampler2D textureSampler;
uniform sampler2D normalSampler;
uniform samplerCube cubeSampler;
uniform float refractionRatio;
uniform vec3 fogColor;
uniform float extinctionCoefficient;
uniform float inScatteringCoefficient;
uniform int colorAnimationRepresentation;
uniform float colorAnimationDelta;
uniform int colorQuantizationCoefficient;
uniform vec3 woodTextureColor1;
uniform vec3 woodTextureColor2;
uniform vec2 modelResolution;

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
layout (location = 10) in vec3 modelVertexPosition;

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

float random(vec2 seed) {
	return fract(sin(dot(seed.xy, vec2(12.9898,78.233))) * 43758.5453123);
}

float noise(vec2 seed) {
    vec2 i = floor(seed);
    vec2 f = fract(seed);
    vec2 u = f * f * (3.0 - 2.0 * f);
    return mix(
		mix(random(i), random(i + vec2(1.0, 0.0)), u.x),
		mix(random(i + vec2(0.0, 1.0)), random(i + vec2(1.0, 1.0)), u.x),
		u.y
	);
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

		if (hasWoodTexture) {
			vec2 st = vec2(modelVertexPosition.x / modelResolution.x, modelVertexPosition.y / modelResolution.y);
			vec2 position = st.yx * vec2(12, 5);
			float angle = noise(position) * 0.7;
			position = mat2(cos(angle), - sin(angle), sin(angle), cos(angle)) * position;
			float pattern = smoothstep(0.0, 1.0, abs(sin(position.x * 15.0) + 1.0) * 0.5);
			ambientColor = pattern * woodTextureColor1 + (1 - pattern) * woodTextureColor2;
			specularColor = ambientColor;
			diffuseColor = ambientColor;
		}

		if (hasNormalMapping) {
			vec3 orthogonalTangent = normalize(tangent - dot(tangent, normal) * normal);
			vec3 bitangent = cross(normal, orthogonalTangent);
			vec3 mapNormal = texture(normalSampler, uv).xyz;
			mapNormal = 2.0 * mapNormal - vec3(1.0, 1.0, 1.0);
			mat3 tbn = mat3(orthogonalTangent, bitangent, normal);
			normal = normalize(tbn * mapNormal);
		}

		if ((hasEnvironmentMapping) && (hasSkyBox)) {
			vec3 reflected = vec3(0);
			if (shouldRefract) {
				reflected = refract(-modelToCamera, normal, refractionRatio);
			} else {
				reflected = reflect(-modelToCamera, normal);
			}

			specularColor = texture(cubeSampler, reflected).rgb;
			diffuseColor = specularColor;
		}

		if (hasColorAnimation) {
			ambientColor = applyColorAnimation(ambientColor, colorAnimationRepresentation, colorAnimationDelta);
			specularColor = applyColorAnimation(specularColor, colorAnimationRepresentation, colorAnimationDelta);
			diffuseColor = applyColorAnimation(diffuseColor, colorAnimationRepresentation, colorAnimationDelta);
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

	finalColor = clamp(finalColor, 0, 1);
	if (hasToonShading) {
		finalColor = round(finalColor * colorQuantizationCoefficient) / colorQuantizationCoefficient;
	}
    
    outColor = vec4(finalColor, 1);
}