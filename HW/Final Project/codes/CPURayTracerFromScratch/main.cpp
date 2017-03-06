//Basic ray tracer, following tutorial list here https://www.youtube.com/playlist?list=PLHm_I0tE5kKPPWXkTTtOn8fkcwEGZNETh
//by Ahmed H. Mahmoud 2017
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <cmath>
#include <limits>

#include "Vect.h"
#include "Ray.h"
#include "Camera.h"
#include "Color.h"
#include "Light.h"
#include "Sphere.h"
#include "Object.h"
#include "Plane.h"
#include "Source.h"
#include "Triangle.h"
struct RGBType{
	double r, g, b;
};

int thisone; //pixel of the a (x,y) pair


void SaveBMP(const char *filename, int w, int h, int dpi, RGBType*data){ 
	FILE *f;
	int k = w*h;
	int s = 4 * k;
	int filesize = 54 + s;
	double factor = 39.375;;
	int m = static_cast<int>(factor);

	int ppm = dpi*m;

	unsigned char bmpfileheader[14] = { 'B', 'M', 0, 0, 0, 0, 0, 0, 0, 0, 54, 0, 0, 0 };
	unsigned char bmpinfoheader[40] = { 40, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 24, 0 };
	bmpfileheader[2] = (unsigned char)(filesize);
	bmpfileheader[3] = (unsigned char)(filesize>>8);
	bmpfileheader[4] = (unsigned char)(filesize>>16);
	bmpfileheader[5] = (unsigned char)(filesize>>24);

	bmpinfoheader[4] = (unsigned char)(w);	
	bmpinfoheader[5] = (unsigned char)(w >> 8);
	bmpinfoheader[6] = (unsigned char)(w >> 16);
	bmpinfoheader[7] = (unsigned char)(w >> 24);

	bmpinfoheader[8] = (unsigned char)(h);
	bmpinfoheader[9] = (unsigned char)(h >> 8);
	bmpinfoheader[10] = (unsigned char)(h >> 16);
	bmpinfoheader[11] = (unsigned char)(h >> 24);

	bmpinfoheader[21] = (unsigned char)(s);
	bmpinfoheader[22] = (unsigned char)(s >> 8);
	bmpinfoheader[23] = (unsigned char)(s >> 16);
	bmpinfoheader[24] = (unsigned char)(s >> 24);

	bmpinfoheader[25] = (unsigned char)(ppm);
	bmpinfoheader[26] = (unsigned char)(ppm >> 8);
	bmpinfoheader[27] = (unsigned char)(ppm >> 16);
	bmpinfoheader[28] = (unsigned char)(ppm >> 24);

	bmpinfoheader[29] = (unsigned char)(ppm);
	bmpinfoheader[30] = (unsigned char)(ppm >> 8);
	bmpinfoheader[31] = (unsigned char)(ppm >> 16);
	bmpinfoheader[32] = (unsigned char)(ppm >> 24);

	f = fopen(filename, "wb");
	fwrite(bmpfileheader, 1, 14, f);
	fwrite(bmpinfoheader, 1, 40, f);

	for (int i = 0; i < k; i++){
		RGBType rgb = data[i];
		double red = (data[i].r) * 255;
		double green = (data[i].g) * 255;
		double blue = (data[i].b) * 255;

		unsigned char color[3] = { (int)floor(blue), (int)floor(green), (int)floor(red) };

		fwrite(color, 1, 3, f);
		
	}
	fclose(f);
}

