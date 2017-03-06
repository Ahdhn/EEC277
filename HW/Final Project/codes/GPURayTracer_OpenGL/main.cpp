#include <iostream>
#define GLEW_STATIC 
#include <GL\glew.h>
#include <GLFW\glfw3.h>
#include <memory>
#include "shader.h"

void KeyCallback(GLFWwindow*window, int key, int scancode, int action, int mode)
{
	//when a user presses the escape key, we set the WindowShouldClose propertry to true,
	//closing the application
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS){
		glfwSetWindowShouldClose(window, GL_TRUE);
	}
}
int main(int argc, char*argv)
{
	
	glfwInit();//initialize GLFW
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);//opengl version major
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);//opengl version minor
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);//don't let the user resize my window 

	GLFWwindow* window = glfwCreateWindow(800, 600, "EEC277 Project", nullptr, nullptr);//create window object
	if (window == nullptr){
		std::cout << "Failed to create GLFW window!!!" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);//make the conext of my window the main context on the current thread
	glfwSetKeyCallback(window, KeyCallback);

	glewExperimental = GL_TRUE;//enusre using more moder techniques (!!!) for managing opengl functionality
	if (glewInit() != GLEW_OK){
		std::cout << "Failed to initialize GLEW!!" << std::endl;
		return -1;
	}

	//std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;
	//the size of the rendering window
	int width, height;
	
	glfwGetFramebufferSize(window, &width, &height);
	glViewport(0, 0, width, height);//the lower left corner of the window, and its width and height 
	                                //of the rendering window in pixels as reterived from GLFW itself

	
	Shader ourShader("basicVShader2.glsl", "basicFShader2.glsl");//invoke this if you have the shader in text file 

	/*GLfloat vertices[] = { //for single triangle no color
		-0.5f, -0.5f, 0.0f, 
		0.5f, -0.5f, 0.0f, 
		0.0f, 0.5f, 0.0f
	};*/
	
	GLfloat vertices[] = { //for rectangle (should use indexed drawing to get it right)
		//first triangle 
		0.9f, 0.9f, 0.0f, //top right
		0.9f, -0.9f, 0.0f,//bottom right
		-0.9f, -0.9f, 0.0f,//top left
		-0.9f, 0.9f, 0.0f //top left
	};

	/*GLfloat vertices[] = { //for single triangle no color
		      //Positions        //Colors
	      -0.5f, -0.5f, 0.0f,   1.0f, 0.0f, 0.0f, //bottom right 
	       0.5f, -0.5f, 0.0f,   0.0f, 1.0f, 0.0f, //bottom left
		   0.0f,  0.5f, 0.0f,   0.0f, 0.0f, 1.0f, //top 
	};*/

	GLuint indices[] = { //indexing starts from zero
		0,1,3, //first triangle
		1,2,3 //second triangle 
	};


	//0) Creat objects for VBO, EBO, VAO
	GLuint VAO;//create vertex array object
	GLuint VBO;//store a large number of vertices in the gpu memory
	GLuint EBO;;//create Element Buffer Object

	glGenVertexArrays(1, &VAO);//create an object for VAO
	glGenBuffers(1, &VBO);//create an object for VBO
	glGenBuffers(1, &EBO);//create an object for EBO

	//1) Bind VAO object
	glBindVertexArray(VAO);//bind the VAO, then bind and set vertex buffer(s) and attribute pointer(s)		

	//2) Bind and copy vertices array in the vertex buffer 
	glBindBuffer(GL_ARRAY_BUFFER, VBO);//from that point on any buffer calls we make (on the GL_ARRAY_BUFFER target) will be used to 
	                                   //used to configure the currently bound buffer, which is VBO. 	
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices),vertices, GL_STATIC_DRAW); //copies user-defined data into the currently bound buffer
	                                                                          //GL_DYNAMIC_DRAW : the data is likely to change a lot.
	                                                                          //GL_STATIC_DRAW: the data will most likely not change at all or very rarely.
	                                                                          //GL_STREAM_DRAW : the data will change every time it is drawn.
	//3) Copy index array in a element buffer 
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);//bind EBO 
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);//copy index array in a element buffer

	//4) Set the vertex attributes pointers 
	//tell opengl how it should interpret the vertex data (set vertex attribute) 
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FLOAT, 3 * sizeof(GLfloat), (GLvoid*)0); //First parameter specifies which vertex attribute we want to configure
	                                                                                  //(0 since we specified the location of the position vertex att in vertex shader with layout (location = 0))
	                                                                                  //Second parameter specifies the size of the vertex attribute (vec3 is 3 values)
	                                                                                  //Thrid parameter specifies the type of the data (a vec* in GLSL consists of floating point values)
	                                                                                  //Forth parameter specifies if we want the data normalized (to 0 and 1 (or -1 for signed data))
	                                                                                  //Fifth parameter is the stride which tells the space between consecutive vertex attribute sets. 
	                                                                                  //since the next set of position data is located 3 times the size of GLfloat away 
	                                                                                  //Sixth parameter is of type GLvoid* (should be casted). It is the offset of where the position data begins in the buffer
	                                                                                  //since the position data is at the start if the data array, this value is just 0   	

	//Position Attribute
	//glVertexAttribPointer(0, 3, GL_FLOAT, GL_FLOAT, 6 * sizeof(GLfloat), (GLvoid*)0); //the difference here is the length of the stride, it is 6 now since we pass position(3 floats) and color (3 floats)
	glEnableVertexAttribArray(0);//enable attribute (position attribute)
	//Color Attribute
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FLOAT, 6 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat))); //color attribute is simialr to position attribute only differ in the attribute number which is set in vertex shader as layout (location=1)
	glEnableVertexAttribArray(1);//enable attribute (color attribute)
	


	//5) Unbind the VAO (DO not bind EBO, this causes error) 
	glBindBuffer(GL_ARRAY_BUFFER, 0);// Note that this is allowed, the call to glVertexAttribPointer registered VBO as the currently bound vertex buffer object so afterwards we can safely unbind	
	glBindVertexArray(0);//Unbibd VAO (good thing to do to unbind any buffer/array to preven strange bugs)

	//This draws wireframe 
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);//First parameter says we want to apply wireframe to the front and back of all triangles 
	                                          //Second parameter asks to draw stuff in lines 
	
	

	while (!glfwWindowShouldClose(window)){//checks if the window closes
		glfwPollEvents();//checks for events (mouse, keyboard)

		//rendering commands 
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		//clear the screen (other wise we would still see the results from previous iteration)
		//specify which buffer to clear (it could be GL_COLO_BUFFER_BIT, GL_DEPTH_BUFFER_BIT, or GL_STENCIL_BUFFER_BIT)
		glClear(GL_COLOR_BUFFER_BIT);
			
		ourShader.Use();//used when the shader invoked from text file 
				
		/*GLfloat timeValue = glfwGetTime();//get time of this frame
		GLfloat greenValue = (sin(timeValue) / 2.0) + 0.5;//change the color vale as a function of time 
		GLint vertexColorLocation = glGetUniformLocation(shaderProgram, "ourColor");//look for the uniform value we want to change (color)
		glUseProgram(shaderProgram);//specify the shader because the we set the uniform on the currently active shader program 
		glUniform4f(vertexColorLocation, 0.0f, greenValue, 0.0f, 1.0f);//because we want to change 4 floats of uniform (we could also have used fv -> float vector/array)*/


		//2) Draw object 
		glBindVertexArray(VAO);
		//glDrawArrays(GL_TRIANGLES, 0, 3);//first parameter is the type of the opengl primitives type to draw with
		                                 //second parameter is the starting index of the vertex array we want to draw 
		                                 //thrid parameter is the number of vertices we want to draw (1 triangle = 3 vertices)
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0); //First parameter is the mode we want to draw in 
		                                                     //Second parameter count of the number of elements want to draw (6 indices as we want to draw 6 vertices in total)
		                                                     //Third parameter type of the indices 
		                                                     //Forth parameter is offset in the EBO 
		glBindVertexArray(0);



		//swap the front buffer with the back buffer
		glfwSwapBuffers(window);
	}

	//de-allocate all resources 
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);

	
	//terminate GLFW
	glfwTerminate();
	return 0;
}
