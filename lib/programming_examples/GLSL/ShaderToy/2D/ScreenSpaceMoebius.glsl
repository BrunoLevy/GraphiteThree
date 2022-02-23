//stage GL_FRAGMENT_SHADER
//import <GLUP/ShaderToy.h>

// === Drawing transformed-space isolines using screen-space metrics ================
// Inspired by "Mobius Weave" by Shane. https://shadertoy.com/view/XtsBRS
// Cleaning and application here: https://www.shadertoy.com/view/llsfRj
// Conformal version here: https://www.shadertoy.com/view/MllBzj

#define UNIQ // if on, inverse gives one unique solution. Also, dist more robust !
#define S(v) smoothstep(2./iResolution.y, 0., v )

// --- direct transforms
vec2 Mobius(vec2 p, vec2 z1, vec2 z2)
{
	z1 = p - z1; p -= z2;
	return mat2(z1,z1.y,-z1.x) * p / dot(p, p);
}

vec2 spiralZoom(vec2 p, vec2 offs, float n, float spiral, float zoom, vec2 phase)
{
	p -= offs;
	float a = atan(p.y, p.x)/6.283 + iTime/32.;
	float d = length(p);
	return mat2(n,1, spiral,-zoom) * vec2(a, log(d)) + phase;
}

// --- inverse transforms
float k; // for tests
vec4 iMobius(vec2 p, vec2 z1, vec2 z2, float s)      // s = -1 or 1, 2 solution for each
{   float sb = 1.;
#ifdef UNIQ                                          // ... or if activating signs,
    s *= sign(p.x); sb = sign(p.y);                  // unique sol: s=1,x1,y1 otherwise 4
#endif
    float l = length(p), t = p.y/p.x,                // solve  u = v*l ; tan(u,v) = p.y/p.x
          c = s / sqrt(1.+t*t),                      // c = cos(atan( ) )
        v = length(z1-z2) / sqrt( 1.+ l*l -2.*l*c ), // c = (u²+v²-|z1z2|² ) / 2uv
        u = v*l;                                   
    vec2  a = 2.*(z1-z2);                            // solve |P-z1| = u ; |P-z2| = v
    float b = u*u-v*v + dot(z2,z2)-dot(z1,z1);       // ||²-||²: a.P + b = 0
    // y = -( b + a.x* x ) / a.y                     // normalize by a.y
    b /= a.y; a /= a.y;                              // ( in Shane example a.y was = 0 ! )
    float A = 1. +  a.x*a.x,                         //  |P-z1|² = u² , P = (x,y)
          B =     2.* b*a.x -2.*dot(vec2(1,-a.x),z1),
          C =           b*b +2.*b*z1.y + dot(z1,z1) - u*u,
          D = max(0., B*B - 4.*A*C),
         x1 = (-B+sb*sqrt(D))/(2.*A), y1 = -( b + a.x*x1 ),
         x2 = (-B-sb*sqrt(D))/(2.*A), y2 = -( b + a.x*x2 );
    k += 1./ (.1*abs(t)); // for tests
 // if (abs(A)<1e-5) { x1 = x2 = -C/B; y1 = y2 = -( b + a.x*x1 ) / a.y; } // degenerate case
	return vec4(x1,y1, x2,y2);                       // 2 solutions per s
}
// horizontal red/blue glitches at t ~ 0 ( or D ~ 0 ) -> solved by #UNIQ
// vertical red/blue glitches at t ~ inf ( or c ~ 0 )

vec2 ispiralZoom(vec2 p, vec2 offs, float n, float spiral, float zoom, vec2 phase)
{
    p = inverse(mat2(n,1, spiral,-zoom)) * (p-phase) ;
    p.x = (p.x - iTime/32.) * 6.283;             // ( p.x, p.y )  = ( a, log(d) )
    return exp(p.y) * vec2(cos(p.x),sin(p.x)) + offs;
}

// --- demo configuration 
vec2 Z1 = vec2(-.875, -.525),  // in Shane example Z1.y=Z2.y was causing a degenerescence
     Z2 = vec2(.375, -.125);

vec2 MobiusSpiral(vec2 q)      // total direct transform
{
    q = Mobius(q, Z1, Z2);
    return spiralZoom(q, vec2(-.5), 5., 3.14159*.2, .5, vec2(-1, 1)*iTime*.125);
}
float DrawInvMobiusSpiral(vec2 q, vec2 uv0, float r) // total inverse transform + draw iso-X
{
    vec4 v; float s = 0.;
    q = ispiralZoom(q, vec2(-.5), 5., 3.14159*.2, .5, vec2(-1, 1)*iTime*.125);
    v = iMobius(q, Z1, Z2, 1.);  
#ifdef UNIQ
    return S(length(uv0-v.xy) - r );
#else
    s += S(length(uv0-v.xy) - r ); // note that only the quadrant of uv0 contributes
    s += S(length(uv0-v.zw) - r ); // (but costly to 1st determine it: transfo + inverse + tests )
    v = iMobius(q, Z1, Z2, -1.);   
    s += S(length(uv0-v.xy) - r ); 
    s += S(length(uv0-v.zw) - r ); 
	return s;
#endif
}

void mainImage(out vec4 O, vec2 U)
{
	vec2 R = iResolution.xy,
        uv0 = (U -.5*R) / R.y,
    uv = MobiusSpiral(uv0);
     
    O -= O;
    vec2 p = uv*4., q;
  //O += .5*DrawInvMobiusSpiral(uv, uv0, .01 ); return; // check inverse ok ( = grey + white overlaps if !UNIQ )


    if ( mod(iTime,2.) > 1. ) {
        p = abs(fract(p+.5)-.5);                       // --- isolines in Moebius space
        O += .5*float(min(p.x,p.y)<.1);
        return;
    }
    
    // --- to get iso-X in screenspace, 
    //     take it in final space ( floor(...) ),  inverse it, and draw distance to the result.
    const int N = 2, dN = N/2-1;
    for (int i=0; i<N; i++) {
        //q = floor(p)/4.;
        q = vec2( floor(p.x+float(i-dN)), p.y ) / 4.;  // --- H isolines in screen space
        O.r += DrawInvMobiusSpiral(q, uv0, .005 );
    
        q = vec2( p.x, floor(p.y+float(i-dN)) ) / 4.;  // --- V isolines in screen space
        O.b += DrawInvMobiusSpiral(q, uv0, .005 );
    
        for (int j=0; j<N; j++) {
            q = floor(p+vec2(i-dN,j-dN) ) / 4.;        // --- dots in screen space
            O.g += DrawInvMobiusSpiral(q, uv0, .02 );
        }
    }
    //O += k/1e4; // for tests
} 
