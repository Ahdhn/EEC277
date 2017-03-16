//stolen from //https://www.shadertoy.com/view/XdcSzH
//how to use ssbo http://stackoverflow.com/questions/32035423/trivial-opengl-shader-storage-buffer-object-ssbo-not-working

#version 430 

const int ConstNumForArray = 5000;

layout (std430, binding=1) buffer BVHnodeBuffer
{
	ivec2 BVHnode[ConstNumForArray];//x is the left node and y if the right node 
									//BVHnode[0].x = left child of node 0
									//BVHnode[0].y = right child of node 0
									//if either child is <=0 then it is an index (the abs value) to leafNode array	
};
layout (std430, binding=2) buffer SphereDataBuffer
{ 
	vec4 scene_sphere[ConstNumForArray];
};
/*layout (std430, binding=3) buffer StackBuffer
{
	//int BVHleafNode[ConstNumForArray];//this is an index to scene_sphere 	                    
	int stack[ConstNumForArray + 1];
};*/
 layout (std430, binding=4) buffer MinBboxBuffer
{
	vec4 BVHnodeBoxMin [ConstNumForArray];	//we only need x,y,z but for some mysterious reason it only works with vec4
};
layout (std430, binding=5) buffer MaxBboxBuffer
{
	vec4 BVHnodeBoxMax [ConstNumForArray];	
};

const vec3 iResolution = vec3(800, 600, 0); //viewport resolution in pixels (defined in main)
//int stack[500];
const float maxDist = 1000.0;
const vec3 amb = vec3(1.0);
const float eps = 1e-3;

struct Camera{
	vec3 up, right, forward;
	vec3 position;
};

//uniform struct Triangle {
//	vec3 A;//A, B and C are the three points of the triangle
//	       //such that A stores the x,y and z of this point 
//	vec3 B;
//	vec3 C;
//	vec4 color;
//	//vec2 material;
//};


Camera cam;
vec4 spheres[9];
vec4 colors[9];
vec2 materials[9];

//uniform vec4 scene_sphere[10];
uniform int numSphere;

/*uniform struct Node{
	vec3 bbox_min;
	vec3 bbox_max;	
	int objectID;
};
uniform Node BVH[64];
uniform int nodeArr[64];
uniform int leafNodeArr[64];*/


//uniform Triangle scene_tri[100];
//uniform int numTriangle;

void init()
{
	
    //the spheres of the scene 	
    spheres[0] = vec4(       0.7,    0.7,    -1.5,    0.1);//light source sphere
    spheres[1] = vec4(      -0.7,   0.25,    -1.5,    0.1);//light source sphere
    spheres[2] = vec4(       0.0,    0.5,    -1.5,    0.3);//big spheres in the center 
	//five big spheres that simulate the five walls 
    spheres[3] = vec4( 1001.0,       0,       0, 1000.0); 
    spheres[4] = vec4(-1001.0,       0,       0, 1000.0);
    spheres[5] = vec4(      0,  1001.0,       0, 1000.0); 
    spheres[6] = vec4(      0, -1001.0,       0, 1000.0);
    spheres[7] = vec4(      0,       0, -1003.0, 1000.0);
	
    //The diffuse color of the spheres 
    colors[0] = vec4(1.0, 0.8, 0.0,-1.0);
    colors[1] = vec4(0.0, 0.0, 1.0,-1.0);
    colors[2] = vec4(1.0,0.8, 0.0, -1.0);    
    colors[3] = vec4(1.0, 1.0, 0.0, 0.7);
    colors[4] = vec4(0.0, 1.0, 0.0, 0.7);
    colors[5] = vec4(1.0, 1.0, 1.0, 0.7);
    colors[6] = vec4(1.0, 1.0, 1.0, 0.7);
    colors[7] = vec4(1.0, 0.0, 0.0, 0.7);
	colors[8] = vec4(0.9, 0.89, 0.11, 1.0);

    //this is a hint if we want to make the object reflective (first id), refractive (second id) or both
    materials[0] = vec2 (0.0, 0.0);
    materials[1] = vec2 (0.0, 0.0);				
    materials[2] = vec2 (0.2, 0.8);	//not refraction here   
	//walls just have diffuse colors (no reflection or refraction) 
    materials[3] = vec2 (0.0, 0.0);				
    materials[4] = vec2 (0.0, 0.0);				
    materials[5] = vec2 (0.0, 0.0);				
    materials[6] = vec2 (0.0, 0.0);				
    materials[7] = vec2 (0.0, 0.0);	
	materials[8] = vec2 (0.9, 0.9);	
	   

    cam.up       = vec3(0.0, 1.0, 0.0);
    cam.right    = vec3(1.0, 0.0, 0.0);
    cam.forward  = vec3(0.0, 0.0,-1.0);
    cam.position = vec3(0.0, 0.0,-0.2);
}

