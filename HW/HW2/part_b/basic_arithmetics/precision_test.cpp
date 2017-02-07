#include "precision_test.h" 



GLFWwindow* initialize_glew() {

	// openGl window initialization
	glfwInit();
	// Set all the required options for GLFW
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Create a GLFWwindow object that we can use for GLFW's functions
	GLFWwindow* window = glfwCreateWindow(800, 600, "Precision Test", nullptr, nullptr);
	glfwMakeContextCurrent(window);

	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK)
	{
		std::cout << "Failed to initialize GLEW" << std::endl;
		exit(1);
	}
	///// end of the initi
	return window;
}


int main()
{
	std::string ops[] = { "SIN", "COS" , "LG2", "EX2" };
 	bool print_output = false;
	double result = 0;
	std::ofstream file_t("final_average_relative_error.txt",std::ios::out);
	file_t.precision(30);
	file_t << "\nThis file have the values of the average realtive error" << std::endl;

	
		//std::string ops[] = { "SIN", "COS" , "LG2", "EX2" };
		//void run_test(std::string op, float start_test, float end_test, float step);

		GLFWwindow* window = initialize_glew();
		result = run_test(window, ops[0], 0.01, 7.2831, 0.001, print_output);
		file_t << "Sin, " << result<<std::endl;
 
		result=run_test(window, ops[1], 0.01, 7.2831, 0.001, print_output);
		file_t << "Cos, " << result << std::endl;

		result = run_test(window, ops[2], 0.1, 100, 0.05, print_output);
		file_t << "Log2, " << result << std::endl;

		result = run_test(window, ops[3], 0.1, 100, 0.05, print_output);
		file_t << "Power2, " << result << std::endl;

		file_t.close();
		return 0;
	}//main


double run_test(GLFWwindow* window, std::string op, float test_start, float test_end, float step, bool print_output) {

	std::ofstream file_t(op, std::ios::out);
	file_t.precision(30);
	file_t << "Relative error for "<<op << ", test started with input "<< test_start<<" ended at "<< test_end <<" with a step of "<<step <<std::endl;

		double sum_gpu = 0;
		double sum_cpu = 0;
		double max_error = 0;

		for (float i = test_start; i < test_end; i += step)
		{
 
			float gpu_result = get_gpu_result(window, op, i);
			double cpu_result = CalculateCPUValue(op, i);
			sum_gpu +=  fabs(gpu_result);
			sum_cpu +=  fabs(cpu_result);
			//calulating the relative error 
			double absolute_error = fabs( fabs(cpu_result) - fabs(gpu_result));
			double relative_error = (absolute_error == 0.0) ? 0.0 : fabs(absolute_error / cpu_result);
			relative_error = (relative_error > 1.0) ? 1.0 : relative_error;
			file_t << relative_error << std::endl;
			if (relative_error > max_error)
				max_error = relative_error;		

			if (print_output)
				std::cout << "CPU: " << cpu_result << " GPU: " << gpu_result << " relative errror: "<< relative_error << std::endl;
		}//for each iteration 
		double temp = fabs(sum_cpu - sum_gpu);
		double average_error = temp / sum_cpu;
		std::cout << "The average relative error for " << op << " is " << average_error << std::endl;
		file_t << "The average relative error for " << op << " is " << average_error << std::endl;
		file_t << "The max realtive error for " << op << " is " << max_error << std::endl ;
		file_t.close();

		return average_error;

}

/*This function calculates the given operation on the CPU*/
double CalculateCPUValue(std::string op, float x) {

	if(!op.compare("SIN"))
		return sin(x);
	else if (!op.compare("COS"))
		return cos(x);
	else if (!op.compare("RSQ"))
		return 1/ sqrt(x);
	else if (!op.compare("EX2"))
		return pow(2,x);
	else if (!op.compare("LG2"))
		return log(x) / log(2.0);
	else
		return  (double) x;

}//CalculateCPUValue

/*This function edit the fragment shader string to be used with any arithmetic operation*/
std::string get_frg_shader(std::string op)
{
	std::string shader(FRAGMENT_SHADER);
	std::string search("---");
	std::string replace = op;
	int pos = 0;

	while ((pos = shader.find(search, pos)) != std::string::npos)
	{
		shader.replace(pos, search.length(), replace);
		pos += replace.length();
	}//while

	return shader;

}//get_frg_shader()


 /*This function returns the relative error between the CPU and the GPU for a cerain operation*/
