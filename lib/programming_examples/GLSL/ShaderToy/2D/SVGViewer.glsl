//stage GL_FRAGMENT_SHADER
//import <GLUP/ShaderToy.h>

// ongoing work. filled version here https://www.shadertoy.com/view/MlVSWc

// Bezier display adapted from revers https://www.shadertoy.com/view/MlGSz3
// SVG spec: https://www.w3.org/TR/2008/REC-SVGTiny12-20081222/paths.html

#define M(x,y)             x0 = _x = x;   y0 =_y = y;
#define L(x,y)             d = min(d, line(uv, vec2(_x,_y), vec2(x,y)) ); _x=x,_y=y;
#define C(x1,y1,x2,y2,x,y) d = min(d, bezier(uv, vec2(_x,_y), vec2(x1,y1),vec2(x2,y2), vec2(x,y)) ); _x=x,_y=y; 
#define z                  d = min(d, line(uv, vec2(_x,_y), vec2(x0,y0)) );
// other SVG commands easy to implement, but not tested.

float bezier(vec2 uv, vec2 A, vec2 B, vec2 C, vec2 D);
float line(vec2 p, vec2 a, vec2 b);
float contrast = 1.;

// === SVG drawing ===============================================================

float SVG(vec2 uv) {
    uv *= 400.; 
    contrast = .75*iResolution.x/ 400.;
    float d = 1e38, _x, _y, x0, y0;

// nvidia logo from https://upload.wikimedia.org/wikipedia/fr/4/47/Nvidia_%28logo%29.svg
 
    
    M( 82.2115,102.414 )
    C( 82.2115,102.414, 104.7155,69.211, 149.6485,65.777 )
    L( 149.6485,53.73  )
    C( 99.8795,57.727, 56.7818,99.879,  56.7818,99.879   )
    C( 56.7818,99.879, 81.1915,170.445, 149.6485,176.906 )
    L( 149.6485,164.102 )
    C( 99.4105,157.781, 82.2115,102.414, 82.2115,102.414 ) 
    // z
        
    M( 149.6485,138.637 )
    L( 149.6485,150.363 )
    C( 111.6805,143.594, 101.1415,104.125, 101.1415,104.125 )
    C( 101.1415,104.125, 119.3715,83.93,   149.6485,80.656  )
    L( 149.6485,93.523 )
    C( 149.6255,93.523, 149.6095,93.516,  149.5905,93.516   )
    C( 133.6995,91.609, 121.2855,106.453,  121.2855,106.453 )
    C( 121.2855,106.453, 128.2425,131.445, 149.6485,138.637 ) 
    // z

    M( 149.6485,31.512 )
    L( 149.6485,53.73 )
    C( 151.1095,53.617,  152.5705,53.523,  154.0395,53.473  )
    C( 210.6215,51.566,  247.4885,99.879,  247.4885,99.879  )
    C( 247.4885,99.879,  205.1455,151.367, 161.0315,151.367 )
    C( 156.9885,151.367, 153.2035,150.992, 149.6485,150.363 )
    L( 149.6485,164.102 )
    C( 152.6885,164.488, 155.8405,164.715, 159.1295,164.715 )
    C( 200.1805,164.715, 229.8675,143.75,  258.6135,118.937 )
    C( 263.3795,122.754, 282.8915,132.039, 286.9025,136.105 )
    C( 259.5705,158.988, 195.8715,177.434, 159.7585,177.434 )
    C( 156.2775,177.434, 152.9345,177.223, 149.6485,176.906 )
    L( 149.6485,196.211 )
    L( 305.6805,196.211 )
    L( 305.6805,31.512 )
    L( 149.6485,31.512 ) 
    // z
          
    M( 149.6485,80.656 )
    L( 149.6485,65.777 )
    C( 151.0945,65.676, 152.5515,65.598, 154.0395,65.551     )
    C( 194.7275,64.273, 221.4225,100.516, 221.4225,100.516   )
    C( 221.4225,100.516, 192.5905,140.559, 161.6765,140.559  )
    C( 157.2275,140.559, 153.2385,139.844, 149.6485,138.637  )
    L( 149.6485,93.523 )
    C( 165.4885,95.437, 168.6765,102.434, 178.1995,118.309   )
    L( 199.3795,100.449 )
    C( 199.3795,100.449, 183.9185,80.172, 157.8555,80.172    )
    C( 155.0205,80.172, 152.3095,80.371, 149.6485,80.656 ) 
    // z

    return d;
}

const mat4   M = mat4(-1,  3, -3,   1,         // Bspline Matrix
                       3, -6,  3,   0,
                      -3,  3,  0,   0,
                       1,  0,  0,   0);
/*
#define T .5
const mat4   M = mat4(-T,   2.-T,  T-2.,    T, //  Catmull-Rom Matrix
                       2.*T, T-3., 3.-2.*T,-T,
                      -T,      0,   T,      0,
                       0,      1,   0,      0);
*/

// Spline Interpolation
vec2 interpolate(vec2 G1, vec2 G2, vec2 G3, vec2 G4, float t) {
    vec2 A = G1 * M[0][0] + G2 * M[0][1] + G3 * M[0][2] + G4 * M[0][3];
    vec2 B = G1 * M[1][0] + G2 * M[1][1] + G3 * M[1][2] + G4 * M[1][3];
    vec2 C = G1 * M[2][0] + G2 * M[2][1] + G3 * M[2][2] + G4 * M[2][3];
    vec2 D = G1 * M[3][0] + G2 * M[3][1] + G3 * M[3][2] + G4 * M[3][3];

    return t * (t * (t * A + B) + C) + D;
}

float line(vec2 p, vec2 a, vec2 b) {
	vec2 pa = p - a, ba = b - a;
	float h = clamp(dot(pa, ba) / dot(ba, ba), 0., 1.);
	vec2 d = pa - ba * h;
	return dot(d,d); //length(d); // optimization by deferring sqrt
}


float bezier( vec2 uv, vec2 A, vec2 B, vec2 C, vec2 D)    
{
    float d = 1e5;
    vec2 p = A;
    for (float t = 0.; t <= 1.01; t += .025) {
        vec2 q = interpolate(A, B, C, D, t);
        d = min(d, line(uv, p, q));
		p = q;
	}

	return d;
}


void mainImage(out vec4 O, vec2 U) {
    vec2 R = iResolution.xy;
    U.y = R.y-U.y; U /= R.x;
    
    float d = sqrt(SVG(U)); // SVG(U); // optimization by deferring sqrt here
    d *= contrast;
	O = vec4(d); // *vec4(1,.2,.05,1);
	if (fract(iTime/2.)>.5) O = clamp(d,0.,1.) + .05*vec4(0,sin(d),0,0);
}
