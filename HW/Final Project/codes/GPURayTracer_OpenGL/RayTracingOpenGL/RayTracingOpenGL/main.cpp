#include <iostream>
#define GLEW_STATIC 
#include <GL\glew.h>
#include <GLFW\glfw3.h>
#include <vector>
#include <string.h>  
#include <string>  
#include <algorithm>
#include "3DBVH.h"
#include "shader.h"
#include "GLFWTimer.h"

Node* BVH;
int BVH_depth;
Node* leafNodes;
Node* nodes;

static GLuint CreateTexture(const int TextureId, GLenum format, const int width, const int height, float*data)
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
				 width, height, 0,/* width, height, border */
		         GL_LUMINANCE, GL_FLOAT,/* external format, type */
				 data);
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
void ReadObjSendTriangle(std::string fname, int&numVert, int&numTri, double**&triangles)
{
	//read obj file by doing two passes over it
	//one for counting the number of vertices and triangles
	//the other to store them 
	FILE* objfile = fopen(fname.c_str(), "r");
	
	while (objfile == NULL){
		std::cout << "Error at ReadObjSendTriangle(). File '" << fname << "' does not exist!!!" << std::endl;
		std::cout << "Please enter a valid file name: ";
		std::cin >> fname;
		objfile = fopen(fname.c_str(), "r");				
	}
	char line[81];
	numVert = 0;
	numTri = 0;
	while (fgets(line, 80, objfile) != NULL){
		if (line[0] == 'v' || line[0] == 'V'){ numVert++; }
		if (line[0] == 'f' || line[0] == 'F'){ numTri++; }		
	}
	fclose(objfile);

	double xmax(-DBL_MAX), xmin(DBL_MAX), ymax(-DBL_MAX), ymin(DBL_MAX), zmax(-DBL_MAX), zmin(DBL_MAX);

	//read the vertices 
	double**vert = new double*[numVert];	
	std::fstream objfile2;
	objfile2.open(fname); 
	char tuna='d';
	for (int i = 0; i < numVert; i++){
		vert[i] = new double[3];
		objfile2 >> tuna >> vert[i][0] >> vert[i][1] >> vert[i][2];

		xmax = std::max(xmax, vert[i][0]);
		xmin = std::min(xmin, vert[i][0]);

		ymax = std::max(xmax, vert[i][1]);
		ymin = std::min(xmin, vert[i][1]);

		zmax = std::max(xmax, vert[i][2]);
		zmin = std::min(xmin, vert[i][2]);
	}
	
	//scale so that the model is visiable in the scene 

	//shift the model to the corner (-0.7,-0.7,-1.5)
	double lx(abs(xmax - xmin)), ly(abs(ymax - ymin)), lz(abs(zmax - zmin));
	double scale_factor = std::max(lx, ly);	
	scale_factor = std::max(scale_factor, lz);
	
			
	for (int i = 0; i < numVert; i++){
		vert[i][0] = (vert[i][0] - xmin) / (0.5*scale_factor);
		vert[i][1] = (vert[i][1] - ymin) / (0.5*scale_factor);
		vert[i][2] = (vert[i][2] - zmin) / (0.5*scale_factor);
		vert[i][0] -= 0.7;
		vert[i][1] -= 1.7;
		vert[i][2] -= 3.0;
	}

	//read and store triangles 
	triangles = new double*[numTri];
	for (int i = 0; i < numTri; i++){
		triangles[i] = new double[6]; //(x1,y1,z1)(x2,y2,z2)(x3,y3,z3)
		int v1, v2, v3;
		objfile2 >> tuna >> v1 >> v2 >> v3;
		v1--; v2--; v3--;
		triangles[i][0] = vert[v1][0];
		triangles[i][1] = vert[v1][1];
		triangles[i][2] = vert[v1][2];
		triangles[i][3] = vert[v2][0];
		triangles[i][4] = vert[v2][1];
		triangles[i][5] = vert[v2][2];
		triangles[i][6] = vert[v3][0];
		triangles[i][7] = vert[v3][1];
		triangles[i][8] = vert[v3][2];
	}


	//clean up
	for (int i = 0; i < numVert; i++){
		delete[] vert[i];
	}
	delete[] vert;

}
void ReadSphere(std::string fname, int&numSphere, double**&spheres)
{
	
	std::fstream datfile;
	datfile.open(fname);
	datfile >> numSphere;
	spheres = new double*[numSphere];

	double xmax(-DBL_MAX), xmin(DBL_MAX), ymax(-DBL_MAX), ymin(DBL_MAX), zmax(-DBL_MAX), zmin(DBL_MAX);

	for (int i = 0; i < numSphere; i++){
		spheres[i] = new double[4];
		datfile >> spheres[i][0] >> spheres[i][1] >> spheres[i][2] >> spheres[i][3];
		
		xmax = std::max(xmax, spheres[i][0]);
		xmin = std::min(xmin, spheres[i][0]);

		ymax = std::max(ymax, spheres[i][1]);
		ymin = std::min(ymin, spheres[i][1]);

		zmax = std::max(zmax, spheres[i][2]);
		zmin = std::min(zmin, spheres[i][2]);
	}

	double lx(abs(xmax - xmin)), ly(abs(ymax - ymin)), lz(abs(zmax - zmin));
	double scale_factor = std::max(lx, ly);
	scale_factor = std::max(scale_factor, lz);
	scale_factor *= 1.2;
	for (int i = 0; i < numSphere; i++){
		//scaling for the torus
		spheres[i][0] = (spheres[i][0] - xmin) / scale_factor;
		spheres[i][1] = (spheres[i][1] - ymin) / scale_factor;
		spheres[i][2] = (spheres[i][2] - zmin) / scale_factor;
		spheres[i][3] /= scale_factor; 
		spheres[i][0] -= 0.5;
		spheres[i][1] -= 0.5;
		spheres[i][2] -= 2.0;
	}
	numSphere = 2;
	BVH = BVHstart(numSphere, spheres, BVH_depth, leafNodes, nodes); 
	BVH_depth--;

}

