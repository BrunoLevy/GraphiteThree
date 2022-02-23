//stage GL_FRAGMENT_SHADER
//import <GLUP/ShaderToy.h>

/*
* License Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License.
*/

float 	t;

#define I_MAX		800
#define E			0.01
#define FAR			30.


vec4	march(vec3 pos, vec3 dir);
vec3	camera(vec2 uv);
vec3	calcNormal(in vec3 pos, float e, vec3 dir);
vec2	rot(vec2 p, vec2 ang);
void	rotate(inout vec2 v, float angle);
float	mylength(vec2 p);
float	mylength(vec3 p);

vec3	id;
vec3	h;

void mainImage(out vec4 c_out, in vec2 f)
{
    h *= 0.;
    t = iTime;
    vec3	col = vec3(0., 0., 0.);
	vec2	uv  = vec2(.35+f.x/iResolution.x, f.y/iResolution.y);
	vec3	dir = camera(uv);
    vec3	pos = vec3(-.0, .0, 25.0-sin(iTime*.125)*25.*0.-21.+2.);

    vec4	inter = (march(pos, dir));

    col = 1. - h;
    col *= (1.-max(inter.x*.001251, 1.-inter.w*.125))*inter.y;
    //col *= inter.y;
    c_out =  vec4(col*1., h.x);
}

/*
**	Leon's mod polar from : https://www.shadertoy.com/view/XsByWd
*/

#define	PI			3.14159
#define TAU			PI*2.

vec2 modA (vec2 p, float count) {
    float an = TAU/count;
    float a = atan(p.y,p.x)+an*.5;
    a = mod(a, an)-an*.5;
    return vec2(cos(a),sin(a))*length(p);
}

/*
**	end mod polar
*/

float	scene(vec3 p)
{
    float	mind = 1e5;
    p.z -= -20.;
    p.z -= iTime*5.;
    rotate(p.xy, p.z*.5 + 1.0*sin(p.z*.125 - iTime*1.) + 1.*iTime
           + (mod(floor(p.x*.5), 2.)-1. == 0. ? -1. : -1.)*p.z*.5*0. );
    
    vec3	ap = p;
    ap.y += sin(iTime*-1.+p.z*.5)*2.;
    ap.x += cos(iTime*-1.+p.z*.5)*2.;
    ap.xy = modA(ap.xy, 50.-(sin(iTime*.0+p.z*.125)*40.));
    ap.x -= 1.;

    vec3	pr = p;
    pr.y -= min(sin(iTime*.125+4.14), .0)*1.*sin(iTime*-1.+p.z*.5)*2.;
    pr.x -= min(sin(iTime*.125+4.14), .0)*1.*cos(iTime*-1.+p.z*.5)*2.;
    id = vec3(floor(p.xy*.5), floor(p.z*2.));
    rotate(pr.xy, iTime*-1.5+floor(p.z*2.)+p.z*2.5 );
    pr.xy = abs(pr.xy)-.5;
    pr.xy = abs(pr.xy)-.06125;
    pr.z = (fract(pr.z*2.)-.5);
	mind = mylength(pr.xyz)-.051;
    pr.xyz = abs(pr.xyz)-.045;
    mind = min( mind, mylength(pr.xyz)-.025);
    mind = min( mind, max(max(mylength(ap.xy)-.035, pr.z-.25), 0.) );
    mind = min(mind, max(max(mylength(vec2(pr.yz ))-0.1, ap.x-.35), -ap.x+.135));
    mind = min(mind, mylength(vec2(mylength(ap.xy)-.00, fract(pr.z*1.35)-.5 ))-.105);
    
    return(mind);
}


vec4	march(vec3 pos, vec3 dir)
{
    vec2	dist = vec2(0.0, 0.0);
    vec3	p = vec3(0.0, 0.0, 0.0);
    vec4	step = vec4(0.0, 0.0, 0.0, 0.0);
	vec3	dirr;

    for (int i = -1; i < I_MAX; ++i)
    {
        dirr = dir;
    	p = pos + dirr * dist.y;
        dist.x = scene(p);
        dist.y += dist.x*.125;
        vec3	s = p;
        float	d = max(length(s.xy)-8., -(length(s.xy)-8.1));
        h -= vec3(.3, .2, .0)*.1/ (max(d, .00001)+.01);
        h += (
            .001/(dist.x*dist.x+0.01) 
            -
            1./(dist.y*dist.y+40.)
             )
            *
            vec3
        (
    		abs(sin(id.z+0.00) )
            ,
            abs(sin(id.z+1.04) )
            ,
            abs(sin(id.z+2.08) )
        );
        // log trick by aiekick
        if (log(dist.y*dist.y/dist.x/1e5)>0. || dist.x < E || dist.y >= FAR)
        {
            if (dist.x < E || log(dist.y*dist.y/dist.x/1e5)>0.)
	            step.y = 1.;
            break;
        }
        step.x++;
    }
    step.w = dist.y;
    return (step);
}

// Utilities

float	mylength(vec3 p)
{
	float	ret = 1e5;
    
    p = p*p;
    p = p*p;
    p = p*p;
    
    ret = p.x + p.y + p.z;
    ret = pow(ret, 1./8.);
    
    return ret;
}

float	mylength(vec2 p)
{
	float	ret = 1e5;
    
    p = p*p;
    p = p*p;
    p = p*p;
    
    ret = p.x + p.y;
    ret = pow(ret, 1./8.);
    
    return ret;
}

void rotate(inout vec2 v, float angle)
{
	v = vec2(cos(angle)*v.x+sin(angle)*v.y,-sin(angle)*v.x+cos(angle)*v.y);
}

vec2	rot(vec2 p, vec2 ang)
{
	float	c = cos(ang.x);
    float	s = sin(ang.y);
    mat2	m = mat2(c, -s, s, c);
    
    return (p * m);
}


vec3 calcNormal( in vec3 pos, float e, vec3 dir)
{
    vec3 eps = vec3(e,0.0,0.0);

	return normalize(vec3(
           march(pos+eps.xyy, dir).w - march(pos-eps.xyy, dir).w,
           march(pos+eps.yxy, dir).w - march(pos-eps.yxy, dir).w,
           march(pos+eps.yyx, dir).w - march(pos-eps.yyx, dir).w ));
}

vec3	camera(vec2 uv)
{
    float		fov = 1.;
	vec3		forw  = vec3(0.0, 0.0, -1.0);
	vec3    	right = vec3(1.0, 0.0, 0.0);
	vec3    	up    = vec3(0.0, 1.0, 0.0);

    return (normalize((uv.x-.85) * right + (uv.y-0.5) * up + fov * forw));
}

