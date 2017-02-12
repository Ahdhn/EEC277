#include "Vect.h"

Vect::Vect()
{
	x = 0;
	y = 0;
	z = 0;
}
Vect::Vect(double i, double j, double k)
{
	x = i;
	y = j;
	z = k;
}

Vect::~Vect()
{
}
double Vect::getVectX()
{
	return x;
}
double Vect::getVectY()
{
	return y;
}
double Vect::getVectZ()
{
	return z;
}

double Vect::magnitude()
{
	return sqrt(x*x + y*y + z*z);
}
Vect Vect::normalize()
{
	double mag = magnitude();
	return Vect(x / mag, y / mag, z / mag);
}
Vect Vect::negative()
{
	return Vect(-x, -y, -z);
}
double Vect::dotProduct(Vect v)
{
	return x*v.getVectX() + y*v.getVectY() + z*v.getVectZ();
}
Vect Vect::crossProduct(Vect v)
{
	return Vect(y*v.getVectZ() - z*v.getVectY(),
		        z*v.getVectX() - x*v.getVectZ(),
		        x*v.getVectY() - y*v.getVectX());
}
Vect Vect::vectAdd(Vect v)
{
	return Vect(x + v.getVectX(), y + v.getVectY(), z + v.getVectZ());
}
Vect Vect::vectMult(double scalar)
{
	return Vect(x *scalar, y  *scalar, z *scalar);
}