//stage GL_FRAGMENT_SHADER
//import <GLUP/ShaderToy.h>

#define N 3   // tested neighborhood. Make it odd > 3 if R is high 
#define R .5  // jittering radius. .5 = anywhere in the tile. could be < or >

// rand 1D, rand 2D, signed rand
#define rnd(p)	fract(sin( dot(p, vec2(12.9898, 78.233) )       ) * 43758.5453 )
#define rnd2(p) fract(sin( (p) * mat2(127.1,311.7, 269.5,183.3) ) * 43758.5453 )
             // mat3( 127.1,311.7, 74.7, 269.5,183.3,246.1, 113.5,271.9,124.6))
#define srnd(p)  ( 2.*rnd(p) -1. )
#define srnd2(p) ( 2.*rnd(p) -1. )

void mainImage( out vec4 O, vec2 U )
{
    U /= 100.;
    float m=1e9,m2, c=1e2, v,w;
    
    for (int k=0; k<N*N; k++) {          // --- visit 3x3 neighbor tiles ------------------
        vec2 iU = floor(U)+.5,           // tile center 
              g = iU + vec2(k%N,k/N)-1., // neighbor cell
              p =  g+ srnd2(g)*R -U     // vector to jittered cell node
                 +.1*sin(iTime+vec2(1.6,0)+3.14*srnd(g)),         // time jittering
              q = p * mat2(1,-1,1,1)*.707;                        // pi/4 rotation
      //p *= 1.5+1.*srnd(g+.1);                                   // jittering scale
      //q = p * mat2(sin(iTime+3.14*srnd(g)+1.57*vec4(1,2,0,1))); // jittering rotation. NB don't compile if in declaration
        c = min(c,length(p));            // L2 distance to node (to display node)
        
                                         // --- choose distance kind ------------------
      //v = length(p);                   // L2 distance
      //v = abs(p.x)+abs(p.y);           // L1 distance
        p = abs(p); v = max(p.x,p.y);    // Linfinity distance ( = Manhattan = taxicab distance)
      //q = abs(q); w = max(q.x,q.y);    // rotated version
      //v = w;
      //v = max(v, w);                   // octogonal distance
        if (v < m) m2 = m, m = v;        // keep 1st and 2nd min distances to node
        else if (v < m2) m2 = v;        
    }
    
  //v = m;                               // custom distance to node
    v = m2-m;                            // distance to voronoi diagram

    int t = int(2.+iTime/2.)%7;         // --- coloring scheme ------------------
    O =   t==0 ? vec4(0,m2,m,0)          // display 1st and 2nd min distances
        : t==1 ? vec4(sin(30.*v))        // iso-lines
        : t==2 ? vec4( 1.-v )            // raw distance
        : t==3 ? vec4(.025,.05,.1,0)/v   // Voronoi diagram + chromatic effect
        : t==4 ? vec4(smoothstep(0.,2.,100.*v)) // Voronoi contours
        : t==5 ? vec4(sin(30.*m))        // cell iso-lines
        : t==6 ? vec4( 1.-m )            // cell raw distance
        : O;

    O.r +=  .03/c;                        // debug: display nodes
  //U=fract(U); O.b += .01/min(U.x,U.y);  // debug: display tiles
}
