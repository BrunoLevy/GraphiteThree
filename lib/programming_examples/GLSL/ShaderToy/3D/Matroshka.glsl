//stage GL_FRAGMENT_SHADER
//import <GLUP/ShaderToy.h>

// Matroshka! - 2016 Martijn Steinrucken - BigWings
// countfrolic@gmail.com
// License Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License.
// 
// This effect matches the music. In order for it to be in sync, its best to restart
// the effect as soon as the music loads.
//
// Use the mouse to look around. Watch at least 30 seconds to see the whole effect!
// 
// I tried to create a painted look. To make the flowers and leaves move use the PSYCO define.
// A video of the effect can be found here:
// https://www.youtube.com/watch?v=0kbZzVolycw

// Because dolls are different sizes and they are leaning when they walk, 
// it can happen that we overstep into the next cell, causing gaps in the doll.
// This is why, in the marching loop, I make sure to always start a new cell at the cell boundary.
// Another way to fix this is to make the step size 0.6 x the doll distance but that obviously 
// makes the ray marching slower. I'm not sure which solution is faster because im at a steady
// 60fps for both solutions on my machine. 
// Some better performance metrics would really help!
//
// I sunk way too much time into this, time to move on to something else! I hope you like it!
//
// SONG: Skorge - Tetris Theme
// https://soundcloud.com/officialskorge/skorge-tetris-theme

// Use these to change the effect

//#define PSYCO
//#define TEXTUREMODE
#define INVERTMOUSE 1.
#define MAX_STEPS 150
#define MIN_DISTANCE 0.1
#define MAX_DISTANCE 100.
#define RAY_PRECISION 0.002

#define S(x,y,z) smoothstep(x,y,z)
#define B(x,y,z,w) S(x-z, x+z, w)*S(y+z, y-z, w)
#define sat(x) clamp(x,0.,1.)
#define SIN(x) (sin(x)*.5+.5)
#define COS(x) (cos(x)*.5+.5)

float X2(float x) { return x*x; }

vec3 mainCol = vec3(1.0, .2, .1);		// the main color
vec3 lastCol = vec3(.1,.3, .1);			// the last main color
vec3 secCol = vec3(.8, .6, .1);			// the secondary color
vec3 lineCol = vec3(1.1);				// the line color

vec3 grid = vec3(5., 10., 5.);			// grid of marching dolls
vec4 stones = vec4(.2, .3, .5, 2.);		// controls size and shape of stones

float SOLO;								// wether we are in solo mode or not	

const vec3 light = vec3(1., 1., 1.)*.577; 
const vec3 lf=vec3(1., 0., 0.);
const vec3 up=vec3(0., 1., 0.);
const vec3 fw=vec3(0., 0., 1.);
const float pi2 = 1.570796326794896619;
const float pi = 3.141592653589793238;
const float twopi = 6.283185307179586;

vec2 mouse;
vec3 bg; // global background color

float time;

float remap01(float a, float b, float t) {return (t-a)/(b-a);}
float L2(vec3 p) {return dot(p, p);}
float L2(vec2 p) {return dot(p, p);}

float N1( float x ) { return fract(sin(x)*5346.1764); }
float N2(vec2 p)
{	// Dave Hoskins - https://www.shadertoy.com/view/4djSRW
	vec3 p3  = fract(vec3(p.xyx) * vec3(443.897, 441.423, 437.195));
    p3 += dot(p3, p3.yzx + 19.19);
    return fract((p3.x + p3.y) * p3.z);
}
float N2(float x, float y) { return N2(vec2(x, y)); }

vec3 N31(float p)		
{	
   vec3 p3 = fract(vec3(p) * vec3(443.897, 441.423, 437.195));
   p3 += dot(p3, p3.yzx+19.19);
   return fract((p3.xxy+p3.yzz)*p3.zyx); 
}

vec2 N21(float p)
{
	vec2 p2 = fract(vec2(p) * vec2(443.897, 441.423));
	p2 += dot(p2, p2.yx + 19.19);
    return fract((p2.xx+p2.yx)*p2.yy);

}

float Noise(vec2 uv) {
    // noise function I came up with
    // ... doesn't look exactly the same as what i've seen elswhere
    // .. seems to work though :)
    vec2 id = floor(uv);
    vec2 m = fract(uv);
    m = 3.*m*m - 2.*m*m*m;
    
    float top = mix(N2(id.x, id.y), N2(id.x+1., id.y), m.x);
    float bot = mix(N2(id.x, id.y+1.), N2(id.x+1., id.y+1.), m.x);
    
    return mix(top, bot, m.y);
}

float NoiseTex(vec2 uv, float seed, float octaves) {
    float v=0.;
    uv += N21(seed);
    
    for(float i=1.; i<=11.; i++) {
    	v += Noise(uv)/i;
        uv *= 2.;
        
        if(i>octaves) break;
    }
    
    return v*.5;
}


struct ray {
    vec3 o;
    vec3 d;
};

struct camera {
    vec3 p;			// the position of the camera
    vec3 forward;	// the camera forward vector
    vec3 left;		// the camera left vector
    vec3 up;		// the camera up vector
	
    vec3 center;	// the center of the screen, in world coords
    vec3 i;			// where the current ray intersects the screen, in world coords
    ray ray;		// the current ray: from cam pos, through current uv projected on screen
    vec3 lookAt;	// the lookat point
    float zoom;		// the zoom factor
};
camera cam;


void CameraSetup(vec2 uv, vec3 position, vec3 lookAt, float zoom) {
    cam.p = position;
    cam.lookAt = lookAt;
    cam.forward = normalize(cam.lookAt-cam.p);
    cam.left = cross(up, cam.forward);
    cam.up = cross(cam.forward, cam.left);
    cam.zoom = zoom;
    cam.center = cam.p+cam.forward*cam.zoom;
    cam.i = cam.center+cam.left*uv.x+cam.up*uv.y;
    cam.ray.o = cam.p;						// ray origin = camera position
    cam.ray.d = normalize(cam.i-cam.p);	// ray direction is the vector from the cam pos through the point on the imaginary screen
}


