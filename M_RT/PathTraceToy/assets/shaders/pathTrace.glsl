#version 330 core
in vec2 TexCoord;

out vec4 FragColor;

// uniform --------------------------------------------------------------

uniform int width;
uniform int height;
uniform int nTriangles;
uniform int frameCounter;

uniform samplerBuffer triangleTex;
uniform samplerBuffer nodesTex;

uniform sampler2D accumTex;

uniform vec3 cameraPos;
uniform mat4 cameraRotate;

// global ---------------------------------------------------------------

#define PI         3.14159265358979323
#define INV_PI     0.31830988618379067
#define TWO_PI     6.28318530717958648
#define INV_TWO_PI 0.15915494309189533

#define EPS 0.0003
#define INF 1000000.0

#define TRIANGLE_SIZE 12
#define BVHNODE_SIZE 4

// struct ---------------------------------------------------------------

struct Triangle{
    vec3 p1, p2, p3;
    vec3 n1, n2, n3;
};

struct BVHNode{
    int left, right;
    int n, index;  
    vec3 pMin, pMax;
};

struct Ray{
	vec3 origin;
	vec3 direction;
};

// 物体表面材质定义
struct Material {
    vec3 emissive;          // 作为光源时的发光颜色
    vec3 baseColor;
    float subsurface;
    float metallic;
    float specular;
    float specularTint;
    float roughness;
    float anisotropic;
    float sheen;
    float sheenTint;
    float clearcoat;
    float clearcoatGloss;
    float IOR;
    float transmission;
};

// 光线求交结果
struct HitResult {
    bool isHit;             // 是否命中
    bool isInside;          // 是否从内部命中
    float t;         // 与交点的距离
    vec3 hitPoint;          // 光线命中点
    vec3 normal;            // 命中点法线
    vec3 viewDir;           // 击中该点的光线的方向
    Material material;      // 命中点的表面材质
};

// random --------------------------------------------------------------

/*
 * 生成随机向量，依赖于 frameCounter 帧计数器
 * 代码来源：https://blog.demofox.org/2020/05/25/casual-shadertoy-path-tracing-1-basic-camera-diffuse-emissive/
*/

uint seed = uint(uint((TexCoord.x) * width) * uint(1973) +
    uint((TexCoord.y) * height) * uint(9277) +
    uint(frameCounter) * uint(26699)) | uint(1);

uint wang_hash(inout uint seed) {
    seed = uint(seed ^ uint(61)) ^ uint(seed >> uint(16));
    seed *= uint(9);
    seed = seed ^ (seed >> 4);
    seed *= uint(0x27d4eb2d);
    seed = seed ^ (seed >> 15);
    return seed;
}

float rand() {
    return float(wang_hash(seed)) / 4294967296.0;
}

// get data -------------------------------------------------------------

Triangle getTriangle(int i){
    Triangle t;
    int offset = i * TRIANGLE_SIZE;
    // point
    t.p1 = texelFetch(triangleTex, offset + 0).xyz;
    t.p2 = texelFetch(triangleTex, offset + 1).xyz;
    t.p3 = texelFetch(triangleTex, offset + 2).xyz;

    // normal
    t.n1 = texelFetch(triangleTex, offset + 3).xyz;
    t.n2 = texelFetch(triangleTex, offset + 4).xyz;
    t.n3 = texelFetch(triangleTex, offset + 5).xyz;

    return t;
}

Material getMaterial(int i) {
    Material m;

    int offset = i * TRIANGLE_SIZE;
    vec3 param1 = texelFetch(triangleTex, offset + 8).xyz;
    vec3 param2 = texelFetch(triangleTex, offset + 9).xyz;
    vec3 param3 = texelFetch(triangleTex, offset + 10).xyz;
    vec3 param4 = texelFetch(triangleTex, offset + 11).xyz;

    m.emissive  = texelFetch(triangleTex, offset + 6).xyz;
    m.baseColor = texelFetch(triangleTex, offset + 7).xyz;

    m.subsurface = param1.x;
    m.metallic   = param1.y;
    m.specular   = param1.z;

    m.specularTint = param2.x;
    m.roughness    = param2.y;
    m.anisotropic  = param2.z;

    m.sheen     = param3.x;
    m.sheenTint = param3.y;
    m.clearcoat = param3.z;

    m.clearcoatGloss = param4.x;
    m.IOR            = param4.y;
    m.transmission   = param4.z;

    return m;
}

BVHNode getBVHNode(int i) {
    BVHNode node;
    int offset = i * BVHNODE_SIZE;

    ivec3 childs   = ivec3(texelFetch(nodesTex, offset + 0).xyz);
    ivec3 leafInfo = ivec3(texelFetch(nodesTex, offset + 1).xyz);
    node.left  = int(childs.x);
    node.right = int(childs.y);
    node.n     = int(leafInfo.x);
    node.index = int(leafInfo.y);

    node.pMin = texelFetch(nodesTex, offset + 2).xyz;
    node.pMax = texelFetch(nodesTex, offset + 3).xyz;

    return node;
}

