//stage GL_FRAGMENT_SHADER
//import <GLUP/ShaderToy.h>

#define SHADER_TOY

#ifdef SHADER_TOY
#define CAMERA_Z 2.5
const vec3 cameraStart       = vec3(0.0,  0.7, -CAMERA_Z);
const vec3 cameraLookat      = vec3(0.0,  0.0, 0.0);
const vec3 lightDir          = vec3(2.0,  2.4, -1.0);
const vec3 lightColour       = vec3(1.6,  1.8,  2.2);
const float specular         = 64.0;
const float specularHardness = 512.0;
const vec3  diffuse          = vec3(0.25, 0.25, 0.25);
const float ambientFactor    = 0.65;
const bool ao                = true;
const bool shadows           = true;
const bool rotateWorld       = true;
const bool antialias         = false;
const float distanceMin      = 0.003;
#else
uniform vec2 iResolution;
uniform float iTime;
uniform vec3 cameraPos;
uniform vec3 cameraLookat;
uniform vec3 lightDir;
uniform vec3 lightColour;
uniform float specular;
uniform float specularHardness;
uniform vec3 diffuse;
uniform float ambientFactor;
uniform bool ao;
uniform bool shadows;
uniform bool rotateWorld;
uniform bool antialias;
uniform float distanceMin;
#endif

/* ****************** Eyeball Texture *********************** */

// Based on tutorial by Inigo Quilez:
//  https://www.youtube.com/watch?v=emjuqqyq_qc

//alternative noise implementation
float hash( float n ) {
    return fract(sin(n)*43758.5453123);
}

float noise( in vec2 x ) {
    vec2 p = floor(x);
    vec2 f = fract(x);

    f = f*f*(3.0-2.0*f);

    float n = p.x + p.y*57.0;

    return mix(mix( hash(n+  0.0), hash(n+  1.0),f.x), mix( hash(n+ 57.0), hash(n+ 58.0),f.x),f.y);
}

mat2 m = mat2(0.8, 0.6, -0.6, 0.8);

float fbm(in vec2 p)
{
    float f = 0.0;
    f += 0.5000*noise(p); p*=m*2.02;
    f += 0.2500*noise(p); p*=m*2.03;
    f += 0.1250*noise(p); p*=m*2.01;
    f += 0.0625*noise(p); p*=m*2.04;
    f /= 0.9375;
    return f;
}

vec3 EyeUVMap( in vec2 uv )
{
    float pi            = 3.1415;
    float irisCoverage  = 0.20;
    
    float r = uv.y*1.0/irisCoverage;
    float a = uv.x * pi * 2.0;
    vec2 p = vec2(r*cos(a), r*sin(a));

    //change this to whatever you want the background
    //color to be
    vec3 bg_col = vec3(1.0);

    vec3 col = bg_col;

    //float ss = 0.5 + 0.25*sin(iTime);
    float ss = 0.75;
    float anim = 1.0 + 0.5*ss*clamp(1.0-r, 0.0, 1.0);
    r *= anim;

    if (r < 0.8) {
        // Outer iris, color variation
        col = vec3(0.0, 0.3, 0.4);

        float f = fbm(5.0*p);
        col = mix(col, vec3(0.2, 0.5, 0.4), f);

        // Central iris
        f = 1.0 - smoothstep(0.2, 0.5, r);
        col = mix(col, vec3(0.9, 0.6, 0.2), f);

        a += 0.05*fbm(20.0*p);

        // Iris, white striations
        f = smoothstep(0.3, 1.0, fbm(vec2((6.0+ss*0.25)*r, 20.0*a)));
        col = mix(col, vec3(1.0), f);

        // Iris, black striations
        f = smoothstep(0.4, 0.9, fbm(vec2(10.0*r, 15.0*a)));
        col *= 1.0 - 0.5*f;

        // Iris, outer shadow
        f = smoothstep(0.6, 0.8, r);
        col *= 1.0 - 0.5*f;

        // Pupil
        f = smoothstep(0.2, 0.25, r);
        col *= f;

        // Blend iris into sclera
        f = smoothstep(0.75, 0.8, r);
        col = mix(col, bg_col, f);
    } else {
        // Veins
        a += 0.15*fbm(10.0*p);
        
        float f = smoothstep(0.35, 1.0, fbm(vec2(0.5*r, 30.0*a)));
        col -= vec3(0.0,1.0,1.0) * (1.0 - uv.y) * f;
    }

    return col;
}

/* ****************** Mandelbulb Scene *********************** */

// Source code adapted from:
// https://github.com/kevinroast/webglshaders/blob/master/mandelbulb.html

