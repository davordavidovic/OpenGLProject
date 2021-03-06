/*********************************************************
FILE : main.cpp (csci3260 2018-2019 Final Project)
*********************************************************/
/*********************************************************/
/*Students Information
Student ID: 1155123101					Student ID: 1155059798
Student Name: Davor Davidovikj			Student Name: Chu Chao Ying
*********************************************************/

#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS //deals with deprecated c++ functions from the provided loaders
#endif

#include "Dependencies\glew\glew.h"
#include "Dependencies\freeglut\freeglut.h"
#include "Dependencies\glm\glm.hpp"
#include "Dependencies\glm\gtc\matrix_transform.hpp"
#include "Dependencies\glm\gtc\type_ptr.hpp"
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#define STB_IMAGE_IMPLEMENTATION 
#include "stb_image.h" //image loader for skybox .tga images
#define M_PI 3.1415926535897932384626433832795 //pi for rotations
#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600
using namespace std;
using glm::vec3;
using glm::vec4;
using glm::mat4;

const double EULER = 2.71828182845904523536;
glm::mat4 temp = glm::mat4(1.0f); //a utility identity matrix
float sceneRotate = 0.0f, ringRotate = 0.0f, rockRotate = 0.0f, rockExpand = 0.0f;
float objMoveX = 0.0f, objMoveZ = 0.0f;
float diff = 0.6f, spec = 100.0f, amb = 0.0f; //macros for diffuse and specular brightness
float diff2 = 0.2f, spec2 = 20.f; //macros for second light
float mMoverx = 0.0f, mMovery = 0.0f;
bool shipRing[5] = { false };
bool shipStar = false, shipEarth = false, shipCollide = false;
float shipTilt = 0.0f;
float speed = 0.0f;
float acc = pow(EULER, (speed - 6));

const int amount = 200;
glm::mat4 *modelMatrices = new mat4[amount];
bool renderRock[amount] = { true };

float lightx = 0.0f; //macro for light position movement
float rotateFactor = 0.0f;
int orgnx = -1, orgny = -1;
float anglex = 0.0f, angley = 0.0f;
float lx = 0.0f, ly = 0.0f; //move factor of camera

bool flag = true, space = true;
GLuint spaceCraftVAO, earthVAO, cubeVAO, wonderStarVAO, ringVAO, rockVAO;
GLuint texture[8]; //array of used textures
int drawsize[8];
GLint programID, skyboxProgramID; //two programs used

//spacecraft's left/right rotation
float sc_angle = 0.0f;

// TODO: debug bump mapping
GLuint TextureID_0;
GLuint TextureID_1;

bool checkStatus(
	GLuint objectID,
	PFNGLGETSHADERIVPROC objectPropertyGetterFunc,
	PFNGLGETSHADERINFOLOGPROC getInfoLogFunc,
	GLenum statusType)
{
	GLint status;
	objectPropertyGetterFunc(objectID, statusType, &status);
	if (status != GL_TRUE)
	{
		GLint infoLogLength;
		objectPropertyGetterFunc(objectID, GL_INFO_LOG_LENGTH, &infoLogLength);
		GLchar* buffer = new GLchar[infoLogLength];

		GLsizei bufferSize;
		getInfoLogFunc(objectID, infoLogLength, &bufferSize, buffer);
		cout << buffer << endl;

		delete[] buffer;
		return false;
	}
	return true;
}


bool checkShaderStatus(GLuint shaderID)
{
	return checkStatus(shaderID, glGetShaderiv, glGetShaderInfoLog, GL_COMPILE_STATUS);
}

bool checkProgramStatus(GLuint programID)
{
	return checkStatus(programID, glGetProgramiv, glGetProgramInfoLog, GL_LINK_STATUS);
}

string readShaderCode(const char* fileName)
{
	ifstream meInput(fileName);
	if (!meInput.good())
	{
		cout << "File failed to load..." << fileName;
		exit(1);
	}
	printf("Loaded shader code to read.\n");
	return std::string(
		std::istreambuf_iterator<char>(meInput),
		std::istreambuf_iterator<char>()
	);
}

