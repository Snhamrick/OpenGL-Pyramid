#include <glad/glad.h>
#include <GLFW/glfw3.h>     // GLFW library


#include <iostream>         // cout, cerr
#include <cstdlib>			// EXIT_FAILURE

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "shader.h"

// GLM Inclusions
#include <glm/glm.hpp>		
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>



using namespace std;

namespace
{
	//Constants for Window Size
	const int WINDOW_WIDTH = 800;
	const int WINDOW_HEIGHT = 600;

	//Structure for Mesh
	struct GLMesh {
		GLuint vaos[2];									//Variable for mesh VAO
		GLuint vbos[2];									//Variable for mesh VBOs
		GLuint nVertices;								//Variable for mesh vertices (unrequired but left for modification convinence)
		GLuint nIndices;								//Cariable for mesh indices
	};

	//Vairables for Main Window, Mesh, Shader Program, TextureID
	GLFWwindow* window = nullptr;
	GLMesh mesh;
	GLuint textureID;

	//Shader Programs
	GLuint programID;
	GLuint rightLightProgramID;
	GLuint leftLightProgramID;

	//Variables for Camera Positioning
	glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
	glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
	glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

	//Cursor Input Variables
	bool firstMouse = true;
	float yaw = -90.0f;	
	float pitch = 0.0f;
	float lastX = 800.0f / 2.0;
	float lastY = 600.0 / 2.0;

	//Pyramid Color and Position
	glm::vec3 pyramidPos(0.0f, 0.0f, 0.0f);
	glm::vec3 pyramidScale(2.0f, 2.0f, 2.0f);

	//Light Color and Position
	glm::vec3 rightLightPos(1.0f, 2.0f, 0.0f);
	glm::vec3 leftLightPos(-4.0f, -1.0f, 0.0f);
	glm::vec3 rightLightScale(0.5f, 0.5f, 0.5f);
	glm::vec3 leftLightScale(0.5f, 0.5f, 0.5f);
	glm::vec3 rightLightColor(0.1f, 5.0f, 0.1f);
	glm::vec3 leftLightColor(9.0f, 0.1f, 0.1f);


	//Time and Speed Variables
	float deltaTime = 0.0f;
	float lastFrame = 0.0f;
	float speedInput = 1.0f;
}

/*Defined functions
Function to initialize Window, Function to Create Texture, Function to Process User Input,
Function to for Cursor Callback, Function for Scroll Callback, Function to resize Window,
Function to Create Shader Program, Function to destroy Shader Program, Function to Create Mesh, 
Function to Destroy Mesh, Function to Render Object, Function to Flip Texture Image Vertically*/

bool InitializeWindow(int, char* [], GLFWwindow** window);
unsigned int CreateTexture(const char* filename);
void ProcessInput(GLFWwindow* window);
void Cursor_callback(GLFWwindow* window, double xposIn, double yposIn);
void Scroll_callback(GLFWwindow* window, double xOffset, double yOffset);
void WindowResize(GLFWwindow* window, int width, int height);
bool CreateShaderProgram(const char* vertexShaderSource, const char* fragShaderSource, GLuint& programID);
void DestroyShaderProgram(GLuint programID);
void CreateMesh(GLMesh& mesh);
void DestroyMesh(GLMesh& mesh);
void Render();
void flipImageVertically(unsigned char* image, int width, int height, int channels);


//MAIN FUNCTION

int main(int argc, char* argv[]) {

	if (!InitializeWindow(argc, argv, &window)) {											//Initialize window and check
		return EXIT_FAILURE;
	}

	// glad: load all OpenGL function pointers
	// ---------------------------------------
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return EXIT_FAILURE;
	}

	
	CreateMesh(mesh);																		//Create Mesh


	glClearColor(0.0f, 0.0f, 0.0f, 0.1f);



	//RENDER LOOP

	while (!glfwWindowShouldClose(window)) {	

		float currentFrame = static_cast<float>(glfwGetTime());			//Determine time difference since previous frame
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;
		ProcessInput(window);											//Process User Input

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);									//Set background color to desired color

		
		Render();														//Render Object

		glfwPollEvents();												//Check for user input
	}

	DestroyMesh(mesh);													//Destroy Mesh
	DestroyShaderProgram(programID);									//Destroy Shader Program

	exit(EXIT_SUCCESS);													//EXIT
}