float get_gpu_result(GLFWwindow* window, std::string op, float input_number )
{
	std::string shader = get_frg_shader(op);
	float gpuResult = 0;
	double cpuResult = 0;


	///////////////////////////////////////////////////////////////
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);

	glViewport(0, 0, width, height);
	//Set up
	
	// Build and compile our shader program
	// Vertex shader
	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
	glCompileShader(vertexShader);
	// Check for compile time errors
	GLint success;
	GLchar infoLog[512];
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
	}


	// Link shaders
	GLuint shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glLinkProgram(shaderProgram);
	// Check for linking errors
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
	}
	glDeleteShader(vertexShader);
	// Set up vertex data (and buffer(s)) and attribute pointers
	GLfloat vertices[] = {
		// Positions        
		0.5f, -0.5f, 0.0f,  // Bottom Right
		-0.5f, -0.5f, 0.0f,  // Bottom Left
		0.0f,  0.5f, 0.0f   // Top 
	};
	GLuint VBO, VAO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	// Bind the Vertex Array Object first, then bind and set vertex buffer(s) and attribute pointer(s).
	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	// Position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);

	glBindVertexArray(0); // Unbind VAO

	glDrawBuffer(GL_FRONT);
	glReadBuffer(GL_FRONT);
	glDisable(GL_DEPTH_TEST);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//create the frag shader and framebuffer
	FrameBuffer mFrameBuffer;

	//LoadShader and generate the ID of the program
	GLuint PID;
	glGenProgramsARB(1, &PID);
	//Bind()
	glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, PID);
	//loading the shader 
	glProgramStringARB(GL_FRAGMENT_PROGRAM_ARB, GL_PROGRAM_FORMAT_ASCII_ARB, shader.length(), shader.c_str());

	//making sure no errors
	int mErrorCode = 0;
	std::string	mErrorString = "";
	if (glGetError() == GL_INVALID_OPERATION) {
		glGetIntegerv(GL_PROGRAM_ERROR_POSITION_ARB, &mErrorCode);
		mErrorString = (const char *)glGetString(GL_PROGRAM_ERROR_STRING_ARB);
		std::cout << "ERROR::SHADER load FAILED\n" << mErrorString << std::endl;
	}

	//Enable();
	glEnable(GL_FRAGMENT_PROGRAM_ARB);
	glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, PID);
	glUseProgram(shaderProgram);
	//glUseProgram(PID);
	// passing the input to the shader's uniform (local parameter)
	glProgramLocalParameter4fARB(GL_FRAGMENT_PROGRAM_ARB, 0, input_number, 0, 0, 0);



	GLint maxRenderBufferSize;

	glGetIntegerv(GL_MAX_RENDERBUFFER_SIZE, &maxRenderBufferSize);
	if (maxRenderBufferSize <= width || maxRenderBufferSize <= height) {
		std::cout << "CAN'T INIT RENDER TARGET" << std::endl;
	}

	glGenFramebuffers(1, &(mFrameBuffer.frameBuffer));
	glGenRenderbuffers(1, &(mFrameBuffer.depthRenderBuffer));
	glGenTextures(1, &(mFrameBuffer.texture));
	glBindTexture(GL_TEXTURE_2D, mFrameBuffer.texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, width, height, 0, GL_RED, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	// bind renderbuffer and create a 16-bit depth buffer
	// width and height of renderbuffer = width and height of
	// the texture
	glBindRenderbuffer(GL_RENDERBUFFER, mFrameBuffer.depthRenderBuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, width, height);

	// bind the framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, mFrameBuffer.frameBuffer);
	// specify texture as color attachment
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
		GL_TEXTURE_2D, mFrameBuffer.texture, 0);

	// specify depth_renderbufer as depth attachment
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, mFrameBuffer.depthRenderBuffer);

	//check
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "CAN'T INIT RENDER TARGET" << std::endl;

	// passing the input to the shader's uniform (local parameter)
	glProgramLocalParameter4fARB(GL_FRAGMENT_PROGRAM_ARB , 0, input_number , 0, 0, 0);
  
	// Draw the triangle
	glBindVertexArray(VAO);
	glDrawArrays(GL_TRIANGLES, 0, 3);
	glBindVertexArray(0);
	//read the a GL_RED (color.X) of one pixel on the triangle
	glReadPixels(300, 300, 1, 1, GL_RED, GL_FLOAT, &gpuResult);

	

	//Cleaning up
	glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, 0);
	glDisable(GL_FRAGMENT_PROGRAM_ARB);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glDeleteRenderbuffers(1, &(mFrameBuffer.depthRenderBuffer));
	glDeleteFramebuffers(1, &(mFrameBuffer.frameBuffer));
	glDeleteTextures(1, &(mFrameBuffer.texture));
	glDeleteProgramsARB(1, &PID);



	return gpuResult;

}//get_error()





