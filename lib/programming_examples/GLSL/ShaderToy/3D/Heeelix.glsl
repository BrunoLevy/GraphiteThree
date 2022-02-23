//stage GL_FRAGMENT_SHADER
//import <GLUP/ShaderToy.h>

//#define ANOTHER_LEVEL


/* SHADERTOY FROM HERE */

float guiLead = 0.6;
float guiInnerRatio = 0.4407892623709694;
float guiFocal = 3.;
float guiRotateModel = 0.4560833039600971;
float guiDebug = 0.6749066960348409;
float guiZipOffset = 30.;
float guiZipSize = 60.;
float guiZipSpeed = 3.3;
float guiZoom = 0.1;
float guiModelScale = 7.749066960348409;

mat4 cameraMatrix = mat4(
    -0.7063226699829102,
    0.7052236199378967,
    0.06198469549417496,
    0,
    -0.30620118975639343,
    -0.3832840919494629,
    0.8714323043823242,
    0,
    0.6382971405982971,
    0.5965006947517395,
    0.48660656809806824,
    0,
    0.14653973281383514,
    0.6211488246917725,
    0.13233166933059692,
    1
);

vec3 camPosition = vec3(0.14653973281383514, 0.6211488246917725, 0.13233166933059692);

float time;

#define PI 3.14159265359
#define HALF_PI 1.5707963267948966
#define TAU 6.28318530718
#define PHI 1.618033988749895


// --------------------------------------------------------
// Utils
// --------------------------------------------------------

#define saturate(x) clamp(x, 0., 1.)

// Rotate around a coordinate axis (i.e. in a plane perpendicular to that axis) by angle <a>.
// Read like this: R(p.xz, a) rotates "x towards z".
// This is fast if <a> is a compile-time constant and slower (but still practical) if not.
void pR(inout vec2 p, float a) {
    p = cos(a)*p + sin(a)*vec2(p.y, -p.x);
}

// http://www.neilmendoza.com/glsl-rotation-about-an-arbitrary-axis/
mat3 rotationMatrix(vec3 axis, float angle)
{
    axis = normalize(axis);
    float s = sin(angle);
    float c = cos(angle);
    float oc = 1.0 - c;
    
    return mat3(
        oc * axis.x * axis.x + c,           oc * axis.x * axis.y - axis.z * s,  oc * axis.z * axis.x + axis.y * s,
        oc * axis.x * axis.y + axis.z * s,  oc * axis.y * axis.y + c,           oc * axis.y * axis.z - axis.x * s,
        oc * axis.z * axis.x - axis.y * s,  oc * axis.y * axis.z + axis.x * s,  oc * axis.z * axis.z + c
    );
}

float range(float vmin, float vmax, float value) {
  return (value - vmin) / (vmax - vmin);
}

float rangec(float a, float b, float t) {
    return clamp(range(a, b, t), 0., 1.);
}

float vmax(vec2 v) {
    return max(v.x, v.y);
}

float fBox2(vec2 p, vec2 b) {
    vec2 d = abs(p) - b;
    return length(max(d, vec2(0))) + vmax(min(d, vec2(0)));
}

// Repeat space along one axis. Use like this to repeat along the x axis:
// <float cell = pMod1(p.x,5);> - using the return value is optional.
float pMod1(inout float p, float size) {
    float halfsize = size*0.5;
    float c = floor((p + halfsize)/size);
    p = mod(p + halfsize, size) - halfsize;
    return c;
}

vec3 cartToPolar(vec3 p) {
    float x = p.x; // distance from the plane it lies on
    float a = atan(p.y, p.z); // angle around center
    float r = length(p.zy); // distance from center
    return vec3(x, a, r);
}

vec3 polarToCart(vec3 p) {
    return vec3(
        p.x,
        sin(p.y) * p.z,
        cos(p.y) * p.z
    );
}

vec2 closestPointOnLine(vec2 line, vec2 point){
    line = normalize(line);
    float d = dot(point, line);
    return line * d;
}

// Closest of two points
vec3 closestPoint(vec3 pos, vec3 p1, vec3 p2) {
    if (length(pos - p1) < length(pos - p2)) {
        return p1;
    } else {
        return p2;
    }
}

// --------------------------------------------------------
// Helix
// https://www.shadertoy.com/view/MstcWs
// --------------------------------------------------------

