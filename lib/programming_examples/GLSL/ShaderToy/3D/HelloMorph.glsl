//stage GL_FRAGMENT_SHADER
//import <GLUP/ShaderToy.h>

// Hello Morph! By David Hoskins. Jan 2014.
// Aardman's (from Wallace & Gromit fame) early work.
// http://www.youtube.com/watch?v=jSMRPKM1evk

// Morph, the plasticine animation from British childhoods in the 70s and 80s:-
// https://www.youtube.com/watch?v=daQfoN_xXIc

// COMMENT THE NEXT LINE TO REMOVE STOP MOTION FRAME JUDDER...
#define STOP_MOTION_EFFECT

// Some often adjusted defines in one place...
#define elbowR		vec3(1.0, -.1, 0.3)
#define shoulderR	vec3(0.4, 0.56,  -.05)
#define wristR		vec3(.5, -.4, -0.1)
#define shoulderL	vec3(-0.4, 0.56, -.05)
#define sunColour	vec3(1.0)
#define skinColour  vec3(.65, .22, 0.14)
#define sunDir		vec3(.42562, .59588, -.681005)
#define PI 3.14159265359

// Animation variables.
// Possibly a bad idea using globals, but it seems OK if there's only a few of them.
// They make it much quicker and easier than passing everything in functions.
float wave;
float hel;
float low;
float nod;
float time;

//----------------------------------------------------------------------------------------
float Hash(vec2 p)
{
	return fract(sin(dot(p, vec2(32.3391, 38.5373))) * 74638.5453);
}

//----------------------------------------------------------------------------------------
// 2D rotations for 3D vectors make them quicker on axis rotations...
vec2 Rotate2(vec2 p, float a)
{
	float si = sin(a);
	float co = cos(a);
	return mat2(co, si, -si, co) * p;
}

//----------------------------------------------------------------------------------------
float Segment(vec3 p,  vec3 a, vec3 b, float r1, float r2)
{
	vec3 pa = p - a;
	vec3 ba = b - a;
	float h = clamp( dot(pa,ba)/dot(ba,ba), 0.0, 1.0 );
	return length( pa - ba*h ) - r1 + r2*h;
}

//----------------------------------------------------------------------------------------
float Mouth( vec3 p, vec3 a)
{
	float curve = cos(p.x*(5.35+sin(-time)*1.25))*.11;
	p.y += curve;
	a.y += pow(abs(curve), 2.0);
	return length(max(abs(p) - a,0.0)) -.02;
}

// IÃ±igo's distance functions...
//----------------------------------------------------------------------------------------
float  Sphere( vec3 p, float s )
{
    return length(p)-s;
}

//----------------------------------------------------------------------------------------
float Cylinder( vec3 p, vec2 h )
{
  return max( length(p.xz)-h.x, abs(p.y)-h.y );
}

//----------------------------------------------------------------------------------------
float RoundBox( vec3 p, vec3 b, float r )
{
	//b.x -= p.y * .08;
	return length(max(abs(p)-b,0.0))-r;
}

//----------------------------------------------------------------------------------------
float Nose(vec3 p, vec3 a, float r )
{
	float h = clamp( dot(p,a)/dot(a,a), 0.0, 1.0 );
	return length( p - a*h ) - r;
}

//----------------------------------------------------------------------------------------
float sMin( float a, float b )
{
    float k = .12;
	float h = clamp(0.5 + 0.5*(b-a)/k, 0.0, 1.0 );
	return mix( b, a, h ) - k*h*(1.-h);
}

