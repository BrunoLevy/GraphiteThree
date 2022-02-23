//stage GL_FRAGMENT_SHADER
//import <GLUP/ShaderToy.h>

// variant of https://shadertoy.com/view/4lBfRD

void mainImage( out vec4 O, vec2 U )
{
	vec2 R = iResolution.xy;
    U = (U+U-R)/R.y;
    float t = .1*mod(iTime+5.,14.), t2, r = 1., c,s;
    
    O -= O;
    for( float i=0.; i< 99.; i++)
        t2 = smoothstep(0.,1.,t/.2-.6*floor(i/2.))*.2,               // local time
	    U *= mat2(c=cos(t2),s=sin(t2),-s,c),                         // rotale
        r /= abs(c) + abs(s),                                        // scale
        O = smoothstep(3./R.y, 0., max(abs(U.x),abs(U.y)) - r) - O;  // draw square and revert
}
