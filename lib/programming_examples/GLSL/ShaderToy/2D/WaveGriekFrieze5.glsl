//stage GL_FRAGMENT_SHADER
//import <GLUP/ShaderToy.h>

/**/ // variant of https://shadertoy.com/view/MljBDG

#define l(a)  2.6*(a)*(a)                             // approx of spiral arc-length

void mainImage(out vec4 O, vec2 U) 
{
	vec2 R = iResolution.xy, V;
    U = 5.* ( U+U - R ) / R.y;                        // normalized coordinates
    U = vec2( atan(U.y,U.x)/6.283 +.5, length(U) );   // polar coordinates
    U.y-= U.x;                                        // lenght along spiral
    U.x = l( ceil(U.y)+U.x ) - iTime;                 // arc-length
    O   = vec4(1.- pow( abs( 2.*fract(U.y)-1.),10.)); // inter-spires antialiasing
    V   = ceil(U); U = fract(U)-.5;                   // cell along spiral: id + loc coords
 // vortices (small spirals) : assume col = step(0,y) then rotate( (0,0), space&time*(.5-dist) )
    U.y = dot( U, cos( vec2(-33,0)                    // U *= rot, only need U.y -> (-sin,cos)
                       +  .3*( iTime + V.x )          // rot amount inc with space/time
                         * max( 0., .5 - length(U) )  // rot amount dec with dist
             )       );
	O *= smoothstep( -1., 1., U/fwidth(U) ).y;        // draw antialiased vortices
}

/**/

        



        
/** // 266 chars  (golfed version of above)
     // -15 without animation  -43 without big spiral antialiasing

#define mainImage(O,u)                                       \
	vec2 R = iResolution.xy, U = 5.* ( u+u - R ) / R.y;      \
    float a = atan(U.y,U.x)/6.283 +.5, l = length(U)-a;      \
    a += ceil(l); a = 2.6*a*a - iTime;                       \
    a = dot( U = fract(vec2(a,l)) - .5  ,                    \
             cos( .3*(iTime+a) * max(0.,.5-length(U)) - vec2(33,0) )); \
    O  += min(1., a/fwidth(a) +.5) - pow( abs( 2.*fract(l) -1. ), 10.) 

/**/

/** // 324 chars  (golfed version of above)

#define l(a)  (a)*(a)/2.     // (a)/2. * sqrt(1.+(a)*(a)) // approx of spiral arc-length

#define mainImage(O,u)                                         \
	vec2 R = iResolution.xy,                                   \
         U = u+u-R;                                            \
         U = vec2( atan(U.y,U.x)/6.283+.5, length(U)*5./R.y ); \
    U.y-= U.x;  U.x = l(2.3*(ceil(U.y)+U.x)) - iTime;          \
    O  += 1. - pow( abs( 1.-2.*fract(U.y) ), 10.);             \
    R   = ceil(U);                                             \
    U.y = dot( cos( .3*(iTime+R.x) * max(0.,.5-length(U = fract(U)-.5)) - vec2(33,0) ), U); \
	O  *= smoothstep(-1.,1.,U.y/fwidth(U.y))

/**/
        
 

        
        
/** // 319 chars

#define mainImage(O,u)                                   \
	vec2 R = iResolution.xy,                             \
         U = 5.*(u+u-R)/R.y, V;                          \
         U = vec2( atan(U.y,U.x)/6.283+.5, length(U) );  \
    float a = ceil( U.y-= U.x );                         \
    U.x = ( U.x+a/3. ) * a*6. - iTime;                   \
    O += 1. - pow( 1.- abs( 1.-2.*fract(U.y-.5) ), 10.); \
    V = ceil(U);                                         \
    U.y = dot( cos( .3*(iTime+V.x) * max(0.,.5-length(U = fract(U)-.5)) - vec2(33,0) ), U); \
	O *= smoothstep(-1.,1.,U.y/fwidth(U.y))
        
/**/
 

        
        
/** // 277 chars    missing: antialiasing along large spiral

#define mainImage(O,u)                                   \
	vec2 R = iResolution.xy,                             \
         U = 5.*(u+u-R)/R.y, V;                          \
         U = vec2( atan(U.y,U.x)/6.283+.5, length(U) );  \
    U.y -= U.x;                                          \
    U.x = ( U.x+ceil(U.y)/3. ) * ceil(U.y)*6. - iTime;   \
    V = ceil(U);                                         \
    U.y = dot( cos( .3*(iTime+V.x) * max(0.,.5-length(U = fract(U)-.5)) - vec2(33,0) ), U); \
	O += smoothstep(-1.,1.,U.y/fwidth(U.y))
        
/**/


