#version 450 
layout (location=0) in vec3 position;//take in the position 
void main(void)
{	
	//doing nothing here 
	gl_Position = vec4(position,1.0);
};