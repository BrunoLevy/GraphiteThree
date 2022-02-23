//stage GL_FRAGMENT_SHADER
//import <GLUP/ShaderToy.h>

#define NOISE 2 // Perlin, Worley1, Worley2

#define PI 3.14159

// --- noise functions from https://www.shadertoy.com/view/XslGRr
// Created by inigo quilez - iq/2013
// License Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License.

const mat3 m = mat3( 0.00,  0.80,  0.60,
           		    -0.80,  0.36, -0.48,
             		-0.60, -0.48,  0.64 );

float hash( float n ) {
    return fract(sin(n)*43758.5453);
}

float noise( in vec3 x ) { // in [0,1]
    vec3 p = floor(x);
    vec3 f = fract(x);

    f = f*f*(3.-2.*f);

    float n = p.x + p.y*57. + 113.*p.z;

    float res = mix(mix(mix( hash(n+  0.), hash(n+  1.),f.x),
                        mix( hash(n+ 57.), hash(n+ 58.),f.x),f.y),
                    mix(mix( hash(n+113.), hash(n+114.),f.x),
                        mix( hash(n+170.), hash(n+171.),f.x),f.y),f.z);
    return res;
}

float fbm( vec3 p ) { // in [0,1]
    float f;
    f  = 0.5000*noise( p ); p = m*p*2.02;
    f += 0.2500*noise( p ); p = m*p*2.03;
    f += 0.1250*noise( p ); p = m*p*2.01;
    f += 0.0625*noise( p );
    return f;
}
// --- End of: Created by inigo quilez --------------------

// more 2D noise
vec2 hash12( float n ) {
    return fract(sin(n+vec2(1.,12.345))*43758.5453);
}
float hash21( vec2 n ) {
    return hash(n.x+10.*n.y);
}
vec2 hash22( vec2 n ) {
    return hash12(n.x+10.*n.y);
}
float cell;   // id of closest cell
vec2  center; // center of closest cell

vec3 worley( vec2 p ) {
    vec3 d = vec3(1e15);
    vec2 ip = floor(p);
    for (float i=-2.; i<3.; i++)
   	 	for (float j=-2.; j<3.; j++) {
                vec2 p0 = ip+vec2(i,j);
            	float a0 = hash21(p0), a=5.*a0*iTime+2.*PI*a0; vec2 dp=vec2(cos(a),sin(a)); 
                vec2  c = hash22(p0)*.5+.5*dp+p0-p;
                float d0 = dot(c,c);
                if      (d0<d.x) { d.yz=d.xy; d.x=d0; cell=hash21(p0); center=c;}
                else if (d0<d.y) { d.z =d.y ; d.y=d0; }
                else if (d0<d.z) {            d.z=d0; }  
            }
    return sqrt(d);
}

// distance to Voronoi borders, as explained in https://www.shadertoy.com/view/ldl3W8 
float worleyD( vec2 p) {
    float d = 1e15;
    vec2 ip = floor(p);
    for (float i=-2.; i<3.; i++)
   	 	for (float j=-2.; j<3.; j++) {
                vec2 p0 = ip+vec2(i,j);
            	float a0 = hash21(p0), a=5.*a0*iTime+2.*PI*a0; vec2 dp=vec2(cos(a),sin(a)); 
                vec2  c = hash22(p0)*.5+.5*dp+p0-p;
                float d0 = dot(c,c);
 	    float c0 = dot(center+c,normalize(c-center));
        d=min(d, c0);
    }

    return .5*d;
}


float grad, scale = 5.; 

// my noise
float tweaknoise( vec2 p) {
    float d=0.;
    for (float i=0.; i<5.; i++) {
        float a0 = hash(i+5.6789), a=1.*a0*iTime+2.*PI*a0; vec2 dp=vec2(cos(a),sin(a)); 
                
        vec2 ip = hash12(i+5.6789)+dp;
        float di = smoothstep(grad/2.,-grad/2.,length(p-ip)-.5);
        d += (1.-d)*di;
    }
    //float d = smoothstep(grad/2.,-grad/2.,length(p)-.5);
#if NOISE==1 // 3D Perlin noise
    float v = fbm(vec3(scale*p,.5));
#elif NOISE==2 // Worley noise
    float v = 1. - scale*worley(scale*p).x;
#elif NOISE>=3 // trabeculum 2D
    if (d<0.5) return 0.;
    grad=.8, scale = 5.;
	vec3 w = scale*worley(scale*p);
    float v;
    if (false) // keyToggle(32)) 
        v =  2.*scale*worleyD(scale*p);
    else
 	v= w.y-w.x;	 //  v= 1.-1./(w.y-w.x);
#endif
    
    return v*d;
    //return smoothstep(thresh-grad/2.,thresh+grad/2.,v*d);
}


void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
    grad = 0.05+4.*(1.+cos(iTime))*.5;
    vec2 p = 2.*(fragCoord.xy / iResolution.y -vec2(.9,.5));
  
    float c0=tweaknoise(p), c=sin(c0*5.);

    vec3 col; // = vec3(c);
    col = .5+.5*cos(c0*5.+vec3(0.,2.*PI/3.,-2.*PI/3.));
    col *= vec3(sin(12.*c0)); 
    // col = mix(col,vec3(cos(12.*c0)),.5);
    col = mix(col,vec3(c),.5+.5*cos(.13*(iTime-6.)));

   fragColor = vec4(col,1.);
}
