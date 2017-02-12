#include "Ray.h"

Ray::Ray()
{
	origin = Vect(0, 0, 0);
	direction = Vect(1,0,0);	
}
Ray::Ray(Vect o, Vect d)
{
	origin =  o;
	direction = d;
}

Ray::~Ray()
{
}
