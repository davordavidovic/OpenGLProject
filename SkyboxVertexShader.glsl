#version 450

in layout(location=0) vec3 pos;

out vec3 texCoords;

uniform mat4 proj;
uniform mat4 view;
uniform mat4 model;

void main()
{
	texCoords = pos;
	gl_Position = proj * view * model * vec4(pos, 1.0);

}