#define AO_SAMPLES 5
#define RAY_DEPTH 256
#define MAX_DEPTH 20.0
#define PI 3.14159265

#define FLOOR_YPOS -2.0
#define EYE_ZPOS   -0.82

vec2 delta = vec2(distanceMin, 0.);

vec3 RotateY(vec3 p, float a)
{
    float c,s;
    vec3 q=p;
    c = cos(a);
    s = sin(a);
    p.x = c * q.x + s * q.z;
    p.z = -s * q.x + c * q.z;
    return p;
}

vec3 RotateX(vec3 p, float a) {
    return RotateY(p.yxz, a).yxz;
}

#ifdef SHADER_TOY
float FlatPeakFunc(float t) {
    // http://math.stackexchange.com/questions/100655/cosine-esque-function-with-flat-peaks-and-valleys
    return sin(0.5*PI*cos(t));
    /* Alternative:
        const float b = 2.;
        return sqrt((1.+b*b)/(1.+pow(b*cos(t),2.)))*cos(t);
     */    
}

float CameraOrbitAngle() {
    return PI - FlatPeakFunc(iTime*0.5) * PI * 1.1;
}

vec3 CameraOrbit() {
    vec3 pos = cameraStart - cameraLookat;
    if (rotateWorld) pos = RotateY(pos, CameraOrbitAngle());
    return pos + cameraLookat;
}

vec3 CameraInvariant(vec3 pos) {
    if (rotateWorld) pos = RotateY(pos, -CameraOrbitAngle());
    return pos;
}
#endif

float Plane(vec3 p, vec3 n)
{
   return dot(p, n);
}

float Sphere(vec3 p, float r)
{
    return length(p) - r;
}

// Added by marciot, to render an eye.
float EyeBall(vec3 p) {
    return Sphere(p-vec3(0.,0.,EYE_ZPOS), 0.37);
}

// Formula for original MandelBulb from http://blog.hvidtfeldts.net/index.php/2011/09/distance-estimated-3d-fractals-v-the-mandelbulb-different-de-approximations/
float MandelBulb(vec3 pos, const int limitIterations)
{
    const int Iterations = 12;
    const float Bailout = 8.0;
    //float Power = 5.0 + cos(iTime*0.0125)*4.0;
    //float Power = 5.0 + sin(0.5)*4.0;
    float Power = 9.0;

    vec3 z = pos;
    float dr = 1.0;
    float r = 0.0;
    for (int i = 0; i < Iterations; i++)
    {
        r = length(z);
        if (r > Bailout || i == limitIterations) break;   // TODO: test if better to continue loop and if() rather than break?

        // convert to polar coordinates
        float theta = acos(z.z/r);
        float phi = atan(z.y,z.x);
        dr = pow(r, Power-1.0)*Power*dr + 1.0;

        // scale and rotate the point
        float zr = pow(r,Power);
        theta = theta*Power;
        phi = phi*Power;

        // convert back to cartesian coordinates
        z = zr*vec3(sin(theta)*cos(phi), sin(phi)*sin(theta), cos(theta));
        z += pos;
    }
    return 0.5*log(r)*r/dr;
}

float MandelBulb(vec3 pos) {
    return MandelBulb(pos, 12);
}

// This should return continuous positive values when outside and negative values inside,
// which roughly indicate the distance of the nearest surface.
float Dist(vec3 pos)
{
    return min(
        // Floor is at y=-2.0
        min(
            Plane(pos-vec3(0.,FLOOR_YPOS,0.), vec3(0.,1.,0.)),
            MandelBulb(pos)
        ),
        EyeBall(pos)
    );
}

// Based on original by IQ - optimized to remove a divide
float CalcAO(vec3 p, vec3 n)
{
    float r = 0.0;
    float w = 1.0;
    for (int i=1; i<=AO_SAMPLES; i++)
    {
        float d0 = float(i) * 0.3;
        r += w * (d0 - Dist(p + n * d0));
        w *= 0.5;
    }
    return 1.0 - clamp(r,0.0,1.0);
}

// Based on original code by IQ
float SelfShadow(vec3 ro, vec3 rd)
{
    float k = 32.0;
    float res = 1.0;
    float t = 0.1;          // min-t see http://www.iquilezles.org/www/articles/rmshadows/rmshadows.htm
    for (int i=0; i<16; i++)
    {
        float h = Dist(ro + rd * t);
        res = min(res, k*h/t);
        t += h;
        if (t > 4.0) break; // max-t
    }
    return clamp(res, 0.0, 1.0);
}