void installShaders()
{
	GLuint vertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint fragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
	GLuint skyboxVertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint skyboxFragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	const GLchar* adapter[1];
	string temp = readShaderCode("VertexShaderCode.glsl");
	adapter[0] = temp.c_str();
	glShaderSource(vertexShaderID, 1, adapter, 0);
	temp = readShaderCode("FragmentShaderCode.glsl");
	adapter[0] = temp.c_str();
	glShaderSource(fragmentShaderID, 1, adapter, 0);
	temp = readShaderCode("SkyboxVertexShader.glsl");
	adapter[0] = temp.c_str();
	glShaderSource(skyboxVertexShaderID, 1, adapter, 0);
	temp = readShaderCode("SkyboxFragmentShader.glsl");
	adapter[0] = temp.c_str();
	glShaderSource(skyboxFragmentShaderID, 1, adapter, 0);


	glCompileShader(vertexShaderID);
	glCompileShader(fragmentShaderID);
	glCompileShader(skyboxVertexShaderID);
	glCompileShader(skyboxFragmentShaderID);

	printf("Compiled shaders\n");

	if (!checkShaderStatus(vertexShaderID) || !checkShaderStatus(fragmentShaderID) || !checkShaderStatus(skyboxVertexShaderID) || !checkShaderStatus(skyboxFragmentShaderID)) {
		printf("-------------------------------------------------\n");
		printf("Failed installing shaders. Shader status error.\n");
		return;
	}

	programID = glCreateProgram();
	glAttachShader(programID, vertexShaderID);
	glAttachShader(programID, fragmentShaderID);
	glLinkProgram(programID);

	skyboxProgramID = glCreateProgram();
	glAttachShader(skyboxProgramID, skyboxVertexShaderID);
	glAttachShader(skyboxProgramID, skyboxFragmentShaderID);
	glLinkProgram(skyboxProgramID);
	printf("Shaders attached and program linked.\n");

	if (!checkProgramStatus(programID)) {
		printf("Failed installing main shaders. Program status status error\n");
		return;
	}

	if (!checkProgramStatus(skyboxProgramID)) {
		printf("Failed installing skybox shaders. Program status status error\n");
		return;
	}

	glDeleteShader(vertexShaderID);
	glDeleteShader(fragmentShaderID);
	glDeleteShader(skyboxVertexShaderID);
	glDeleteShader(skyboxFragmentShaderID);
} //installing custom made shaders 

void specialKeyboardFunc(int key, int x, int y) {

	if (key == GLUT_KEY_UP) {
		objMoveX += cos(sc_angle)*0.03f;
		objMoveZ += sin(sc_angle)*0.03f;
		//cout << "movez x:\n" << objMoveX;
		//cout << "movez z:\n" << objMoveZ;

	}
	else if (key == GLUT_KEY_DOWN) {
		objMoveX -= cos(sc_angle)*0.03f;
		objMoveZ -= sin(sc_angle)*0.03f;
		//cout << "movez x:\n" << objMoveX;
		//cout << "movez z:\n" << objMoveZ;
	}
	else if (key == GLUT_KEY_LEFT) {
		objMoveX += sin(sc_angle)*0.03f;
		objMoveZ -= cos(sc_angle)*0.03f;
		shipTilt -= 0.01f;
	}
	else if (key == GLUT_KEY_RIGHT) {
		objMoveX -= sin(sc_angle)*0.03f;
		objMoveZ += cos(sc_angle)*0.03f;
		shipTilt += 0.01f;
	}


}

