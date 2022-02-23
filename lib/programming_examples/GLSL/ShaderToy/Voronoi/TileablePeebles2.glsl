//stage GL_FRAGMENT_SHADER
//import <GLUP/ShaderToy.h>

// inspired from simplified https://www.shadertoy.com/view/MtlyR8 (Voronoï with various metrics)
// + tilability, multiple seed per cell

#define C 50.  // cell size
#define B  2.  // number of blob per cell. B=1 -> Poisson disk. Large or random B -> Poisson
#define R  .5  // jittering blob location. .5 = anywhere in the tile. could be < or >
#define dr .5  // blob radius fluctuates in [1-r,1+r]
#define N  3   // tested neighborhood. Make it odd > 3 if R is high 

#define srnd(p)  ( 2.* fract(43758.5453*sin( dot(p, vec2(12.9898, 78.233) )      ) ) - 1. )
#define srnd2(U) ( 2.* fract(4567.89* sin(4567.8*(U)* mat2(1,-13.17,377.1,-78.7) ) ) - 1. )

void mainImage( out vec4 O, vec2 U )
{
    float H = iResolution.y,
          S = round(H/C);             // make number of cells integer for tilability
    U /= H;
    if (U.x > 1.) { O-=O; return; }   
 // U = fract(2.*U);                  // check tilability

    U *= S; 
    float m=1e9, v, r, r0=1e2;
    
    for (int k=0; k<N*N; k++)                  // neihborhood
        for (float i=0.; i<B; i++) {                // B blobs per cell
            vec2 iU = floor(U),
                  g = mod( iU + vec2(k%N,k/N)-1. , S),  // cell location within neighborhood
                  p = g+.5 + R* srnd2(g+i*.1) ;         // blob location
            
            p = mod( p - U +S/2. , S) - S/2.;           // distance to blob center
            r =  1. + dr* srnd(g+i*.1+.1);              // blob radius
            v = length(p) / r;                          // distance in blob referential
 
         // if (v < 1. && r < r0 ) m = v,r0 = r;        // smallest blob win
            if (v < m) m = v;                           // nearest win
    }
    
     O =   vec4( 1.-m*m );
}
