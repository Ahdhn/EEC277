#ifndef _LIGHT_H
#define _LIGHT_H

#include "Vect.h"
#include "Color.h"
#include "Source.h"
class Light :public Source
{
public:

	Vect position;
	Color color;


	Light();
	Light(Vect, Color);


	~Light();

	//method functions 

	virtual Vect getLightPosition(){ return  position; }
	virtual Color getLightColor(){ return  color; }


private:

};

#endif 
