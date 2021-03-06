#define _CRT_SECURE_NO_DEPRECATE
#include <stdio.h>
#include <fstream>
#include <iostream>
#include <cstdlib>
#include <cmath>
#include <string>
#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

/////////////////////////////////////////////////////////////////
// the shader programs
/////////////////////////////////////////////////////////////////

/*
//Fragment shader coded in shader assembly language
//We used Assembly to guarantee direct access to the FP ALU
//References: http://www.renderguild.com/gpuguide.pdf
//https://www.khronos.org/registry/OpenGL/extensions/ARB/ARB_fragment_program.txt
*/

const static std::string FRAGMENT_SHADER =
"!!ARBfp1.0\n"						// declear the an ARB fragment Program  
"TEMP R0; \n"						// create an temp register
"PARAM c0 = program.local[0]; \n"	// getting the local parameter 0 that has been passed to the program (input)
"--- R0.x, c0.x; \n"				// take the sin or any other native operation and put it in the temp variable
"MOV result.color, R0; \n"			// output our temp variable as the color attribute of the fragment program result
"END\n";

const static std::string FRAGMENT_SHADER_SQRT =
"!!ARBfp1.0\n"						// declear the an ARB fragment Program  
"TEMP R0; \n"						// create an temp register
"TEMP R1; \n"						// create an temp register
"PARAM c0 = program.local[0]; \n"	// getting the local parameter 0 that has been passed to the program (input)
"RSQ R0.x, c0.x; \n"				// 
"RCP R1.x, R0.x; \n"				// 
"MOV result.color, R1; \n"			// output our temp variable as the color attribute of the fragment program result
"END\n";

/*
//Vertex shadder that does nothing but passing the vertices
*/
const GLchar* vertexShaderSource = "#version 330 core\n"
"layout (location = 0) in vec3 position;\n"
"layout (location = 1) in vec3 color;\n"
"out vec3 ourColor;\n"
"void main()\n"
"{\n"
"gl_Position = vec4(position, 1.0);\n"
"ourColor = color;\n"
"}\0";

/////////////////////////////////////////////////////////////////
// functions prototypes
/////////////////////////////////////////////////////////////////

/*This function returns the relative error between the CPU and the GPU for a cerain operation*/
float get_gpu_result(GLFWwindow* window, std::string op, float input_number);
/*This function edit the fragment shader string to be used with any arithmetic operation*/
std::string get_frg_shader(std::string op);
/*This function calculates the given operation on the CPU*/
double CalculateCPUValue(std::string op, float x);

double run_test(GLFWwindow* window, std::string op, float start_test, float end_test, float step, bool print_out);
GLFWwindow* initialize_glew();
std::string get_file_name(std::string op);

//Struct to save the frame buffers
typedef struct {
	GLuint frameBuffer;
	GLuint depthRenderBuffer;
	GLuint texture;
} FrameBuffer;
