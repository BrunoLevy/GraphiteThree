//stage GL_FRAGMENT_SHADER
//import <GLUP/ShaderToy.h>

/*
 * Voronoi Scales
 * 
 * Copyright (C) 2018  Alexander Kraus <nr4@z10.info>
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

const float pi = acos(-1.);
const vec2 c = vec2(1., 0.);

#define rand(a0) fract(sin(dot(a0.xy ,vec2(12.9898,78.233)))*43758.5453)

mat3 rot(vec3 p)
{
    return mat3(c.xyyy, cos(p.x), sin(p.x), 0., -sin(p.x), cos(p.x))
        *mat3(cos(p.y), 0., -sin(p.y), c.yxy, sin(p.y), 0., cos(p.y))
        *mat3(cos(p.z), -sin(p.z), 0., sin(p.z), cos(p.z), c.yyyx);
}

float norm(vec2 x)
{
    return length(x);
}

/* compute voronoi distance and closest point.
 * x: coordinate
 * return value: vec3(distance, coordinate of control point)
 */
vec3 vor(vec2 x)
{
    vec2 y = floor(x);
   	float ret = 1.;
    
    //find closest control point. ("In which cell am I?")
    vec2 pf=c.yy, p;
    float df=10., d;
    
    for(int i=-1; i<=1; i+=1)
        for(int j=-1; j<=1; j+=1)
        {
            p = y + vec2(float(i), float(j));
            p += rand(p);
            
            d = norm(x-p);
            
            if(d < df)
            {
                df = d;
                pf = p;
            }
        }
    
    //compute voronoi distance: minimum distance to any edge
    for(int i=-1; i<=1; i+=1)
        for(int j=-1; j<=1; j+=1)
        {
            p = y + vec2(float(i), float(j));
            p += rand(p);
            
            vec2 o = p - pf;
            d = norm(.5*o-dot(x-pf, o)/dot(o,o)*o);
            ret = min(ret, d);
        }
    
    return vec3(ret, pf);
}

vec2 scene(vec3 x)
{
    x = rot(1.e-1*iTime*c.yyx)*x-c.yxy;
    
    vec3 v = vor(6.*x.xy);
    
    vec2 sda = vec2(-.05*v.x-.0+abs(x.z), rand(v.yz)),
        sdb = vec2(abs(.03*v.x-.0+abs(x.z)-.01*sin(2.5*length(x.xy+c.xy)-5.*iTime-pi)), 3.);
	vec2 sdf = mix(sda, sdb, step(sdb.x, sda.x));
    
    return sdf;
}

const float dx = 1.e-5;
vec3 normal(vec3 x)
{
    float s = scene(x).x;
    return normalize(vec3(
        scene(x+dx*c.xyy).x-s, 
        scene(x+dx*c.yxy).x-s, 
        scene(x+dx*c.yyx).x-s
    ));
}

void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
    vec2 uv = fragCoord/iResolution.yy-.5, s = c.yy;
    
    //raymarching
    vec3 o = 3.*c.yyx, t = 2.*c.yxy, d = normalize(t-o), r = c.xyy, u = cross(d, r), x = c.yyy, 
        ro = o + uv.x * r + uv.y * u;
    
    float depth = 0.;
    
    for(int i=0; i<100; ++i)
    {
        x = ro + depth * d;
        s = scene(x);
        if(s.x < 1.e-4) break;
        if(depth > 1000.) 
        {
			fragColor = c.yyyx;
            return;
        }
        depth += s.x;
    }
    
    //colorize
    vec3 n = normal(x), l = c.yyx, re = normalize(reflect(-l, n)), v = normalize(x-ro),
        col;
    if(s.y == 1.)
        col = .3*c.xyy+.3*c.xyy*dot(l,n)+.7*c.xxy*pow(dot(re,v), 4.);
	else
    	col = .1*c.yyx+.1*c.yyx*dot(l,n)+c.yxx*pow(dot(re,v), 4.);

    col = abs(.7*rot(vec3(1.1,1.2,1.3)*iTime+s.y)*col);
        
    fragColor = vec4(col,1.0);
}