// Closest point on a helix for one revolution
vec3 closestHelixSection(vec3 p, float lead, float radius) {

    p = cartToPolar(p);
    p.y *= radius;

    vec2 line = vec2(lead, radius * PI * 2.);
    vec2 closest = closestPointOnLine(line, p.xy);

    closest.y /= radius;
    vec3 closestCart = polarToCart(vec3(closest, radius));

    return closestCart;
}

// Closest point on a helix for infinite revolutions
vec3 closestHelix(vec3 p, float lead, float radius) {
    float c = pMod1(p.x, lead);
    vec3 offset = vec3(lead, 0, 0);
    vec3 A = closestHelixSection(p, lead, radius);
    vec3 B = closestHelixSection(p + offset, lead, radius) - offset;
    vec3 C = closestHelixSection(p - offset, lead, radius) + offset;
    vec3 closest = closestPoint(p, A, closestPoint(p, B, C));
    closest += offset * c;
    return closest;
}

// Cartesian to helix coordinates
void pModHelix(inout vec3 p, float lead, float radius) {
    vec3 closest = closestHelix(p, lead, radius);
    float helixAngle = atan((2. * PI * radius) / lead);
    vec3 normal = normalize(closest - vec3(closest.x,0,0));
    vec3 tangent = vec3(1,0,0) * rotationMatrix(normal, helixAngle);
    float x = (closest.x / lead) * radius * PI * 2.;
    float y = dot(p - closest, cross(tangent, normal));
    float z = dot(p - closest, normal);
    p = vec3(x, y, z);
}

float pModHelixScale(inout vec3 p, float lead, float innerRatio) {
    float radius = mix(.25, .5, innerRatio);
    pModHelix(p, lead, radius);
    float scale = mix(.5, 0., innerRatio);
    p /= scale;
    return 1. / scale;
}

float pModHelixUnwrap(inout vec3 p, float lead, float innerRatio, float t) {
    float radius = mix(.25, .5, innerRatio);
    float width = cos(asin(t));
    float adjust = (1. / width);
    float offset = ((.5 * adjust) - .5) * 7.;

    vec3 pp = p;
    pp.z -= radius;
    pR(pp.xy, PI * -.5);
    pp.x *= -1.;

    p.z += offset;
    radius += offset;
    pModHelix(p, lead, radius);

    p = mix(p, pp, rangec(.8, 1., t));

    float scale = mix(.5, 0., innerRatio);
    p /= scale;
    return 1. / scale;
}


// --------------------------------------------------------
// Modelling
// --------------------------------------------------------

struct Model {
    float dist;
    vec3 albedo;
    int id;
};

float anim(float t, float index) {
    float overlap = .5;
    float steps = 2.;
    float all = mix(steps, 1., overlap);
    float width = 1. / (all - 1.);
    float each = width * (1.- overlap);
    float start = index * each - width * .5;
    float end = start + width;
    return range(start, end, t);
}

float unzip(vec3 p, float t) {
    float size = guiZipSize;
    float speed = guiZipSpeed;

    t *= size * speed;

    if (sign(p.y) != sign(p.x)) {
        float radius = mix(.25, .5, guiInnerRatio);
        float scale = mix(.5, 0., guiInnerRatio);
        float factor = radius / scale * PI * 2.;
        t -= (factor - .5);
    }

    return range(size, 0., abs(p.x) + size - t);
}

void addPipe(inout float d, vec3 p, float scale, float tt) {

    float t = clamp(tt, 0., 1.);

    float boundry = 1.;
    float part;
    float separate = (
        rangec(0., boundry * .01, t) * .3 +
        rangec(boundry * .01, boundry, t) * .7
    );

    float round = rangec(.0, 1., t);

    part = fBox2(p.yz, vec2(mix(guiLead * 2., .5, separate), .5));
    part = mix(part, length(p.yz) - .5, round);
    part /= scale;

    d = mix(d, part, smoothstep(.0, .01, t));
}

void unzipHelixModel(inout float d, inout float scale, inout vec3 p, float lead, float innerRatio, float step, float invert) {
    float offset = guiZipOffset / lead;
    scale *= pModHelixScale(p, lead, innerRatio);
    p.x *= -1.;
    float t1 = unzip(p + vec3(offset,0,0) * invert, anim(time, step));
    addPipe(d, p, scale, t1);
}

