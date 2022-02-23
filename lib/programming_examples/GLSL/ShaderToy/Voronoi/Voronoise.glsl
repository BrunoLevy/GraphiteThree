//stage GL_FRAGMENT_SHADER
//import <GLUP/ShaderToy.h>

// The MIT License
// Copyright © 2014 Inigo Quilez
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions: The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software. THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.


// This is a procedural pattern that has 2 parameters, that generalizes cell-noise, 
// perlin-noise and voronoi, all of which can be written in terms of the former as:
//
// cellnoise(x) = pattern(0,0,x)
// perlin(x) = pattern(0,1,x)
// voronoi(x) = pattern(1,0,x)
//
// From this generalization of the three famouse patterns, a new one (which I call 
// "Voronoise") emerges naturally. It's like perlin noise a bit, but within a jittered 
// grid like voronoi):
//
// voronoise(x) = pattern(1,1,x)
//
// Not sure what one would use this generalization for, because it's slightly slower 
// than perlin or voronoise (and certainly much slower than cell noise), and in the 
// end as a shading TD you just want one or another depending of the type of visual 
// features you are looking for, I can't see a blending being needed in real life.  
// But well, if only for the math fun it was worth trying. And they say a bit of 
// mathturbation can be healthy anyway!


// Use the mouse to blend between different patterns:

// ell noise    u=0,v=0
// voronoi      u=1,v=0
// perlin noise u=0,v1=
// voronoise    u=1,v=1

// More info here: http://iquilezles.org/www/articles/voronoise/voronoise.htm

vec3 hash3( vec2 p )
{
    vec3 q = vec3( dot(p,vec2(127.1,311.7)), 
				   dot(p,vec2(269.5,183.3)), 
				   dot(p,vec2(419.2,371.9)) );
	return fract(sin(q)*43758.5453);
}

float iqnoise( in vec2 x, float u, float v )
{
    vec2 p = floor(x);
    vec2 f = fract(x);
		
	float k = 1.0+63.0*pow(1.0-v,4.0);
	
	float va = 0.0;
	float wt = 0.0;
    for( int j=-2; j<=2; j++ )
    for( int i=-2; i<=2; i++ )
    {
        vec2 g = vec2( float(i),float(j) );
		vec3 o = hash3( p + g )*vec3(u,u,1.0);
		vec2 r = g - f + o.xy;
		float d = dot(r,r);
		float ww = pow( 1.0-smoothstep(0.0,1.414,sqrt(d)), k );
		va += o.z*ww;
		wt += ww;
    }
	
    return va/wt;
}

void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
	vec2 uv = fragCoord.xy / iResolution.xx;

    vec2 p = 0.5 - 0.5*sin( iTime*vec2(1.01,1.71) );
	
	if( iMouse.w>0.001 ) p = vec2(0.0,1.0) + vec2(1.0,-1.0)*iMouse.xy/iResolution.xy;
	
	p = p*p*(3.0-2.0*p);
	p = p*p*(3.0-2.0*p);
	p = p*p*(3.0-2.0*p);
	
	float f = iqnoise( 24.0*uv, p.x, p.y );
	
	fragColor = vec4( f, f, f, 1.0 );
}
