//stage GL_FRAGMENT_SHADER
//import <GLUP/ShaderToy.h>

#define S(v)     cos( 6.28* abs(v) *2.5 )
#define B(v,c,s) if (abs(v)/.1 c ) O += (1.-O) * ( .5+.5* S((v)*6.)  ) * (.5+.5*s)
// variant: flat bands
//#define B(v,c,s) if (abs(v)/.1 c ) O += (1.-O) * smoothstep(0., 3e2/R.y, S((v)*6.) +1. ) * (.5+.5*s)

void mainImage( out vec4 O, vec2 u )
{
	vec2 R = iResolution.xy,
         U = (u+u-R)/R.y;
    float a = atan(U.y,U.x), l = length(U);
    O -= O;
    
    B( U.y, < 1. , -S(U.x) );                 // horizontal
    B( U.x, < 1. , -S(max(.2,abs(U.y))) );    // vertical

    if ( l/.1 > 2. ) 
    {
        U *= mat2(1,-1,1,1)/1.4;              // diagonal
        B( U.y, < 1. , S(U.x) );
        B( U.x, < 1. , S(U.y) );
    }

    B( l, > 3. , cos(a*4.) * sign(S(l)) );    // circular  variant : > 1. or > 1.6

    O = .1+.9*O; 
  //O = sqrt(O);
    O.r *= 1.2;
}
