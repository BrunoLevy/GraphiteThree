//stage GL_FRAGMENT_SHADER
//import <GLUP/ShaderToy.h>

// Vegetal Flux
// Leon 09-05-2017
// Refactored in 07-01-2018
// using code from IQ, Mercury, Koltes

#define PI 3.14159
#define TAU PI*2.
#define t iTime*.2
#define repeat(v,r) (mod(v,r)-r/2.)

const vec4 red = vec4(0.9,0.1,0.2,1);
const vec4 green = vec4(0.584, 0.901, 0.270, 1);
const vec4 green2 = vec4(0.254, 0.301, 0.211, 1);
const vec4 blue = vec4(0.631, 0.901, 0.901, 1);
const vec4 orange = vec4(0.901, 0.835, 0.270, 1);

struct Shape { float dist; vec4 color; };

float rng (vec2 seed) { return fract(sin(dot(seed*.1,vec2(324.654,156.546)))*46556.24); }
mat2 rot (float a) { float c=cos(a),s=sin(a); return mat2(c,s,-s,c); }
float sphere (vec3 p, float r) { return length(p)-r; }
float cyl (vec2 p, float r) { return length(p)-r; }
float disk (vec3 p, float r, float h) { return max(length(p.xy)-r, abs(p.z)-h); }
float smoo (float a, float b, float r) { return clamp(.5+.5*(b-a)/r,0.,1.); }
float smin (float a, float b, float r) { float h = smoo(a,b,r); return mix(b,a,h)-r*h*(1.-h); }
float amod (inout vec2 p, float c) {
	float ca = (2.*3.14159)/c;
	float a = atan(p.y,p.x)+ca*.5;
	float index = floor(a/ca);
	a = mod(a,ca)-ca*.5;
	p = vec2(cos(a),sin(a))*length(p);
	return index;
}

Shape map (vec3 pos) {

	Shape shape;
	shape.dist = 1000.;
	shape.color = vec4(1);

	float twist = .5;
	float count = 8.;
	float interval = 2.;
	float outter = 2.;

	vec3 p = pos;
	p.xz *= rot(sin(pos.y*twist + t));
	float index = amod(p.xz, count);
	float sens = mix(-1.,1.,mod(index,2.));
	p.x -= outter;

	float stem = cyl(p.xz, 0.04+.02*sin(p.y*4.-iTime*sens));

	float leaf;
	p.y = repeat(p.y+index+t*sens, interval);
	p.xy *= rot(.25*sens);
	p.yz *= rot(.15*sens);
	p.x -= .8;
	p.y -= sin(abs(p.z)*3.)*.1;
	p.y -= sin(abs(p.x-.7)*3.)*.1;
	leaf = disk(p.xzy,0.7, 0.01);

	float innerStem;
	p = pos;
	p.xz *= rot(pos.y+sin(pos.y+t*10.)-t*4.);
	index = amod(p.xz, 3.);
	p.x -= 0.3+.2*(.5+.5*sin(pos.y+t));
	innerStem = cyl(p.xz, 0.05);

	float seed;
	p = pos;
	interval = 0.6;
	p.y = repeat(p.y+t*4., interval);
	seed = sphere(p, 0.3*(.5+.5*sin(pos.y+.5)));

	float water;
	p = pos;
	p.xz *= rot(pos.y*.5+t);
	p.x -= 1.2;
	p.xz *= rot(pos.y*.5-t*9.);
	index = amod(p.xz, 8.);
	p.x -= 0.1+(.5*(.5+.5*sin(pos.y+3.*t)));
	water = cyl(p.xz, 0.04);

	float sceneLeaves = smin(leaf, stem, .3);
	float scene = min(seed, innerStem);

	shape.color = mix(red, orange, smoo(innerStem, seed, .1));
	shape.color = mix(shape.color, green2, smoo(stem, scene, .3));
	scene = min(stem, scene);
	shape.color = mix(shape.color, green, smoo(sceneLeaves, scene, .1));
	shape.color = mix(shape.color, blue, step(water, scene));

	shape.dist = min(water, min(sceneLeaves, scene));
	return shape;
}

vec4 raymarch (vec2 coord)
{
	vec2 uv = (coord.xy-.5*iResolution.xy)/iResolution.y;
  	float dither = rng(uv+fract(iTime));
	vec3 eye = vec3(0,0,-8);
	vec3 ray = normalize(vec3(uv,1.));
	vec3 pos = eye;
	vec4 color = vec4(0.);
	for (float i = 0.; i <= 1.; i += 1./30.) {
		Shape shape = map(pos);
		if (shape.dist < 0.01) {
			color = shape.color * (1.-i);
			break;
		}
		shape.dist *= .9 + .1 * dither;
		pos += ray * shape.dist;
	}
    return color;
}

void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
	fragColor = raymarch(fragCoord);
}