bool InitializeWindow(int, char* [], GLFWwindow** window) {												//Function to Initialize window

	//Initialize GLFW
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	//Platform Compatiability
#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	//Create Window
	* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "PyramidWindow", NULL, NULL);
	//Check Window Creation
	if (window == NULL) {
		std::cout << "Failed to Create GLFW Window" << std::endl;
		glfwTerminate();
		return false;																			//Failure returns false
	}	

	//Set Window to Current and FramebufferSizeCallback
	glfwMakeContextCurrent(*window);
	glfwSetFramebufferSizeCallback(*window, WindowResize);										//Allow user to resize window
	glfwSetCursorPosCallback(*window, Cursor_callback);											//Allow cursor input
	glfwSetScrollCallback(*window, Scroll_callback);											//Allow scroll input

	glfwSetInputMode(*window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);								//Set to recieve mouse input

	return true;																				//Success returns true
}



void ProcessInput(GLFWwindow* window) {																			//Function to Process User Input
	
	//Constant for camera movement speed
	float cameraSpeed = static_cast<float>(speedInput * deltaTime);
	//Camera Movement 
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) cameraPos += cameraSpeed * cameraFront;												//W: Forward
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) cameraPos -= cameraSpeed * cameraFront;												//S: Backward
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;			//A: Left
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;			//D: Right
	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) cameraPos += cameraSpeed * cameraUp;													//E: Up
	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) cameraPos -= cameraSpeed * cameraUp;													//Q: Down

	//IF user presses escape close the window
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)glfwSetWindowShouldClose(window, true);				
}

void Cursor_callback(GLFWwindow* window, double xposIn, double yposIn)																		//Function for Cursor Callback
{
	float xpos = static_cast<float>(xposIn);
	float ypos = static_cast<float>(yposIn);

		if (firstMouse)								//Check to see if it is first input
		{
			lastX = xpos;							//If so measure offsets from here
			lastY = ypos;
			firstMouse = false;
		}


		//Determine offset difference
		float xoffset = xpos - lastX;				
		float yoffset = lastY - ypos; 
		lastX = xpos;
		lastY = ypos;

		float sensitivity = 0.4f;					//Set Cursor Sensitivity
		yoffset *= sensitivity;

		yaw += xoffset;								//Adjust yaw and pitch
		pitch += yoffset;

		//Ensure screeen will not flip out of bounds
		if (pitch > 89.0f)
			pitch = 89.0f;
		if (pitch < -89.0f)
			pitch = -89.0f;

		//Alter the cameraFront accordingly
		glm::vec3 front;
		front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
		front.y = sin(glm::radians(pitch));
		front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
		cameraFront = glm::normalize(front);
	}


void Scroll_callback(GLFWwindow* window, double xOffset, double yOffset) {					//Function for Scroll Callback
	speedInput -= (float)yOffset;															//Speed input eqaul to the offset
	if (speedInput < 1.0f)																	//Minimum camera speed is 1.0
		speedInput = 1.0f;
	if (speedInput > 7.0f)																	//Maximum camera speed is 7.0
		speedInput = 7.0f;
}


void WindowResize(GLFWwindow* window, int width, int height) {													//Function to Resize Window

	//Set Window to new Width and Height
	glViewport(0, 0, width, height);																			
}


