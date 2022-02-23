//stage GL_FRAGMENT_SHADER
//import <GLUP/ShaderToy.h>

//#define col(i) vec4( vec3(C[i]), 1 )
//#define col(i) vec4(   .6 + .6 * cos(l-iTime+float(i) +vec3(0,23,21) ), 1 )        // colored
  #define col(i) vec4( ( .6 + .6 * cos(l-iTime+float(i) +vec3(0,23,21) ) )*C[i], 1 ) // + border

#define blend(i) O += (1.-O.a) * col(i) * C[i]
    
void mainImage( out vec4 O, vec2 U )
{
	vec2 R = iResolution.xy;
    U = (U+U-R)/R.y;
    O -= O;
    
    float a = atan(U.y,U.x), l = length(U);
    l =  2.*l; a =  30.*a/6.28;
    
    vec4 s = l-vec4(0,0,.5,.5),
         A = fract( a-vec4(.25,.75,0,.5) )-.5, L = fract(s-iTime) - .5, // 4 overlapping polar tilings
         r = sqrt(A*A*abs(s) + L*L),                         // ellipse (s*s would be const width)
         C = smoothstep(.1,0., r-.3);                        // 4 feathers mask
    
    int c = 2* int( L.z > L.x );                             // sort ranks
    ivec2 T = ivec2( A[1+c] > A[0+c], A[1+2-c] > A[0+2-c] ); // sort rows

    blend(  T.x  +  c  );                                    // blend by sorted order
    blend( 1-T.x +  c  );
    blend(  T.y  + 2-c );
    blend( 1-T.y + 2-c );

    
	
}
