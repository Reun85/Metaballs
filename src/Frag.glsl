#version 430

// Camera 
// ---------------------------------
uniform vec3 eye;
uniform vec3 at;
uniform vec3 up;
uniform float aspect = 1;
uniform float fov = radians(27.0);
// ---------------------------------


// Balls
// -----------------------------------------------------------------------------------------------
const uint BALLS_MAX = 10;
uniform vec4[BALLS_MAX] balls;
uniform vec3 balls_ambient = vec3(1.0);
uniform vec3 balls_diffuse= vec3(1.0,1.0,1.0);
uniform vec3 balls_specular= vec3(1.0,1.0,1.0);
uniform float shininess = 0.5;
uniform int ball_count;
// -----------------------------------------------------------------------------------------------


// LIGHTING
// -----------------------------------------------------------------------------------------------
// Max 10 lightsources.
const uint LIGHTS_MAX = 3;
uniform vec4[LIGHTS_MAX] lightsources_pos = vec4[3](vec4(0.0,1.0,0.0,0.0),vec4(10,0.0,0.0,1.0),vec4(-10,-10.0,0.0,1.0));
uniform vec3 lightsource_ambient= vec3(0.125);
uniform vec3[LIGHTS_MAX] lightsources_diffuse= vec3[3](vec3(1.0,1.0,1.0),vec3(1.0,1.0,1.0),vec3(1.0,1.0,1.0));
uniform vec3[LIGHTS_MAX] lightsources_specular= vec3[3](vec3(1.0,0.0,0.0),vec3(0.0,1.0,0.0),vec3(0.0,0.0,1.0));
uniform int lightsources_count = 3;

uniform float lightConstantAttenuation    = 1.0;
uniform float lightLinearAttenuation      = 0.5;
uniform float lightQuadraticAttenuation   = 0.2;
// -----------------------------------------------------------------------------------------------

// skybox
uniform samplerCube texImage;

// konstans
uniform float tr = 0.5;
#define PI 3.14159265359


// From Pipeline
// -----------------------------------------------------------------------------------------------
// bejövő érték
in vec2 vs_out_normPos;

// kimenő érték - a fragment színe
out vec4 fs_out_col;
// -----------------------------------------------------------------------------------------------

// f function
// -----------------------------------------------------------------------------------------------


float finner(vec3 inp, vec3 c,float r){
// r>0
	float d = length(c-inp);
	if (d < 0) {
		return 1;
	}
	else if (d> r){
		return 0;
	}
	d = d/r;
	float d2 = d*d;
	float d3 = d2*d;
	return 2*d3 - 3*d2 + 1;
}

float F(vec3 inp){
	float ret = 0;
	for(int i = 0; i < ball_count;i++){
		ret+=finner(inp,balls[i].xyz,balls[i].w);
	}
	return ret;
}

// -----------------------------------------------------------------------------------------------

// normal calculation
// -----------------------------------------------------------------------------------------------



vec3 numeraticnormal(vec3 inp){
	float eps = 0.0001;
	float dx = F(inp+vec3(eps,0,0))-F(inp-vec3(eps,0,0));
	float dy = F(inp+vec3(0,eps,0))-F(inp-vec3(0,eps,0));
	float dz = F(inp+vec3(0,0,eps))-F(inp-vec3(0,0,eps));
	// osztunk 2*epszilonnal és utána normalizálunk, felesleges az osztás művelet.
	return normalize(vec3(dx,dy,dz));
}
vec3 normal(vec3 inp, int i1, int i2){
	// return correctnormal(inp,i1,i2);
	return -numeraticnormal(inp);
}

// normal
// -----------------------------------------------------------------------------------------------


// finding intersection
// -----------------------------------------------------------------------------------------------

// Keeps track of starting f values instead of the sign of pairs.
float findintersectionweirdlowermem(vec3 startingpos, vec3 dir,out int ball1, out int ball2){
	float dt = 0.1; // We use logarithmic search at the end.
	int maxstepcount = 30;
	int logarithmicstepcount = 10;


	float t = dt;
	vec3 p = startingpos;
	bool cont = true;
	bool startingispos = F(p) >tr;
	int i = 0;
	while(i < maxstepcount && cont){
		p = startingpos + dir*t;
		float f1 = F(p)-tr;
		if (f1 > 0 != startingispos)
			cont = false;
		t += dt;
		i++;
	}
	if(cont){
		ball1 = -1;
		ball2 = -1;
		return -1;
	}
	ball1 = 1;
	ball2 = 1;
	t = t-dt;
	float tprev = t-dt;
	float mid;
	
	for(int k = 0; k < logarithmicstepcount; k++){
		mid = (tprev+t)*0.5;
		p = startingpos + dir*mid;
		float f1 = F(p)-tr;
		if((f1 > 0) != startingispos){
			t = mid;
		}
		else{
			tprev =mid;
		}
	}
	mid = (tprev+t);
	return  mid*0.5;
}

