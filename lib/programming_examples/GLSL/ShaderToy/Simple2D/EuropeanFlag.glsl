//stage GL_FRAGMENT_SHADER
//import <GLUP/ShaderToy.h>

/**/  // --- 333 chars - true stars version ( cf https://www.shadertoy.com/view/4lGXDG )

#define S(s,d)  sin( mod(-atan(U.y,U.x) s 3.14, v) d )

void mainImage( out vec4 O,  vec2 U )
{
    float v = 2.5, // 2.*6.28/5.
          l = ceil( 5.* length( U = (U+U -(O.xy=iResolution.xy)) / O.y)-.5 ) / 5.;
    O = mix( vec4(0,0,.7,1), 
             vec4(1,.8,0,1), 
             l==.6 ? smoothstep(.0,.1,
                                -10.*length(U-= l* sin( ceil(1.91*atan(U.y,U.x)-.5)/1.91 + vec2(1.57,0) ))
                                +.3/ min(max(S(+,),S(+,+v)),
                                         max(S(-,),S(-,+v))))
                   : 0. );
}
/**/



/**  // --- 261 chars ( round stars )

void mainImage( out vec4 O,  vec2 U )
{
    float l = ceil( 5.* length( U = (U+U -(O.xy=iResolution.xy)) / O.y)-.5 ) / 5.;
    O = mix( vec4(0,0,.7,1), 
             vec4(1,.8,0,1), 
             l==.6 ? smoothstep( .0, .1,
                                 -10.*length(U-= l* sin( ceil(1.91*atan(U.y,U.x)-.5)/1.91 + vec2(1.57,0) ))
                                 +.6+.2*sin(5.*atan(U.y,U.x))
                               )
                   : 0. );
}
/**/



/**  // --- 280 chars

void mainImage( out vec4 O,  vec2 U )
{
	U = (U+U -(O.xy=iResolution.xy)) / O.y;
    
    float l = floor(5.*length(U)+.5)/5.,
          a = floor(12.*atan(U.y,U.x)/6.283+.5)*6.283/12.;
         U -= l*vec2(cos(a),sin(a));
    O = mix( vec4(0,0,.7,1), 
             vec4(1,.8,0,1), 
             l==.6 ? smoothstep(.0,.1, .6+.2*sin(5.*atan(U.y,U.x))-length(U)*10.) : 0. );
}
/**/



/** // --- 304 chars

void mainImage( out vec4 O,  vec2 U )
{
	U = (U+U -(O.xy=iResolution.xy)) / O.y;
    O = vec4(0,0,.7,1);
    
    float l = length(U), a = atan(U.y,U.x);
    l = floor(5.*l+.5)/5.; a = floor(12.*a/6.283+.5)*6.283/12.;
    if (l==.6) {
        U -= l*vec2(cos(a),sin(a));
        l = length(U)*2.*5., a = atan(U.y,U.x);
	    O = mix(O,vec4(1,.8,0,1), smoothstep(.0,.1, .6+.2*sin(5.*a)-l) );
    }
}
/**/