// DE functions from IQ
// https://www.shadertoy.com/view/Xds3zN

float smin( float a, float b, float k )
{
    float h = clamp( 0.5+0.5*(b-a)/k, 0.0, 1.0 );
    return mix( b, a, h ) - k*h*(1.0-h);
}

float smax( float a, float b, float k )
{
	float h = clamp( 0.5 + 0.5*(b-a)/k, 0.0, 1.0 );
	return mix( a, b, h ) + k*h*(1.0-h);
}

float sdSphere( vec3 p, vec3 pos, float s ) { return length(p-pos)-s; }

float sSph( vec3 p, vec3 scale, float s )
{
    return (length(p/scale)-s)*min(scale.x, min(scale.y, scale.z));
}

float sdCappedCylinder( vec3 p, vec2 h )
{
  vec2 d = abs(vec2(length(p.xz),p.y)) - h;
  return min(max(d.x,d.y),0.0) + length(max(d,0.0));
}

mat3 RotY(float angle) {
	float s = sin(angle);
    float c = cos(angle);
    
    return mat3(c, 0., s,  0., 1., 0.,  -s, 0., c);
}


mat3 RotZ(float angle) {
	float s = sin(angle);
    float c = cos(angle);    
    
    return mat3(c, -s, 0.,  s, c, 0.,  0., 0., 1.);
}

struct de {
    // data type used to pass the various bits of information used to shade a de object
	float d;	// distance to the object
   	vec4 p;		// local coordinate pos (xyz)
    float m; 	// material
    vec2 uv;	// uv coordinates
    vec3 id;
    
    float t; // transition between inside and outside
    float open; 	// how far open the split matroshkas are
    float inside;	// wether its the inside matroshka. This whole struct is a mess... oww well
    float seed;	// seed used to generate random values
    
    
    // shading parameters
    vec3 pos;		// the world-space coordinate of the fragment
    vec3 nor;		// the world-space normal of the fragment
    vec3 rd;		// the world-space view dir of the fragment
    float fresnel;	
};
    
struct rc {
    // data type used to handle a repeated coordinate
	vec3 id;	// holds the floor'ed coordinate of each cell. Used to identify the cell.
    vec3 h;		// half of the size of the cell
    vec3 p;		// the repeated coordinate
};
    
rc Repeat(vec3 pos, vec3 size) {
	rc o;
    o.h = size*.5;					
    o.id = floor(pos/size);			// used to give a unique id to each cell
    o.p = mod(pos, size)-o.h;
    return o;
}


vec2 PolarCoords(vec2 uv) {
	// carthesian coords in polar coords out
    return vec2(atan(uv.x, uv.y), length(uv));
}

vec2 SpiralCoords(vec2 st, float turns) {
	// polar coords in... spiral coords out. Spiral coordinates are neat!
    st.x = st.x/twopi +.5;
    st.y *= turns;
    float s = st.y+st.x;
    float l = (floor(s)-st.x);
    float d = fract(s);
    return vec2(l, d);
}


float circ(vec2 uv, float x, float y, float aspect, float radius, float edge) {
	vec2 p = uv-vec2(x, y);
    p.x *= aspect;
    
    edge *= .02;
    radius *= radius;								// comparing to r^2 to avoid sqrt
    return S(radius+edge, radius-edge, dot(p, p));	// not sure if thats actually faster
}

float circ(inout vec4 base, vec2 uv, float x, float y, float aspect, float radius, float edge, vec3 col) {
    float alpha = circ(uv, x, y, aspect, radius, edge);
    base = mix(base, vec4(col, 1.), alpha);
    return alpha;
}

float square(vec2 uv, vec4 rect, float blur) {
	// returns 1 when uv is inside the square, 0 otherwise
    return B(rect.x, rect.z, blur, uv.x) * B(rect.y, rect.w, blur, uv.y);
}

vec2 within(vec2 uv, vec4 rect) {
	// converts a uv from a global coordinate to a rect coordinate
    return (uv.xy-rect.xy)/(rect.zw-rect.xy);
}

float TowerMask(vec2 uv, float blur) {
    uv.x = abs(uv.x-.5);
    
    float y = uv.y * 2.5;
    
    float c = exp(-y*y)*pow(sat(y), .6)*.8;
    c = S(c, c-blur, uv.x);
    
    float width = mix(.15, .35, S(-.3, -.6, uv.y));
    float tower = width+uv.y*uv.y*.05;
    
    tower = S(tower+.01+blur, tower, uv.x);
    tower *= S(0.5, 0.2, uv.y);
    
    c = max(c, tower);
    return c;
}

float Kremlin(vec2 uv) {
    float c = TowerMask(within(uv, vec4(0.35, 0.7,.63,1.1)), .1);
    c += TowerMask(within(uv, vec4(0.15, 0.5,.43,.8)), .1);
    c += TowerMask(within(uv, vec4(0.65, 0.5,.83,.8)), .1);
    c += TowerMask(within(uv, vec4(0.525, 0.3,.72,.8)), .1);
    c += TowerMask(within(uv, vec4(0.025, 0.25,.22,.6)), .1);
    c += TowerMask(within(uv, vec4(0.8, 0.25,.95,.5)), .15);
    c = sat(c);
    c *= S(.0, .4, uv.y);
    return c;
}