// Return the distance of the intersection point from the starting point. (according to dir)
// If there is no intersection, return -1.
// If there is more than one intersection, return the smallest one.
// Returns the indexes of the balls in ball1 and ball2
float findintersection(vec3 pos, vec3 dir, out int ball1, out int ball2){
	return findintersectionweirdlowermem(pos,dir,ball1,ball2);
}

// find intersection
// -----------------------------------------------------------------------------------------------

// TASK 1:
// Does not yet use clever findintersection optimisation
float grayscaleRayMarchNoCam(){
	// sugár kezdőpontja.
	vec3 pos = vec3(vs_out_normPos.x*2.0,3.0,vs_out_normPos.y*2.0);
	vec3 dir = vec3(0,-1,0);

	float smallest = 100000;

	int i,j;
	float t = findintersection(pos,dir,i,j);
	if(t > 0 && t < smallest){
		smallest = t;
	}
	return 1-smallest/10.0;
}
// Task 2:
// With camera :)
float grayscaleRayMarch(){

	vec3 u,v,w;
	w = normalize(eye-at);
	u = normalize(cross(up,w));
	v = normalize(cross(w,u));
	// vs_out_normPos tartománya [-1,1] így nem kell normalizálni ugyanerre a tartományra.
	// vec3 pos = eye + u*vs_out_normPos.x*aspect*tan(fov/2) + v*vs_out_normPos.y*tan(fov/2)-w;
	float i = vs_out_normPos.x*aspect*tan(fov/2.0);
	float j = vs_out_normPos.y*tan(fov/2.0);
	vec3 pos = eye + u*i+ v*j-w;
	vec3 dir = normalize(pos-eye);



	float smallest = 100000;
	int i1 = -1;
	int i2 = -1;
	float t = findintersection(pos,dir,i,j);
	if(t > 0 && t < smallest){
		smallest = t;
	}
	
	return 1-smallest/10.0;
}


// TASK 2.5:
vec4 RayMarchNormals(){

	vec3 u,v,w;
	w = normalize(eye-at);
	u = normalize(cross(up,w));
	v = normalize(cross(w,u));
	// vs_out_normPos tartománya [-1,1] így nem kell normalizálni ugyanerre a tartományra.
	float x = vs_out_normPos.x*aspect*tan(fov/2.0);
	float y = vs_out_normPos.y*tan(fov/2.0);
	vec3 pos = eye + u*x+ v*y-w;
	vec3 dir = normalize(pos-eye);

	
	int i1,i2;
	float t;
	t = findintersection(pos,dir,i1,i2);
	if(t<0){
		return vec4(0);
	}
	else{
		pos = pos + dir*t;
		vec3 n = normal(pos,i1,i2);
		return vec4(n*0.5+0.5,1.0);
		// return vec4(n,1.0);
	}

}

// TASK 3:
vec4 RayMarchNoLight(){
	vec3 u,v,w;
	w = normalize(eye-at);
	u = normalize(cross(up,w));
	v = normalize(cross(w,u));
	// vs_out_normPos tartománya [-1,1] így nem kell normalizálni ugyanerre a tartományra.
	float x = vs_out_normPos.x*aspect*tan(fov/2.0);
	float y = vs_out_normPos.y*tan(fov/2.0);
	vec3 pos = eye + u*x+ v*y-w;
	vec3 dir = normalize(pos-eye);
	
	int i1,i2;
	float t;
	vec3 n;
	int i = 0;
	bool cont = true;
	while(i<5 && cont){
		// t = findintersectionoptimized(pos,dir,i1,i2);
		t = findintersection(pos,dir,i1,i2);
		if(i1 == -1){
			cont = false;
		}
		else{
			pos = pos + dir*t;
			n = normal(pos,i1,i2);
			pos += n*0.1; // Make sure we don't hit the same object.
			dir = normalize(reflect(dir,n));
		}
		i++;
	}
	return texture(texImage, dir);
}

// WITH LIGTHNING

vec4 BlinnPhong(int lightindex, vec3 pos, vec3 dir, vec3 normal) {
	
	vec4 lightPos = lightsources_pos[lightindex];
	vec3 lightAmbient = lightsource_ambient;
	vec3 lightDiffuse = lightsources_diffuse[lightindex];
	vec3 lightSpecular = lightsources_specular[lightindex];
	vec3 lightDir;
	float LightDistance=0.0;
	
	if ( lightPos.w == 0.0 ) // irány fényforrás (directional light)
	{
		lightDir	= lightPos.xyz;
		lightDir = normalize(lightDir);
	}
	else // pont fényforrás (point light)
	{
		lightDir	= lightPos.xyz-pos;
		lightDir = normalize(lightDir);
		LightDistance = length(lightDir);
	}
	int i1,i2;

	vec3 ambient = balls_ambient * lightAmbient;

	if(findintersection(pos,lightDir,i1,i2) > 0){
		// Cannot see the lightsource.
		// Only return the ambient light.
		return vec4(ambient,1.0);
	}
	float attenuation = 1.0 / ( lightConstantAttenuation + lightLinearAttenuation * LightDistance + lightQuadraticAttenuation * LightDistance * LightDistance);

	// Diffuse
	float diffuseFactor = max(dot(lightDir,normal), 0.0) * attenuation;
	vec3 diffuse = diffuseFactor * lightDiffuse * balls_diffuse;

	// Blinn's change 
	vec3 H = normalize(lightDir - dir);
	float spec = pow(max(dot(normal, H), 0.0), 1.0/shininess)*attenuation;

	vec3 specular = spec * lightSpecular*balls_specular;

	return vec4( ambient+diffuse+specular, 1.0 );
}

