// GLMTest.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include <iostream>
#include <glm/glm.hpp>
#define GLEW_STATIC


#include <GL/glew.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <GLFW/glfw3.h>

#define STB_IMAGE_IMPLEMENTATION
#include "Thirdparty/stb_image.h"

#define GLSL(version,A) "#version " #version "\n" #A
GLFWwindow* window;

using namespace glm;
int Datawidth, Dataheight;
const char * vertexShaderCode = GLSL(120,
	attribute vec4 position;
	attribute vec4 color;
	attribute vec2 textureCoor;
	attribute vec3 normal;

	uniform mat4 model;
	uniform mat4 view;
	uniform mat4 projection;

	varying vec4 dstColor;
	varying vec2 fTextureCoor;
	void main() {
		
		vec3 nNormal = normalize(normal);
		vec3 lightPos = normalize(vec3(10, 10, 10));

		float intensity = max(dot(nNormal, lightPos),0.0f);

		fTextureCoor = textureCoor;
		dstColor = vec4((color.rgb*intensity),1.0);
		gl_Position = projection*view*model*position;
	
	}
);

const char * fragmentShaderCode = GLSL(120,

	uniform sampler2D texture;
	
	varying vec2 fTextureCoor;
	varying vec4 dstColor;
	void main()
	{
		float ambientStrength = 0.1f;
		vec4 light = vec4(1.0, 1.0, 1.0, 1.0)*ambientStrength;
		vec4  material = vec4(1.0, 1.0, 1.0, 1.0);
		//gl_FragColor = dstColor * vec4(1,1,1,1);
		gl_FragColor = texture2D(texture,fTextureCoor);

		//gl_FragColor = (dstColor+light)*material;
	}
);


float axisX = 0;
float axisY = 0;

/*-----------------------------------------------------------------------------
*  FUNCION TO CHECK FOR SHADER COMPILER ERRORS
*-----------------------------------------------------------------------------*/

void compilerCheck(GLuint ID) {
	GLint comp;
	glGetShaderiv(ID, GL_COMPILE_STATUS, &comp);

	if (comp == GL_FALSE) {
		std::cout << "Shader Compilation FAILED" << std::endl;
		GLchar messages[256];
		glGetShaderInfoLog(ID, sizeof(messages), 0, &messages[0]);
		std::cout << messages;
	}
}

void linkCheck(GLuint ID) {
	GLint linkStatus, validateStatus;
	glGetProgramiv(ID, GL_LINK_STATUS, &linkStatus);

	if (linkStatus == GL_FALSE) {
		std::cout << "Shader Linking FAILED" << std::endl;
		GLchar messages[256];
		glGetProgramInfoLog(ID, sizeof(messages), 0, &messages[0]);
		std::cout << messages;
	}

	glValidateProgram(ID);
	glGetProgramiv(ID, GL_VALIDATE_STATUS, &validateStatus);

	std::cout << "Link: " << linkStatus << "  Validate: " << validateStatus << std::endl;
	if (linkStatus == GL_FALSE) {
		std::cout << "Shader Validation FAILED" << std::endl;
		GLchar messages[256];
		glGetProgramInfoLog(ID, sizeof(messages), 0, &messages[0]);
		std::cout << messages;
	}
}

/*-----------------------------------------------------------------------------
*  Start Main Function
*-----------------------------------------------------------------------------*/
#include <vector>

struct Vertex
{
	Vertex() = default;
	/*Vertex(float _x, float _y, float _z , float _r , float _g , float _b)
	{
		position.x = _x;
		position.y = _y;
		position.z = _z;

		color.r = _r;
		color.g = _g;
		color.b = _b;
		color.a = 1.0f;
	};*/

	glm::vec3 position;
	glm::vec4 color;
	glm::vec2 texCoor;
	glm::vec3 normal;
};

std::vector<Vertex> model;
std::vector<GLuint> modelIndices;

float boxStartA = 0;
float boxStartB = 0;