void MoveTrianglesToShader(int numTri, double**triangles, GLuint progNum)
{


	GLint locNum = glGetUniformLocation(progNum, "numTriangle");
	if (locNum == -1){
		std::cout << "Error (0) MoveTrianglesToShader(). Can not find the location!!!" << std::endl;
		system("pause");
	}
	glUniform1i(locNum, numTri);

	for (int i = 0; i < numTri; i++){ 

		std::string  A_Str = "scene_tri[" + std::to_string(i) + "].A";
		std::string  B_Str = "scene_tri[" + std::to_string(i) + "].B";
		std::string  C_Str = "scene_tri[" + std::to_string(i) + "].C";
		
		GLint locA = glGetUniformLocation(progNum, A_Str.c_str());
		GLint locB = glGetUniformLocation(progNum, B_Str.c_str());
		GLint locC = glGetUniformLocation(progNum, C_Str.c_str());
		if (locA == -1 || locB == -1 || locC == -1){
			std::cout << "Error (1) MoveTrianglesToShader(). Can not find the location!!!" << std::endl;
			system("pause");
		}
		glUniform3f(locA, triangles[i][0], triangles[i][1], triangles[i][2]);
		glUniform3f(locB, triangles[i][3], triangles[i][4], triangles[i][5]);
		glUniform3f(locC, triangles[i][6], triangles[i][7], triangles[i][8]);		
	}
}
void MoveSphereToShader(int numSphere, double**spheres, GLuint progNum)
{	

	GLint locNum = glGetUniformLocation(progNum, "numSphere");
	if (locNum == -1){
		std::cout << "Error (0) MoveSphereToShader(). Can not find the location!!!" << std::endl;
		system("pause");
	}
	else{
		glUniform1i(locNum, numSphere);
	}


	//send the tree data
	/*for (int i = 0; i < numSphere; i++){
		std::string  myStr_nodearr = "nodeArr[" + std::to_string(i) + "]";
		GLint locNodearr = glGetUniformLocation(progNum, myStr_nodearr.c_str());
		if (locNodearr == -1){
			std::cout << "Error (2) MoveSphereToShader(). Can not find the location!!!" << std::endl;
			system("pause");
		}
		else{
			Node myNode = nodes[i];
			if (myNode.childAIdx > 0){
				//it's a space 
			}
			else{

			}
			//glUniform1i(locNodearr, );
		}

	}*/


	/*Node*current = BVH;
	int currentLevel = 1;
	int arrId = 0;	
	Node **everyNode = new Node*[arrLen];
	everyNode[0] = new Node;
	everyNode[0] = BVH;
	everyNode[1] = BVH->childA;
	everyNode[2] = BVH->childB;
	int start = 1;
	int stop;
	while (currentLevel < BVH_depth){
		stop = pow(2, currentLevel) + start;
		for (int i = start; i < stop; i++){
			everyNode[i] = new Node;
			if (!everyNode[i]->isLeaf){
				Node*left = everyNode[i]->childA;
				Node*right = everyNode[i]->childB;				
				everyNode[i * 2 + 1] = left;
				everyNode[i * 2 + 2] = right;
			}
			else{
				Node tmp;
				tmp.isLeaf = true;
				tmp.objectID = everyNode[i]->objectID;
				everyNode[i] = &(tmp);
			}
			start++;
		}
		currentLevel++;
	}

	for (int i = 0; i < arrLen; i++){

		if (everyNode[i]->objectID != -1){
			std::string  myStr_bbox_min = "BVH[" + std::to_string(i) + "].bbox_min";
			GLint locBbox_min = glGetUniformLocation(progNum, myStr_bbox_min.c_str());
			if (locBbox_min == -1){
				std::cout << "Error (2) MoveSphereToShader(). Can not find the location!!!" << std::endl;
				system("pause");
			}
			else{
				glUniform3f(locBbox_min, everyNode[i]->bbox_min.x, everyNode[i]->bbox_min.y, everyNode[i]->bbox_min.z);
			}
			
			//bbox_max
			std::string  myStr_bbox_max = "BVH[" + std::to_string(i) + "].bbox_max";
			GLint locBbox_max = glGetUniformLocation(progNum, myStr_bbox_max.c_str());
			if (locBbox_max == -1){
				std::cout << "Error (3) MoveSphereToShader(). Can not find the location!!!" << std::endl;
				system("pause");
			}
			else{
				glUniform3f(locBbox_max, everyNode[i]->bbox_max.x, everyNode[i]->bbox_max.y, everyNode[i]->bbox_max.z);
			}
		}

		//objectID
		std::string  myStr_objectID = "BVH[" + std::to_string(i) + "].objectID";
		GLint locObjectID = glGetUniformLocation(progNum, myStr_objectID.c_str());
		if (locObjectID == -1){
			std::cout << "Error (4) MoveSphereToShader(). Can not find the location!!!" << std::endl;
			system("pause");
		}
		else{
			glUniform1i(locObjectID, everyNode[i]->objectID);
		}
	}*/
	

	/*for (int i = 0; i < numSphere; i++){
		//bbox_min
		
		std::string  myStr_bbox_min = "BVH[" + std::to_string(i) + "].bbox_min";
		GLint locBbox_min = glGetUniformLocation(progNum, myStr_bbox_min.c_str());
		if (locBbox_min == -1){
			std::cout << "Error (1) MoveSphereToShader(). Can not find the location!!!" << std::endl;
			system("pause");
		}
		else{
			glUniform3f(locBbox_min, BVH->bbox_min.x, BVH->bbox_min.y, BVH->bbox_min.z);
		}

		//bbox_max
		std::string  myStr_bbox_max = "BVH[" + std::to_string(i) + "].bbox_max";
		GLint locBbox_max = glGetUniformLocation(progNum, myStr_bbox_max.c_str());
		if (locBbox_max == -1){
			std::cout << "Error (2) MoveSphereToShader(). Can not find the location!!!" << std::endl;
			system("pause");
		}
		else{
			glUniform3f(locBbox_max, BVH->bbox_max.x, BVH->bbox_max.y, BVH->bbox_max.z);
		}
		
		//childA
		std::string  myStr_childA = "BVH[" + std::to_string(i) + "].childA";
		GLint locChildA = glGetUniformLocation(progNum, myStr_childA.c_str());
		if (locChildA == -1){
			std::cout << "Error (3) MoveSphereToShader(). Can not find the location!!!" << std::endl;
			system("pause");
		}
		else{
			//glUniform1i(locChildA, BVH->childA);
		}

		//childB
		std::string  myStr_childB = "BVH[" + std::to_string(i) + "].childB";
		GLint locChildB = glGetUniformLocation(progNum, myStr_childB.c_str());
		if (locChildB == -1){
			std::cout << "Error (4) MoveSphereToShader(). Can not find the location!!!" << std::endl;
			system("pause");
		}
		else{
			//glUniform1i(locChildB, BVH->childB);
		}

		//objectID
		std::string  myStr_objectID = "BVH[" + std::to_string(i) + "].objectID";
		GLint locObjectID = glGetUniformLocation(progNum, myStr_objectID.c_str());
		if (locObjectID == -1){
			std::cout << "Error (5) MoveSphereToShader(). Can not find the location!!!" << std::endl;
			system("pause");
		}
		else{
			glUniform1i(locObjectID, BVH->objectID);
		}
	}*/
	/*GLint locBoxMin = glGetUniformLocation(progNum, "BVH[0].bbox_min");
	if (locBoxMin == -1){
		std::cout << "Error (1) MoveSphereToShader(). Can not find the location!!!" << std::endl;
		system("pause");
	}
	else{
		glUniform3f(locBoxMin, BVH->bbox_min.x, BVH->bbox_min.y, BVH->bbox_min.z);
	}
	
	GLint locBoxMax = glGetUniformLocation(progNum, "BVH[0].bbox_max");
	if (locBoxMax == -1){
		std::cout << "Error (2) MoveSphereToShader(). Can not find the location!!!" << std::endl;
		system("pause");
	}
	else{
		glUniform3f(locBoxMax, BVH->bbox_max.x, BVH->bbox_max.y, BVH->bbox_max.z);
	}*/
	

	//send spheres data 
	for (int i = 0; i < numSphere; i++){
		std::string  myStr = "scene_sphere[" + std::to_string(i) + "]";	
		GLint locA = glGetUniformLocation(progNum, myStr.c_str());		
		if (locA == -1 ){
			std::cout << "Error (5) MoveSphereToShader(). Can not find the location!!!" << std::endl;
			system("pause");
		}
		else{
			glUniform4f(locA, spheres[i][0], spheres[i][1], spheres[i][2], spheres[i][3]);
		}		
	}
}
void MoveSphereToSSBO(int numSphere, double**Spheres, GLuint progNum)
{
	/*GLint lociRes = glGetUniformLocation(progNum, "iResolution");
	if (lociRes == -1){
		std::cout << "Error (0) MoveSphereToShader(). Can not find the location!!!" << std::endl;
		system("pause");
	}
	else{
		glUniform3f(lociRes, 800,600,0);
	}*/

	GLint locNum = glGetUniformLocation(progNum, "numSphere");
	if (locNum == -1){
		std::cout << "Error (1) MoveSphereToShader(). Can not find the location!!!" << std::endl;
		system("pause");
	}
	else{
		glUniform1i(locNum, numSphere);
	}

	
	//***************************************************pass the children id's as i array 
	int* node_arr = new int[2 * numSphere];
	int id = 0;
	for (size_t i = 0; i < numSphere; i++){
		node_arr[id] = nodes[i].childAIdx;
		id++;
		node_arr[id] = nodes[i].childBIdx;
		id++;
	}
	GLuint ssbo1;
	glGenBuffers(1, &ssbo1);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo1);
	glBufferData(GL_SHADER_STORAGE_BUFFER, numSphere * 2 * sizeof(int), node_arr, GL_DYNAMIC_COPY);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, ssbo1);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	

	//***************************************************pass the sphere x,y,z,r
	float* sph_arr = new float[4 * numSphere];
	id = 0;
	for (size_t i = 0; i < numSphere; i++){
		int objID = leafNodes[i].objectID;
		sph_arr[id] = float(Spheres[objID][0]);
		id++;
		sph_arr[id] = float(Spheres[objID][1]);
		id++;
		sph_arr[id] = float(Spheres[objID][2]);
		id++;
		sph_arr[id] = float(Spheres[objID][3]);
		id++;
	}

	GLuint ssbo2;
	glGenBuffers(1, &ssbo2);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo2);
	glBufferData(GL_SHADER_STORAGE_BUFFER, numSphere * 4 * sizeof(float), sph_arr, GL_DYNAMIC_COPY);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, ssbo2);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	//delete[]sph_arr;

	
	//***********************************pass the leaf id's (these are indcies for the sphere data)
	id = 0;
	for (size_t i = 0; i < numSphere; i++){
		node_arr[id] = leafNodes[i].objectID;
		id++;		
	}
	GLuint ssbo3;
	glGenBuffers(1, &ssbo3);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo3);
	glBufferData(GL_SHADER_STORAGE_BUFFER, numSphere * sizeof(int), node_arr, GL_DYNAMIC_COPY);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, ssbo3);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	delete[]node_arr;		


	//************************************************ pass the min bounding box
	//float* BBox = new float[3 * numSphere]; 
	id = 0;
	for (size_t i = 0; i < numSphere; i++){
		if (nodes[i].isLeaf){ //if leaf, then there is no node here and it is an object and we don't need a box for this one
			sph_arr[id] = 0.0f;
			id++;
			sph_arr[id] = 0.0f;
			id++;
			sph_arr[id] = 0.0f;
			id++;
			sph_arr[id] = 0.0f;
			id++;
		}
		else{
			sph_arr[id] = float(nodes[i].bbox_min.x);
			id++;
			sph_arr[id] = float(nodes[i].bbox_min.y);
			id++;
			sph_arr[id] = float(nodes[i].bbox_min.z);
			id++;
			sph_arr[id] =0.0f;
			id++;
		}
	}

	GLuint ssbo4;
	glGenBuffers(1, &ssbo4);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo4);
	glBufferData(GL_SHADER_STORAGE_BUFFER, 4 * numSphere* sizeof(float), sph_arr, GL_DYNAMIC_COPY);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, ssbo4);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	
	//************************************************* pass the max bounding box
	id = 0;
	for (size_t i = 0; i < numSphere; i++){
		if (nodes[i].isLeaf){
			sph_arr[id] = 0.0f;
			id++;
			sph_arr[id] = 0.0f;
			id++;
			sph_arr[id] = 0.0f;
			id++;
			sph_arr[id] = 0.0f;
			id++;
		}
		else{
			sph_arr[id] = float(nodes[i].bbox_max.x);
			id++;
			sph_arr[id] = float(nodes[i].bbox_max.y);
			id++;
			sph_arr[id] = float(nodes[i].bbox_max.z);
			id++;
			sph_arr[id] = 0.0f;
			id++;
		}
	}

	GLuint ssbo5;
	glGenBuffers(1, &ssbo5);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo5);
	glBufferData(GL_SHADER_STORAGE_BUFFER, numSphere * 4 * sizeof(float), sph_arr, GL_DYNAMIC_COPY);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, ssbo5);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	

}
int main(int argc, char*argv)
{	
	int numVert,numTri(0);
	double**triangles = NULL;

	int numSphere;
	double**spheres = NULL;
	//ReadObjSendTriangle("cube.obj", numVert, numTri,triangles);
	ReadSphere("sphere_torus.dat", numSphere, spheres);
	//===============================================  Initialize the window and read, compile, and link the shaders ===============================================//
	GLFWwindow* myWin= CreateMainWindow(800, 600);
	Shader ourShader("basicVShader2.glsl", "basicFShader2.glsl");	
	ourShader.Use();//use/call the shader now to start populating the uniform variables 

	//=============================================== Move Data To shaders uniforms ===============================================//				

	//MoveTrianglesToShader(numTri, triangles, ourShader.Program); 
	MoveSphereToSSBO(numSphere, spheres, ourShader.Program);
	//MoveSphereToShader(numSphere, spheres, ourShader.Program);

	//=============================================== Initialize the main scene: two big triangles ===============================================//		
	GLuint VBO, VAO;
	CreateMainScene(VAO, VBO);
	
	//timing stuff
	GLFWTimer* timer = new GLFWTimer();
	int frameCounter = 0, fps = 0;

	
	//=============================================== Main Loop ===============================================//
	while (!glfwWindowShouldClose(myWin)){//checks if the window closes
		timer->getTimeDiff();

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

		frameCounter++;//count number of frames and report them at 1 second interval 
		if (timer->getRefreshedTime() > 1.0){//if difference in between two time measures is greater than 1.0
			fps = frameCounter;
			std::cout << "\nfps=" << fps << std::endl;
			frameCounter = 0;
			timer->resetTime();
		}                                   

		glfwSwapBuffers(myWin);
	}

	//de-allocate all resources 
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);

	
	//terminate GLFW
	glfwTerminate();
	return 0;

	
}