void keyboard(unsigned char key, int x, int y)
{
	if (key == 'a' || key == 'A')
	{
		sceneRotate -= 1.0f;
		cout << sceneRotate;
	}

	if (key == 'd' || key == 'D')
	{
		sceneRotate += 1.0f;
	}

	if (key == 'c' || key == 'C')
	{
		lightx -= 50.0f;
		cout << lightx;
	}

	if (key == 'v' || key == 'V')
	{
		lightx += 50.0f;
	}

	//--------------------diffuse light control--------------------
	if (key == 'q' || key == 'Q')
	{
		if (diff > 0.0f) {
			diff -= 0.05f;
		}
	}

	if (key == 'w' || key == 'W')
	{
		if (diff < 1.0f) {
			diff += 0.05f;
		}
	}

	if (key == 'e' || key == 'e')
	{
		if (diff2 > 0.0f) {
			diff2 -= 0.05f;
		}
	}

	if (key == 'r' || key == 'R')
	{
		if (diff2 < 1.0f) {
			diff2 += 0.05f;
		}
	}

	//--------------------specular light control-------------------
	if (key == 'z' || key == 'Z')
	{
		spec += 2;
	}

	if (key == 'x' || key == 'X')
	{
		if (spec > 2) {
			spec -= 2;
		}
	}

	if (key == 'k' || key == 'K')
	{
		spec2 += 2;
	}

	if (key == 'l' || key == 'L')
	{
		if (spec2 > 2) {
			spec2 -= 2;
		}
	}

	//--------------------ambient light control--------------------
	if (key == 'm' || key == 'M')
	{
		amb += 0.01f;
		cout << amb;
	}

	if (key == 'n' || key == 'N')
	{
		amb -= 0.01f;
		cout << amb;

	}
}

void move(int button, int state, int x, int y)
{
	if (space) { //checks switch
		if (button == GLUT_LEFT_BUTTON) { //checks if the mouse button is the left button

			if (state == GLUT_UP) { //if it is released, orgn is -1 so that no rotation can happen (mMove func wont be entered)
				anglex -= mMoverx;
				angley -= mMovery;
				orgnx = -1;
				orgny = -1;
			}
			else {
				orgnx = x; //else the origin is the current coordinate of camera
				orgny = y;
			}
		}
	}

}

void PassiveMouse(int x, int y)
{
	//determine direction and magnitude of spacecraft's left/right rotation
	//by tracing positon of cursor relative to window's central vertical line
	sc_angle = x - SCREEN_WIDTH / 2;
	//scale down rotation magnitude so that a small mouse movement will not
	//result in a big rotation
	sc_angle /= 50;
	//restrict range of angles the spacecraft can take
	sc_angle = std::max(-45.0f, std::min(sc_angle, 45.0f));


}

bool loadOBJ(
	const char * path,
	std::vector<glm::vec3> & out_vertices,
	std::vector<glm::vec2> & out_uvs,
	std::vector<glm::vec3> & out_normals
) {

	printf("Loading OBJ file %s...\n", path);

	std::vector<unsigned int> vertexIndices, uvIndices, normalIndices;
	std::vector<glm::vec3> temp_vertices;
	std::vector<glm::vec2> temp_uvs;
	std::vector<glm::vec3> temp_normals;

	FILE * file = fopen(path, "r");
	if (file == NULL) {
		printf("Impossible to open the file! Are you in the right path? See Tutorial 6 for details\n");
		getchar();
		return false;
	}

	while (1) {

		char lineHeader[128];
		int res = fscanf(file, "%s", lineHeader);
		if (res == EOF)
			break; // EOF = End Of File. Quit the loop.

				   // else : parse lineHeader

		if (strcmp(lineHeader, "v") == 0) {
			glm::vec3 vertex;
			fscanf(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z);
			temp_vertices.push_back(vertex);
		}
		else if (strcmp(lineHeader, "vt") == 0) {
			glm::vec2 uv;
			fscanf(file, "%f %f\n", &uv.x, &uv.y);
			uv.y = -uv.y;
			temp_uvs.push_back(uv);
		}
		else if (strcmp(lineHeader, "vn") == 0) {
			glm::vec3 normal;
			fscanf(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z);
			temp_normals.push_back(normal);
		}
		else if (strcmp(lineHeader, "f") == 0) {
			std::string vertex1, vertex2, vertex3;
			unsigned int vertexIndex[3], uvIndex[3], normalIndex[3];
			int matches = fscanf(file, "%d/%d/%d %d/%d/%d %d/%d/%d\n", &vertexIndex[0], &uvIndex[0], &normalIndex[0], &vertexIndex[1], &uvIndex[1], &normalIndex[1], &vertexIndex[2], &uvIndex[2], &normalIndex[2]);
			if (matches != 9) {
				printf("File can't be read by our simple parser :-( Try exporting with other options\n");
				return false;
			}
			vertexIndices.push_back(vertexIndex[0]);
			vertexIndices.push_back(vertexIndex[1]);
			vertexIndices.push_back(vertexIndex[2]);
			uvIndices.push_back(uvIndex[0]);
			uvIndices.push_back(uvIndex[1]);
			uvIndices.push_back(uvIndex[2]);
			normalIndices.push_back(normalIndex[0]);
			normalIndices.push_back(normalIndex[1]);
			normalIndices.push_back(normalIndex[2]);
		}
		else {
			char stupidBuffer[1000];
			fgets(stupidBuffer, 1000, file); //don't read this line and put it in a random string
		}

	}

	// For each vertex of each triangle
	for (unsigned int i = 0; i < vertexIndices.size(); i++) {

		// Get the indices of its attributes
		unsigned int vertexIndex = vertexIndices[i];
		unsigned int uvIndex = uvIndices[i];
		unsigned int normalIndex = normalIndices[i];

		// Get the attributes thanks to the index
		glm::vec3 vertex = temp_vertices[vertexIndex - 1];
		glm::vec2 uv = temp_uvs[uvIndex - 1];
		glm::vec3 normal = temp_normals[normalIndex - 1];

		// Put the attributes in buffers
		out_vertices.push_back(vertex);
		out_uvs.push_back(uv);
		out_normals.push_back(normal);

	}

	printf("Done loading model!...\n", path);

	return true;
}