vec3 getRayDir(vec2 fragCoord)
{
	//take xy coordinates of a pixel and figure out the ray that passes through it using the 
	//view port dimesions and camera position
	vec2 uv = (fragCoord.xy/iResolution.xy)*2.0 -1.0;
	uv.x*=iResolution.x/iResolution.y;
	return normalize(uv.x*cam.right+ uv.y*cam.up+cam.forward);
}

bool intersectSphere(vec3 ro, vec3 rd, vec4 sp, float tm, out float t)
{
	//does a ray of starting point ro and direction rd (which have been trimmed to tm)
	//intersects with sphere sp. if so return true and update the trimming parameter t 

	bool r = false;
	vec3 v = ro-sp.xyz;
	float b = dot(v,rd);
	float c = dot(v,v)-sp.w*sp.w;
	t = b*b -c;
	if(t>0.0){
		t = -b - sqrt(t);
		r = (t>0.0) && (t<tm);
	}

	return r;
}
void swap_float(float one, float two)
{
	float temp = one;
	one = two;
	two = temp;
}
bool intersectBox(vec3 ro, vec3 rd, vec3 minCorner, vec3 maxCorner)
{

	float tx1 = (minCorner.x - ro.x)/rd.x;
	float tx2 = (maxCorner.x - ro.x)/rd.x;

	float ty1 = (minCorner.y - ro.y)/rd.y;
	float ty2 = (maxCorner.y - ro.y)/rd.y;

	float tz1 = (minCorner.z - ro.z)/rd.z;
	float tz2 = (maxCorner.z - ro.z)/rd.z;

	float txn = min(tx1, tx2);
	float txf = max(tx1, tx2);

	float tyn = min(ty1, ty2);
	float tyf = max(ty1, ty2);

	float tzn = min(tz1, tz2);
	float tzf = max(tz1, tz2);

	float tnear = max(txn, max(tyn, tzn));
	float tfar  = min(txf, min(tyf, tzf));
									
	if (tfar > tnear){
		return true;
	}
	else{
		return false;
	}
	
}