float SoftShadow(vec3 ro, vec3 rd)
{
    float k = 16.0;
    float res = 1.0;
    float t = 0.1;          // min-t see http://www.iquilezles.org/www/articles/rmshadows/rmshadows.htm
    for (int i=0; i<48; i++)
    {
        float h = Dist(ro + rd * t);
        res = min(res, k*h/t);
        t += h;
        if (t > 8.0) break; // max-t
    }
    return clamp(res, 0.0, 1.0);
}

vec3 GetNormal(vec3 pos)
{
    if (pos.y < FLOOR_YPOS + distanceMin)
    {
        return vec3(0.0,1.0,0.0);
    }
    else
    {
        vec3 n;
        n.x = Dist( pos + delta.xyy ) - Dist( pos - delta.xyy );
        n.y = Dist( pos + delta.yxy ) - Dist( pos - delta.yxy );
        n.z = Dist( pos + delta.yyx ) - Dist( pos - delta.yyx );

        return normalize(n);
    }
}

// Added by marciot, for creepy eye
vec3 EyeBallColor(vec3 p) {
    vec3 eyeCenter = vec3(0.,0.,EYE_ZPOS);
    vec3 p0 = p-eyeCenter;

#ifdef SHADER_TOY
    // TODO: This is kind of spaghetti code, probably better way to do this
    const float eyeRollMax = PI/4.;
    vec3 mouseVector = vec3(
        (iMouse.x/iResolution.x - 0.5) * 2.,
        (iMouse.y/iResolution.y - 0.5) * 2.,
        CAMERA_Z
    );
    mouseVector = CameraInvariant(mouseVector);
    vec3  cameraPointDir = cameraLookat - cameraStart;
    float cameraElevation = atan(cameraPointDir.y/cameraPointDir.z);
    float eyeRollLateral  = atan(mouseVector.x/mouseVector.z);
    float eyeRollVertical = atan(mouseVector.y/mouseVector.z)-cameraElevation;
    p0 = RotateY(p0, clamp(eyeRollLateral, -eyeRollMax, eyeRollMax));
    p0 = RotateX(p0, clamp(eyeRollVertical,-eyeRollMax, eyeRollMax));
#endif
    vec2 uv = vec2(
        atan(p0.y,p0.x)/(2.*PI), 
        acos(-p0.z/length(p0))/PI
    );
    return EyeUVMap(uv);
}

// Added by marciot, colorize the mandelbulb.
vec3 MandelBulbColor(vec3 pos) {
    float d1  = MandelBulb(pos, 1);
    float d6  = MandelBulb(pos, 6);
    float d9  = MandelBulb(pos, 9);
    float g = clamp( 0.5  + abs((d6-d1)*50.) ,0.,1.);
    float b = clamp( 0.25 + abs((d9-d1)*25.) ,0.,5.);
    return vec3(1., g, b);
}

// Based on a shading method by Ben Weston. Added AO and SoftShadows to original.
const vec3 sssColour = vec3(0.5,0.5,1.0);
vec4 Shading(vec3 pos, vec3 rd, vec3 norm)
{
    vec3 light;
    bool isEye   = EyeBall(pos) < distanceMin;
    bool isFloor = pos.y < FLOOR_YPOS+distanceMin;
    bool isBulb  = !isEye && !isFloor;

    // simple pos test on pos.y for floor (see Dist() above) - different colour and no spec for floor
    if(isFloor) {
        light = vec3(0.1,0.66,0.2) * max(0.0, dot(norm, lightDir));
        if (shadows) light *= SoftShadow(pos, lightDir);   // softer edged shadows on floor
        if (ao) light += CalcAO(pos, norm) * max(ambientFactor-0.25, 0.0);
    }
    else
    {
        light = lightColour * max(0.0, dot(norm, lightDir));
        vec3 heading = normalize(-rd + lightDir);
        float spec = pow(max(0.0, dot(heading, norm)), specularHardness);
        light = (diffuse * light) + (spec * specular * lightColour);
        if (shadows) light *= SelfShadow(pos, lightDir);   // harder edged shadows on object
        if (ao) light += CalcAO(pos, norm) * ambientFactor;
    }

    if(isBulb) light     *= MandelBulbColor(pos);
    if(isEye)  light.rgb *= EyeBallColor(pos);
    
    return vec4(light, 1.0);
}

// Original method by David Hoskins
vec3 Sky(in vec3 rd)
{
    rd = CameraInvariant(rd);
    float sunAmount = max(dot(rd, lightDir), 0.0);
    float v = pow(1.0 - max(rd.y,0.0),6.);
    vec3 sky = mix(vec3(.1, .2, .3), vec3(.32, .32, .32), v);
    sky += lightColour * sunAmount * sunAmount * .25 + lightColour * min(pow(sunAmount, 800.0)*1.5, .3);

    return clamp(sky, 0.0, 1.0);
}

