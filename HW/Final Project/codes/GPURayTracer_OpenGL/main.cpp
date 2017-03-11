#include <iostream>
#define GLEW_STATIC 
#include <GL\glew.h>
#include <GLFW\glfw3.h>
#include "shader.h"

static GLuint CreateTexture(const int TextureId, GLenum format, const int size)
{
	///invoke all necessary opengl calls to create texture memory
	GLuint myTexture;
	glActiveTexture(GL_TEXTURE0 + TextureId);
	glGenTextures(1, &myTexture);
	glBindTexture(GL_TEXTURE_2D, myTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexImage2D(GL_TEXTURE_2D, 0, /* target, level of detail */
		         format,  /* internal format */
				 size, size, 0,/* width, height, border */
		         GL_LUMINANCE, GL_FLOAT,/* external format, type */
		         0);
	return myTexture;
}

GLFWwindow* CreateMainWindow(int Width, int Height)
{
	//initialize the window by the given hight and width 

	glfwInit();//initialize GLFW
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);//opengl version major
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);//opengl version minor
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);//don't let the user resize my window 
	GLFWwindow* window = glfwCreateWindow(Width, Height, "EEC277 Project", nullptr, nullptr);//create window object
	if (window == nullptr){
		std::cout << "Failed to create GLFW window!!!" << std::endl;
		glfwTerminate();
		system("pause");
	}
	glfwMakeContextCurrent(window);//make the conext of my window the main context on the current thread	
	glewExperimental = GL_TRUE;//enusre using more moder techniques (!!!) for managing opengl functionality
	if (glewInit() != GLEW_OK){
		std::cout << "Failed to initialize GLEW!!" << std::endl;
		system("pause");
	}
	int w, h;
	glfwGetFramebufferSize(window, &w, &h);
	glViewport(0, 0, w, h);
	return window;
	
}
void CreateMainScene(GLuint&VAO, GLuint&VBO)
{
	//create the main scene (two gain triangles inside which everything else is drawn)
	//returns the VAO and VBO
	// VAO --> vertex array object
	// VBO --> vertex buffer object

	GLfloat vertices[] = {
		0.9f, 0.9f,  //top right
		0.9f, -0.9f,//bottom right
		-0.9f, -0.9f, //top left
		-0.9f, 0.9f //top left
	};

	const GLuint  indices[] = { //indexing starts from zero
		0, 1, 3, //first triangle
		1, 2, 3 //second triangle 
	};

	//Creat objects for VBO, EBO, VAO
	

	GLuint EBO;//element buffer object

	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glGenBuffers(1, &EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);


	//Position Attribute
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FLOAT, 2 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);


	//Unbind the VAO (DO not bind EBO, this causes error) 
	//glBindBuffer(GL_ARRAY_BUFFER, 0);// Note that this is allowed, the call to glVertexAttribPointer registered VBO as the currently bound vertex buffer object so afterwards we can safely unbind	
	//glBindVertexArray(0);//Unbibd VAO (good thing to do to unbind any buffer/array to preven strange bugs)
		
}


int main(int argc, char*argv)
{
	//===============================================  Initialize the window and read, compile, and link the shaders ===============================================//
	GLFWwindow* myWin= CreateMainWindow(800, 600);
	Shader ourShader("basicVShader2.glsl", "basicFShader2.glsl");	

	//float spheres[] = { 0.7, 0, -1.5, 0.1 };
	
	//int textID=0;
	//GLuint sphereText = CreateTexture(textID, GL_RGBA32F_ARB, sizeof(spheres));
	//=============================================== Initialize the main scene: two big triangles ===============================================//		
	GLuint VBO, VAO;
	CreateMainScene(VAO, VBO);
	
	


	//=============================================== Main Loop ===============================================//
	while (!glfwWindowShouldClose(myWin)){//checks if the window closes
		glfwPollEvents();//checks for events (mouse, keyboard)

		//rendering commands 
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);//clear the screen		
		glClear(GL_COLOR_BUFFER_BIT);//specify which buffer to clear 

		ourShader.Use();
		
		//Draw object 
		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0); //First parameter is the mode we want to draw in 
		                                                     //Second parameter count of the number of elements want to draw (6 indices as we want to draw 6 vertices in total)
		                                                     //Third parameter type of the indices 
		                                                     //Forth parameter is offset in the EBO 
		glfwSwapBuffers(myWin);
	}

	//de-allocate all resources 
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);

	
	//terminate GLFW
	glfwTerminate();
	return 0;
}
