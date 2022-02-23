//stage GL_FRAGMENT_SHADER
//import <GLUP/ShaderToy.h>

// short version of https://www.shadertoy.com/view/MldSR8


/**/ // ---------------------------------------- coyote: 220 chars (rearanged by Fab)
void mainImage( out vec4 O,  vec2 U )
{
    U += U - (O.xy=iResolution.xy);
	float L = length( U *= 10.5/O.y ), l;
    O -= O;
   
    abs(U.x) < 10.5 ?

        U /= L > 8.5 ? 1. : 3.-L*L/36.,  // bubble

        l = length(fract(U+=.5)-.5),
        L = length(abs(floor(U))+1.),

        O +=  l<.35 ? 1.                 // inner
            : l<.5  ? .03*L              // mid
            :         .9-.1*L            // outer
    : O;
}
/**/

/** // ---------------------------------------- 235 chars

void mainImage( out vec4 O,  vec2 U )
{
	vec2 R = iResolution.xy;
         U = (U+U-R)/R.y * 10.5;
	float L = length(U), l;
    
    O -= O;
    if (abs(U.x) > 10.5) return;
    
    U /= L > 8.5 ? 1. : 3.-L*L/36.;      // bubble
    
    l = length(2.*fract(U +.5)-1.);
    L = length(abs(ceil(U-.5))+1.);
    
    O +=   l<.7 ? 1.                     // inner
         : l<1. ? .03*L                  // mid
         :        .9-.1*L ;              // outer     
}

/**/