float calcInter(vec3 ro, vec3 rd, out vec4 ob, out vec4 col, out vec2 mat)
{
	//calculate the intersection witht the objects in the scene
	float tm = maxDist;
	float t;

	//for(int i=0; i<numTriangle; i++){
	//	if(intersectTriangle(ro,rd,scene_tri[i],tm,t)){
	//		ob=special;
	//		//col=colors[0];			
	//		col = vec4(abs(ro.x),abs(ro.y),abs(ro.z),-1.0);
	//		tm=t;
	//		mat= materials[0];
			
	//	}
	//}
	
	//1) test with all the lights and walls

	if(intersectSphere(ro,rd,spheres[0],tm,t)){ ob=spheres[0]; col=colors[0]; tm=t; mat= materials[0];}
	if(intersectSphere(ro,rd,spheres[1],tm,t)){ ob=spheres[1]; col=colors[1]; tm=t; mat= materials[1];}
	if(intersectSphere(ro,rd,spheres[2],tm,t)){ ob=spheres[2]; col=colors[2]; tm=t; mat= materials[2];}
	if(intersectSphere(ro,rd,spheres[3],tm,t)){ ob=spheres[3]; col=colors[3]; tm=t; mat= materials[3];}
	if(intersectSphere(ro,rd,spheres[4],tm,t)){ ob=spheres[4]; col=colors[4]; tm=t; mat= materials[4];}
	if(intersectSphere(ro,rd,spheres[5],tm,t)){ ob=spheres[5]; col=colors[5]; tm=t; mat= materials[5];}
	if(intersectSphere(ro,rd,spheres[6],tm,t)){ ob=spheres[6]; col=colors[6]; tm=t; mat= materials[6];}
	if(intersectSphere(ro,rd,spheres[7],tm,t)){ ob=spheres[7]; col=colors[7]; tm=t; mat= materials[7];}	
	
	if(false){	
		//No acceleration (brute force)
		if(intersectBox(ro,rd,vec3(BVHnodeBoxMin[0].x,BVHnodeBoxMin[0].y,BVHnodeBoxMin[0].z),vec3(BVHnodeBoxMax[0].x,BVHnodeBoxMax[0].y,BVHnodeBoxMax[0].z))){
		//if(intersectBox(ro,rd,vec3(-1.6523619999999999,-0.69418700000000000,-1.6586470000000000),vec3(1.6649229999999999,0.68866700000000003,1.6404209999999999))){
			for(int i=0;i<numSphere;i++){		    
				if(intersectSphere(ro,rd,scene_sphere[i],tm,t)){ 				
					ob=scene_sphere[i];									
					col = colors[8];						
					tm=t;
					mat= materials[8];
				}
			}
		}


	}
	else{
	 	
		int stack[1000];
		//using BVH tree 		
		stack[0] = 1;
		stack[1] = 0;
		bool start = true; 
		
		while (stack[0]>0){
				
			//while the stack is not empty
			int id = stack[stack[0]];//take the last node in the stack to test it
			stack[0] = stack[0] - 1; //as it being tested, take it away 
			
			if(id<=0 && !start){
				
				//it is an object/sphere 
				id = abs(id);
				//id = BVHleafNode[id];//get the id to index the scene_sphere

			

				if(intersectSphere(ro,rd,scene_sphere[id],tm,t)){ //test for hitting the object/sphere
					ob=scene_sphere[id];									
					col = colors[8];						
					tm=t;
					mat= materials[8];
				}
				//id = stack2[stack2[0]];//take the last node in the stack to test it
				//stack2[0] = stack2[0] - 1;
			}else{
				
				start = false;
				//it's another node 
				//test if we are hitting this box 
				if(intersectBox(ro,rd,vec3(BVHnodeBoxMin[id].x,BVHnodeBoxMin[id].y,BVHnodeBoxMin[id].z),vec3(BVHnodeBoxMax[id].x,BVHnodeBoxMax[id].y,BVHnodeBoxMax[id].z))){				
					//if we hit it, we update the id with the left child
					//and store the right child to the stack to be tested later 

					//id = BVHnode[id].x;
										
					stack[0] = stack[0] + 1;
					stack[stack[0]] = BVHnode[id].y;
					
					stack[0] = stack[0] + 1;
					stack[stack[0]] = BVHnode[id].x;
				}				
			}						
		}


	}

	return tm;
}

bool inShadow(vec3 ro, vec3 rd, float d)
{	
	//does the intersection point (ro) with notaml direction (rd)
	//lies in the shadow
	float t;
	bool ret = false;
	//first two sphere are light sources and thus we don't calculate the shadow with them
	if(intersectSphere(ro,rd,spheres[2],d,t)){ ret = true; }
	if(intersectSphere(ro,rd,spheres[3],d,t)){ ret = true; }
	if(intersectSphere(ro,rd,spheres[4],d,t)){ ret = true; }
	if(intersectSphere(ro,rd,spheres[5],d,t)){ ret = true; }
	if(intersectSphere(ro,rd,spheres[6],d,t)){ ret = true; }

	
	//if(intersectBox(ro,rd,vec3(BVH[0].bbox_min.x,BVH[0].bbox_min.y,BVH[0].bbox_min.z),vec3(BVH[0].bbox_max.x,BVH[0].bbox_max.y,BVH[0].bbox_max.z))){			
		/*for(int i=0;i<numSphere;i++){
			if(intersectSphere(ro,rd,scene_sphere[i],d,t)){ 
				ret = true;
			}
		}*/
	//}
									 
	return ret;
}
vec3 calcShade(vec3 pt, vec4 ob, vec4 col, vec2 mat, vec3 n)
{
	float dist, diff;
	vec3 lcol, l;

	vec3 color =vec3(0.0);
	vec3 ambcol = amb * (1.0 -col.w)*col.rgb;
	vec3 scol = col.w * col.rgb;

	if(col.w > 0.0){//if it is not a light 
		l = spheres[0].xyz - pt;
		dist = length(l);
		l = normalize(l);
		lcol = colors[0].rgb;
		diff = clamp(dot(n,l),0.0,1.0);
		color += (ambcol*lcol + lcol*diff*scol)/(1.0 + dist*dist);
		if(inShadow(pt,l,dist)){
			color*=0.7;
	    }

		l = spheres[1].xyz - pt;
		dist = length(l);
		l = normalize(l);
		vec3 lcol = colors[1].rgb;
		diff = clamp(dot(n,l),0.0,1.0);
		color +=(ambcol*lcol + lcol*diff*scol)/(1.0 + dist*dist);

		if(inShadow(pt,l,dist)){
			color*=0.8;
		}
	}
	else{
		color =col.rgb;
	}
	return color;
}

