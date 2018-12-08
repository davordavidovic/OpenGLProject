#version 450

in vec2 uv;
in vec3 normalWorld;
in vec3 vertexPositionWorld;

out vec4 daColor;

uniform sampler2D textureSampler;
uniform sampler2D textureSampler2;

uniform vec4 lightColor;
uniform vec4 lightColor2;
uniform vec3 lightPositionWorld;
uniform vec3 lightPositionWorld2;
uniform vec3 eyePositionWorld;
uniform float diffuseBrightness;
uniform float diffuseBrightness2;
uniform float specularBrightness;
uniform float specularBrightness2;

uniform int hasNormalMapping;

void main()
{
	vec3 normal = normalWorld;

	if (hasNormalMapping == 1) {
		normal = texture(textureSampler2, uv).rgb;
		normal = normal * 2.0 - 1.0;
	}

	//ambient
	float ambientCoeff = 0.8;
	vec3 ambientLight = ambientCoeff * lightColor.rgb;

	//ambient (light2)
	float ambientCoeff2 = 0.1;
	vec3 ambientLight2 = ambientCoeff2 * lightColor2.rgb;

	//diffuse
	vec3 lightVectorWorld = normalize(lightPositionWorld - vertexPositionWorld);
	float brightness = dot(lightVectorWorld, normalize(normal)) * diffuseBrightness;
	vec4 diffuseLight = vec4(brightness, brightness, brightness, 1.0) * lightColor;
	
	//diffuse (light2)
	vec3 lightVectorWorld2 = normalize(lightPositionWorld2 - vertexPositionWorld);
	float brightness2 = dot(lightVectorWorld2, normalize(normal)) * diffuseBrightness2;
	vec4 diffuseLight2 = vec4(brightness2, brightness2, brightness2, 1.0) * lightColor2;

	//specular
	vec3 reflectedLightVectorWorld = reflect(-lightVectorWorld, normal);
	vec3 eyeVectorWorld = normalize(eyePositionWorld - vertexPositionWorld);
	float s = clamp(dot(reflectedLightVectorWorld, eyeVectorWorld), 0, 1);
	float s1 = pow(s, specularBrightness);
	vec4 specularLight = vec4(0, 0, s1, 1);

	//specular (light2)
	float s2 = pow(s, specularBrightness2);
	vec4 specularLight2 = vec4(s2, 0, 0, 1);

	vec3 tempColor = texture(textureSampler, uv).rgb;
	daColor = vec4((tempColor * (ambientLight + ambientLight2)), 1.0) + clamp(diffuseLight, 0, 1) + specularLight;
	daColor = daColor + clamp(diffuseLight2, 0, 1) + specularLight2;
}