void CreateMesh(GLMesh& mesh) {													//Function to Create Mesh

	//Vertex Array
	GLfloat vertices[] = {

		//Triangle 1 (Front Face)
		0.0f, 0.5f, 0.0f,   0.0f, 0.0f, 1.0f,   0.5f, 1.0f,				//Apex Index 0
		-0.5f, -0.5f, 0.5f,	0.0f, 0.0f, 1.0f,	0.0f, 0.0f,				//Front Left Index 1
		0.5f, -0.5f, 0.5f,  0.0f, 0.0f, 1.0f,  	1.0f, 0.0f,				//Front Right Index 2

		//Triangle 2 (Left Face)
		0.0f, 0.5f, 0.0f,     -1.0f, 0.0f, 0.0f, 0.5f, 1.0f,  					//Apex Index 0
		-0.5f, -0.5f, 0.5f,   -1.0f, 0.0f, 0.0f, 1.0f, 0.0f,					//Front Left Index 1
		-0.5f, -0.5f, -0.5f,  -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 					//Back Left Index 3
		
		//Triangle 3 (Back Face)
		0.0f, 0.5f, 0.0f,     0.0f, 0.0f, -1.0f, 0.5f, 1.0f,  					//Apex Index 0
		-0.5f, -0.5f, -0.5f,  0.0f, 0.0f, -1.0f, 1.0f, 0.0f,					//Back Left Index 3
		0.5f, -0.5f, -0.5f,	  0.0f, 0.0f, -1.0f, 0.0f, 0.0f,					//Back Right Index 4

		//Triangle 4 (Right Face)
		0.0f, 0.5f, 0.0f,   1.0f, 0.0f, 0.0f, 0.5f, 1.0f, 						//Apex Index 0
		0.5f, -0.5f, 0.5f,  1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 						//Front Right Index 2
		0.5f, -0.5f, -0.5f,	1.0f, 0.0f, 0.0f, 1.0f, 0.0f,						//Back Right Index 4

		//Triangle 5 (Left Base)
		-0.5f, -0.5f, 0.5f,  0.0f, -1.0f, 0.0f, 0.0f, 0.0f,     				//Front Left Index 1
		-0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f,					//Back Left Index 3
		0.5f, -0.5f, -0.5f,	 0.0f, -1.0f, 0.0f, 1.0f, 1.0f,					//Back Right Index 4

		//Triangle 6 (Right Base)
		-0.5f, -0.5f, 0.5f,  0.0f, -1.0f, 0.0f,	 0.0f, 0.0f,    					//Front Left Index 1
		0.5f, -0.5f, 0.5f,   0.0f, -1.0f, 0.0f, 1.0f, 0.0f,   					//Front Right Index 2
		0.5f, -0.5f, -0.5f,	 0.0f, -1.0f, 0.0f,  1.0f, 1.0f 					//Back Right Index 4
	};


	glGenVertexArrays(2, &mesh.vaos[0]);												//Generate mesh VAO
	glGenBuffers(2, mesh.vbos);														//Generate 2 mesh VBO
	glBindVertexArray(mesh.vaos[0]);													//Bind VAO
	glBindBuffer(GL_ARRAY_BUFFER, mesh.vbos[0]);									//Bind first VBO Array Buffer
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);		//Set Buffer Data

	//Indices Array
	GLushort indices[] = { 
		0,1,2,							//Front of Pyramid
		0,1,3,							//Left of Pyramid
		3,4,0,							//Back of Pyramid
		0,4,2,							//Right of Pyramid
		3,4,1,							//Posterior Part of Base
		1,4,2							//Anterior Part of Base
};

	mesh.nIndices = sizeof(indices) / sizeof(indices[0]);																//Set mesh number of indices
	const GLuint floatsPerVertex = 3;																					//Vec3 for Position
	const GLuint floatsPerNormal = 3;																					//Vec2 for Texture
	const GLuint floatsPerTexture = 2;
	GLint stride = sizeof(float) * (floatsPerVertex + floatsPerNormal + floatsPerTexture);													//Set Stride
	glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);											//First Vertex Attribute Pointer is Position
	glEnableVertexAttribArray(0);																						//Enable Position
	glVertexAttribPointer(1, floatsPerNormal, GL_FLOAT, GL_FALSE, stride, (char*)(sizeof(float) * floatsPerVertex));	//Second Vertex Attribute Pointer is Texture
	glEnableVertexAttribArray(1);																						//Enable Texture 
	glVertexAttribPointer(2, floatsPerTexture, GL_FLOAT, GL_FALSE, stride, (char*)(sizeof(float) * (floatsPerVertex + floatsPerNormal)));
	glEnableVertexAttribArray(2);


	glBindVertexArray(mesh.vaos[1]);
	glBindBuffer(GL_ARRAY_BUFFER, mesh.vbos[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, (void*)0);
	glEnableVertexAttribArray(0);



}

unsigned int CreateTexture(const char* filename) {

	unsigned int textureID;
	glGenTextures(1, &textureID);

	int width, height, nrComponents;
	unsigned char* data = stbi_load(filename, &width, &height, &nrComponents, 0);
	if (data)
	{
		GLenum format;
		if (nrComponents == 1)
			format = GL_RED;
		else if (nrComponents == 3)
			format = GL_RGB;
		else if (nrComponents == 4)
			format = GL_RGBA;

		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);
	}
	else
	{
		std::cout << "Texture failed to load at path: " << filename << std::endl;
		stbi_image_free(data);
	}

	return textureID;
}