float getFresnel(vec3 n, vec3 rd, float r0)
{
	//fresnel euqations 
	float ndotv = clamp(dot(n,-rd),0.0,1.0);
	return r0 + (1.0-r0)*pow(1.0 - ndotv,0.5);
}
vec3 getReflection(vec3 ro, vec3 rd)
{
	vec3 color = vec3(0);
	vec4 ob, col;
	vec2 mat;
	float tm = calcInter(ro, rd, ob, col, mat);
	if(tm < maxDist){
		vec3 pt = ro+rd*tm;
		vec3 n = normalize(pt - ob.xyz);
		color = calcShade(pt, ob, col, mat, n);
	}
	return color;
}

void main(void){


	init();//initiate the objects in the scene 
	float fresnel, tm;
	vec4 ob, col;
	vec2 mat;
	vec3 pt, refCol, n, refl;
	vec3 mask = vec3(1.0);
	vec3 color = vec3(0.0);
	vec3 ro = cam.position;
	vec3 rd = getRayDir (gl_FragCoord.xy);

	
	
	for(int i=0; i<5;i++){//do five iteration to simulate recursion 
		tm = calcInter(ro,rd,ob, col, mat);
		if(tm < maxDist){
			pt = ro + rd*tm;
			n = normalize(pt - ob.xyz);
			fresnel = getFresnel(n, rd, mat.x);
			mask *=fresnel;

			if(mat.y>0.0){//refractive
				ro = pt - n*eps;
				refl = reflect(rd,n);
				refCol = getReflection(ro, refl);
				color += refCol*mask;
				mask = col.rgb * (1.0 -fresnel) * (mask/fresnel);
				rd = refract(rd, n, mat.y);
			}
			else if(mat.x>0.0){//reflective
				color += calcShade(pt, ob, col, mat, n) * (1.0 - fresnel) * mask/ fresnel;
				ro = pt + n*eps;
				rd = reflect(rd,n);
			}
			else{//diffuse 
				color += calcShade(pt, ob, col, mat, n) *mask/fresnel;
				break;
			}
		}
	}		

	gl_FragColor = vec4(color,1.0f);	
	
	
	/*if (BVHnode[7].x == 7 && BVHnode[7].y == 6){
		gl_FragColor = vec4(color,1.0f);	
	}else{
		gl_FragColor = vec4(1.0f,1.0f,1.0f,1.0f);	
	}*/
	
};


//bool intersectTriangle(vec3 ro, vec3 rd,  Triangle myTri,float tm, out float t){	
//	vec3 AB = myTri.B - myTri.A;
//	vec3 AC = myTri.C - myTri.A;
//	float det = determinant( mat3(AB, AC, -1.0f*rd));	
//	if(det == 0.0f){
//		return false;
//	} else {
//		vec3 oA = ro -  myTri.A;		
//		mat3 Di = inverse(mat3(AB, AC, -1.0f*rd));
//		vec3 solution = Di*oA;
//		if(solution.x >= -0.0001 && solution.x <= 1.0001){
//			if(solution.y >= -0.0001 && solution.y <= 1.0001){
//				if(solution.x + solution.y <= 1.0001){
//					t = solution.z;
//					if(t>0.0 && t<tm){
//						return true;
//					}else{
//						return false;
//					}		
//				}
//			}
//		}
//		return false;
//	}
//}