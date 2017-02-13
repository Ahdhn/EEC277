#include "Triangle.h"

Triangle::Triangle()
{
	A = Vect(1, 0, 0);
	B = Vect(0, 1, 0);
	C = Vect(0, 0, 1);
	color = Color(0.5, 0.5, 0.5, 0);
}
Triangle::Triangle(Vect pointA, Vect pointB, Vect pointC, Color colorValue)
{
	A = pointA;
	B = pointB;
	C = pointC;	
	color = colorValue;
}
Triangle::~Triangle()
{
}
Vect Triangle::getTriangleNormal(){
	Vect CA(C.getVectX() - A.getVectX(), C.getVectY() - A.getVectY(), C.getVectZ() - A.getVectZ());
	Vect BA(B.getVectX() - A.getVectX(), B.getVectY() - A.getVectY(), B.getVectZ() - A.getVectZ());
	normal = CA.crossProduct(BA).normalize();
	return  normal; 
}
double Triangle::getTriangleDistance(){ 
	normal = getTriangleNormal();
	distance = normal.dotProduct(A);
	return  distance; 
}
Vect Triangle::getNormalAt(Vect)
{
	normal = getTriangleNormal();
	return normal;

}

double Triangle::findIntersection(Ray ray)
{
	Vect ray_direction = ray.getRayDirection();
	Vect ray_origin = ray.getRayOrigin();
	normal = getTriangleNormal();
	distance = getTriangleDistance();

	double a = ray_direction.dotProduct(normal);

	if (a == 0){
		//means the ray is parallel to the plane
		return -1;
	}
	else{
		double b = normal.dotProduct(ray.getRayOrigin().vectAdd(normal.vectMult(distance).negative()));

		double distance2plane = -1.0*b / a;

		//using barycentric coordinates to find if we are inside the tirangle 

		//[CA x QA] * n > = 0
		//[BC x QC] * n > = 0
		//[AB x QB] * n > = 0


		double Qx = ray_direction.vectMult(distance2plane).getVectX() + ray_origin.getVectX();
		double Qy = ray_direction.vectMult(distance2plane).getVectY() + ray_origin.getVectY();
		double Qz = ray_direction.vectMult(distance2plane).getVectZ() + ray_origin.getVectZ();

		Vect Q(Qx, Qy, Qz);
		Vect CA(C.getVectX() - A.getVectX(), C.getVectY() - A.getVectY(), C.getVectZ() - A.getVectZ());
		Vect QA(Q.getVectX() - A.getVectX(), Q.getVectY() - A.getVectY(), Q.getVectZ() - A.getVectZ());
		double test1 = (CA.crossProduct(QA)).dotProduct(normal);

		Vect BC(B.getVectX() - C.getVectX(), B.getVectY() - C.getVectY(), B.getVectZ() - C.getVectZ());
		Vect QC(Q.getVectX() - C.getVectX(), Q.getVectY() - C.getVectY(), Q.getVectZ() - C.getVectZ());
		double test2 = (BC.crossProduct(QC)).dotProduct(normal);

		Vect AB(A.getVectX() - B.getVectX(), A.getVectY() - B.getVectY(), A.getVectZ() - B.getVectZ());
		Vect QB(Q.getVectX() - B.getVectX(), Q.getVectY() - B.getVectY(), Q.getVectZ() - B.getVectZ());
		double test3 = (AB.crossProduct(QB)).dotProduct(normal);
		if (test1 >= 0 && test2 >= 0 && test3 >= 0){ 
			//inside the triangle 
			return distance2plane;
		}
		else{
			//outside the triangle 
			return -1;
		}
		
	}


}