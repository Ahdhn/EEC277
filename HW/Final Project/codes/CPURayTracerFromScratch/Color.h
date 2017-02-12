#ifndef _COLOR_H
#define _COLOR_H

#include "math.h"

class Color
{
public:

	double red, green, blue, special;

	Color();
	Color(double, double, double, double);


	~Color();


	//method function 
	double getColorRed();
	double getColorGreen();
	double getColorBlue();
	double getColorSpecial();

	void setColorRed(double);
	void setColorGreen(double);
	void setColorBlue(double);
	void setColorSpecial(double);

	double brightness();
	Color colorScalar(double scalar);
	Color colorAdd(Color color);
	Color colorMultiply(Color color);
	Color colorAverage(Color color);
	Color clip();


private:

};

#endif 
