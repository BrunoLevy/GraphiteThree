//stage GL_FRAGMENT_SHADER
//import <GLUP/ShaderToy.h>

void mainImage( out vec4 O, vec2 U )
{
    O = vec4(0);
    vec2 R = iResolution.xy; 
    
#if 0 // do not :
    
    for ( int j = 0; j< int(R.y); j+=20 )
        for ( int i = 0; i< int(R.x); i+=20 ) {
            if ( int(U.x)==i || int(U.y)==j ) O++;        // grid
            if ( length(U-vec2(i,j)-10.) < 5. ) O.x++;    // disks
        }

#else    // do :
    
    vec2 V = mod(U, 20.);
    O = vec4( V.x >= 19.0 || V.y >= 19.0 );                   // grid
    if ( length(V-10.) < 5. ) O.x++;                      // disks

#endif
}