Model map(vec3 p) {

    float part, d, t1, t2, t3, t4;
    float lead = guiLead;
    float innerRatio = guiInnerRatio;
    vec2 uv1, uv2, uv3;

    p /= guiModelScale;

    vec3 pp = p;

    d = 1e12;

    float s = mix(.5, 0., innerRatio);

    float scale = 1./pow(1./s, time);

    pR(p.xy, PI * -.5 * time + guiRotateModel * PI * 2.);
    
    p *= scale;
    p.z += .5;

    scale *= pModHelixUnwrap(p, lead, innerRatio, time);
    p.x *= -1.;
    scale *= pModHelixScale(p, lead, innerRatio);
    p.x *= -1.;

    #ifdef ANOTHER_LEVEL
        scale *= pModHelixScale(p, lead, innerRatio);
        p.x *= -1.;
    #endif

    d = min(d, length(p.yz) - .5);
    d /= scale;

    unzipHelixModel(d, scale, p, lead, innerRatio, -1., 1.);
    unzipHelixModel(d, scale, p, lead, innerRatio, 0., -1.);
    unzipHelixModel(d, scale, p, lead, innerRatio, 1., 1.);

    d *= guiModelScale;

    return Model(d, vec3(0), 1);
}


// --------------------------------------------------------
// Ray Marching
// Adapted from: https://www.shadertoy.com/view/Xl2XWt
// --------------------------------------------------------

const float MAX_TRACE_DISTANCE = 1.5; // max trace distance
const float INTERSECTION_PRECISION = .001; // precision of the intersection
const int NUM_OF_TRACE_STEPS = 100;
const float FUDGE_FACTOR = 1.; // Default is 1, reduce to fix overshoots

struct CastRay {
    vec3 origin;
    vec3 direction;
};

struct Ray {
    vec3 origin;
    vec3 direction;
    float len;
};

struct Hit {
    Ray ray;
    Model model;
    vec3 pos;
    bool isBackground;
    vec3 normal;
    bool isOutline;
};

// Faster runtime
vec3 _calcNormal(vec3 pos){
    vec3 eps = vec3(.0001,0,0);
    vec3 nor = vec3(
        map(pos+eps.xyy).dist - map(pos-eps.xyy).dist,
        map(pos+eps.yxy).dist - map(pos-eps.yxy).dist,
        map(pos+eps.yyx).dist - map(pos-eps.yyx).dist );
    return normalize(nor);
}

// Faster compilation
const int NORMAL_STEPS = 6;
vec3 calcNormal(vec3 pos){
    vec3 eps = vec3(.001,0,0);
    vec3 nor = vec3(0);
    float invert = 1.;
    for (int i = 0; i < NORMAL_STEPS; i++){
        nor += map(pos + eps * invert).dist * eps * invert;
        eps = eps.zxy;
        invert *= -1.;
    }
    return normalize(nor);
}


Hit raymarch(CastRay castRay){

    float currentDist = INTERSECTION_PRECISION * 2.0;
    float outlineDist = INTERSECTION_PRECISION * 2.0;
    Model model;

    float outline = .0023;
    bool isOutline = false;
    bool miss = false;

    float lastDist = currentDist;

    Ray ray = Ray(castRay.origin, castRay.direction, 0.);

    for( int i=0; i< NUM_OF_TRACE_STEPS ; i++ ){
        if (currentDist < INTERSECTION_PRECISION || ray.len > MAX_TRACE_DISTANCE) {
            break;
        }
        model = map(ray.origin + ray.direction * ray.len);
        lastDist = currentDist;
        currentDist = model.dist;
        miss = currentDist > lastDist;
        outlineDist = currentDist * -1. + outline;
        isOutline = outlineDist > .0 && outlineDist < currentDist && miss;
        if (isOutline) {
            currentDist = outlineDist;
        }
        ray.len += currentDist * FUDGE_FACTOR;
    }

    bool isBackground = false;
    vec3 pos = vec3(0);
    vec3 normal = vec3(0);

    if (ray.len > MAX_TRACE_DISTANCE) {
        isBackground = true;
    } else {
        pos = ray.origin + ray.direction * ray.len;
        normal = calcNormal(pos);
    }

    return Hit(ray, model, pos, isBackground, normal, isOutline);
}


// --------------------------------------------------------
// Rendering
// --------------------------------------------------------

