//http://www.iquilezles.org/www/articles/simplegpurt/simplegpurt.htm
//The vertex shader is used to partially calculate the ray direction (raydir) for the corners of the full screen quad 
//it then interpolated down the fragment shader 
//fpar00[ ] contains all the input to the shader 
//fpar00[0] contains the information of the first sphere (a position and a radius)
//fpar00[1] contains the information of the second sphere
//fpar00[2] & fpar00[3] are the two cylinders
//fpar00[4] & fpar00[5] are the two planes
//fpar00[6] to fpar00[11] are the color of the objects 
//fpar00[12] contains the camera position
//fpar00[13] to fpar00[15] contain the camera matrix (3x4 matrix)

#version 330 core
layout (location=0) in vec3 position;//take in the position 
varying vec3 raydir;//and produce a ray from it 
//uniform vec4 fpar00[16];
uniform vec4 cam_max_1 = vec4(1,0,0,0);//1st raw in the camera matrix
uniform vec4 cam_max_2 = vec4(0,1,0,0);//2nd raw in the camera matrix
uniform vec4 cam_max_3 = vec4(0,0,1,0);//3rd raw in the camera matrix

void main(void)
{	
	
	gl_Position = vec4(position,1.0);
	vec3 r;

	r = position.xyz*vec3(1.333, 1.0, 0.0) + vec3(0.0, 0.0, -1.0);

	raydir.x = dot(r,cam_max_1.xyz);
	raydir.y = dot(r,cam_max_2.xyz);
	raydir.z = dot(r,cam_max_3.xyz);


};