vec3 background(vec3 r, vec2 uv, float starBurst) {
    float u = dot(r, up)*.5+.5;
    vec3 upCol = vec3(1., .4, .1);
    vec3 col;
    
    float t = iTime*4.;			
    
    if(SOLO>.5){										// splitting shells	
        float colFade = S(.0, .1, fract(time));
        upCol = mix(lastCol, mainCol, colFade)*2.5;		// make the background match the doll color
    	col = mix(upCol*.05, upCol, u);
        
    	vec2 st = PolarCoords(uv);
        
        starBurst *= sat(sin(st.x*3.+t)) + sat(sin(st.x*4.+t*.7654));	// add starburst 
        col += starBurst*.5*B(.3, .7, .2, u);
        
        col *= col;
    } else {											// marching bg
        float x = atan(r.x, r.z);						// from -pi to pi	
		float y = pi*0.5-acos(r.y);  					// from -1/2pi to 1/2pi		
        
        col = upCol*u*2.;
        float a = sin(r.x);
    
        float beam = sat(sin(10.*x+a*y*5.+t));			// add light beams
        beam *= sat(sin(7.*x+a*y*3.5-t));
        float beam2 = sat(sin(42.*x+a*y*21.-t));
        beam2 *= sat(sin(34.*x+a*y*17.+t));
        beam += beam2;
        col *= 1.+beam*.1;
        
        col += dot(r, fw);								// light gradient from front
        
        vec4 r = vec4(-.3, .0, .3, .4);					// add kremlin!
        if(x>r.x && x<r.z && y>r.y && y<r.w) {
            col *= 1.-Kremlin(within(vec2(x, y), r))*.3;
        }
    }
    return col;
}

vec4 Eyes(vec2 uv, float seed) {
    
    if(uv.x<.5) {								// add seductive wink ;)
    	float a = pow(SIN(time*2.), 700.)*4.;
    	uv.y = (uv.y-.5)*(1.+a) + .5;
    }
    
    vec3 n = N31(seed+675.75864);
    vec3 green = vec3(.2, .6, .1);
    vec3 blue = vec3(.3, .3, .9);
    vec3 eyeCol = mix(green, blue, n.x)*(.7+n.y);
    vec4 eyes = vec4(0.);
    float ar = 2.3;
    float blur = .5;
    float size = .2;
    
    vec2 lp = vec2(.35, .5);
    vec2 rp = vec2(.65, .5);
    
    vec2 glint = vec2(.05);
    vec2 pupil = SOLO*(mouse-.5)*vec2(-.3, .3); 		// make her looking at you (SOLO mode only)
    
    float eyeMask = circ( eyes, uv, lp.x, lp.y, ar, size, blur, eyeCol);
	eyeMask += circ( eyes, uv, rp.x, rp.y, ar, size, blur, eyeCol);
    
    circ(eyes, uv, lp.x+pupil.x, lp.y-pupil.y, ar, size*.6, blur, vec3(.1)); //pupil
    circ(eyes, uv, lp.x-glint.x, lp.y+glint.y, ar, size*.5, blur*.5, vec3(1.)); //glint
    
    circ(eyes, uv, rp.x+pupil.x, rp.y-pupil.y, ar, size*.6, blur, vec3(.1)); //pupil
    circ(eyes, uv, rp.x-glint.x, rp.y+glint.y, ar, size*.5, blur*.5, vec3(1.)); //glint
    
    eyes.a = eyeMask;
    
    vec2 uv2 = vec2(.5-abs(uv.x-.5), uv.y);
    
    float eyeLine = circ(uv2, lp.x-.02, lp.y+.05, ar*.8, size*1.05, blur);
    eyeLine -= circ(uv2, lp.x-.02, lp.y-.03, ar*.8, size*1.05, blur);
    eyeLine = sat(eyeLine);
    eyes = mix(eyes, vec4(0.,0.,0.,1.), eyeLine);
   
    vec2 lash = vec2(lp.x-.15, lp.y+.27);
    float eyeLash = circ(uv2, lash.x, lash.y, ar, size, blur);
    eyeLash -= circ(uv2, lash.x+.005, lash.y+.05, ar, size, blur);
    eyeLash = sat(eyeLash);
    eyes = mix(eyes, vec4(0.,0.,0.,1.), eyeLash);
    
    lash += vec2(.1, .1);
    ar =3.;
    eyeLash = circ(uv2, lash.x, lash.y, ar, size, blur);
    eyeLash -= circ(uv2, lash.x+.005, lash.y+.05, ar, size, blur);
    eyeLash = sat(eyeLash);
    eyes = mix(eyes, vec4(0.,0.,0.,1.), eyeLash);
    
    return eyes;
}

vec4 Mouth(vec2 uv, float seed) {
	
    float smile = .3;//sin(iTime)*.5;;
    
    vec4 lipUpCol = vec4(1., .1, .1, 1.);
    vec4 insideCol =  vec4(1., 1., 1., 1.);
    
    uv.y -= .5;
    uv.x = abs(uv.x*2.-1.);	// mirror in the middle
    
    uv *= 1.5;
    
    float upMid = .5-pow((uv.x-.25)*2., 2.);
    float upSide = pow(1.-uv.x, .5)*.25;
    float upper = smax(upMid, upSide, .2);
    
    float lowMid = uv.x*uv.x-.5;
    float lowSide = sqrt(1.-uv.x)/-5.;
    
    float lower = smin(lowMid, lowSide, .2);
    
    float curve = uv.x*uv.x*smile;
    
    vec4 col = lipUpCol*B(lower+curve, upper+curve,.05, uv.y);
    col = mix(col, insideCol, circ(uv, 0., curve, .2, .1, .1));
    
    return col;
}