GLuint loadCubemap(vector<std::string> faces)
{
	GLuint textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

	int width, height, nrChannels;
	for (unsigned int i = 0; i < faces.size(); i++)
	{
		unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
		if (data)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
				0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data
			);
			stbi_image_free(data);
		}
		else
		{
			std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
			stbi_image_free(data);
		}
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	return textureID;
}

GLuint loadBMP_custom(const char * imagepath) {

	printf("Reading image %s\n", imagepath);

	unsigned char header[54];
	unsigned int dataPos;
	unsigned int imageSize;
	unsigned int width, height;
	unsigned char * data;

	FILE * file = fopen(imagepath, "rb");
	if (!file) { printf("%s could not be opened. Are you in the right directory ? Don't forget to read the FAQ !\n", imagepath); getchar(); return 0; }

	if (fread(header, 1, 54, file) != 54) {
		printf("Not a correct BMP file\n");
		return 0;
	}
	if (header[0] != 'B' || header[1] != 'M') {
		printf("Not a correct BMP file\n");
		return 0;
	}
	if (*(int*)&(header[0x1E]) != 0) {
		printf("Not a correct BMP file\n");
		return 0;
	}
	if (*(int*)&(header[0x1C]) != 24) { printf("Not a correct BMP file\n");    return 0; }

	dataPos = *(int*)&(header[0x0A]);
	imageSize = *(int*)&(header[0x22]);
	width = *(int*)&(header[0x12]);
	height = *(int*)&(header[0x16]);

	if (imageSize == 0) {
		imageSize = width * height * 3;
	}

	if (dataPos == 0) {
		dataPos = 54;
	}

	data = new unsigned char[imageSize];
	fread(data, 1, imageSize, file);
	fclose(file);

	GLuint textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, data);
	delete[] data;
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glGenerateMipmap(GL_TEXTURE_2D);

	return textureID;

	//TODO: Create one OpenGL texture and set the texture parameter 

}

