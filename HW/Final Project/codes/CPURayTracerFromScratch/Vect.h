#ifndef _VECT_H
#define _VECT_H

#include "math.h"

class Vect
{
public:

	double x, y, z;

	Vect();
	Vect(double, double, double);
	

	~Vect();

	//method functions 
	
	double getVectX();
	double getVectY();
	double getVectZ();

	double magnitude();
	Vect normalize();
	Vect negative();
	double dotProduct(Vect);
	Vect crossProduct(Vect);
	Vect vectAdd(Vect);
	Vect vectMult(double);

private:

};

#endif 
