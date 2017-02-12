#ifndef _OBJECT_H
#define _OBJECT_H

#include "Ray.h"
#include "Vect.h"
#include "Color.h"

class Object
{

public:	
	//This is the top of the heirarchy where all classes are sub-classes of it 
	Object();
	~Object();


	//method functions 	
	virtual Color getColor(){ return  Color(0.0, 0.0, 0.0, 0.0); }
	virtual double findIntersection(Ray ray);//find the intersection between a ray and the object 
	virtual Vect getNormalAt(Vect intersection_position){ return Vect(0, 0, 0); }
private:

};

#endif 