// sample -----------------------------------------------------------------

vec3 SampleHemisphere(){
    float z = rand();
    float r = sqrt(max(0.0, 1.0 - z * z));
    float phi = TWO_PI * rand();
    return vec3(r * cos(phi), r * sin(phi), z);
}

vec3 toNormalHemisphere(vec3 v, vec3 N) {
    vec3 helper = vec3(1, 0, 0);
    if(abs(N.x) > 0.999)
        helper = vec3(0, 0, 1);
    vec3 tangent = normalize(cross(N, helper));
    vec3 bitangent = normalize(cross(N, tangent));
    return v.x * tangent + v.y * bitangent + v.z * N;
}

// intersect -----------------------------------------------------------------

HitResult hitTriangle(Triangle triangle, Ray ray) {
    HitResult res;
    res.t = INF;
    res.isHit = false;
    res.isInside = false;

    vec3 p1 = triangle.p1;
    vec3 p2 = triangle.p2;
    vec3 p3 = triangle.p3;

    vec3 S = ray.origin;    // 射线起点
    vec3 d = ray.direction;     // 射线方向
    vec3 N = normalize(cross(p2 - p1, p3 - p1));    // 法向量

    // 从三角形背后（模型内部）击中
    if(dot(N, d) > 0.0f) {
        N = -N;
        res.isInside = true;
    }

    // 如果视线和三角形平行
    if(abs(dot(N, d)) < 0.00001f)
        return res;

    // 距离
    float t = (dot(N, p1) - dot(S, N)) / dot(d, N);
    if(t < 0.0005f)
        return res;    // 如果三角形在光线背面

    // 交点计算
    vec3 P = S + d * t;

    // 判断交点是否在三角形中
    vec3 c1 = cross(p2 - p1, P - p1);
    vec3 c2 = cross(p3 - p2, P - p2);
    vec3 c3 = cross(p1 - p3, P - p3);
    bool r1 = (dot(c1, N) > 0 && dot(c2, N) > 0 && dot(c3, N) > 0);
    bool r2 = (dot(c1, N) < 0 && dot(c2, N) < 0 && dot(c3, N) < 0);

    // 命中，封装返回结果
    if(r1 || r2) {
        res.isHit = true;
        res.hitPoint = P;
        res.t = t;
        res.normal = N;
        res.viewDir = d;
        // 根据交点位置插值顶点法线
        float alpha = (-(P.x - p2.x) * (p3.y - p2.y) + (P.y - p2.y) * (p3.x - p2.x)) / (-(p1.x - p2.x - 0.00005) * (p3.y - p2.y + 0.00005) + (p1.y - p2.y + 0.00005) * (p3.x - p2.x + 0.00005));
        float beta = (-(P.x - p3.x) * (p1.y - p3.y) + (P.y - p3.y) * (p1.x - p3.x)) / (-(p2.x - p3.x - 0.00005) * (p1.y - p3.y + 0.00005) + (p2.y - p3.y + 0.00005) * (p1.x - p3.x + 0.00005));
        float gama = 1.0 - alpha - beta;
        vec3 Nsmooth = alpha * triangle.n1 + beta * triangle.n2 + gama * triangle.n3;
        Nsmooth = normalize(Nsmooth);
        res.normal = (res.isInside) ? (-Nsmooth) : (Nsmooth);
    }

    return res;
}
/*
HitResult hitTriangle(Triangle triangle, Ray ray){
    HitResult ret;
    ret.t = INF;
    ret.isHit = false;

    vec3 p1 = triangle.p1;
    vec3 p2 = triangle.p2;
    vec3 p3 = triangle.p3;

    vec3 O = ray.origin;    
    vec3 D = ray.direction;     
    vec3 N = normalize(cross(p2 - p1, p3 - p1)); 

    vec3 E1 = p2 - p1;
    vec3 E2 = p3 - p1;
    vec3 S = O - p1;
    vec3 S1 = cross(D, E2);
    vec3 S2 = cross(S, E1);
    float inv = dot(S1, E1);

    float t = dot(S2, E2) / inv;
    float belta = dot(S1, S) / inv;
    float gamma = dot(S2, D) / inv;
    float alpha = 1.0 - belta - gamma;

    if(belta > 0.0 && gamma > 0.0 && alpha > 0.0 && t < ret.t){
        ret.t = t;
        ret.isHit = true;
        ret.viewDir = D;
        ret.hitPoint = O + D * t;
        ret.normal = alpha * triangle.n1 + belta * triangle.n2 + gamma * triangle.n3;
    }
    return ret;
}*/

