//stage GL_FRAGMENT_SHADER
//import <GLUP/ShaderToy.h>

/*
 * Gerstner Waves
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

//Changes:
//1: removed the incompatible inverse(.) call
//2: removed the S() and C() functions
//3: removed array setup and combined in one single function call

//universal constants
const float pi = acos(-1.);
const vec2 c = vec2(1.,0.);
const int nwaves = 9;

float rand(vec2 a0)
{
    return fract(sin(dot(a0.xy ,vec2(12.9898,78.233)))*43758.5453);
}

void wave(in int i, out float st, out float am, out vec2 di, out float fr, out float sp)
{
    //setup wave params
	st = abs(.35*rand(vec2(float(i))));//qi
	am = .02+.005*rand(vec2(float(i+2)));//ai
    di = (1.e0+vec2(1.7e0*rand(vec2(i,i+1)), 2.e0*rand(vec2(i+1,i))));//di
    fr = 6.+12.*rand(vec2(float(i+5)));//wi
    sp = 55.e-1+52.e-1*rand(vec2(float(i+4)));//phi
}

void gerst(in vec2 xy, in float t, out vec3 val, out vec3 deriv)
{
    val = vec3(xy, 0.);
    deriv = c.yyy;
    
   	float st,fr,sp,am;
    vec2 di;
    
    for(int i=0; i<nwaves; ++i)
    {
   		wave(i, st, am, di, fr, sp);
        
        //gen values
        float d = dot(di, xy);
		val += vec3(st*am*di*cos(fr*d+sp*t), am*sin(fr*d+sp*t));
    }

    for(int i=0; i<nwaves; ++i)
    {
        wave(i, st, am, di, fr, sp);
        
        //gen derivatives
        deriv += vec3(
            -di*fr*am*cos(fr*dot(di,val.xy)+sp*t),
            1.-st*fr*am*sin(fr*dot(di,val.xy)+sp*t)
        );
    }
    
    deriv = normalize(deriv);
}

void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
    //raytrace and colorize
    vec2 uv = fragCoord/iResolution.yy;
	vec3 o = c.yyx, r = 1.*c.xyy, u = 1.*c.yxy+c.yyx, d = normalize(cross(u,r)),
        ro = o+uv.x*r+uv.y*u;
    
    vec3 l = (c.yyx-3.*c.yxy),
        //p = inverse(mat3(d,c.xyy,c.yxy))*ro, //unportable!
        p = mat3(c.yxy, c.yyx, 1./d.z, -d.x/d.z, -d.y/d.z)*ro,
        n, val;
        
    gerst(p.xy, iTime, val, n);
    
    vec3 re = normalize(reflect(-l, n)), 
        v = normalize(p-ro);
    
    vec3 col = .2*c.yxx+.2*c.yyx*dot(l, n)+3.6e1*c.xxx*pow(dot(re,v), 4.);

    // Output to screen
    fragColor = vec4(col,1.0);
}