#ifndef _PLANE_H
#define _PLANE_H

#include "math.h"
#include "Object.h"
#include "Vect.h"
#include "Color.h"
#include "Ray.h"

class Plane : public Object
{

	Vect normal;	
	double distance;//distance from the center of plane to the center of the scene 
	Color color;

public:

	Plane();
	Plane(Vect, double, Color);


	~Plane();

	//method functions 
	Vect getPlaneNormal(){ return  normal; }
	double getPlaneDistance(){ return  distance; }
	virtual Color getColor(){ return  color; }
	virtual Vect getNormalAt(Vect);
	virtual double findIntersection(Ray ray); //the disatnce from ray origin to the point of intersection 

private:

};

#endif 