vec4 Hair(vec2 uv, float seed) {
    vec3 n = N31(seed - 845.32);
    vec3 hair1Col = vec3(.4, .25, .15);
    vec3 blond = vec3(1.8, 1.7, .2);
    vec3 brunette = vec3(.8, .5, .3);
    
    vec3 hair2Col = mix(blond, brunette, n.x);
    hair1Col = hair2Col *.5;
    
	vec4 col = vec4(0.);
    
    if(n.y>.5) {											// hair style 1
        circ(col, uv, .8, .55, 3.5, .6, .5, hair1Col);
        circ(col, uv, .8, .6, 3.5, .54, .5, hair2Col);

        circ(col, uv, .55, .6, 3.5, .6, .5, hair1Col);
        circ(col, uv, .55, .63, 3.5, .54, .5, hair2Col);

        circ(col, uv, .4, .6, 3.5, .6, .5, hair1Col);
        circ(col, uv, .41, .63, 3.3, .54, .5, hair2Col);

        circ(col, uv, .25, .5, 3.5, .6, .5, hair1Col);
        circ(col, uv, .26, .53, 3.3, .54, .5, hair2Col);
    } else {												// hair style 2
        uv.x = abs(uv.x-.5);
        float spread = n.y*2.*.09;
        float d = length((uv-vec2(.2+spread, .9))*vec2(3., 1.));

        col.rgb = hair1Col*(1.+sin(d*20.)*.5*n.z);
        col.a=S(.9, .85, d);
    }
    
    return col;
}

vec4 Face(vec2 uv, float seed) {
    vec3 scarfCol = vec3(1., .9, .5);
    vec3 faceCol = vec3(1., 1., .8);
    vec3 lineCol = vec3(.1);
   
    vec4 col = vec4(0.);
    
    vec2 st = PolarCoords(uv-.5);
    scarfCol += sin(st.x*5.)*.1;
    circ(col, uv, .5, .5, 1.4, .5, .5, scarfCol); // scarf
    float face = circ(col, uv, .5, .45, 1.3, .4, .04, faceCol);	// face

    vec2 uv2 = uv;
    uv2.x = abs(uv.x-.5)+.5;

    circ(col, uv2, .65, .27, 1.3, .06, .3, vec3(1., .7, .7)); //rouge
    circ(col, uv2, .52, .29, 2., .02, .005, vec3(.1));		// nostrils

    vec4 eyeArea = vec4(.2, .31, .8, .55);
    if(square(uv, eyeArea, .01)>0.) {
        vec4 eyes = Eyes(within(uv, eyeArea), seed);
        col = mix(col, eyes, eyes.a);
    }

    float eyeBrows = circ(uv2, .6, eyeArea.w-.03, .7, .06, .01); 
    eyeBrows -= circ(uv2, .6, eyeArea.w-.05, .7, .06, .01); 
    eyeBrows = sat(eyeBrows);
    col = mix(col, vec4(lineCol, 1.), eyeBrows);

    vec4 hairArea = vec4(.1, .6, .9, .9);
    vec4 hair = Hair(within(uv, hairArea), seed); 
    hair.rgb *= .5;
    col = mix(col, hair, hair.a*face);
    
    vec4 mouthArea = vec4(.35, .05, .65, .25);
    float mouthMask = square(uv, mouthArea, .01);
    if(mouthMask>0.) {
    	vec4 mouth = Mouth(within(uv, mouthArea), seed);
    	col = mix(col, mouth, mouth.a);
    }
    
    return col;
}

vec4 Scarf(vec2 uv, float seed) {
    vec4 lCol = vec4(lineCol, 1.);
    vec4 scarfCol = vec4(secCol, 1.);
    
    float lineThickness = .01;    
    float x = uv.x*twopi;
	float scarfY = .25+COS(x)*-.15;
    float scarfMask = S(scarfY, scarfY+.01, uv.y);
    
    scarfY -= lineThickness;
    float lineMask = S(scarfY, scarfY+.01, uv.y);
    vec4 col = mix(lCol*lineMask, scarfCol, scarfMask);
    
    float y;
    
    if(uv.y>scarfY && uv.y<scarfY+.2) {		// scarf band
        y = scarfY+.1;
    	col = mix(col, lCol, B(y, .01+y, .005, uv.y));
   		x *= 10.;
        
        y = scarfY+.05+sin(x)*.04;
        col = mix(col, lCol, B(y, .01+y, .01, uv.y));

		float d = length(vec2(fract(x/pi), (uv.y-scarfY)*10.)-vec2(.5, .6-sin(x)*.1));
        col = mix(col, vec4(1.), S(.2, .14, d));
    }
    
    vec2 mirrorUV = abs(uv-vec2(.5, -0.03));
    vec2 st = PolarCoords(mirrorUV*vec2(2.2, 1.) - vec2(.32, .5));
    vec2 sc;
    if(st.y<.21) {						// add spiral decoration on the side of the head...
        st.y *= 2.2;
        st.x += .4;
    	sc = SpiralCoords(st, 5.);
    	col = mix(col, lCol, B(.3, .5, .05, sc.y)*S(1.15, 1., sc.x));
    }
    
    st = PolarCoords(mirrorUV*vec2(2.2, 1.) - vec2(.532, .385));
    if(st.y<.11) {						// quite cumbersome.. there must be an easier way...
        st.y *= 3.;
        st.x += 3.4;
        sc = SpiralCoords(st, 5.);
    	col = mix(col, lCol, B(.3, .5, .05, sc.y)*S(1.2, 1.05, sc.x));
    }
    uv.x -= .5;
    
    vec2 tiePos = vec2(0., .3);
    vec2 uv2 = uv - tiePos;
    st = vec2(atan(uv2.x*3., uv2.y-.05), length(uv2));
    
    y = COS(st.x*4.+pi);
    y = min(y, COS(st.x*4.+pi+.5));
    
    
    float creases = mix(SIN(st.x*16.+pi), 1., sat(st.y*7.));
    creases = S(.9, .1, creases);
    y*=.2;
    
    col = mix(col, lCol, S(.02+y, .01+y, st.y));
    col = mix(col, vec4(scarfCol.rgb, 1.), S(.005+y, .0+y, st.y));
    col = mix(col, lCol, creases);
    
    circ(col, uv, tiePos.x, tiePos.y, 4., .1, .05, lineCol.rgb);
    circ(col, uv, tiePos.x, tiePos.y, 4., .085, .05, scarfCol.rgb); 
    
    return col;
}

