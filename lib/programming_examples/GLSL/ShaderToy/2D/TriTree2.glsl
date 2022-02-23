//stage GL_FRAGMENT_SHADER
//import <GLUP/ShaderToy.h>

// variant of https://www.shadertoy.com/view/Mt2XDc

void mainImage( out vec4 o,  vec2 U )
{
    o = vec4(0.0);
    float r=.05, z=4., t=iTime, H = iResolution.y, uz;
    U /=  H;                              // object : disc(P,r)
    vec2 P = iMouse.xy, C=vec2(-.7,0), fU;
    P =  length(P)>10. ? P/H :  .5+.5*vec2(cos(t),sin(t*.7));  
    U =(U-C)/z; P=(P-C)/z; r/= z;         // unzoom for the whole domain falls within [0,1]^n
    
    mat2 M = mat2(1,0,.5,.87), IM = mat2(1,0,-.577,1.155);
    U = IM*U;         // goto triangular coordinates (there, regular orthonormal grid + diag )
    
    o.b = .25;                            // backgroud = cold blue

    for (int i=0; i<int(log2(H)); i++) {  // to the infinity, and beyond ! :-)
        fU = min(U,1.-U); uz = 1.-U.x-U.y;
      //if (min(min(fU.x,fU.y),abs(uz)) < z*r/H) { o--; break; } // cell border
    	if (length(P-M*vec2(.5-sign(uz)/6.)) - r > .6) break;    // cell is out of the shape

                                          // --- iterate to child cell
        fU = step(.5,U);                  // select grid-child
        U = 2.*U - fU;                    // go to new local frame
        P = 2.*P - M*fU;  r *= 2.;
        
        o += .13;                         // getting closer, getting hotter
    }
    
    vec3 u = vec3(U,1.-U.x-U.y), c;      // local diamond barycentric coordinates
    u = fract(u*sign(u.z));              // local triangle barycentric coordinates
    
                                         // --- display

 #define S(v) smoothstep(fwidth(v), 0.,(v)) // * step((fwidth(v)),.1)
    o = vec4(u, 0);                      // paint coordinates 
    //o +=  smoothstep(.01,.0,abs(o-.5));// mid lines
    //o = vec4( length(o) );
#if 1                                     // disk around center    
    o +=  smoothstep(.51,.5, length(1./3.-u)*1.27  ) ;
#endif
#if 0                                     // disks around edges    
    c.x = length(vec2(u.r*1.73,u.g-u.b)) /2. ;
    c.y = length(vec2(u.g*1.73,u.b-u.r)) /2. ;
    c.z = length(vec2(u.b*1.73,u.r-u.g)) /2. ;
    o.rgb =  smoothstep(.51,.5, c*2.);    // radius = 1/4 
#endif
#if 0                                     // disks around corners    
    c.x = length(vec2((1.-u.r)*1.73,u.g-u.b)) /2. ;
    c.y = length(vec2((1.-u.g)*1.73,u.b-u.r)) /2. ;
    c.z = length(vec2((1.-u.b)*1.73,u.r-u.g)) /2. ;
    o.rgb =  smoothstep(.51,.5, c);                   // colored disks
  //o -= smoothstep(.06,.05, min(c.x,min(c.y,c.z)));  // black disks
  //o = vec4( sin(31.*min(c.x,min(c.y,c.z))) );       // concentric curves
#endif
    
    o.gb *= smoothstep(.9,1.,length(P-M*U)/r); // draw object
}