float softshadow( in vec3 ro, in vec3 rd, in float mint, in float tmax )
{
    float res = 1.0;
    float t = mint;
    float ph = 1e10;
    
    for( int i=0; i<32; i++ )
    {
        float h = map( ro + rd*t ).dist;
        res = min( res, 10.0*h/t );
        t += h;
        if( res<0.0001 || t>tmax ) break;
        
    }
    return clamp( res, 0.0, 1.0 );
}


float calcAO( in vec3 pos, in vec3 nor )
{
    float occ = 0.0;
    float sca = 1.0;
    for( int i=0; i<5; i++ )
    {
        float hr = 0.01 + 0.12*float(i)/4.0;
        vec3 aopos =  nor * hr + pos;
        float dd = map( aopos ).dist;
        occ += -(dd-hr)*sca;
        sca *= 0.95;
    }
    return clamp( 1.0 - 3.0*occ, 0.0, 1.0 );    
}


vec3 doLighting(vec3 pos, vec3 nor, vec3 ref, vec3 rd) {

    vec3 col;
    vec3 up = normalize(vec3(1));

    // lighitng        
    float occ = mix(calcAO( pos, nor ), 1., .8);
    vec3  lig = normalize(vec3(0,.2,1));
    float amb = clamp(dot(nor, up) * .5 + .5, 0., 1.);
    float dif = clamp( dot( nor, lig ), 0.0, 1.0 );
    float fre = pow( clamp(1.0+dot(nor,rd),0.0,1.0), 2.0 );
    vec3  hal = normalize( lig-rd );
    float spe = pow(clamp( dot( nor, hal ), 0.0, 1.0 ),16.0);

    vec3 cA = vec3(.7,.3,.9);
    vec3 cB = vec3(.4,.9,.8);
    vec3 cC = vec3(.7,0,.7);

    col = mix(cA, cB, rangec(.0, 1., dot(-rd, nor))); // need better ramp
    col = mix(col, vec3(.8,.5,1), rangec(.5, 1., dif) * .5);
    col += cC * rangec(.5, 1., dif) * .1;

    dif *= softshadow( pos, lig, 0.02, 2.5 ) * .9;

    vec3 lin = vec3(0);
    lin += .5 * dif;
    lin += .1 * spe * dif;
    lin += .2 * fre * occ;
    lin += .5 * amb * occ;
    lin += .4 * occ;
    col = col*lin;

    return col;
}

void render(inout vec3 color, Hit hit){

    vec3 background = vec3(.1)* vec3(.5,0,1);
    background = color;

    if (hit.isBackground) {
        color = background;
        return;
    }

    if (hit.isOutline) {
        color = vec3(background * .33);
    } else {
        vec3 ref = reflect(hit.ray.direction, hit.normal);
        color = doLighting(
            hit.pos,
            hit.normal,
            ref,
            hit.ray.direction
        );
    }

    float fog = length(camPosition - hit.pos);
    fog = smoothstep(float(MAX_TRACE_DISTANCE) * .36, float(MAX_TRACE_DISTANCE), fog);
    color = mix(color, background, fog);
}

void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
    vec2 p = (-iResolution.xy + 2.0*fragCoord.xy)/iResolution.y;

    vec3 color = mix(vec3(.4,.3,.5) * .9, vec3(.6), -.2);

    vec3 bgA = vec3(.6,.5,.8) * .55;
    vec3 bgB = vec3(.7,.9,1.) * .5;
    color = mix(bgA, bgB, dot(p, normalize(vec2(.2,-.6))) * .5);

    time = iTime;
    time *= .6;
    time = mod(time, 1.);

    float camDist = length(camPosition);

    mat4 camMat = cameraMatrix;
    float focalLength = guiFocal;
    vec3 rd = normalize(
        (vec4(p, -focalLength, 1) * camMat).xyz
    );

    Hit hit = raymarch(CastRay(camPosition, rd));

    render(color, hit);

    vec2 uv = fragCoord/iResolution.xy;
    float vig = pow(
        16. * uv.x * uv.y * (1. - uv.x) * (1. - uv.y),
        0.075
    );
    color *= vec3(.9, .95, 1.) * vig * 1.1;

    color = mix(color, vec3(pow(length(color * .6), 2.)), .1);
    color *= 1.05;
    color = pow(color, vec3(1.2,1.3,1.2));

    fragColor = vec4(color,1.0);
}
 