float PointFlower(vec2 st, float numPoints, float base, float pointiness) {
	st.y *= 4.;
    float x = st.x*numPoints;
    float y = pointiness*(abs(cos(x))+(.25-abs(cos(x+pi2)))*2.)/(2.+abs(cos(x*2.+pi2))*8.)-base;
    return st.y+y;
}

vec4 Flower(vec2 uv, vec4 pf, vec4 baseCol, vec4 lineCol) {
    vec2 st = PolarCoords(uv-.5);
    
    #ifdef PSYCO
    st.x += sin(st.y*10.)*sin(iTime);
    #else
    st.x += sin(st.y*10.)*.2;
    #endif
    
    float dist = PointFlower(st, pf.x, pf.y, pf.z);//3., .4, .4);
    float alpha = S(.5, .4, dist);
    
    baseCol.rgb *= S(.0, .15, st.y);
    vec4 col = baseCol*alpha;
    
    float edge = S(.2, .5, dist)*alpha;
    col = mix(col, lineCol, edge);			// dark painted edge
   
    float highlight = SIN(st.x*pf.w);
    highlight *= B(.0, .3, .1, dist);
    highlight *= SIN(st.x);
    col = mix(col, vec4(1.), highlight);			// highlight
        
    col = mix(col, vec4(1.), S(.06, .05, st.y));  
    
    return col;
}

vec4 Leaf(vec2 uv, float stemHeight, float sweep ) {
    // uv is in 0-1 range
    
    uv.y += sweep*uv.x*uv.x;
    vec2 uv2 = (uv-vec2(.22, .5)) * vec2(4., 3.); 
    
    float side = sign(uv2.y);
    float ay = abs(uv2.y);
    
    float start = sqrt(uv2.x);
    float end = 1.-pow(uv2.x/3., 2.);
    float y = smin(start, end, 0.4);
    
    float offs = pow(abs(ay-mix(.75, -.0, uv2.x))*4., 2.);
    float veins = sin(uv2.x*30.-offs+side);
    
    y *= 1.+veins*.07;
    
    float s = S(y+.04, y, ay);
    vec4 leafCol = vec4(.2, .5, .1, s);
    
    s *= 1.+veins*.2*B(.3, 2.4, .3, uv2.x)*S(.02, .3, ay);
    s *= mix(.2, 1., ay);
    
    leafCol.rgb = (leafCol.rgb+S(-.2, .2, uv2.y)*.35)*s;
    
    s = S(.0, .23, uv.x);
    float h = mix(stemHeight, .5, s);
    float t = mix(.02, .01, s);
    t *= S(.5, .15, uv.x);
    t *= S(.15, .4, uv.x);
    float stemMask = B(h-t, h+t, .01, uv.y);
    stemMask *= S(.95, .23, uv.x);
    leafCol = mix(leafCol, vec4(.2,.5,.1,1.), stemMask);
    
    return leafCol;
}

vec4 RoseTex(vec2 uv, vec3 n) {
    vec4 flowerCol = vec4(n*n, 1.);						// flower col is just a more saturated version of main col
  	vec4 lCol = vec4(lineCol, 1.);
    
    vec2 uv2 = uv;
    
    if(uv.x>.5) {										// mirror and flip half of the texture
    	uv.x = uv.x-.5;
        uv.y = 1.-uv.y;
    }
    
    float y = sin(uv.x*twopi)*.1;						// add a wavy vine down the middle
    float vine = B(.495+y, .505+y, .01, uv.y);
    
    vec2 p1 = uv-vec2(.27, .4);							// add spiral shaped vines
    vec2 st = PolarCoords(p1);
    vec2 sc = SpiralCoords(st, 5.);
    vine += B(.48, .52, .02, sc.y)*S(.7, .4, sc.x);
    
    vec4 col = vec4(.1, .5, .1, 1.)*vine;				// add vine
    
    vec2 lc = uv*4.-vec2(1.5, 1.7);
    
    float sweep = .2;
    #ifdef PSYCO
    sweep = sin(iTime*.5)*.5;
    #endif
    
    vec4 leaf = Leaf(lc, .5, sweep);
    
    col = mix(col, leaf, leaf.a);
    lc.x +=2.;
    lc.y = 1.-lc.y-.4;
    leaf = Leaf(lc, .5, sweep);
    col = mix(col, leaf, leaf.a);
    
    vec2 fc = sc;
    fc.x = fract(fc.x*10.);
    fc.x = (fc.x-.5)*.5+.5;
    vec4 smallFlower = Flower(fc, vec4(2.5, .4, .3, 5.), flowerCol, lCol);
    smallFlower.a *= B(.1, .4, .01, sc.x);
    col = mix(col, smallFlower, smallFlower.a);
    
    vec4 rect = vec4(0., .3, .5, .9);
    vec4 flower = Flower(within(uv, rect), vec4(3., .4, .4, 30.), flowerCol, lCol);
    col = mix(col, flower, flower.a);
    
    return col;
}

vec3 MatroshkaTex(de o) {
    vec3 n = N31(o.seed);
    vec3 col = mainCol;
	
    lineCol *= max(.1, S(.8, .9, n.x));

    vec4 faceArea = vec4(.35, .65, .65, .95);
    
    if(o.uv.y>faceArea.y-.2) {
        vec2 scarfUv = within(o.uv, vec4(0., faceArea.y-.2, 1., 1.));
        vec4 scarf = Scarf(scarfUv, o.seed);
        col = mix(col, scarf.rgb, scarf.a);
    }
    
    if(o.uv.y<faceArea.y) {
    	vec4 body = RoseTex(fract(o.uv+vec2(.4,.2)), n);
    	col = mix(col, body.rgb, body.a);
    }
    
    float faceMask = square(o.uv, faceArea, .001);
    if(faceMask>0.) {
		vec4 face = Face(within(o.uv, faceArea), o.seed);
        col = mix(col, face.rgb, face.a);
    }
    
    return col;

}

