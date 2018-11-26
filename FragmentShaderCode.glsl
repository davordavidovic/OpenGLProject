#version 450

in vec2 uv;
out vec4 daColor;
in vec3 normalWorld;
in vec3 vertexPositionWorld;

uniform sampler2D textureSampler;
uniform sampler2D textureSampler2;
uniform vec3 ambientLight;
uniform vec3 lightPositionWorld;
uniform vec3 eyePositionWorld;
uniform float diffuseBrightness;
uniform float specularBrightness;
uniform int hasNormalMapping;

void main()
{
	if (hasNormalMapping == 1) {
		vec3 normal = texture(textureSampler2, uv).rgb;
		normal = normalize(normal * 2.0 - 1.0);
	}
	//diffuse
	vec3 lightVectorWorld = normalize(lightPositionWorld - vertexPositionWorld);
	float brightness = dot(lightVectorWorld, normalize(normalWorld)) + diffuseBrightness;
	vec4 diffuseLight = vec4(brightness, brightness, brightness, 1.0);

	//specular
	vec3 reflectedLightVectorWorld = reflect(-lightVectorWorld, normalWorld);
	vec3 eyeVectorWorld = normalize(eyePositionWorld - vertexPositionWorld);
	float s = clamp(dot(reflectedLightVectorWorld, eyeVectorWorld), 0, 1);
	s = pow(s,specularBrightness);
	vec4 specularLight = vec4(0, 0, s, 1);

	vec3 tempColor = texture(textureSampler, uv).rgb;
	daColor = vec4((tempColor * ambientLight), 1.0) + clamp(diffuseLight, 0, 1) + specularLight;
}