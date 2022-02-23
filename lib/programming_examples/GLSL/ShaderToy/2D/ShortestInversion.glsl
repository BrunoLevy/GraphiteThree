//stage GL_FRAGMENT_SHADER
//import <GLUP/ShaderToy.h>

// inspired from https://www.shadertoy.com/view/XsyXzt

// -2 by coyote 
void mainImage( out vec4 O,  vec2 U )
{
    U -= .5*(O.xy=iResolution.xy);
    
               // inversion
    U = sin(6.3*  U*O.y/dot(U,U) );
    U *= U.y;                           // checker
 
	O = .5+  U.xxxx / fwidth(U.x);      // anti-aliasing
}



/**   // 122 chars

void mainImage( out vec4 O,  vec2 U )
{
    U -= .5*(O.xy=iResolution.xy);
    
               // inversion
    U = sin(6.3*  U*O.y/dot(U,U) );
    U.x *= U.y;                         // checker
 
	O += .5+  U.x / fwidth(U.x) -O;     // anti-aliasing
}
**/



/**  // 132 chars

void mainImage( out vec4 O,  vec2 U )
{
    U -= .5*(O.xy=iResolution.xy);
    U *= O.y/dot(U,U);                  // inversion

    U = sin(6.28*U); float c = U.x*U.y; // checker
 
	O = vec4(.5+  c / fwidth(c));       // anti-aliasing
}
**/



/**  // 135 chars

void mainImage( out vec4 O,  vec2 U )
{
    U = ( U - .5*(O.xy=iResolution.xy) ) / O.y;
    U /= dot(U,U);                      // inversion

    U = sin(6.28*U); float c = U.x*U.y; // checker
 
	O = vec4(.5+  c / fwidth(c));       // anti-aliasing
}
**/