float MatroshkaDist(vec3 p) {	    
    float head = sSph(p-vec3(0., 2.4, 0.), vec3(.9,  .8, .9), .7);
    float body = sSph(p-vec3(0., 1., 0.), vec3(.95, 1.48, .95), .9);

    float d = smin(head, body, .4);		// merge head and body
    d = smax(d, -p.y, .05);				// flatten bottom
    
    float y = remap01(0., 2.96, p.y);
    d += sin(y*100.)*.001;
    d += B(.3985, .40, .003, y)*.001;	// groove where the top and bottom half meet
    
    return d;
}

vec2 MatroshkaTop(vec3 p) {													// returns distance to top half of doll (x) as well as inside or outside (y)
	float d = MatroshkaDist(p);												// get normal (closed) doll dist first
    float outside = d;														// save outside doll dist
    d = smax(d, -sdSphere(p, vec3(0., 0.6, 0.), 1.), .05);					// take away bottom half of doll
    d = smax(d, -sSph(p-vec3(0., 1., 0.), vec3(.7, 1.4, .7), .95), 0.02);	// hollow out top half
    float t = S(-.005, -.01, outside-d);									// calculate wether outside or inside of doll (used for shading)
    return vec2(d, t);
}

vec2 MatroshkaBottom(vec3 p) {												// returns distance to bottom half of doll (x) as well as inside or outside (y)
    float d = MatroshkaDist(p);												// get normal (closed) doll dist first
    float outside = d;														// save outside doll dist
    d = smax(d, -sdSphere(p, vec3(0., 3.0, 0.), 2.), .03);					// take away top half
    d = min(d, sdCappedCylinder(p-vec3(0.,1.,0.), vec2(.8, .25)));			// add extra ridge
    d = smax(d, -sSph(p-vec3(0., 1.15, 0.), vec3(.7, 1., .7), 1.1), 0.02);	// hollow out bottom part
    float t = S(.005, .01, d-outside);										// calculate wether outside or inside of doll (used for shading)
    return vec2(d, t);
}


vec2 GetStoneTiles(vec3 p) {
    return vec2(p.x+sin(p.z*stones.x), p.z+sin(p.x*stones.y)*stones.z)*stones.w;
}

float Ground(vec3 p) {
    float d = p.y;
    vec2 tiles = GetStoneTiles(p);;
    vec2 id = floor(tiles);
    float bump = N2(id.x, id.y);

    vec2 tUv = tiles*pi;
    float tileMask = abs( sin(tUv.x)*sin(tUv.y) );	// make a mask that fades to 0 on the edges
    tileMask = 1.-pow(1.-tileMask, 8.);
    d -= tileMask*.1*bump;
 
    vec3 n = N31(id.x+id.y*165.32);				// add surface detail
    float detail = sin(n.x*p.x*15.)*sin(n.y*p.z*15.)*.05;
    detail += sin(n.z*p.x*p.z*5.)*.005;
    d += detail;					
   
    return d;
}

vec3 SplitMatroshkaDist(vec3 p, float open, float size) {
	// returns distance and id to splitting matroshkas
    // x = distance, y = material, z = transition
    
    float dBaby = MatroshkaDist(p*size)/size;
    vec2 bottom = MatroshkaBottom(p+vec3(0., open, 0.));
    vec2 top = MatroshkaTop(p-vec3(0., open, 0.));

    float dShell = min(bottom.x, top.x);
    
    vec3 m = vec3(0.);
    if(dBaby < dShell)
        m = vec3(dBaby, 0., 0.);
    else if(bottom.x<top.x)
        m = vec3(bottom.x, 1., bottom.y);
    else
        m = vec3(top.x, -1., top.y);
        
    return m;
}