int winningObjectIndex(std::vector<double>object_intersections)
{
	//return the index of the winning intersection
	int index_of_minimum_value;

	//prevent unnecessary calculations 
	if (object_intersections.size() == 0){
		//if there were no intersection
		return -1;
	}
	else if (object_intersections.size() == 1){ 
		if (object_intersections.at(0) > 0){ 
			//if that intersection is greater than zero then its our index of min value
			return 0;
		}
		else{
			//otherwise the only intersection value is negative 
			return -1;
		}
	}
	else{
		// otherwise there is more than one intersection 
		// first find the maximum value 
		double max = 0;
		for (int i = 0; i < object_intersections.size(); i++){
			if (max < object_intersections.at(i)){
				max = object_intersections.at(i);
			}
		}
		//the starting from the max value find the min positive value
		if (max > 0){
			//we only want +ve intersections
			for (int index = 0; index < object_intersections.size(); index++){
				if (object_intersections.at(index) > 0 && object_intersections.at(index) <= max){
					max = object_intersections.at(index);
					index_of_minimum_value = index;
				}
			}
			return index_of_minimum_value;
		}
		else{
			//all intersections were -ve
			return -1;
		}
	}
}

Color getColorAt(Vect intersection_position, Vect intersection_ray_direction, std::vector<Object*> scene_objects, int index_of_winning_object,
	            std::vector<Source*>light_sources, double accuracy, double ambientLight)
{
	//most complicated function
	//get us shadow, reflection, shiness, etc
	Color winning_object_color = scene_objects.at(index_of_winning_object)->getColor();
	Vect winning_object_normal = scene_objects.at(index_of_winning_object)->getNormalAt(intersection_position);

	if (winning_object_color.getColorSpecial() == 2){ 
		//checkred/tile floor pattern 
		int square = (int)floor(intersection_position.getVectX()) + (int)floor(intersection_position.getVectZ());
		if (square % 2 == 0){
			//black tile 
			winning_object_color.setColorRed(0);
			winning_object_color.setColorGreen(0);
			winning_object_color.setColorBlue(0);
		}else{
			//white tile 
			winning_object_color.setColorRed(1);
			winning_object_color.setColorGreen(1);
			winning_object_color.setColorBlue(1);
		}
	}

	Color final_color = winning_object_color.colorScalar(ambientLight);//all changes should be down to this color 

	//add reflection
	if (winning_object_color.getColorSpecial() > 0 && winning_object_color.getColorSpecial()<=1.0){
		//reflection from objects with specular intensity 
		double dot1 = winning_object_normal.dotProduct(intersection_ray_direction.negative());
		Vect scalar1 = winning_object_normal.vectMult(dot1);
		Vect add1 = scalar1.vectAdd(intersection_ray_direction);
		Vect scalar2 = add1.vectMult(2);
		Vect add2 = intersection_ray_direction.negative().vectAdd(scalar2);
		Vect reflection_direction = add2.normalize();

		Ray reflection_ray(intersection_position, reflection_direction);
		//determine what the ray intersects with first
		std::vector<double>reflection_intersections;
		for (int reflection_index = 0; reflection_index < scene_objects.size(); reflection_index++){
			reflection_intersections.push_back(scene_objects.at(reflection_index)->findIntersection(reflection_ray));
		}

		int index_of_winning_object_with_reflection = winningObjectIndex(reflection_intersections);

		if (index_of_winning_object_with_reflection != -1){ 
			//reflection ray missed everything else 
			if (reflection_intersections.at(index_of_winning_object_with_reflection) > accuracy){
				//detemine the position and direction at the point ofintersection with the reflection ray
				//the ray only affect thet color if it refelcted off something 
				Vect reflection_intersection_position = intersection_position.vectAdd(reflection_direction.vectMult(reflection_intersections.at(index_of_winning_object_with_reflection)));
				Vect reflection_intersection_ray_direction = reflection_direction;

				Color reflection_intersection_color = getColorAt(reflection_intersection_position, reflection_intersection_ray_direction, scene_objects, index_of_winning_object_with_reflection, light_sources, accuracy, ambientLight);

				final_color = final_color.colorAdd(reflection_intersection_color.colorScalar(winning_object_color.getColorSpecial()));

			}
		}

	}


	for (int light_index = 0; light_index < light_sources.size(); light_index++){
		Vect light_direction = light_sources.at(light_index)->getLightPosition().vectAdd(intersection_position.negative()).normalize();
		
		float cosine_angle = winning_object_normal.dotProduct(light_direction);

		if (cosine_angle > 0.0){ 
			//test for shadow
			bool shadowed = false;

			Vect distance_to_light = light_sources.at(light_index)->getLightPosition().vectAdd(intersection_position.negative());

			float distance_to_light_magnitude = distance_to_light.magnitude();


			//create new ray from the intersection point to the light source 
			//if this new ray intersect any thing else, then the intersection is in the shadow
			Ray shadow_ray(intersection_position, light_sources.at(light_index)->getLightPosition().vectAdd(intersection_position.negative()).normalize());

			std::vector<double> secondary_intersections;
			for (int object_index = 0; object_index < scene_objects.size() && !shadowed; object_index++){ 
				secondary_intersections.push_back(scene_objects.at(object_index)->findIntersection(shadow_ray));
			}

			//test if it shadow 
			for (int c = 0; c < secondary_intersections.size(); c++){
				if (secondary_intersections.at(c) > accuracy){ 
					if (secondary_intersections.at(c) <=distance_to_light_magnitude){
						shadowed = true;
					}
					break;
				}
			}

			
			if (!shadowed){
				//get color of the shadow
				final_color = final_color.colorAdd(winning_object_color.colorMultiply(light_sources.at(light_index)->getLightColor()).colorScalar(cosine_angle));

				//add shinness
				if (winning_object_color.getColorSpecial() > 0.0 && winning_object_color.getColorSpecial()<=1.0){ 
					//betweetn 0 and 1, it's shiny YAY
					double dot1 = winning_object_normal.dotProduct(intersection_ray_direction.negative());
					Vect scalar1 = winning_object_normal.vectMult(dot1);
					Vect add1 = scalar1.vectAdd(intersection_ray_direction);
					Vect scalar2 = add1.vectMult(2);
					Vect add2 = intersection_ray_direction.negative().vectAdd(scalar2);
					Vect reflection_direction = add2.normalize();
					double specular = reflection_direction.dotProduct(light_direction);
					if (specular > 0.0){
						specular = pow(specular, 10);
						final_color = final_color.colorAdd(light_sources.at(light_index)->getLightColor().colorScalar(specular*winning_object_color.getColorSpecial()));
						
					}
				}
			}
		}
	}

	return final_color.clip();
}

