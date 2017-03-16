//a ray is originated at "vec3(ro)" and into normalized direction "vec3(rd)"
//SceneInfo[ ] contains all the input to the shader 
//SceneInfo[0] contains the information of the first sphere (a position and a radius)
//SceneInfo[1] contains the information of the second sphere
//SceneInfo[2] & SceneInfo[3] are the two cylinders
//SceneInfo[4] & SceneInfo[5] are the two planes
//SceneInfo[6] to SceneInfo[11] are the color of the objects 
//SceneInfo[12] contains the camera position
//SceneInfo[13] to SceneInfo[15] contain the camera matrix 
//http://www.iquilezles.org/www/articles/simplegpurt/simplegpurt.htm
//https://gist.github.com/num3ric/4408481
//https://github.com/hjwdzh/myRaytracer
//http://www.clockworkcoders.com/oglsl/rt/gpurt1.htm
//https://github.com/larsbrinkhoff/glsl-sphere-tracing
//https://github.com/StanEpp/OpenGL_Raytracing
//https://www.shadertoy.com/view/XdcSzH

#version 330 core
//uniform vec4 SceneInfo[16];
uniform sampler2D tex00;
in vec3 raydir;
out vec4 color;
 
uniform vec4 sp1 = vec4(0,0,0,1.0);//1st sphere
uniform vec4 sp2 = vec4(1.75,0,0,0.5);//2nd sphere
uniform vec4 sp1_color = vec4(0.5, 1.0, 0.5, 1.0);//1st sphere color
uniform vec4 sp2_color = vec4(0.5, 0.25, 0.25, 1.0);//2nd sphere color
uniform vec4 cam_pos = vec4(3.0,1.5,-4,1.0);//camera position 


bool intSphere (in vec4 sp, in vec3 ro, in vec3 rd, in float tm, out float t){

    //intersection with a sphere
	//ray at ro and direction at rd
    bool r =false;
	vec3 d = ro-sp.xyz; //ray_origin - sphere_center 
	float b = dot(rd,d); //dot product of (ray_origin - sphere_center), ray_direction
	float c = dot(d,d) - sp.w*sp.w; //d^2 - radius^2
	t = b*b - c; //discriminant (we don't have to divide by two since it will cancel out later)
	if(t > 0.0){
	//if the ray intersects the sphere 
	   t =-b-sqrt(t); //(told you it gonna cancel)
	   r = (t>0.0) && (t<tm);
	}
	return r;
}

bool intCylinder(in vec4 sp, in vec3 ro, in vec3 rd, in float tm, out float t){

     //intersection with a cylinder 
	 //ray at ro and direction at rd
	 bool r = false;
	 vec3 d = ro-sp.xyz;
	 float a = dot(rd.xz,rd.xz);
	 float b = dot(rd.xz, d.xz);
	 float c = dot (d.xz,d.xz) - sp.w*sp.w;
	 t = b*b - a*c;
	 if(t>0.0){
	    t = (-b-sqrt(t)*sign(sp.w))/a;
		r = (t>0.0) && (t<tm);
	 }
	 return r;
}

bool intPlane (in vec4 pl, in vec3 ro, in vec3 rd, in float tm, out float t){
     //intersection with a plane
	 //ray at ro and direction at rd
	 t = -(dot(pl.xyz,ro)+pl.w)/dot(pl.xyz,rd);
	 return (t>0.0) && (t<tm);
}

float calcInter(in vec3 ro, in vec3 rd, out vec4 ob, out vec4 co){
	//Calculate the intersection for different objects you have in the scene 
	//always keep track of the closest object

	float tm = 10000.0;
	float t;
	
	if(intSphere  (/*SceneInfo[0]*/sp1,ro,rd,tm,t)) {ob = sp1/*SceneInfo[0]*/; co=sp1_color /*SceneInfo[6]*/;  tm=t;}
	if(intSphere  (/*SceneInfo[1]*/sp1,ro,rd,tm,t)) {ob = sp2/*SceneInfo[1]*/; co=sp2_color /*SceneInfo[7]*/;  tm=t;}

	//if(intCylinder(SceneInfo[2],ro,rd,tm,t)) {ob = SceneInfo[2]; co=SceneInfo[8];  tm=t;}
	//if(intCylinder(SceneInfo[3],ro,rd,tm,t)) {ob = SceneInfo[3]; co=SceneInfo[9];  tm=t;}
	//if(intPlane   (SceneInfo[4],ro,rd,tm,t)) {ob = SceneInfo[4]; co=SceneInfo[10]; tm=t;}
	//if(intPlane   (SceneInfo[5],ro,rd,tm,t)) {ob = SceneInfo[5]; co=SceneInfo[11]; tm=t;}

	return tm;
}