de castRay( ray r ) {
    float dO=MIN_DISTANCE;							// the distance from the camera
    float dS=MAX_DISTANCE;							// the distance from the surface
    float t;										// used to keep track of time
    vec3 p=vec3(0.);								// local position (after repeat, resize etc)
	
    de o;
    o.m = -1.;
    
    if(SOLO<.5) {									// we are in marching mode (as in marching matroshkas, not ray marching ;))
        // Dolls ...
        t = iTime*.96;						// try to match to the beat of the music
        
        rc q;										// holds the repeated coordinate
        float pt = t*pi;							// 'polar' time
        float s = sin(pt*5.);
        float shuffle = s*.1 + t;					// used to move dolls forward
        float headBounce = s*.05 + 1.05;			// used to scale height periodically
        s = sin(pt*2.5);
        mat3 leftRight = RotY(s*.2) * RotZ(s*.1);	// used to make the doll lean left and right
        
        for( int i=0; i<MAX_STEPS; i++ )
        {
            p = r.o + r.d * dO;						// Ray march
            vec3 P=p;
            p.z -= shuffle;							// move forward
            q = Repeat(p, grid);    				// make a grid of them
            p.xz = q.p.xz;							// keep only ground layer
            p.y *= headBounce;						// make them bounce up and down
            p *= leftRight; 						// make them sway left-right
            float si = fract((q.id.x+q.id.z+5.)/5.);// make them different sizes
            s = .8 + si;							// .8 < s < 1.8
            p*=s;									
            
            dS = MatroshkaDist(p)/s;				// calculate distance to doll
            
            vec3 rC = ((2.*step(0., r.d)-1.)*q.h-q.p)/r.d;	// ray to cell boundary
            float dC = min(min(rC.x, rC.y), rC.z)+.01;		// distance to cell just past boundary
            dS = min(dS, dC);								// if cell boundary is closer than just set to the beginning of the next cell
            
            dO += dS;								// add the distance from the surface to the distance from the camera
            if( dS<RAY_PRECISION || 				// if we hit, 
               dO>MAX_DISTANCE  ||	 				// or we are past far clipping..	
               (r.d.y>0. && p.y>6.5) ||				// or if we are looking up and the ray is already above all of them..
               (p.y<.0 && r.d.y<=0.)) 				// or if we are looking down and the ray is already below the ground
               break;								// break out of the loop 
        }
		
        if(dS<=RAY_PRECISION) {							// if we got really close to the surface, we count it as a hit
            o.m=2.;									// set material id so we know later on how to shade this
            o.d=dO;									// set distance from the camera
            o.p.xyz=p;									// save local coordinate (coordinate inside of the grid cell)
            o.p.w = s;
            o.seed = q.id.z + floor((q.id.x+q.id.z+5.)/10.)*100.;
        } else if(r.d.y<0. && o.m<0.) {				// only consider ground if we are looking down and nothing was hit yet
            // Ground ...
            dO = -((r.o.y-.08)/r.d.y);				// fast forward by doing a ray-plane intersection first
            
            for( int i=0; i<MAX_STEPS; i++ )
            {
                p.xyz = r.o + r.d * dO;				// ray march
                dS = Ground( p.xyz );				// get distance to ground
                dO += dS;							// add distance from the surface to the distance from the camera
                if( dS<RAY_PRECISION || 			// if we hit 
                   dO>MAX_DISTANCE ) 				// or if we are past far clipping
                    break;							// break out of the loop
            }

            if(dS<=RAY_PRECISION) {					// if we got really close to the surface, we count it as a hit
                o.m=1.;
                o.d=dO;
                
                p.z -= shuffle;							// move forward
                q = Repeat(p, grid);    				// make a grid of them
                p.xz = q.p.xz;							// keep only ground layer
                p.y *= headBounce;						// make them bounce up and down
                p *= leftRight; 						// make them sway left-right
                s = .8+fract((q.id.x+q.id.z+5.)/5.); 	// make them different sizes
            	p*=s;
                o.p.xyz=p;
            }
        }
    } else {										// we are in solo mode
    	t = fract(time);
        float open = (1.-X2(1.-t))*3.5;				// how far the two shells are apart as a function of time
        float size = mix(1.2, 1., t);				// grow as it matures
        vec3 m;
        
        for( int i=0; i<MAX_STEPS; i++ ) {
       		p = r.o + r.d * dO;						// Ray march
            m = SplitMatroshkaDist(p, open, size);
            if( m.x<RAY_PRECISION || dO>MAX_DISTANCE ) break;
            dO += m.x;
        }
        
         if(m.x<=RAY_PRECISION) {						// if we got really close to the surface, we count it as a hit
             o.d=dO;
             o.m=2.;
             o.t = m.z;
             
             if(m.y==0.) {   						// inside matroshka	
            	o.seed = floor(time+1.);
            	o.p.xyz=p*size;
                o.p.w = size;
                o.open = open;
                o.inside = 1.;
             } else {								// outside matroshka
             	o.seed = floor(time);
                o.p.xyz = p+vec3(0., m.y*open, 0.);
                o.open = open;
                o.p.w = 1.;
                o.inside = 0.;
             }
         }      
    }
    
    return o;
}

float SplitMatroshkaAO( de o, float dist ) {
	float occ = 0.0;
    float sca = 1.0;
    for( int i=0; i<5; i++ )
    {
        float hr = 0.01 + dist*float(i)/5.0;
        vec3 aopos =  o.nor * hr + o.pos;
        float dd = SplitMatroshkaDist( aopos, o.open, o.p.w ).x/o.p.w;
        occ += -(dd-hr)*sca;
        sca *= 0.35;
    }
    return sat( 1.0 - 3.0*occ );    
}

vec3 MatroshkaNormal( vec3 p )
{
	vec3 eps = vec3( 0.001, 0.0, 0.0 );
	vec3 nor = vec3(
	    MatroshkaDist(p+eps.xyy) - MatroshkaDist(p-eps.xyy),
	    MatroshkaDist(p+eps.yxy) - MatroshkaDist(p-eps.yxy),
	    MatroshkaDist(p+eps.yyx) - MatroshkaDist(p-eps.yyx) );
	return normalize(nor);
}

float GroundAO( de o, float dist ) {
	float occ = 0.0;
    float sca = 1.0;
    for( int i=0; i<5; i++ )
    {
        float hr = 0.01 + dist*float(i)/5.0;
        vec3 aopos =  o.nor * hr + o.pos;
        float dd = Ground( aopos );
        occ += -(dd-hr)*sca;
        sca *= 0.35;
    }
    return sat( 1.0 - 3.0*occ );    
}

vec3 GroundNormal( vec3 p )
{
	vec3 eps = vec3( 0.001, 0.0, 0.0 );
	vec3 nor = vec3(
	    Ground(p+eps.xyy) - Ground(p-eps.xyy),
	    Ground(p+eps.yxy) - Ground(p-eps.yxy),
	    Ground(p+eps.yyx) - Ground(p-eps.yyx) );
	return normalize(nor);
}