//----------------------------------------------------------------------------------------
vec2 Map( in vec3 pos )
{
    vec2 res = vec2( 1000.0, 3);
	float d;
	pos.y += -1.0;
	vec3 elbowL		= vec3(-.95-wave*.05, .2, -0.25);
	vec3 wristL		= vec3(elbowL.x+1.0*sin(wave)*.55, elbowL.y+cos(wave)*.55, -.75);
	
	// Head...
	vec3 p = pos;
	// Rotate around Y axis for waist movment...
	float h = sin(time*2.3) * .1 - wave*0.01;
	p.xz = Rotate2(p.xz, h);
	vec3 p2 = p;
	// Rotate head around X axis...
	p2.zy = Rotate2(p.zy, nod+hel*2.0+wave*.012);
	p2 -= vec3(0.0, 0.0, -0.05);
	
	d = Sphere(p2, .37);
	if (d < res.x)
	{
		res.x = d;
		vec3 p3 = vec3(abs(p2.x), p2.yz);
		if (dot(normalize(p3*vec3(1.0, .9, 1.0)), normalize(vec3(.32, 0.24, -.7))) > .95) res.y = 4.0;
		if (dot(normalize(p3), normalize(vec3(.32, 0.18-nod*.2, -.8))) > .993) res.y = 5.0;
	}

	// Mouth and inside colour...
	float mo = -Mouth(p2-vec3(0.0, -.057-hel, -0.3), vec3(.155-low, -.006+hel, .2));
	if (res.x  < mo ) res = vec2(mo, 2.0);

	// Nose
	p2 = p2-vec3(0.0, 0.0, 0.0);
	res.x = min(res.x, Nose(p2, vec3(.0,0.0,-.5), 0.06));
	
	// Neck...
	p = p-vec3(0.0, -.4, .1);
	d = Cylinder(p, vec2(0.171, .17));
	res.x = sMin(res.x, d);

	// Body...	
	p = p-vec3(0.0, -.82, 0.0);
	d = RoundBox(p, vec3(0.175, .45, 0.0), .26);
	res.x = sMin(res.x, d);
			
	// Right arm upper...
	p = p-vec3(0.0, 0.0, 0.0);
	d = Segment(p, shoulderR, elbowR, .17, .05);
	res.x = sMin(res.x, d);
	// Right arm lower...
	d = Segment(p, elbowR, wristR, .15, .05);
	res.x = sMin(res.x, d);
	// Right hand...	
	d = Segment(p*vec3(1.0, .75, 1.0), wristR+vec3(0, .1, 0.), wristR+vec3(-.15, .05, -.15), .13, .02);
	res.x = min(res.x, d);
	
	// Left arm upper...
	d = Segment(p, shoulderL, elbowL, .17, .05);
	res.x = sMin(res.x, d);
	// Left arm lower...
	d = Segment(p, elbowL, wristL, .15, .05);
	res.x = sMin(res.x, d);
	
	// Left Hand...	
	p = (p-wristL);
	p.z -= p.x*.5;
	p.yx = Rotate2(p.yx, -wave*1.5);
	d = Segment(p, vec3(0.0), -vec3(-.25, -0.15, 0.1), .06, .01);
	res.x = sMin(res.x, d);
	d = RoundBox(p-vec3(0.0, .25, 0.0), vec3(.042, .085, -.05), .09);
	res.x = sMin(res.x, d);
	
	// Mirrored legs...
	p = pos + vec3(0.0, .35, -.05);
	p.y += .8;
	p.x = abs(p.x);  // <- does the mirroring.
	vec3 ankle  = vec3(0.3, -1.85, 0.0);
	d = Segment(p, vec3(0.22, -.75, 0.0), ankle, .225, .05);
	res.x = sMin(res.x, d);
	
	// Feet...
	ankle.y -=.3;
	d = Segment(p, ankle, ankle + vec3(0.27, -.05, -0.3), .24, .05);
	d = max((ankle.y-p.y), d);
	res.x = sMin(res.x, d);

	// Do wooden box...
	d = RoundBox(pos + vec3(-4.2, 2.7, -1.0), vec3(2.0, .5, 1.0), .075);
	d = min(d, RoundBox(pos + vec3(-4.2, 1.9, -1.0), vec3(2.0, .185, 1.0), .075));
	if (d < res.x)
	{
		res = vec2(d, 1.0);
	}

    return res;
}

//----------------------------------------------------------------------------------------
vec2 RayMarch( in vec3 ro, in vec3 rd, in vec2 fragCoord, out int hit)
{
	const float precis = 0.01;
	float t = .5 + .1 * Hash(fragCoord.xy);
	hit = 0;
	// Had to remove all 'breaks' and 'continues' from the loop as they broke some systems...
	// (Scary huh?)
	vec2 res = vec2(precis*2.0, 0.0);
    for( int i = 0; i < 63; i++ )
    {
		if (hit == 0 && t < 20.0)
		{
			res = Map(ro + rd * t);
			if(res.x < precis)
			{
				hit = 1;
			}else
				t += max(.005, res.x * .5);
		}
    }
	// Missed scene, so do table with basic ray casting.
	// There's no point in ray-marching the flat gound as it's a
	// waste of cycles, especially for background location and accuracy.
	// (Well, in this case anyway)
	if (hit == 0 && rd.y < 0.0)
	{
		hit = 2;
		rd.y = min(rd.y, 0.0);
		t = (-2.3-ro.y) / rd.y;
		res.y = 6.0;
	}
	// Return the distance to point and material type.
	return vec2( t, res.y);	
}

//----------------------------------------------------------------------------------------
float Shadow( in vec3 ro, in vec3 rd)
{
	float res = 1.0;
    float t = 0.1;
	float h;
	
    for (int i = 0; i < 7; i++)
	{
		h = Map( ro + rd*t ).x;
		res = min(7.0*h / t, res);
		t += h+.04;
	}
    return max(res, 0.0);
}

//----------------------------------------------------------------------------------------
vec3 Normal( in vec3 pos )
{

	const vec2 eps = vec2( 0.015, 0.0);
	vec3 nor = vec3(
	    Map(pos+eps.xyy).x - Map(pos-eps.xyy).x,
	    Map(pos+eps.yxy).x - Map(pos-eps.yxy).x,
	    Map(pos+eps.yyx).x - Map(pos-eps.yyx).x );
	return normalize(nor);
}

