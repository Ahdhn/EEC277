//stolen from //https://www.shadertoy.com/view/XdcSzH

#version 330 core
const vec3 iResolution = vec3(800, 600, 0); //viewport resolution in pixels (defined in main)

const float maxDist = 1000.0;
const vec3 amb = vec3(1.0);
const float eps = 1e-3;

struct Camera{
	vec3 up, right, forward;
	vec3 position;
};

Camera cam;
vec4 spheres[10];
vec4 colors[10];
vec2 materials[10];


void init()
{
    //the spheres of the scene 
    spheres[0] = vec4(      0.7,       0,    -1.5,    0.1);//light source sphere
    spheres[1] = vec4(      -0.7,    0.25,    -1.5,    0.1);//light source sphere
    spheres[2] = vec4(      0,    0.5,    -1.5,    0.3);//big spheres in the center 
	//five big spheres that simulate the five walls 
    spheres[3] = vec4( 1001.0,       0,       0, 1000.0); 
    spheres[4] = vec4(-1001.0,       0,       0, 1000.0);
    spheres[5] = vec4(      0,  1001.0,       0, 1000.0); 
    spheres[6] = vec4(      0, -1001.0,       0, 1000.0);
    spheres[7] = vec4(      0,       0, -1002.0, 1000.0);

    //The diffuse color of the spheres 
    colors[0] = vec4(1.0, 0.8, 0.0,-1.0);
    colors[1] = vec4(0.0, 0.0, 1.0,-1.0);
    colors[2] = vec4(1.0, 1.0, 1.0, 1.0);    
    colors[3] = vec4(0.0, 1.0, 0.0, 0.7);
    colors[4] = vec4(1.0, 0.0, 0.0, 0.7);
    colors[5] = vec4(1.0, 1.0, 1.0, 0.7);
    colors[6] = vec4(1.0, 1.0, 1.0, 0.7);
    colors[7] = vec4(1.0, 1.0, 1.0, 0.7);

    //this is a hint if we want to make the object reflective (first id), refractive (second id) or both
    materials[0] = vec2 (0.0, 0.0);
    materials[1] = vec2 (0.0, 0.0);				
    materials[2] = vec2 (0.1, 0.8);	//not refraction here   
	//walls just have diffuse colors (no reflection or refraction. might chang this later) 
    materials[3] = vec2 (0.0, 0.0);				
    materials[4] = vec2 (0.0, 0.0);				
    materials[5] = vec2 (0.0, 0.0);				
    materials[6] = vec2 (0.0, 0.0);				
    materials[7] = vec2 (0.0, 0.0);	
	   

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

float calcInter(vec3 ro, vec3 rd, out vec4 ob, out vec4 col, out vec2 mat)
{
	//calculate the intersection witht the objects in the scene
	float tm = maxDist;
	float t;
	if(intersectSphere(ro,rd,spheres[0],tm,t)){ ob=spheres[0]; col=colors[0]; tm=t; mat= materials[0];}
	if(intersectSphere(ro,rd,spheres[1],tm,t)){ ob=spheres[1]; col=colors[1]; tm=t; mat= materials[1];}
	if(intersectSphere(ro,rd,spheres[2],tm,t)){ ob=spheres[2]; col=colors[2]; tm=t; mat= materials[2];}
	if(intersectSphere(ro,rd,spheres[3],tm,t)){ ob=spheres[3]; col=colors[3]; tm=t; mat= materials[3];}
	if(intersectSphere(ro,rd,spheres[4],tm,t)){ ob=spheres[4]; col=colors[4]; tm=t; mat= materials[4];}
	if(intersectSphere(ro,rd,spheres[5],tm,t)){ ob=spheres[5]; col=colors[5]; tm=t; mat= materials[5];}
	if(intersectSphere(ro,rd,spheres[6],tm,t)){ ob=spheres[6]; col=colors[6]; tm=t; mat= materials[6];}
	if(intersectSphere(ro,rd,spheres[7],tm,t)){ ob=spheres[7]; col=colors[7]; tm=t; mat= materials[7];}	
	
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
			color*=0.7;
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
		if(tm <maxDist){
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
	  
	gl_FragColor = vec4(color,1.0);	

};