// Camera function by TekF
// Compute ray from camera parameters
vec3 GetRay(vec3 dir, vec2 pos)
{
    pos = pos - 0.5;
    pos.x *= iResolution.x/iResolution.y;

    dir = normalize(dir);
    vec3 right = normalize(cross(vec3(0.,1.,0.),dir));
    vec3 up = normalize(cross(dir,right));

    return dir + right*pos.x + up*pos.y;
}

vec4 March(vec3 ro, vec3 rd)
{
    float t = 0.0;
    for (int i=0; i<RAY_DEPTH; i++)
    {
        vec3 p = ro + rd * t;
        float d = Dist(p);
        if (abs(d) < distanceMin)
        {
            return vec4(p, 1.0);
        }
        t += d;
        if (t >= MAX_DEPTH) break;
    }
    return vec4(0.0);
}

void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
    const int ANTIALIAS_SAMPLES = 4;
    const int DOF_SAMPLES = 16;

    const bool dof = false;

    vec4 res = vec4(0.0);
    
#ifdef SHADER_TOY
    vec3 cameraPos = CameraOrbit();
#endif

    if (antialias)
    {
        vec2 p;
        float d_ang = 2.*PI / float(ANTIALIAS_SAMPLES);
        float ang = d_ang * 0.33333;
        float r = 0.3;
        for (int i = 0; i < ANTIALIAS_SAMPLES; i++)
        {
            p = vec2((fragCoord.x + cos(ang)*r) / iResolution.x, (fragCoord.y + sin(ang)*r) / iResolution.y);
            vec3 ro = cameraPos;
            vec3 rd = normalize(GetRay(cameraLookat-cameraPos, p));
            vec4 _res = March(ro, rd);
            if (_res.a == 1.0) res.xyz += clamp(Shading(_res.xyz, rd, GetNormal(_res.xyz)).xyz, 0.0, 1.0);
            else res.xyz += Sky(rd);
            ang += d_ang;
        }
        res.xyz /= float(ANTIALIAS_SAMPLES);
    }
    else if (dof)
    {
        vec2 p = fragCoord.xy / iResolution.xy;
        vec3 ro = cameraPos;
        vec3 rd = normalize(GetRay(cameraLookat-cameraPos, p));
        vec4 _res = March(ro, rd);

        float d_ang = 2.*PI / float(DOF_SAMPLES);
        float ang = d_ang * 0.33333;
        // cheap DOF! - offset by camera zdiff (as cam/lookat are quite far apart)
        float r = max(0.3, abs(cameraLookat.z - _res.z + 0.0) * .2);
        for (int i = 0; i < DOF_SAMPLES; i++)
        {
            p = vec2((fragCoord.x + cos(ang)*r) / iResolution.x, (fragCoord.y + sin(ang)*r) / iResolution.y);
            ro = cameraPos;
            rd = normalize(GetRay(cameraLookat-cameraPos, p));
            _res = March(ro, rd);
            if (_res.a == 1.0) res.xyz += clamp(Shading(_res.xyz, rd, GetNormal(_res.xyz)).xyz, 0.0, 1.0);
            else res.xyz += Sky(rd);
            ang += d_ang;
        }
        res.xyz /= float(DOF_SAMPLES);
    }
    else
    {
        vec2 p = fragCoord.xy / iResolution.xy;
        vec3 ro = cameraPos;
        vec3 rd = normalize(GetRay(cameraLookat-cameraPos, p));
        vec4 intersect = March(ro, rd);
        if (intersect.a == 1.0) {
            res.xyz = clamp(Shading(intersect.xyz, rd, GetNormal(intersect.xyz)).xyz, 0.0, 1.0);
        } else {
            res.xyz = Sky(rd);
        }
    }

    fragColor = vec4(res.rgb, 1.0);
}

void mainVR( out vec4 fragColor, in vec2 fragCoord, in vec3 ro, in vec3 rd )
{
    vec4 res = vec4(0.0);
    
    ro += vec3(0., 0., -.5);
    
    float scale = 0.15;
    ro /= scale;
    
    vec4 intersect = March(ro, rd);
    if (intersect.a == 1.0) {
        res.xyz = clamp(Shading(intersect.xyz, rd, GetNormal(intersect.xyz)).xyz, 0.0, 1.0);
    } else {
        res.xyz = Sky(rd);
    }
    fragColor = vec4(res.rgb, 1.0);
}