void makeCube(Vect corner1, Vect corner2, Color color, std::vector<Object*>&scene_objects)
{
	//make cube outof tirangles 
	// corner1
	double c1x = corner1.getVectX();
	double c1y = corner1.getVectY();
	double c1z = corner1.getVectZ();

	// corner2
	double c2x = corner1.getVectX();
	double c2y = corner1.getVectY();
	double c2z = corner1.getVectZ();

	Vect A(c2x, c1y, c1z);
	Vect B(c2x, c1y, c2z);
	Vect C(c1x, c1y, c2z);

	Vect D(c2x, c2y, c1z);
	Vect E(c1x, c2y, c1z);
	Vect F(c1x, c2y, c2z);

	//left side 
	scene_objects.push_back(new Triangle(D, A, corner1, color));
	scene_objects.push_back(new Triangle(corner1, E, D, color));

	//far side 
	scene_objects.push_back(new Triangle(corner2, B, A, color));
	scene_objects.push_back(new Triangle(A, D, corner2, color));

	//right side
	scene_objects.push_back(new Triangle(F, C, B, color));
	scene_objects.push_back(new Triangle(B, corner2, F, color));

	//front side
	scene_objects.push_back(new Triangle(E , corner1, C, color));
	scene_objects.push_back(new Triangle(C, F, E, color));

	//top
	scene_objects.push_back(new Triangle(D, E, F, color));
	scene_objects.push_back(new Triangle(F, corner2, D, color));

	//bottom
	scene_objects.push_back(new Triangle(corner1, A, B, color));
	scene_objects.push_back(new Triangle(B, C, corner1, color));
}
int main(int argc, char*argv[])
{
	std::cout << "Rendering...." << std::endl;

	clock_t t1, t2;
	t1 = clock();

	int width(640), height(480), dpi(72);
	int n = width*height;
	double aspectratio = (double)width / (double)height;
	RGBType *pixels = new RGBType[n];

	double ambientLight(0.2), accuracy(0.0000001);

	const int aadepth = 1;//four rays per pixels 
	double aathreshold = 0.1;

	Vect O(0, 0, 0);//sphere origin
	Vect X(1, 0, 0); //vector for direction x,y,z
	Vect Y(0, 1, 0);
	Vect Z(0, 0, 1);
	Vect new_sphere_location(1.75, 0, 0);

	Vect look_at(0, 0, 0);

	//create the cam of the scene 
	Vect campos(3, 1.5, -4);//arbitrary position above the y plane 
	Vect diff_btw(campos.getVectX() - look_at.getVectX(),
		          campos.getVectY() - look_at.getVectY(),
		          campos.getVectZ() - look_at.getVectZ());
	Vect camdir = diff_btw.negative().normalize();
	Vect camright = Y.crossProduct(camdir).normalize();
	Vect camdown = camright.crossProduct(camdir);
	Camera scene_cam(campos, camdir, camright, camdown);


	//create colors
	Color white_light(1.0, 1.0, 1.0, 0.0);
	Color pretty_green(0.5, 1.0, 0.5, 0.3); //with refelctivness 
	Color gray(0.5, 0.5, 0.5, 0);
	Color black(0.0, 0.0, 0.0, 0.0);
	Color checkerboard(0.5, 0.25, 0.25, 2);
	Color orange(0.94, 0.75, 0.31, 0);
	Color pretty_maroon(0.5, 0.25, 0.25, 0.4);

	//create light source 
	Vect light_position(-7, 10, -10);	
	Light scene_light(light_position, white_light);
	std::vector<Source*>light_sources;
	light_sources.push_back(dynamic_cast<Source*>(&scene_light));//we can now add more if we like 


	//scene objects	
	Sphere scene_sphere(O, 1.0, pretty_green);
	Sphere scene_sphere2(new_sphere_location, 0.5, pretty_maroon);
	Plane scene_plane(Y, -1.0, checkerboard);
	//Triangle scene_triangle(Vect(3, 0, 0), Vect(0, 3, 0), Vect(0, 0, 3), orange);

	double xamnt, yamnt; //slightly to the left and right of the direction pointed at by the camera (origin)
	std::vector<Object*>scene_objects;
	scene_objects.push_back(dynamic_cast<Object*>(&scene_sphere));
	scene_objects.push_back(dynamic_cast<Object*>(&scene_sphere2));
	scene_objects.push_back(dynamic_cast<Object*>(&scene_plane));
	//scene_objects.push_back(dynamic_cast<Object*>(&scene_triangle));
	//makeCube(Vect(1, 1, 1), Vect(-1, -1, -1), orange, scene_objects);

	int this_one, aa_index;
	double tempRed[aadepth*aadepth];
	double tempGreen[aadepth*aadepth];
	double tempBlue[aadepth*aadepth];

	for (int x = 0; x < width; x++){//from the left side to the right side 
		for (int y = 0; y < height; y++){//from top to the bottom
			thisone = y*width + x;

			
			//these double loops for anti-aliasing 
			for (int aax = 0; aax < aadepth;aax++){
				for (int aay = 0; aay < aadepth; aay++){ 

					aa_index = aay*aadepth + aax;
					
					//srand(time(0));
					//create the ray from the camer to this pixel

					if (aadepth == 1){ 
						//no anti-aliasing 
						if (width > height){
							//the image is wider than it is tall 
							xamnt = ((x + 0.5) / width)*aspectratio - (((width - height) / (double)height) / 2.0);
							yamnt = ((height - y) + 0.5) / height;
						}
						else if (height > width){
							//the image is taller than it is wide
							xamnt = (x + 0.5) / width;
							yamnt = (((height - y) + 0.5) / height) / aspectratio - (((height - width) / (double)width) / 2);
						}
						else{
							//the image is square 
							xamnt = (x + 0.5) / width;
							yamnt = ((height - y) + 0.5) / height;
						}
					}
					else{
						//with anti-aliasing 
						if (width > height){
							//the image is wider than it is tall 
							xamnt = ((x + (double)aax/((double)aadepth-1)) / width)*aspectratio - (((width - height) / (double)height) / 2.0);
							yamnt = ((height - y) + (double)aay / ((double)aadepth - 1)) / height;
						}
						else if (height > width){
							//the image is taller than it is wide
							xamnt = (x + (double)aax / ((double)aadepth - 1)) / width;
							yamnt = (((height - y) + (double)aay / ((double)aadepth - 1)) / height) / aspectratio - (((height - width) / (double)width) / 2);
						}
						else{
							//the image is square 
							xamnt = (x + 0.5) / width;
							yamnt = ((height - y) + 0.5) / height;
						}
					}



					//origin of the ray is the origin of the cam
					Vect cam_ray_origin = scene_cam.getCameraPosition();
					Vect cam_ray_direction = camdir.vectAdd( camright.vectMult(xamnt - 0.5).vectAdd(camdown.vectMult(yamnt - 0.5))).normalize();

					Ray cam_ray(cam_ray_origin, cam_ray_direction);

					std::vector<double> intersections;

					//loop through each object and find if there is any intersection between this ray and any of the objects
					for (int index = 0; index < scene_objects.size(); index++){
						intersections.push_back(scene_objects.at(index)->findIntersection(cam_ray));
					}


					//check which object is closer 
					int index_of_winning_object = winningObjectIndex(intersections);

					if (index_of_winning_object == -1) {
						//set the background black						
						tempRed[aa_index] = 0.0;
						tempGreen[aa_index] = 0.0;
						tempBlue[aa_index] = 0.0;
					}
					else{
						//index coresponds to object in the scene
						if (intersections.at(index_of_winning_object) > accuracy){
							//determine the position and direction vectors at the point of intersection
							Vect intersection_position = cam_ray_origin.vectAdd(cam_ray_direction.vectMult(intersections.at(index_of_winning_object)));
							Vect intersection_ray_direction = cam_ray_direction;

							Color intersection_color = getColorAt(intersection_position, intersection_ray_direction, scene_objects, index_of_winning_object, light_sources, accuracy, ambientLight); 
														
							tempRed[aa_index] = intersection_color.getColorRed();
							tempGreen[aa_index] = intersection_color.getColorGreen();
							tempBlue[aa_index] = intersection_color.getColorBlue();

						}
					}

				}
			}

			//avergae the pixel color
			double totoalRed(0), totoalGreen(0), totoalBlue(0);
			for (int iColor = 0; iColor < aadepth*aadepth; iColor++){
				totoalRed += tempRed[iColor];
				totoalGreen += tempGreen[iColor];
				totoalBlue += tempBlue[iColor];
			}

			double avgRed = totoalRed / (aadepth*aadepth);
			double avgGreen = totoalGreen / (aadepth*aadepth);
			double avgBlue = totoalBlue / (aadepth*aadepth);

			pixels[thisone].r = avgRed;
			pixels[thisone].g = avgGreen;
			pixels[thisone].b = avgBlue;

		}
	}

	SaveBMP("scene2.bmp", width, height, dpi, pixels);

	delete pixels;
	
	
	t2 = clock();
	float diff = ((float)t2 - (float)t1) / 1000;
	std::cout << diff << " seconds" << std::endl;

	return 0;
}

