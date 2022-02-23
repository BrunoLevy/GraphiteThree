//stage GL_FRAGMENT_SHADER
//import <GLUP/ShaderToy.h>

#define ANIMATE		0.5		// animation speed
//#define COLORED


vec2 Hash22 (vec2 p)
{
    vec2 q = vec2( dot( p, vec2(127.1,311.7) ), 
				   dot( p, vec2(269.5,183.3) ) );
    
	return fract( sin(q) * 43758.5453 );
}

float Hash21 (vec2 p)
{
    return fract( sin( p.x + p.y * 64.0 ) * 104003.9);
}

vec2 Hash12 (float f)
{
    return fract( cos(f) * vec2(10003.579, 37049.7) );
}

float Hash11 (float a)
{
   	return Hash21( vec2( fract(a * 2.0), fract(a * 4.0) ) );
    //return fract( sin(a) * 54833.56 );
}


// from https://www.shadertoy.com/view/ldl3W8
vec4 voronoi (in vec2 x)
{
    vec2 n = floor(x);
    vec2 f = fract(x);

    //----------------------------------
    // first pass: regular voronoi
    //----------------------------------
	vec2 mg, mr;

    float md = 8.0;
    for( int j=-1; j<=1; j++ )
    for( int i=-1; i<=1; i++ )
    {
        vec2 g = vec2(float(i),float(j));
		vec2 o = Hash22( n + g );
		#ifdef ANIMATE
        o = 0.5 + 0.5*sin( iTime * ANIMATE + 6.2831*o );
        #endif	
        vec2 r = g + o - f;
        float d = dot(r,r);

        if( d<md )
        {
            md = d;
            mr = r;
            mg = g;
        }
    }

    //----------------------------------
    // second pass: distance to borders
    //----------------------------------
    md = 8.0;
    for( int j=-2; j<=2; j++ )
    for( int i=-2; i<=2; i++ )
    {
        vec2 g = mg + vec2(float(i),float(j));
		vec2 o = Hash22( n + g );
		#ifdef ANIMATE
        o = 0.5 + 0.5*sin( iTime * ANIMATE + 6.2831*o );
        #endif	
        vec2 r = g + o - f;

        if( dot(mr-r,mr-r)>0.00001 )
        	md = min( md, dot( 0.5*(mr+r), normalize(r-mr) ) );
    }

    return vec4( x - (n + mr + f), md, Hash21( mg + n ) );
}


vec3 HSVtoRGB (vec3 hsv)
{
	vec3 col = vec3( abs( hsv.x * 6.0 - 3.0 ) - 1.0,
					 2.0 - abs( hsv.x * 6.0 - 2.0 ),
					 2.0 - abs( hsv.x * 6.0 - 4.0 ) );

	return (( clamp( col, vec3(0.0), vec3(1.0) ) - 1.0 ) * hsv.y + 1.0 ) * hsv.z;
}

vec3 Rainbow (float color, float dist)
{
    dist = pow( dist, 8.0 );
    return mix( vec3(1.0), HSVtoRGB( vec3( color, 1.0, 1.0 ) ), 1.0 - dist );
}


vec3 VoronoiFactal (in vec2 coord, float time)
{
    const float freq = 4.0;
    const float freq2 = 6.0;
    const int iterations = 4;
    
    vec2 uv = coord * freq;
    
    vec3 color = vec3(0.0);
    float alpha = 0.0;
    float value = 0.0;
    
    for (int i = 0; i < iterations; ++i)
    {
    	vec4 v = voronoi( uv );
    	
        uv = ( v.xy * 0.5 + 0.5 ) * freq2 + Hash12( v.w );
        
        float f = pow( 0.01 * float(iterations - i), 3.0 );
        float a = 1.0 - smoothstep( 0.0, 0.08 + f, v.z );
        
        vec3 c = Rainbow( Hash11( float(i+1) / float(iterations) + value * 1.341 ), i > 1 ? 0.0 : a );
        
        color = color * alpha + c * a;
        alpha = max( alpha, a );
        value = v.w;
    }
    
    #ifdef COLORED
    	return color;
    #else
    	return vec3( alpha ) * Rainbow( 0.06, alpha );
    #endif
}


void mainImage (out vec4 fragColor, in vec2 fragCoord)
{
    vec3 color = VoronoiFactal( fragCoord.xy / iResolution.xx, iTime );
    
    fragColor = vec4( color, 1.0 );
}

