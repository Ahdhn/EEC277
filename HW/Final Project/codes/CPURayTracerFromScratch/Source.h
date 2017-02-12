#ifndef _SOURCE_H
#define _SOURCE_H
#include "Vect.h"
#include "Color.h"
class Source
{
public:
	Source();
	~Source();
	virtual Vect getLightPosition(){ return Vect(0, 0, 0); }
	virtual Color getLightColor(){ return Color(1, 1, 1, 0); }

private:

};



#endif