//https://proceduralthoughts.wordpress.com/2014/08/11/glsl-syntax-highlighting-in-visual-studio-express-2013/
//For formatting shader syntax highlighting on VS2013
//This class read shader from a file, compile it, link it to program and let you use it 
//copied from learnopengl.org 


#ifndef _SHADER_H
#define _SHADER_H

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

#include <GL/glew.h>//to get the required opengl headers

class Shader
{
public:
	//The shader program ID
	GLuint Program;

	//Constructor to read abd build the shader (from a text file on the disk)
	Shader(const GLchar* vertexPath, const GLchar* fragmentPath); 

	//Use the program 
	void Use();
private:

};

void Shader::Use(){
	glUseProgram(this->Program);
}

Shader::Shader(const GLchar* vertexPath, const GLchar* fragmentPath)
{
	//1) Retreive the vertex/fragment source code from filepath
	std::string vertexCode;
	std::string fragmentCode;
	std::ifstream vShaderFile;
	std::ifstream fShaderFile;

	//ensures ifstream objects can throw exceptions
	vShaderFile.exceptions(std::ifstream::badbit);
	fShaderFile.exceptions(std::ifstream::badbit);

	try{
		//open files
		vShaderFile.open(vertexPath);
		fShaderFile.open(fragmentPath);
		std::stringstream vShaderStream, fShaderStream;

		//read file buffer content into streams 
		vShaderStream << vShaderFile.rdbuf();
		fShaderStream << fShaderFile.rdbuf();

		//close file handlers 
		vShaderFile.close();
		fShaderFile.close();

		//convert stream into GLchar array
		vertexCode = vShaderStream.str();
		fragmentCode = fShaderStream.str();
	}
	catch (std::ifstream::failure){
		std::cout << "Error::Shader::File not succesfully read!!!" << std::endl;
	}
	const GLchar *vShaderCode = vertexCode.c_str();
	const GLchar *fShaderCode = fragmentCode.c_str();

	//2) Compile shaders 
	GLuint vertex;
	GLint success;
	GLchar infoLog[512];

	//Vertex shader 
	vertex = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex, 1, &vShaderCode, NULL);
	glCompileShader(vertex);

	//print compile errors 
	glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
	if (!success){
		glGetShaderInfoLog(vertex, 512, NULL, infoLog);
		std::cout << "Error::Shader::Vertex::Compilation Failed\n" << infoLog << std::endl;
	}

	//Fragment shader 
	GLuint fragment = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment, 1, &fShaderCode, NULL);
	glCompileShader(fragment);

	//print compile errors 
	glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
	if (!success){
		glGetShaderInfoLog(fragment, 512, NULL, infoLog);
		std::cout << "Error::Shader::Fragment::Compilation Failed\n" << infoLog << std::endl;
	}

	//3)Linking shaders 
	//Shader Program 
	this->Program = glCreateProgram();
	glAttachShader(this->Program, vertex);
	glAttachShader(this->Program, fragment);
	glLinkProgram(this->Program);

	//print linking errors if any
	glGetProgramiv(this->Program, GL_LINK_STATUS, &success);
	if (!success){
		glGetProgramInfoLog(this->Program, 512, NULL, infoLog);
		std::cout << "Error::Shader::Program::Linking Failed\n" << infoLog << std::endl;
	}

	//4) delete the shader as they are linked into the program now 
	glDeleteShader(vertex);
	glDeleteShader(fragment);
}


#endif /*_SHADER_H*/