vec3 GetMaterial(vec3 pos, vec3 norm, float mat)
{
	// These teture reads had to be moved out of the 'if' statements as
	// Windoes Chrome34 WebGL broke badly in the hands of those ANGLE guys! :p
	vec3 col = texture(iChannel0, pos.xz*.3).xyz*.65;	// Table
	vec3 tx1 = texture(iChannel0, pos.xy*vec2(.05, .25)).xyz * abs(norm.z+norm.x);
	vec3 tx2 = texture(iChannel0, pos.xz*vec2(.05, .25)).xyz * norm.y;

	float blink = step(mod(time-1.0, 3.0), .11);
	if (mat < 1.5)
	{
		// Wooden box is a lighter version of the table texture.
		col =  tx1;
		col += tx2;
		col = sqrt(col);
	}else if (mat < 2.5)
	{
		// Inside mouth...
		col = skinColour*.5;

	}else if (mat < 3.5)
	{
		// Plasticine...
		col = skinColour;

	}else if (mat < 4.5)
	{
		// Eye balls...
		col = skinColour * .7 * blink + vec3(1.0) * (1.0-blink);
	}else if (mat < 5.5)
	{
		// Pupil...
		col = skinColour * .7 * blink;// + vec3(0.0) * (1.0-blink);
	}
	return col;
}

//----------------------------------------------------------------------------------------
void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
	// Stop motion time...
#ifdef STOP_MOTION_EFFECT
	time = mod((floor(iTime*30.0) / 30.0), 20.0)-.7;
#else
	time = mod(iTime, 20.0)-.7;
#endif

	vec3 col = vec3(0.85);	
	vec2 q = fragCoord.xy/iResolution.xy;
    vec2 p = -1.0+2.0*q;
	p.x *= iResolution.x/iResolution.y;
	
	// Animation...
	wave = sin(time*15.0-.8+sin(time)*2.0)*.5+.5;
	wave = wave*wave*(3.0-2.0*wave)-.7;
	float m = fract(time*.19);
	hel = (1.0+sin(m*100.0)) * .02 * (smoothstep(0.0, .015, m) - smoothstep(0.05, .15, m));
	low = (smoothstep(0.05, .1, m)-smoothstep(0.12, .15, m))*.11;
	nod = -(smoothstep(0.22, .28, m)-smoothstep(0.28, .31, m))*.5;
	
	// Camera position...
	float t = clamp(time-3.5, 0.0, 1.0);
	t = t*t*(3.0-2.0*t);
	vec3 origin = mix(vec3(0.0, 1.0, -1.275), vec3(-1.0, 1.0, -5.5), t);
	vec3 target = mix(vec3(0.0, 1.0, 4.0),  vec3( 0.5, -.4, 0.0), t);
	origin = mix(origin, vec3( 1.0, 1.0, -5.0), clamp((time-6.0)*.075, 0.0, 1.0));

	// Camera matrix...
	vec3 cw = normalize( target-origin);
	vec3 cp = vec3( 0.0, 1.0, 0.0 );
	vec3 cu = normalize( cross(cw,cp) );
	vec3 cv = cross(cu,cw);
	vec3 ray = normalize( p.x*cu + p.y*cv + 2.6*cw );

	// Do the rendering...
	vec3 pos, norm;
	int hit = 0;
	vec2 res = RayMarch(origin, ray, fragCoord, hit);

	if (hit > 0)
	{
		pos = origin + res.x * ray;
		// Is it the ground?...
		if(hit == 2) { norm = vec3(0.0, 1.0, 0.0); } else { norm = Normal(pos); }

		col	= GetMaterial(pos, norm, res.y);
		
		float diff = max(dot(norm, sunDir), 0.0);
		float ambi = clamp(.2 + 0.2 * norm.y,0.0, 1.0);
		float shad = Shadow(pos, sunDir);
		float spec = max( 0.0, pow( max( dot(sunDir,reflect(ray, norm)), 0.0), 5.0) ) * .08;
		// Do the lighting... 
		vec3 lite = diff * sunColour * shad + col*ambi;
		col = col * lite + spec * shad;
		// Fog the background...
		col = mix(col, vec3(.85), clamp(res.x*res.x*.001-.2, 0.0, 1.0));
	}	
	
	// Post effects...
	col = pow(col,vec3(.5));
	// Add noise and fake flicker for old animation effect.
	float flick = max(1.-sin(fract(iTime*1.1) * PI), 0.0) * .03;
	col += Hash(floor(p*iResolution.y*.25)-time) * .035 -flick;
	t = 32.0*q.x*q.y*(1.0-q.x)*(1.0-q.y);
	// Make screen edge effects to frame the scene and make it look older...
	col   -= (1.0-pow(t, 0.1)) * .25;
	col.y -= (1.0-pow(t, 0.3-flick*3.0)) * .1;
	col.z -= (1.0-pow(t, 0.3)) * .05;
    fragColor=vec4(clamp(col, 0.0, 1.0), 1.0 );
}