float hitAABB(Ray r, vec3 pMin, vec3 pMax){
    vec3 inv = vec3(1.0 / r.direction.x, 1.0 / r.direction.y, 1.0 / r.direction.z);

    vec3 far  = (pMax - r.origin) * inv;
    vec3 near = (pMin - r.origin) * inv;

    vec3 tmax = max(far, near);
    vec3 tmin = min(far, near);

    float t0 = max(max(tmin.x, tmin.y), tmin.z);
    float t1 = min(min(tmax.x, tmax.y), tmax.z);

    return (t1 >= t0) ? ((t0 > 0.0) ? (t0) : (t1)) : (-1);
}

// hit ---------------------------------------------------------------------------------

HitResult hitArray(Ray ray, int left, int right){
    HitResult ret;
    ret.isHit = false;
    ret.t = INF;
    for(int i = left; i <= right; i++) {
        Triangle triangle = getTriangle(i);
        HitResult r = hitTriangle(triangle, ray);
        if(r.isHit && r.t < ret.t) {
            ret = r;
            ret.material = getMaterial(i);
        }
    }
    return ret;
}

HitResult hitBVH(Ray ray){
    HitResult ret;
    ret.isHit = false;
    ret.t = INF;

    int stack[256];
    int sp = 0;

    stack[sp++] = 0;
    while(sp > 0){
        int top = stack[--sp];
        BVHNode node = getBVHNode(top);

        // leaf node
        if(node.n > 0){
            int left = node.index;
            int right = node.index + node.n - 1;
            HitResult tmp = hitArray(ray, left, right);
            if(tmp.isHit && tmp.t < ret.t){
                ret = tmp;
            }
            continue;
        }
        
        float t1 = INF;
        float t2 = INF;
        if(node.left > 0){
            BVHNode node = getBVHNode(node.left);
            t1 = hitAABB(ray, node.pMin, node.pMax);
        }
        if(node.right > 0){
            BVHNode node = getBVHNode(node.right);
            t2 = hitAABB(ray, node.pMin, node.pMax);
        }

        if(t1 > 0 && t2 > 0){
            if(t1 < t2){
                stack[sp++] = node.right;
                stack[sp++] = node.left;
            }else{
                stack[sp++] = node.left;
                stack[sp++] = node.right;
            }
        } else if(t1 > 0) {
            stack[sp++] = node.left;
        } else if(t2 > 0) {
            stack[sp++] = node.right;
        }
    }

    return ret;
}

// pathTrace ---------------------------------------------------------------------------

vec3 pathTracing(HitResult hit, int maxBounce) {

    vec3 Lo = vec3(0);      // 最终的颜色
    vec3 history = vec3(1); // 递归积累的颜色

    for(int bounce = 0; bounce < maxBounce; bounce++) {
        // 随机出射方向 wi
        vec3 wi = toNormalHemisphere(SampleHemisphere(), hit.normal);

        // 漫反射: 随机发射光线
        Ray randomRay;
        randomRay.origin = hit.hitPoint;
        randomRay.direction = wi;
        HitResult newHit = hitBVH(randomRay);

        float pdf = 1.0 / (2.0 * PI);                                   // 半球均匀采样概率密度
        float cosine_o = max(0, dot(-hit.viewDir, hit.normal));         // 入射光和法线夹角余弦
        float cosine_i = max(0, dot(randomRay.direction, hit.normal));  // 出射光和法线夹角余弦
        vec3 f_r = hit.material.baseColor / PI;                         // 漫反射 BRDF

        // 未命中
        if(!newHit.isHit) {
            // vec3 skyColor = sampleHdr(randomRay.direction);
            // Lo += history * skyColor * f_r * cosine_i / pdf;
            break;
        }

        // 命中光源积累颜色
        vec3 Le = newHit.material.emissive;
        Lo += history * Le * f_r * cosine_i / pdf;

        // 递归(步进)
        hit = newHit;
        history *= f_r * cosine_i / pdf;  // 累积颜色
    }

    return Lo;
}


void main()
{	
	Ray ray;

    ray.origin = cameraPos;
    vec2 AA = vec2((rand() - 0.5) / float(width), (rand() - 0.5) / float(height));
    vec4 dir = vec4((TexCoord * 2.0 - 1.0).xy + AA, -1.0, 0.0);
    ray.direction = normalize(dir.xyz);

    HitResult firstHit = hitBVH(ray);
    vec3 color;

    if(!firstHit.isHit){
        color = vec3(0);
    } else {
        vec3 Le = firstHit.material.emissive;
        vec3 Li = pathTracing(firstHit, 10);
        color = Le + Li;
    }

    vec3 accum = texture2D(accumTex, TexCoord).rgb;
    color = mix(accum, color, 1.0 / float(frameCounter + 1));
    //color = color + accum;
    FragColor = vec4(color, 1.0);
}