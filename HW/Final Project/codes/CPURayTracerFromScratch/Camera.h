#ifndef _CAMERA_H
#define _CAMERA_H

#include "Vect.h"

class Camera
{
public:

	Vect camPos, camDir, camRight, camDown;
	
	Camera();
	Camera(Vect, Vect, Vect, Vect);


	~Camera();

	//method functions 

	Vect getCameraPosition(){ return  camPos; }
	Vect getCameraDirection(){ return  camDir; }
	Vect getCameraRight(){ return  camRight; }
	Vect getCameraDownn(){ return  camDown; }


private:

};

#endif 