vec3 calcNor(in vec4 ob, in vec4 ot, in vec3 po, out vec2 uv){
	//get the norm of a point on an object 
	vec3 no;
	if(ot.w>2.5){
		no.xz = po.xz-ob.xz;
		no.y=0.0;
		no= no/ob.w;
		uv = vec2(no.x,po.y+cam_pos.w/*SceneInfo[12].w*/);
	}
	else if(ot.w>1.5){
		no = ob.xyz;
		uv = po.xz*0.2;
	}
	else{
		no = po-ob.xyz;
		no = no/ob.w;
		uv = no.xy;
	}
	return no;
}



bool calcShadow(in vec3 ro, in vec3 rd, in float l){
	//check if the intersection point lies on the shadow of any other object
	//takes the intersection point and normal to the object 
	float t;

	/*bvec4 ss = bvec4 (intSphere  (SceneInfo[0],ro,rd,l,t),
	                  intSphere  (SceneInfo[1],ro,rd,l,t),
					  intCylinder(SceneInfo[2],ro,rd,l,t),
					  intCylinder(SceneInfo[3],ro,rd,l,t));*/

	bvec2 ss = bvec2(intSphere  (sp1,ro,rd,l,t),
	                 intSphere  (sp2,ro,rd,l,t));
	
	return any(ss);

}

vec4 calcShade (in vec3 po, in vec4 ob, in vec4 co, in vec3 rd, out vec4 re){
	//after getting the intersection point, this shades the pixel
	//by first compute the normal of the object at the intersection point 
	//then we do basic diffuse and specular lighting calculations 
	//also, we cal shadow to check if the points lies in the shadow 

	float di, sp;
	vec2 uv;
	vec3 no;
	vec4 lz;

	lz.xyz = vec3(0.0, 1.5, -3.0) - po;
	lz.w = length(lz.xyz);
	lz.xyz = lz.xyz/lz.w;

	no = calcNor(ob,co,po,uv);

	di = dot(no, lz.xyz);
	re.xyz = reflect(rd,no);

	sp = dot(re.xyz, lz.xyz);	
	sp = max(sp,0.0);
	sp = sp*sp;
	sp = sp*sp;

	if(calcShadow(po, lz.xyz, lz.w)){di=0.0;}

	di = max(di, 0.0);
	co = co*texture2D(tex00,uv) * (vec4(0.21,0.28,0.3,1.0) + 0.5*vec4(1.0, 0.9, 0.65 ,1.0)*di) +sp;

	di = dot(no,-rd);
	re.w =di;
	di = 1.0-di*di;
	di = di*di;

	return (co + 0.6*vec4(di));
}

void main(void){
	//we simulate the recusion of ray tracing by doing two-times calcInter-calcShade 

    float tm;
	vec4 ob, co, co2, re,re2;
	vec3 no,po;
	vec3 ro = cam_pos.xyz/*SceneInfo[12].xyz*/;//ray origin is alwats the cam position 
	vec3 rd = normalize(raydir); //ray direction 

	//1)
	tm = calcInter(ro, rd, ob, co);//get the intersection 
	po = ro + rd*tm;
	co = calcShade(po,ob,co,rd,re);

	//2)
	tm = calcInter(po, re.xyz, ob, co2);
	po+=re.xyz*tm;
	co2 = calcShade(po,ob, co2, re.xyz, re2);


	color = mix (co, co2, 0.5-0.5*re.w)/*+ vec4(SceneInfo[13].w)*/;

	
};
