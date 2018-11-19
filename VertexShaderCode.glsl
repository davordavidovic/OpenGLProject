#version 450

in layout(location=0) vec3 position;
in layout(location=1) vec2 vertexUV;
in layout(location=2) vec3 normal;

out vec2 uv;
out vec3 normalWorld;
out vec3 vertexPositionWorld;

uniform mat4 modelTransformMatrix;
uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;

void main()
{
	vec4 newPosition = modelTransformMatrix * vec4(position, 1.0);
	vec4 projectedPosition = projectionMatrix * viewMatrix * newPosition;
	gl_Position = projectedPosition;

	vec4 normal_temp = modelTransformMatrix * vec4(normal, 0);
	normalWorld = normal_temp.xyz;

	vertexPositionWorld = newPosition.xyz;
	uv = vertexUV;
	
}