void dataFunc(const char * objectName, const char * textureName, GLuint &vao, int objectIndex, int textureIndex) {

	std::vector< glm::vec3 > vertices;
	std::vector< glm::vec2 > uvs;
	std::vector< glm::vec3 > normals; // Won't be used at the moment.

	bool res = loadOBJ(objectName, vertices, uvs, normals);


	GLuint vertexbuffer, uvbuffer, normalbuffer;

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glGenBuffers(1, &vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	if (textureName != "none") {
		texture[textureIndex] = loadBMP_custom(textureName);

		glGenBuffers(1, &uvbuffer);
		glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
		glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(glm::vec2), &uvs[0], GL_STATIC_DRAW);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
	}
	glGenBuffers(1, &normalbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
	glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), &normals[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	drawsize[objectIndex] = vertices.size();

	/*glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(2);*/

}

void createSkybox() {

	float cube[] = {

		-1.0f,  1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		-1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f,
	};


	vector<std::string> faces1
	{
		"purplenebula_rt.jpg",
		"purplenebula_lf.jpg",
		"purplenebula_up.jpg",
		"purplenebula_dn.jpg",
		"purplenebula_ft.jpg",
		"purplenebula_bk.jpg"
	};

	texture[3] = loadCubemap(faces1);
	glGenVertexArrays(1, &cubeVAO);
	glBindVertexArray(cubeVAO);

	GLuint cubeVBO;
	glGenBuffers(1, &cubeVBO);
	glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
	glBufferData(GL_ARRAY_BUFFER, 3 * 36 * sizeof(float), &cube, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
	cout << "Bound cube\n";
}

void sendDataToOpenGL()
{
	createSkybox();
	dataFunc("spaceCraft.obj", "./texture/spacecraftTexture.bmp", spaceCraftVAO, 0, 0);
	texture[2] = loadBMP_custom("./texture/earth_normal.bmp");
	dataFunc("planet.obj", "./texture/earthTexture.bmp", earthVAO, 1, 1);
	dataFunc("planet.obj", "./texture/WonderStarTexture.bmp", wonderStarVAO, 4, 4);
	dataFunc("Ring.obj", "./texture/ringTexture.bmp", ringVAO, 5, 5);
	texture[6] = loadBMP_custom("./texture/greenhair.bmp");
	dataFunc("rock.obj", "./texture/RockTexture.bmp", rockVAO, 7, 7);
	TextureID_0 = glGetUniformLocation(programID, "textureSampler");
	TextureID_1 = glGetUniformLocation(programID, "textureSampler2");

}

void initializeLighting() {

	vec4 lightColor(0.9f, 0.9f, 0.9f, 1.0f);
	vec4 lightColor2(1.0f, 0.0f, 0.0f, 1.0f);

	GLint lightColorUniformLocation = glGetUniformLocation(programID, "lightColor");
	glUniform4fv(lightColorUniformLocation, 1, &lightColor[0]);

	GLint lightColor2UniformLocation = glGetUniformLocation(programID, "lightColor2");
	glUniform4fv(lightColor2UniformLocation, 1, &lightColor2[0]);

	GLint eyePositionUniformLocation = glGetUniformLocation(programID, "eyePositionWorld");
	vec3 eyePosition(1.0f, 1.0f, 1.0f);
	glUniform3fv(eyePositionUniformLocation, 1, &eyePosition[0]);

	GLint lightPositionUniformLocation = glGetUniformLocation(programID, "lightPositionWorld");
	vec3 lightPosition(lightx, 1.0f, -3.0f);
	glUniform3fv(lightPositionUniformLocation, 1, &lightPosition[0]);

	GLint lightPosition2UniformLocation = glGetUniformLocation(programID, "lightPositionWorld2");
	vec3 lightPosition2(0.0f, 5.0f, 0.0f);
	glUniform3fv(lightPosition2UniformLocation, 1, &lightPosition2[0]);

	GLint diffuseBrightness = glGetUniformLocation(programID, "diffuseBrightness");
	glUniform1f(diffuseBrightness, diff);

	GLint diffuseBrightness2 = glGetUniformLocation(programID, "diffuseBrightness2");
	glUniform1f(diffuseBrightness2, diff2);

	GLint specularBrightness = glGetUniformLocation(programID, "specularBrightness");
	glUniform1f(specularBrightness, spec);

	GLint specularBrightness2 = glGetUniformLocation(programID, "specularBrightness2");
	glUniform1f(specularBrightness2, spec2);
}


void paintGL(void)
{
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	GLint hasNormalMappingUniformLocation = glGetUniformLocation(programID, "hasNormalMapping");
	glUniform1i(hasNormalMappingUniformLocation, 0);

	//-------------------------------------------------------------
	//skybox

	glm::mat4 skyboxProj = glm::perspective(glm::radians(60.0f), 16.0f / 9.0f, 0.1f, 20.0f); //perspective projection used
	glm::mat4 skyboxView = glm::lookAt(
		glm::vec3(0.1f, 0.13f, 0), // coordinates of camera
		glm::vec3(0, 0.1f, 0), // camera looks at origin
		glm::vec3(0, 1, 0)  // head is straight up
	);

	glm::mat4 skyboxModel = glm::rotate(temp, sc_angle, glm::vec3(0, 1, 0));


	glDepthMask(GL_FALSE);
	glUseProgram(skyboxProgramID);

	GLint skyboxProjectionMatrix = glGetUniformLocation(skyboxProgramID, "proj");
	glUniformMatrix4fv(skyboxProjectionMatrix, 1, GL_FALSE, &skyboxProj[0][0]);

	GLint skyboxViewMatrix = glGetUniformLocation(skyboxProgramID, "view");
	glUniformMatrix4fv(skyboxViewMatrix, 1, GL_FALSE, &skyboxView[0][0]);

	GLint skyboxModelMatrix = glGetUniformLocation(skyboxProgramID, "model");
	glUniformMatrix4fv(skyboxModelMatrix, 1, GL_FALSE, &skyboxModel[0][0]);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, texture[3]);
	glBindVertexArray(cubeVAO);
	glDrawArrays(GL_TRIANGLES, 0, 36);

	//-------------------------------------------------------------
	//initializations

	glDepthMask(GL_TRUE);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LINE_SMOOTH);
	glEnable(GL_POINT_SMOOTH);
	glEnable(GL_POLYGON_SMOOTH);
	glDepthFunc(GL_LESS);

	glUseProgram(programID);
	initializeLighting(); //creates all lighting 
	glm::mat4 Projection = glm::perspective(glm::radians(60.0f), 4.0f / 3.0f, 0.1f, 20.0f); //perspective projection used
	glm::mat4 View = glm::lookAt(
		glm::vec3(0, 0.1f, -3), // coordinates of camera
		glm::vec3(0, 0.1f, 0), // camera looks at
		glm::vec3(0, 1, 0)  // head is straight up
	);

	GLint modelTransformMatrixUniformLocation =
		glGetUniformLocation(programID, "modelTransformMatrix");

	GLint projectionMatrixUniformLocation =
		glGetUniformLocation(programID, "projectionMatrix");
	glUniformMatrix4fv(projectionMatrixUniformLocation, 1,
		GL_FALSE, &Projection[0][0]);

	GLint viewMatrixUniformLocation =
		glGetUniformLocation(programID, "viewMatrix");
	glUniformMatrix4fv(viewMatrixUniformLocation, 1,
		GL_FALSE, &View[0][0]);

	//-------------------------------------------------------------
	//earth

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture[1]);
	glUniform1i(TextureID_0, 0);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, texture[2]);
	glUniform1i(TextureID_1, 1);

	glBindVertexArray(earthVAO);

	glUniform1i(hasNormalMappingUniformLocation, 1);

	glUniformMatrix4fv(viewMatrixUniformLocation, 1, GL_FALSE, &skyboxView[0][0]);

	float time = glutGet(GLUT_ELAPSED_TIME);

	glm::mat4 ModelEarth = glm::translate(skyboxModel, glm::vec3(objMoveX - 9.0f, 0.0f, objMoveZ));
	ModelEarth = glm::rotate(ModelEarth, glm::radians(10.0f), glm::vec3(0, 0, 1));
	ModelEarth = glm::rotate(ModelEarth, (float)M_PI / 2, glm::vec3(1.0f, 0.0f, 0.0f));
	ModelEarth = glm::rotate(ModelEarth, -glm::radians(float(time) * 0.05f), glm::vec3(0.0f, 0.0f, 1.0f));
	ModelEarth = glm::scale(ModelEarth, glm::vec3(0.05f, 0.05f, 0.05f));
	glUniformMatrix4fv(modelTransformMatrixUniformLocation, 1, GL_FALSE, &ModelEarth[0][0]);

	if ((objMoveX - 9.0f) >= -0.2f && (objMoveX - 9.0f) <= 0.2f && objMoveZ >= -0.2f && objMoveZ <= 0.2f) {
		shipEarth = true;
	}
	else if (!shipEarth) {
		glDrawArrays(GL_TRIANGLES, 0, drawsize[1]);
	}

	//-------------------------------------------------------------
	//spacecraft

	glBindVertexArray(spaceCraftVAO);
	glUniformMatrix4fv(viewMatrixUniformLocation, 1, GL_FALSE, &skyboxView[0][0]);
	float shipBounce = GLUT_ELAPSED_TIME;
	glm::mat4 ModelObject = glm::translate(temp, glm::vec3(-0.06f, 0.0f, 0.0f));
	ModelObject = glm::rotate(ModelObject, (float)-M_PI/2.0f, vec3(0, 1, 0));
	ModelObject = glm::rotate(ModelObject, 0.18f, vec3(1, 0, 0));
	ModelObject = glm::scale(ModelObject, glm::vec3(0.0001f));

	glUniformMatrix4fv(modelTransformMatrixUniformLocation, 1, GL_FALSE, &ModelObject[0][0]);
	glUniform1i(hasNormalMappingUniformLocation, 0);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture[0]);

	for (int i = 0; i < 5; i++) {
		if (shipRing[i] == true) {
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, texture[6]);
		}
	}

	if (shipCollide) {
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture[6]);
		shipCollide = false;
		for (int c = 1; c <= 1000; c++) {    //do nothing for several seconds
			for (int d = 1; d <= 1000; d++) {

			}
		}
	}

	glDrawArrays(GL_TRIANGLES, 0, drawsize[0]);

	//-------------------------------------------------------------
	//wonderStar

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture[4]);

	glBindVertexArray(wonderStarVAO);
	glUniformMatrix4fv(viewMatrixUniformLocation, 1, GL_FALSE, &skyboxView[0][0]);

	glm::mat4 ModelStar = glm::translate(skyboxModel, glm::vec3(objMoveX - 1.0f, 0.0f, objMoveZ));
	ModelStar = glm::rotate(ModelStar, glm::radians(10.0f), glm::vec3(0, 0, 1));
	ModelStar = glm::rotate(ModelStar, (float)M_PI / 2, glm::vec3(1, 0, 0));
	ModelStar = glm::rotate(ModelStar, glm::radians(float(time) * 0.1f), glm::vec3(0.0f, 0.0f, 1.0f));
	ModelStar = glm::scale(ModelStar, glm::vec3(0.05f));
	glUniformMatrix4fv(modelTransformMatrixUniformLocation, 1, GL_FALSE, &ModelStar[0][0]);
	if ((objMoveX - 1.0f) >= -0.2f && (objMoveX - 1.0f) <= 0.2f && objMoveZ >= -0.2f && objMoveZ <= 0.2f) {
		shipStar = true;
	}
	else if (!shipStar) {
		glDrawArrays(GL_TRIANGLES, 0, drawsize[4]);
	}

	//-------------------------------------------------------------
	//rings

	glUniform1i(hasNormalMappingUniformLocation, 0);

	glBindVertexArray(ringVAO);
	glUniformMatrix4fv(viewMatrixUniformLocation, 1, GL_FALSE, &skyboxView[0][0]);

	float ringTranslate = 2.0f;
	for (int i = 0; i < 4; i++) {
		glm::mat4 selfRotRing = glm::rotate(temp, ringRotate, glm::vec3(0, 1, 0));
		selfRotRing = glm::rotate(selfRotRing, (float)M_PI / 2, glm::vec3(0, 0, 1));
		glm::mat4 translateRing = glm::translate(skyboxModel, glm::vec3(objMoveX - ringTranslate, 0, objMoveZ));
		glm::mat4 ModelRing = translateRing * selfRotRing;
		ModelRing = glm::scale(ModelRing, glm::vec3(1.0f / 400));

		if ((objMoveX - ringTranslate) >= -0.2f && (objMoveX - ringTranslate) <= 0.2f && objMoveZ >= -0.2f && objMoveZ <= 0.2f) {
			shipRing[i] = true;
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, texture[6]);
		}
		else {
			shipRing[i] = false;
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, texture[5]);
		}

		glUniformMatrix4fv(modelTransformMatrixUniformLocation, 1, GL_FALSE, &ModelRing[0][0]);
		glDrawArrays(GL_TRIANGLES, 0, drawsize[5]);
		ringTranslate += 2.0f;

	}

	//-------------------------------------------------------------
	//asteroids/rocks

	float posx[amount];
	float posy[amount];
	float posz[amount];
	srand(GLUT_ELAPSED_TIME);
	float radius = 0.2f + sin(rockExpand / 6) / 5;
	float offset = 0.3f;
	float displacement;

	for (int i = 0; i < amount; i++) {
		glm::mat4 model;
		float angle = (float)i / (float)amount * 360.0f;
		displacement = (rand() % (int)(2 * offset * 200)) / 100.0f - offset;
		float x = sin(angle) * radius + displacement;

		displacement = (rand() % (int)(2 * offset * 200)) / 100.0f - offset;
		float y = displacement * 0.4f + 1;

		displacement = (rand() % (int)(2 * offset * 200)) / 100.0f - offset;
		float z = cos(angle) * radius + displacement;
		posx[i] = objMoveX - x - 0.8f;
		posy[i] = y - 1.15f;
		posz[i] = objMoveZ - z + 0.5f;

		model = glm::translate(skyboxModel, glm::vec3(posx[i], posy[i], posz[i]));

		float scale = (rand() % 10) / 10000.0f + 0.01f;
		model = glm::scale(model, glm::vec3(scale));

		float rotAngle = (rand() % 360);

		model = glm::rotate(model, rotAngle, glm::vec3(0.4f, 0.6f, 0.8f));


		modelMatrices[i] = model;
	}
	glm::mat4 rockTrans = glm::translate(temp, glm::vec3(+0.0f, 0.0f, 0.0f));
	glm::mat4 rockOrbit = glm::rotate(rockTrans, rockRotate, glm::vec3(0, 1, 0));
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture[7]);

	for (int i = 0; i < amount; i++) {
		if (posx[i] >= -0.05f && posx[i] <= 0.05f && posy[i] >= 0.0f && posy[i] <= 0.05f && posz[i] >= -0.05f && posz[i] <= 0.05f && renderRock[i]) {
			renderRock[i] = false;
			cout << "x position: " << posx[i];
			cout << "y position: " << posy[i];
			cout << "z position: " << posz[i];
			shipCollide = true;
		}

		if (renderRock[i] == true) {
			glm::mat4 modelMat = modelMatrices[i] * rockOrbit;
			glUniformMatrix4fv(modelTransformMatrixUniformLocation, 1, GL_FALSE, &modelMat[0][0]);
			glBindVertexArray(rockVAO);
			glDrawArrays(GL_TRIANGLES, 0, drawsize[7]);
		}

		//glm::mat4 ModelObject = glm::translate(temp, glm::vec3(0.0f, 0.0f, -2.8f));
		//(objMoveX - ringTranslate) >= -0.2f && (objMoveX - ringTranslate) <= 0.2f && objMoveZ >= -0.2f && objMoveZ <= 0.2f)

	}

	ringRotate += 0.001f;
	rockRotate += 0.0008f;
	rockExpand += 0.001f;


	glFlush();
	glutPostRedisplay();
}

void initializedGL(void) //run only once
{
	glewInit();
	installShaders();
	sendDataToOpenGL();
}


int main(int argc, char *argv[])
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA);
	glutInitWindowSize(SCREEN_WIDTH, SCREEN_HEIGHT);
	glutInitWindowPosition(0, 0);
	glutCreateWindow("Assignment 2");
	for (int i = 0; i < amount; i++) {
		renderRock[i] = true;
	}

	initializedGL();
	glutDisplayFunc(paintGL);

	glutSpecialFunc(specialKeyboardFunc);
	glutKeyboardFunc(keyboard);
	glutMouseFunc(move);
	glutPassiveMotionFunc(PassiveMouse);

	glutMainLoop();

	return 0;
}