void initModel()
{
	//Specify the 8 VERTICES of A Cube
	Vertex cube[] = {
	{glm::vec3(1, -1,  1), glm::vec4(1,1,1,1),	glm::vec2(1,1), glm::vec3(1, -1,  1)},
	{glm::vec3(1,  1,  1), glm::vec4(1,1,1,1),	glm::vec2(1,0),glm::vec3(1,  1,  1)},
	{glm::vec3(-1,  1,  1), glm::vec4(1,1,1,1),	glm::vec2(0,0),glm::vec3(-1,  1,  1)},
	{glm::vec3(-1, -1,  1), glm::vec4(1,1,1,1),	glm::vec2(0,1),glm::vec3(-1, -1,  1)},
	/*
	{glm::vec3(1, -1, -1), glm::vec4(0,1,0,1)},
	{glm::vec3(1,  1, -1), glm::vec4(0,0,1,1)},
	{glm::vec3(-1,  1, -1), glm::vec4(1,0,0,1)},
	{glm::vec3(-1, -1, -1), glm::vec4(0,1,0,1)}*/
	};

	model.assign(std::begin(cube), std::end(cube));


	GLubyte indices[24] = {
					 0,1,2,3/*, //front
					 7,6,5,4, //back
					 3,2,6,7, //left
					 4,5,1,0, //right
					 1,5,6,2, //top
					 4,0,3,7 */}; //bottom

	modelIndices.assign(std::begin(indices), std::end(indices));

/*
	model.emplace_back(Vertex(-1,-0.5,0,
								1.0f,0.0f,0.0f));
	model.emplace_back(Vertex(0,1,0,
								0.0f, 1.0f, 0.0f));
	model.emplace_back(Vertex(1,-0.5,0,
								0.0f, 0.0f, 1.0f));*/
}

GLuint sID;
GLuint positionID;
GLuint colorID;

GLuint normalID;

GLuint texCoorID;
GLuint elementID;
GLuint textureID;

GLuint modelID;
GLuint viewID;
GLuint projectionID;

void initShaders()
{
	sID = glCreateProgram();
	GLuint vID = glCreateShader(GL_VERTEX_SHADER);
	GLuint fID = glCreateShader(GL_FRAGMENT_SHADER);

	//2. LOAD SHADER SOURCE CODE
	glShaderSource(vID, 1, &vertexShaderCode, NULL);
	glShaderSource(fID, 1, &fragmentShaderCode, NULL);

	//3. COMPILE
	glCompileShader(vID);
	glCompileShader(fID);

	compilerCheck(vID);
	compilerCheck(fID);

	glAttachShader(sID, vID);
	glAttachShader(sID, fID);

	glLinkProgram(sID);

	linkCheck(sID);

	glUseProgram(sID);

	positionID = glGetAttribLocation(sID, "position");
	colorID = glGetAttribLocation(sID, "color");
	texCoorID = glGetAttribLocation(sID, "textureCoor");
	normalID = glGetAttribLocation(sID, "normal");

	modelID = glGetUniformLocation(sID, "model");
	viewID = glGetUniformLocation(sID, "view");
	projectionID = glGetUniformLocation(sID, "projection");


	glUseProgram(0);
}