vec4 RayMarch(){
	const int RECURSIVE_ITER_MAX = 29;

	vec3 u,v,w;
	w = normalize(eye-at);
	u = normalize(cross(up,w));
	v = normalize(cross(w,u));
	// vs_out_normPos tartománya [-1,1] így nem kell normalizálni ugyanerre a tartományra.
	float x = vs_out_normPos.x*aspect*tan(fov/2.0);
	float y = vs_out_normPos.y*tan(fov/2.0);
	vec3 pos = eye + u*x+ v*y-w;
	vec3 dir = normalize(pos-eye);

	
	int i1,i2;
	float t = findintersection(pos,dir,i1,i2);
	bool cont = true;
	if(i1 == -1){
		return texture(texImage, dir);
	}
	pos = pos + dir*t;
	vec3 n = normal(pos,i1,i2);
	pos += n*0.1; // Make sure we don't hit the same object.
	dir = normalize(reflect(dir,n));
	vec3 lighting = vec3(0);
	for(int i = 0; i < lightsources_count;i++){
		lighting += BlinnPhong(i,pos,dir,n).xyz;
	}
	int i = 0;
	while(i<RECURSIVE_ITER_MAX&& cont){
		t = findintersection(pos,dir,i1,i2);
		if(i1 == -1){
			cont = false;
		}
		else{
			pos = pos + dir*t;
			n = normal(pos,i1,i2);
			pos += n*0.1; // Make sure we don't hit the same object.
			dir = normalize(reflect(dir,n));
		}
		i++;
	}
	return vec4(lighting,1)*texture(texImage, dir);
}
vec4 RayMarchFullyRecursive(){
	const int RECURSIVE_ITER_MAX = 29;

	vec3 u,v,w;
	w = normalize(eye-at);
	u = normalize(cross(up,w));
	v = normalize(cross(w,u));
	// vs_out_normPos tartománya [-1,1] így nem kell normalizálni ugyanerre a tartományra.
	float x = vs_out_normPos.x*aspect*tan(fov/2.0);
	float y = vs_out_normPos.y*tan(fov/2.0);
	vec3 pos = eye + u*x+ v*y-w;
	vec3 dir = normalize(pos-eye);

	int i1,i2;
	float t = findintersection(pos,dir,i1,i2);
	bool cont = true;
	if(i1 == -1){
		return texture(texImage, dir);
	}
	pos = pos + dir*t;
	vec3 n = normal(pos,i1,i2);
	pos += n*0.1; // Make sure we don't hit the same object.
	vec3 lighting = vec3(0);
	for(int i = 0; i < lightsources_count;i++){
		lighting += BlinnPhong(i,pos,dir,n).xyz;
	}
	vec3 prevdir = dir;
	dir = normalize(reflect(dir,n));
	vec3 innerlight;
	vec3 newdir;
	int i = 0;
	while(i<RECURSIVE_ITER_MAX&& cont){
		t = findintersection(pos,dir,i1,i2);
		if(i1 == -1){
			cont = false;
		}
		else{
			pos = pos + dir*t;
			n = normal(pos,i1,i2);
			pos += n*0.1; // Make sure we don't hit the same object.
			innerlight = vec3(0);
			for(int j = 0; j < lightsources_count;j++){
				innerlight += BlinnPhong(j,pos,dir,n).xyz;
			}
			lighting+=innerlight*dot(dir,prevdir);
			prevdir = dir;
			dir = normalize(reflect(dir,n));
		}
		i++;
	}
	return vec4(lighting,1)*texture(texImage, dir);
	
}

void main()
{
	// Task 1 - No cam:
	// float ret = grayscaleRayMarchNoCam();
	// fs_out_col = vec4(ret,ret,ret,1.0);
	// Task 2 - With Cam:
	// float ret = grayscaleRayMarch();
	// fs_out_col = vec4(ret,ret,ret,1.0);



	// Task 2.5 - With Cam show normals:
	// fs_out_col =  RayMarchNormals();
	// Task 3 - Recursive with Cam and:
	 // fs_out_col = RayMarchNoLight();
	// Task 4 - Recursive reflection with Cam and light:
	// fs_out_col = RayMarch();
	// Task 5 - Recursive reflection with Cam and recursive light:
	 fs_out_col = RayMarchFullyRecursive();
	
}


// 1.56 az üveg étája.
