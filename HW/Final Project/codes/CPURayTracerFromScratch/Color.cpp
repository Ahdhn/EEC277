#include "Color.h"

Color::Color()
{
	red = 0.5;
	green = 0.5;
	blue = 0.5;
	special = 0.5;
}
Color::Color(double redValue, double greenValue, double blueValue, double specialValue)
{
	red = redValue;
	green = greenValue;
	blue = blueValue;
	special = specialValue;
}

Color::~Color()
{
}
double Color::getColorRed()
{
	return red;
}
double Color::getColorGreen()
{
	return green;
}
double Color::getColorBlue()
{
	return blue;
}
double Color::getColorSpecial()
{
	return special;
}
void Color::setColorRed(double redValue)
{
	red = redValue;
}
void Color::setColorGreen(double greenValue)
{
	green = greenValue;
}
void Color::setColorBlue(double blueValue)
{
	blue = blueValue;
}
void Color::setColorSpecial(double specialValue)
{
	special = specialValue;

}
double Color::brightness()
{
	return (red + green + blue) / 3.0;
}
Color Color::colorScalar(double scalar)
{
	return Color(red*scalar, green*scalar, blue*scalar, special);
}
Color Color::colorAdd(Color color)
{
	return Color(red + color.getColorRed(), green + color.getColorGreen(), blue + color.getColorBlue(), special);
}
Color Color::colorMultiply(Color color)
{
	return Color(red * color.getColorRed(), green * color.getColorGreen(), blue * color.getColorBlue(), special);
}

Color Color::colorAverage(Color color)
{
	return Color((red + color.getColorRed()) / 2.0, (green + color.getColorGreen()) / 2.0, (blue + color.getColorBlue()) / 2.0, special);
}
Color Color::clip()
{
	double all_light = red + green + blue;
	double excess_light = all_light - 3;
	if (excess_light > 0.0){
		red = red + excess_light*(red / all_light);
		green = green + excess_light*(green / all_light);
		blue = blue + excess_light*(blue / all_light);
	}

	if (red > 1.0){ red = 1.0; }
	if (green > 1.0){ green = 1.0; }
	if (blue > 1.0){ blue = 1.0; }

	if (red < 0.0){ red = 0.0; }
	if (green < 0.0){ green = 0.0; }
	if (blue < 0.0){ blue = 0.0; }

	return Color(red, green, blue,special);

}