GLuint arrayID;
void createBuffer()
{
	
	GLuint bufferID;
	glGenVertexArrays(1, &arrayID);
	glBindVertexArray(arrayID);
	glGenBuffers(1, &bufferID);
	glBindBuffer(GL_ARRAY_BUFFER, bufferID);
	//send vertex data to GPU memnory
	glBufferData(GL_ARRAY_BUFFER, model.size()*sizeof(Vertex),model.data(),GL_STATIC_DRAW);


	glGenBuffers(1, &elementID);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementID);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, modelIndices.size()*sizeof(GLuint), modelIndices.data(),GL_STATIC_DRAW);
	//tell a shader program to get access to the data
	glEnableVertexAttribArray(positionID);
	glVertexAttribPointer(positionID, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
	glEnableVertexAttribArray(colorID);
	glVertexAttribPointer(colorID, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)sizeof(glm::vec3));
	glEnableVertexAttribArray(texCoorID);
	glVertexAttribPointer(texCoorID, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(glm::vec3) + sizeof(glm::vec4)) );
	glEnableVertexAttribArray(normalID);
	glVertexAttribPointer(normalID, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(glm::vec3) + sizeof(glm::vec4) + sizeof(glm::vec2)));



	//unbind
	glBindVertexArray(0);

	glBindBuffer(GL_ARRAY_BUFFER,0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

/*-----------------------------------------------------------------------------
	   *  Make some rgba data (can also load a file here)
	   *-----------------------------------------------------------------------------*/

unsigned int tw = 40;
unsigned int th = 40;
std::vector<vec4> textureData;


void initTexture() {
	/*bool checker = false;
	for (int i = 0; i < tw; ++i) {
		float tu = (float)i / tw;
		for (int j = 0; j < th; ++j) {
			float tv = (float)j / th;
			textureData.push_back(vec4(tu, 0, tv, checker));
			checker = !checker;
		}
		checker = !checker;
	}*/
	int width, height, nrChannels;

	unsigned char *data = stbi_load("Assets/EnemySword1.png", &width, &height, &nrChannels, 0);

	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glGenerateMipmap(GL_TEXTURE_2D);
	
	if (nrChannels == 3) {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	}
	if (nrChannels == 4) {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	}

	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glGenerateMipmap(GL_TEXTURE_2D);

	//unbind
	glBindTexture(GL_TEXTURE_2D, 0);

	stbi_image_free(data);

}

void UpdateTexture() {
	/*bool checker = false;
	for (int i = 0; i < tw; ++i) {
		float tu = (float)i / tw;
		for (int j = 0; j < th; ++j) {
			float tv = (float)j / th;
			textureData.push_back(vec4(tu, 0, tv, checker));
			checker = !checker;
		}
		checker = !checker;
	}*/
	int width, height, nrChannels;

	unsigned char *data = stbi_load("Assets/EnemySword2.png", &width, &height, &nrChannels, 0);

	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glGenerateMipmap(GL_TEXTURE_2D);

	if (nrChannels == 3) {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	}
	if (nrChannels == 4) {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	}

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glGenerateMipmap(GL_TEXTURE_2D);

	//unbind
	glBindTexture(GL_TEXTURE_2D, 0);

	stbi_image_free(data);

}

void drawArray()
{
	glUseProgram(sID);
	
	glBindTexture(GL_TEXTURE_2D, textureID);

	glfwGetWindowSize(window, &Datawidth, &Dataheight);
	glViewport(0, 0, Datawidth, Dataheight);
	
	static float camPos = 8.0f;
	//camPos += 0.1f;
	if (camPos > 9.0f)
		camPos = 1.0f;

	vec3 eyepos = vec3(0, 0, camPos);
	mat4 viewMat = glm::lookAt(eyepos,			//eye position
								vec3(0,0,0),	//target
								vec3(0,1,0));	//up vector

	//float screen_width = Datawidth;
	//float screen_height = Dataheight;
	
	float screen_width = 1280;
	float screen_height = 720;
	
	float ratio = screen_width / static_cast<float>(screen_height);
	mat4 projectionMat = glm::perspective(1.0f, 
									ratio, 0.1f, -10.0f);

	vec3 scaleValue = vec3(2.0f,2.0f,2.0f);
	vec3 translationValue = vec3(0.0f,0.0f,0.0f);
	vec3 axis = vec3(0, 0, 1);
	static float radius = 0.0f;
	//radius += 0.1;

	mat4 rotationMat = glm::rotate(mat4(1),radius,axis);
	mat4 translationMat = glm::translate(mat4(1), translationValue);
	mat4 scaleMat = glm::scale(glm::mat4(1),scaleValue);

	mat4 modelMat = translationMat* rotationMat*scaleMat;

	glUniformMatrix4fv(viewID, 1, GL_FALSE, glm::value_ptr(viewMat));
	glUniformMatrix4fv(modelID, 1, GL_FALSE, glm::value_ptr(modelMat));
	glUniformMatrix4fv(projectionID, 1, GL_FALSE, glm::value_ptr(projectionMat));

	glBindVertexArray(arrayID);

	glDrawElements(GL_QUADS, modelIndices.size(), GL_UNSIGNED_INT, 0);

	//glDrawArrays(GL_TRIANGLES, 0, 3);

	glBindTexture(GL_TEXTURE_2D, 0);
	glBindVertexArray(0);
	glUseProgram(0);



}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
	{
		glfwTerminate();
		exit(0);
	}

}


int main(void)
{
	

	/* Initialize the library */
	if (!glfwInit())
		return -1;

	/* Create a windowed mode window and its OpenGL context */
	window = glfwCreateWindow(1280, 720, "Hello World", NULL, NULL);
	glfwSetWindowSize(window, 1280, 720);
	
	if (!window)
	{
		glfwTerminate();
		return -1;
	}

	/* Make the window's context current */
	glfwMakeContextCurrent(window);

	GLenum glewError = glewInit();
	if (glewError != GLEW_OK)
	{
		std::cout << "GLEW is not working" << std::endl;
		return -1;
	}

	if (!GLEW_VERSION_2_1)
	{
		std::cout << "OpenGL2.1 does not supported." << std::endl;
		return -1;
	}

	if (GLEW_ARB_vertex_array_object) {
		std::cout << "vertex arrays is supported" << std::endl;
	}

	const GLubyte *glVersion = glGetString(GL_VERSION);
	std::cout << "Graphics driver : " << glVersion << std::endl;

	const GLubyte *glslVersion = glGetString(GL_SHADING_LANGUAGE_VERSION);
	std::cout << "GLSL version : " << glslVersion << std::endl;

	glEnable(GL_DEPTH_TEST);

	initModel();
	initShaders();
	createBuffer();
	initTexture();
	glfwSetKeyCallback(window, key_callback);
	
	int TextureSwap = 0;

	/* Loop until the user closes the window */
	while (!glfwWindowShouldClose(window))
	{
		/* Render here */
		glClearColor(0.5f, 1.0f, 0.7f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
		
		drawArray();

		if (TextureSwap == 0) {
			UpdateTexture();
			TextureSwap++;
		}
		else {
			initTexture();
			TextureSwap--;
		}

		/* Swap front and back buffers */
		glfwSwapBuffers(window);

		/* Poll for and process events */
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