void DestroyMesh(GLMesh& mesh) {																					//Function to Destroy Mesh

	//Delete VAOs and VBOs
	glDeleteVertexArrays(2, mesh.vaos);
	glDeleteBuffers(2, mesh.vbos);

}

bool CreateShaderProgram(const char* vertexShaderSource, const char* fragShaderSource, GLuint& programID) {			//Function to Create Shader Program

	//Variable to check initialization and return message
	int success = 0;
	char infoLog[512];

	programID = glCreateProgram();																	//Create Program

	GLuint vertexShaderID = glCreateShader(GL_VERTEX_SHADER);										//Create Vertex Shader
	GLuint fragShaderID = glCreateShader(GL_FRAGMENT_SHADER);										//Create Fragment Shader

	glShaderSource(vertexShaderID, 1, &vertexShaderSource, NULL);									//Use Vertex Shader Source Code
	glShaderSource(fragShaderID, 1, &fragShaderSource, NULL);										//Use Fragment Shader Source Code

	glCompileShader(vertexShaderID);																//Compile Vertex Shader

	glGetShaderiv(vertexShaderID, GL_COMPILE_STATUS, &success);										//Check and report success of Vertex Shader Compilation
	if (!success) {
		glGetShaderInfoLog(vertexShaderID, 512, NULL, infoLog);
		std::cout << "ERROR: VERTEX SHADER:: COMPILATION FAILURE\n" << infoLog << std::endl;
		return false;
	}

	glCompileShader(fragShaderID);																//Compile Fragment Shader

	glGetShaderiv(fragShaderID, GL_COMPILE_STATUS, &success);									//Check and report success of Fragment Shader Compilation
	if (!success) {
		glGetShaderInfoLog(fragShaderID, 512, NULL, infoLog);
		std::cout << "ERROR: FRAGMENT SHADER:: COMPILATION FAILURE\n" << infoLog << std::endl;
		return false;
	}

	glAttachShader(programID, vertexShaderID);													//Attach Vertex Shader to program
	glAttachShader(programID, fragShaderID);													//Attach Fragment Shader to Program

	glLinkProgram(programID);																	//Link Program

	glGetProgramiv(programID, GL_LINK_STATUS, &success);										//Check and report status of linking
	if (!success) {
		glGetProgramInfoLog(programID, sizeof(infoLog), NULL, infoLog);
		std::cout << "ERROR: SHADER PROGRAM:: LINKING FAILURE\n" << infoLog << std::endl;
		return false;
	}

	glUseProgram(programID);																	//Use Shader Program
		
	return true;																				//Succes returns true

}

void DestroyShaderProgram(GLuint programID) {											//Function to destroy shader progam
	
	//Delete Shader Program
	glDeleteProgram(programID);
}

