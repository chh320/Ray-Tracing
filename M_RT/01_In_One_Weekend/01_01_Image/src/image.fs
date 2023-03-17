#version 460 core
out vec4 FragColor;
in vec2 screenCoord;

uniform vec2 screenSize;

#define PI 3.1415926535

// random number
uint m_u = uint(521288629);
uint m_v = uint(362436069);

uint GetUintCore(inout uint u, inout uint v){
	v = uint(36969) * (v & uint(65535)) + (v >> 16);
	u = uint(18000) * (u & uint(65535)) + (u >> 16);
	return (v << 16) + u;
}

float GetUniformCore(inout uint u, inout uint v){
	uint z = GetUintCore(u, v);
	
	return float(z) / uint(4294967295);
}

float GetUniform(){
	return GetUniformCore(m_u, m_v);
}

uint GetUint(){
	return GetUintCore(m_u, m_v);
}

float rand(){
	return GetUniform();
}

vec2 rand2(){
	return vec2(rand(), rand());
}

vec3 rand3(){
	return vec3(rand(), rand(), rand());
}

vec4 rand4(){
	return vec4(rand(), rand(), rand(), rand());
}

vec3 random_in_unit_sphere(){
	vec3 p;

	float theta = rand() * 2 * PI;
	float phi = rand() * PI;
	p.x = sin(phi) * cos(theta);
	p.y = cos(phi);
	p.z = sin(phi) * sin(theta);
}

struct Ray{
	vec3 origin;
	vec3 direction;
};

Ray CreateRay(vec3 o, vec3 d){
	Ray ray;
	ray.origin = o;
	ray.direction = d;
	return ray;
}

vec3 GetRayLocation(Ray ray, float t){
	return ray.origin + ray.direction * t;
}

struct Camera{
	vec3 lower_left_corner;
	vec3 horizontal;
	vec3 verticle;
	vec3 origin;
};

uniform Camera camera;

Ray CameraGetRay(Camera camera, vec2 uv){
	Ray ray = CreateRay(camera.origin, camera.lower_left_corner + uv.x * camera.horizontal + uv.y * camera.verticle - camera.origin);
	return ray;
}

vec3 GammaCorrectiong(vec3 c){
	return pow(c, vec3( 1.0 / 2.2));
}

struct Sphere{
	vec3 center;
	float radius;
};

Sphere CreateSphere(vec3 center, float radius){
	Sphere sphere;
	sphere.center = center;
	sphere.radius = radius;
	return sphere;
}

struct World{
	int objectCount;
	Sphere objects[10];
};

World CreateWorld(){
	World world;
	world.objectCount = 2;
	world.objects[0] = CreateSphere(vec3(0.0, 0.0, -1.0), 0.5);
	world.objects[1] = CreateSphere(vec3(0.0, -100.5, -1.0), 100.0);

	return world;
}

struct HitRecord{
	float t;
	vec3 position;
	vec3 normal;
};

bool SphereHit(Sphere sphere, Ray ray, float t_min, float t_max, inout HitRecord hitRecord){
	vec3 oc = ray.origin - sphere.center;
	
	float a = dot(ray.direction, ray.direction);
	float b = 2 * dot(ray.direction, oc);
	float c = dot(oc, oc) - sphere.radius * sphere.radius;

	float delta = b * b - 4 * a * c;
	
	if(delta > 0.0){
		float temp = (-b - sqrt(delta)) / (2.0 * a);
		if(temp < t_max && temp > t_min){
			hitRecord.t = temp;
			hitRecord.position = GetRayLocation(ray, temp);
			hitRecord.normal = normalize(hitRecord.position - sphere.center);
			return true;
		}
		temp = (-b + sqrt(delta)) / (2.0 * a);
		if(temp < t_max && temp > t_min){
			hitRecord.t = temp;
			hitRecord.position = GetRayLocation(ray, temp);
			hitRecord.normal = normalize(hitRecord.position - sphere.center);
			return true;
		}
	}
	return false;
}

bool HitWorld(World world, Ray ray, float t_min, float t_max, inout HitRecord rec){
	HitRecord tempRec;
	bool isHitted = false;
	float cloestFar = t_max;
	for(int i = 0; i < world.objectCount; i++){
		if(SphereHit(world.objects[i], ray, t_min, cloestFar, tempRec)){
			rec = tempRec;
			isHitted = true;
			cloestFar = rec.t;
		}
	}
	return isHitted;
}

vec3 RayTrace(Ray ray){
	World world = CreateWorld();
	HitRecord hitRecord;
	if(HitWorld(world, ray, 0.001, 1000000.0, hitRecord)){
		return 0.5 * vec3(hitRecord.normal.x + 1.0, hitRecord.normal.y + 1.0, hitRecord.normal.z+ 1.0);
	}
	vec3  normalizeDir = normalize(ray.direction);
	float t = (normalizeDir.y + 1.0) * 0.5;
	return (1 - t) * vec3(1.0, 1.0, 1.0) + t * vec3(0.5, 0.7, 1.0);
}

void main(){
	vec3 color = vec3(0.0, 0.0, 0.0);
	int sampleCount = 100;
	for(int i = 0; i < sampleCount; i++){
		Ray ray = CameraGetRay(camera, screenCoord + rand2() / screenSize);
		color += RayTrace(ray);
	}
	color /= sampleCount;
	color = GammaCorrectiong(color);

	FragColor = vec4(color, 1.0);
}