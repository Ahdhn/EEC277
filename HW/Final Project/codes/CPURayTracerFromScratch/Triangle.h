#ifndef _TRIANGLE_H
#define _TRIANGLE_H

#include "math.h"
#include "Vect.h"
#include "Color.h"
#include "Ray.h"
#include "Object.h"

class Triangle : public Object
{
	Vect A, B, C;

	Vect normal;
	double distance;//distance from the center of Triangle to the center of the scene 
	Color color;

public:

	Triangle();
	Triangle(Vect, Vect, Vect, Color);


	~Triangle();

	//method functions 
	Vect getTriangleNormal();
	double getTriangleDistance();
	virtual Color getColor(){ return  color; }
	virtual Vect getNormalAt(Vect);
	virtual double findIntersection(Ray ray); //the disatnce from ray origin to the point of intersection 

private:

};

#endif 