void Render() {																				//Function to Render Object

		glEnable(GL_DEPTH_TEST);															//Enable Depth Test for 3D rendering

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);													//Clear Frame and Z-Buffers
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

																//Bind mesh VAO

	Shader lightShader("shaderfiles/6.light_cube.vs", "shaderfiles/6.light_cube.fs");
	Shader pyramidShader("shaderfiles/6.multiple_lights.vs", "shaderfiles/6.multiple_lights.fs");


	unsigned int diffusedMap = CreateTexture("brickWall.png");
	unsigned int specularMap = CreateTexture("brickWall.png");



	pyramidShader.use();																//Use shader program
	pyramidShader.setInt("material.diffuse", 0);
	pyramidShader.setInt("material.specular", 1);
	pyramidShader.setFloat("material.shininess", 25.0f);
	pyramidShader.setVec3("viewPos", cameraPos);


	// directional light
	pyramidShader.setVec3("dirLight.direction", -0.5f, -1.0f, 1.3f);
	pyramidShader.setVec3("dirLight.ambient", 0.05f, 0.05f, 0.05f);
	pyramidShader.setVec3("dirLight.diffuse", 0.4f, 0.4f, 0.4f);
	pyramidShader.setVec3("dirLight.specular", 0.5f, 0.5f, 0.5f);

	// point light 1
	pyramidShader.setVec3("pointLights[0].position", rightLightPos);
	pyramidShader.setVec3("pointLights[0].color", rightLightColor);
	pyramidShader.setVec3("pointLights[0].ambient", 0.5f, 0.5f, 0.5f);
	pyramidShader.setVec3("pointLights[0].diffuse", 0.1f, 0.1f, 0.1f);
	pyramidShader.setVec3("pointLights[0].specular", 0.5f, 0.5f, 0.5f);
	pyramidShader.setFloat("pointLights[0].constant", 1.0f);
	pyramidShader.setFloat("pointLights[0].linear", 0.09);
	pyramidShader.setFloat("pointLights[0].quadratic", 0.032);

	pyramidShader.setVec3("pointLights[1].position", leftLightPos);
	pyramidShader.setVec3("pointLights[1].color", leftLightColor);
	pyramidShader.setVec3("pointLights[1].ambient", 0.1f, 0.1f, 0.1f);
	pyramidShader.setVec3("pointLights[1].diffuse", 0.1f, 0.1f, 0.1f);
	pyramidShader.setVec3("pointLights[1].specular", 0.1f, 0.1f, 0.1f);
	pyramidShader.setFloat("pointLights[1].constant", 1.0f);
	pyramidShader.setFloat("pointLights[1].linear", 0.09);
	pyramidShader.setFloat("pointLights[1].quadratic", 0.032);

	// spotLight
	pyramidShader.setVec3("spotLight.position", rightLightPos);
	pyramidShader.setVec3("spotLight.direction", cameraFront);
	pyramidShader.setVec3("spotLight.ambient", 0.0f, 0.0f, 0.0f);
	pyramidShader.setVec3("spotLight.diffuse", 1.0f, 1.0f, 1.0f);
	pyramidShader.setVec3("spotLight.specular", 1.0f, 1.0f, 1.0f);
	pyramidShader.setFloat("spotLight.constant", 1.0f);
	pyramidShader.setFloat("spotLight.linear", 0.09);
	pyramidShader.setFloat("spotLight.quadratic", 0.032);
	pyramidShader.setFloat("spotLight.cutOff", glm::cos(glm::radians(12.5f)));
	pyramidShader.setFloat("spotLight.outerCutOff", glm::cos(glm::radians(15.0f)));
	
	

	//Model, View Pojection
	glm::mat4 model = glm::translate(pyramidPos) * glm::scale(pyramidScale);				//Set Model equal to all the transformation

	glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);				//Set View using LookAt with cameraDirections

	glm::mat4 projection = glm::perspective(45.0f, (GLfloat)WINDOW_WIDTH / (GLfloat)WINDOW_HEIGHT, 0.1f, 100.0f);	//Set Projection using perspective with FOV 45*

	pyramidShader.setMat4("model", model);
	pyramidShader.setMat4("view", view);
	pyramidShader.setMat4("projection", projection);


	// bind diffuse map
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, diffusedMap);
	// bind specular map
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, specularMap);

	
	glBindVertexArray(mesh.vaos[0]);
	glDrawArrays(GL_TRIANGLES, 0, 18);
	

	lightShader.use();
	model = glm::translate(rightLightPos) * glm::scale(rightLightScale);
	lightShader.setMat4("model", model);
	lightShader.setMat4("view", view);
	lightShader.setMat4("projection", projection);

	glBindVertexArray(mesh.vaos[1]);

	glDrawArrays(GL_TRIANGLES, 0, 18);

	lightShader.use();
	model = glm::translate(leftLightPos) * glm::scale(leftLightScale);
	lightShader.setMat4("model", model);
	lightShader.setMat4("view", view);
	lightShader.setMat4("projection", projection);

	glBindVertexArray(mesh.vaos[1]);

	glDrawArrays(GL_TRIANGLES, 0, 18);

	

																		

	glfwSwapBuffers(window);																//Swap Buffers
}


void flipImageVertically(unsigned char* image, int width, int height, int channels)														//Function to flip texture image vertically
{
	for (int j = 0; j < height / 2; ++j)															//For half of the image
	{
		int index1 = j * width * channels;															//Set Indices
		int index2 = (height - 1 - j) * width * channels;

		for (int i = width * channels; i > 0; --i)													//Loop through image horizontally
		{
			unsigned char tmp = image[index1];														//temp holder
			image[index1] = image[index2];															//swap indices
			image[index2] = tmp;
			++index1;																				//move to next index
			++index2;																				//move to next index
		}
	}
}