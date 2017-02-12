#ifndef _RAY_H
#define _RAY_H

#include "Vect.h"

class Ray
{
public:

	Vect origin, direction;
	

	Ray();
	Ray(Vect, Vect);
	

	~Ray();

	//method functions 
	
	Vect getRayOrigin(){ return  origin; }
	Vect getRayDirection(){ return  direction; }
	

private:

};

#endif 