vec3 GroundMat(de o) {
    vec2 p = o.pos.xz*3.;
    vec2 noise = vec2( NoiseTex(p, 0., 5.), NoiseTex(p, 12., 5.));
    vec3 col = vec3(1., .8, .8)*.5;

    o.nor = GroundNormal(o.pos);					// get normal
	o.nor.xz += (noise-.5)*2.;						// cheap, fake, normal perturb
    o.nor = normalize(o.nor);						// renormalize
    
    vec2 id = floor(GetStoneTiles(o.pos));
    
    col *= 1.+( N2(id) -.5)*.3;						// vary color of stones
    col *= GroundAO( o, 1. );						// get ao in the cracks between stones
    
    // add fake shadows
    o.p.z+=.1;										// offset a little bit to account for the fact that light is coming from the fort
    float dropShadow = S(.8, .4, length(o.p));		// simple drop shadow right below the doll
    o.p.z+=.6; o.p.x*=1.5;							// second shadow is behind the doll and a bit elongated	
    float shadow = S(1.7, .5, length(o.p))*.75;
    shadow = max(shadow, dropShadow);				
    col *= mix(1., .2, shadow);						// add in both shadows
    
    vec3 r = reflect(o.rd, o.nor);					// calculate reflect view vector
    float spec = sat(dot(r, light));
    spec = pow(spec, 2.);
    col += spec*.1*(1.-shadow);					// add specular, make sure its attenuated by the shadow and vary the reflection by stone
    
    return col;
}

vec3 MatroshkaMat(de o, vec2 uv) {

    vec3 col = vec3(1.);
    
    o.uv = vec2(.5-atan(o.p.x, o.p.z)/twopi, remap01(0., 2.96, o.p.y));
        
    o.nor = MatroshkaNormal(o.p.xyz);
    o.fresnel = 1.-dot(o.nor, -o.rd);
    
    mainCol = N31(o.seed);							// generate main col
    secCol = fract(mainCol*23476.76);
    
    col *= MatroshkaTex(o);

    float dif = dot(light, o.nor);
    col *= max(.4, mix(1., dif, .5));				// ehh.. too much screwing around with things.. 
    col *= 1.+o.fresnel*.2;

    vec3 r = reflect(o.rd, o.nor);					// calculate reflected view ray
    float spec = sat(dot(r, light));				// calculate specular reflection
    
    float occ = 1.-S(.25, .0, o.pos.y);				// fake occlusion when we get close to the ground
    
    vec3 ref = background(r, uv, 0.);				// sample background in the direction of the reflection
    ref *= X2(o.fresnel)*.5;						// make sure reflection is strongest at grazing angles
    ref += pow(spec, 8.)*.6;						// add specular highlight
   
    float groove =  1.-B(.3985, .40, .003, o.uv.y);	// groove where the top and bottom half meet
       
    if(SOLO>.5) {
        float fakeAo = B(1.2-o.open, 1.2+o.open, .2, o.pos.y);	// calc fake letterbox ao
        
        if(o.inside>.5) { 										// if this is the inside doll
        	col *= fakeAo;										// .. doll has a darker head when covered by shell
        	ref *= X2(fakeAo);									// .. also no bg ref when covered
         	col += ref*groove;									// add ref, but not where the center groove is
        } else {
            if( o.t>0. )  {
            	vec3 interior = vec3(1., 1., .8);  
        		interior *= SplitMatroshkaAO(o, .5); 
            	interior *= mix(fakeAo, 1., o.open/3.5);
            	col = mix(col, interior, o.t);					// the inside has a different color
            } else
                col += ref*groove;
            col = mix(col, bg, S(1.05, 3.15, o.open));	// then fade it out..
        }        	
    } else {
        ref *= mix(occ, 1., SOLO);						// only occlude close to ground when in marching mode
        col += ref*groove;								// add reflection to final color, no ref where groove is
        col *= sat(occ+.7);							// darken where it contacts the ground
    }
    
   
    return col;
}

vec4 render( ray camRay, vec2 uv ) {
    // outputs a color
    
    vec3 col = vec3(0.);
    de o = castRay(camRay);
   
    if(o.m>0.) {        
        o.pos = camRay.o + o.d*camRay.d;
    	o.rd = camRay.d;
        
        if( o.m==1. )
            col = GroundMat(o);
        else
            col = MatroshkaMat(o, uv);
    }
    
    col = mix(col, background(o.rd, uv, 0.), S(.0, 100., o.d));
    
    return vec4( col, o.m );
}


void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
	vec2 UV = (fragCoord.xy / iResolution.xy)-.5;
    vec2 uv = UV;
   	uv.y *= iResolution.y/iResolution.x;
    vec2 m = iMouse.xy/iResolution.xy;
    if(m.x==0. && m.y==0.) m=vec2(.55, .45);	// hack to get a decent starting cam. Anyone have a better solution for this?
    mouse = m;
  
    vec3 col;
    
    float t = iTime;
    time = iTime*.4;
    
    #ifdef TEXTUREMODE
    	uv = fragCoord.xy / iResolution.xy;
    	//col = RoseTex(uv+.5).rgb;
    	col = vec3(Kremlin(uv));
    	//col = vec3(NoiseTex(uv, floor(t), 6.));
    #else
    
    float turn = (.5+m.x)*twopi;

    vec3 camPos;
    float camDist=8.;
    
    float mt = fract(t/269.)*269.;
    
    SOLO = B(-1., 28.3, 0.01, mt);
    SOLO += B(41.5, 54.8, 0.01, mt);
    SOLO += B(81., 107., 0.01, mt);
    SOLO += B(134., 161., 0.01, mt);
    SOLO += B(227., 240., 0.01, mt);
  
    if(SOLO>.5) {
        camPos = vec3(0., 1.5, 0.);
        mainCol = N31(floor(time+1.));
        lastCol = N31(floor(time));
    } else {
        camPos = vec3(0., 3.5, 0.);
        turn += t*.1;
    }

    float camY = INVERTMOUSE*camDist*cos((m.y)*pi);
    
    vec3 pos = vec3(0., camY, camDist)*RotY(turn);
   	
    CameraSetup(uv, camPos+pos, camPos, 1.);
    
    bg = background(cam.ray.d, uv, 1.);

    vec4 info = render(cam.ray, uv);
  
    if(info.w==-1.) {
        col = bg; 
    } else 
        col = info.rgb;
  	#endif
    
    UV *= 1.1;
    col *= 1.-dot(UV, UV);		// add vignette
    
    fragColor = vec4(col, .1);
}
