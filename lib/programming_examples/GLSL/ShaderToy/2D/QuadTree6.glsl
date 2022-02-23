//stage GL_FRAGMENT_SHADER
//import <GLUP/ShaderToy.h>

// shape generalization from https://www.shadertoy.com/view/lljSDy
// NB: the distance-to-shape evaluated at tile center is quite naive and easy to break.
// try #if 0  ( but too conservative )

float shape(vec2 P, float r) {
    switch( int(iTime)%5 ) {
      case 0: return length(P)/r - 1.;                                    // disk
      case 1: P = abs(P); return max(P.x,P.y)/r - 1.;                     // square
      case 2: P = abs(P*mat2(1,-1,1,1)/1.4); return max(P.x,P.y)/r - 1.;  // diamond
      case 3: return length(P)/r + .1*sin(10.*atan(P.y,P.x)) -1.;         // gear
      case 4: return length(P)/r - cos(3.14/3.)/cos(mod(atan(P.y,P.x)-iTime,6.28/3.)-3.14/3.); // triangle
      case -1: return length(P)/r + .3*sin(10.*atan(P.y,P.x)) -1.;         // gear 2
    }
}

void mainImage( out vec4 o,  vec2 U )
{
    o -= o;
    float r=.1, t=iTime, H = iResolution.y;
    U /=  H;                              // object : disc(P,r)
    vec2 P = .5+.5*vec2(cos(t),sin(t*.7)), fU;  
    U*=.5; P*=.5;                         // unzoom for the whole domain falls within [0,1]^n
    
    o.b = .25;                            // backgroud = cold blue
    
    for (int i=0; i<7; i++) {             // to the infinity, and beyond ! :-)
        fU = min(U,1.-U); if (min(fU.x,fU.y) < 3.*r/H) { o--; break; } // cell border
#if 1
        if (shape(P-.5, r) > .7/r) break; // cell is out of the shape (eval at tile center)
#else
        if (min( min(shape(P          ,r), shape(P-vec2(1,0),r)),
                 min(shape(P-vec2(0,1),r), shape(P-vec2(1,1),r))
                )  > .7/r) break;         // cell is out of the shape (eval at tile corners)
#endif
                // --- iterate to child cell
        fU = step(.5,U);                  // select child
        U = 2.*U - fU;                    // go to new local frame
        P = 2.*P - fU;  r *= 2.;
        
        o += .13;                         // getting closer, getting hotter
    }
               
	o.gb *= smoothstep(-.1,.0,shape(P-U,r)); // draw object
}

