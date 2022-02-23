//stage GL_FRAGMENT_SHADER
//import <GLUP/ShaderToy.h>

/* Penrose variations, by mattz
   License Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License.

   My brother wondered if you could "fold" kite & dart tiles in half for a cool 3D
   look, and I made this to find out (spoiler: yes you can).

   This shader morphs between "flat", "pyramidal", and "folded" tiles before repeating. 

   To implement this, I mainly used pencil & paper, along with these Wikipedia links:

     - https://en.wikipedia.org/wiki/Penrose_tiling
     - https://en.wikipedia.org/wiki/Golden_triangle_(mathematics)

   Other cool Penrose tiling shaders:

     - https://www.shadertoy.com/view/XlXGWM (tomkh)
     - https://www.shadertoy.com/view/4t2XWG (tomkh)
     - https://www.shadertoy.com/view/XdXGDX (daniel_flassig)

   I tried to strive for readability below, feel free to request clarifications
   in the comments!

 */

// I used this when debugging the path, you can also use it to see the overall
// symmetry of the sun deflation
//#define DEBUG_PATH

// maximum recursion depth, fun to play with
const int MAX_DEPTH = 8;

// 1 / golden ratio
const float INVPHI = 0.6180339887498948;

// 36 degrees
const float PI5 = 0.6283185307179586;

// tile types
const int KITE = 0;
const int DART = 1;

const float GAMMA = 2.2;
const float INV_GAMMA = 1.0/GAMMA;

//////////////////////////////////////////////////////////////////////
// homogeneous 2D coordinates of a line passing through 2 points

vec3 line(vec2 x, vec2 y) {
    return cross(vec3(x, 1), vec3(y, 1));
}

//////////////////////////////////////////////////////////////////////
// offset a line by a distance

vec3 offset_line(vec3 l, float d) {
    l /= length(l.xy);
    l.z -= d;
    return l;
}

//////////////////////////////////////////////////////////////////////
// returns signed product indicating which side of line l x is on

float side(vec2 x, vec3 l) {
    return dot(vec3(x, 1), l);
}

//////////////////////////////////////////////////////////////////////
// given the 2D coordinates of a line l, and two 3D points a & b
// compute the intersection of the line from a to b with the plane
// that contains l and is perpendicular to the XY plane

vec3 intersect(vec3 l, vec3 a, vec3 b) {
   
    vec3 p = cross(l, line(a.xy, b.xy));
    p /= p.z;
    
    vec2 pa = p.xy-a.xy;
    vec2 ba = b.xy-a.xy;
    
    float u = dot(pa,ba)/dot(ba, ba);
    
    p.z = mix(a.z, b.z, u);
    
    return p;
    
}

//////////////////////////////////////////////////////////////////////
// split a half-kite into a half-dart and two half-kites.
// see penrose function below for illustration of half-tiles.

int half_kite(inout vec2 a, 
              inout vec2 b,
              inout vec2 c,
              in vec2 p) {
    
    vec2 x = mix(b, a, INVPHI);
    vec2 y = mix(a, c, INVPHI);
    
    vec3 lxy = line(x, y);
    
    if (side(p, lxy) * side(a, lxy) >= 0.) {
        
        b = y;
        c = x;
        
        return DART;
        
    } else {
        
        
        vec3 lby = line(b, y);
        
        if (side(p, lby) * side(c, lby) >= 0.) {
            
            a = b;
            b = c;
            c = y;

        } else {
            
            a = b;
            b = x;
            c = y;

        }
        
        return KITE;
                
    }
    
}

//////////////////////////////////////////////////////////////////////
// split a half-dart into a half-dart and a half-kite.
// see penrose function below for illustration of half-tiles

int half_dart(inout vec2 a, 
              inout vec2 b,
              inout vec2 c,
              in vec2 p) {
    
    vec2 x = mix(a, b, INVPHI);
    
    vec3 lxc = line(x, c);
    
    if (side(p, lxc) * side(b, lxc) >=0.) {
        
        a = b;
        b = c;
        c = x;
        return DART;
        
    } else {
        
        b = x;
        
        return KITE;
        
    }
    
}

//////////////////////////////////////////////////////////////////////
// mirrors p about the vector n if it lies in the other direction
// the vector n is modified to contain half its angle w.r.t. the x-axis
// returns the matrix M that accomplishes the flip
//
// really just 2D folding -- see https://www.shadertoy.com/view/4tX3DS 
// for a more intuitive explanation

mat2 flip(inout vec2 p, inout vec2 n) {
    
    float d = dot(p, n);
    
    mat2 M = mat2(1.) - 2.*outerProduct(n, n);
    
	n = normalize( vec2(n.x - 1., n.y) );
    
    if (d < 0.) {
        p = M * p;
        return M;
    } else {
        return mat2(1.);
    }
    
}

//////////////////////////////////////////////////////////////////////
// given an input point p sufficiently close to the origin,
// computes the coordinates a, b, c  of the half-dart or half-kite 
// triangle containing p and returns the type of triangle.
//
// we do this by repeated deflation of a "sun" pattern, see
// https://en.wikipedia.org/wiki/Penrose_tiling#Deflation_for_P2_and_P3_tilings
//
// the half-tile shapes look roughly like this:
//
//                 _-* b                            _-* b
//               _-   \                           _- /
//             _-      \                        _-  /
//           _-         \                     _-   /
//         _-            \                  _-    /
//       _-               \               _-     /
//     _-                  \            _-      /
//  a * - - - - - X - - - - * c      a *- - X -* c  
//
//            half-kite                half-dart
//
// in both cases, line segment ac is the line of symmetry for each tile,
// and edge ab has the same length for both half-tiles

int penrose(in vec2 p, out vec2 a, out vec2 b, out vec2 c) {
    
    // we start out with a single half-kite on the right side
    // of the Y axis
    int type = KITE;
    
    a = vec2(0);
    b = vec2(sin(PI5), cos(PI5));
    c = vec2(0, 1);

    // use 2D folding to mirror p along lines of symmetry
    // in the sun pattern until it lands inside the wedge
    // described by points b and c above
    
    vec2 n1 = vec2(-cos(4.*PI5), sin(4.*PI5));
    vec2 n2 = vec2(1, 0);

    mat2 M = flip(p, n2); // mirror across y-axis
    M = flip(p, n1) * M;  // 144 degree symmetry lines 
    M = flip(p, n1) * M;  // 72 degree symmetry lines
    M = flip(p, n1) * M;  // 36 degree symmetry lines

    // stop early if p is outside the central sun figure
    vec3 lbc = line(b, c);
    
    if (side(p, lbc) * side(a, lbc) < 0.) {
        return -1;
    }

    // by now, p should live inside the triangle abc, so do
    // a few iterations of deflation to subdivide triangles
    
    for (int i=0; i<MAX_DEPTH; ++i) {
        // precondition: p lives in abc
        if (type == KITE) {
            type = half_kite(a, b, c, p);
        } else {
            type = half_dart(a, b, c, p);
        }
        // postcondition: abc is updated with the sub-triangle containing p
    }
    
    // now undo whatever flips we did to p, to get the "real-world"
    // coordinates of a, b, and c so we can perform per-pixel lighting
    a = a * M;
    b = b * M;
    c = c * M;
    
    // return what type of triangle p is in
    return type;
    
}
  

//////////////////////////////////////////////////////////////////////
// return the surface normal of a triangle with vertices at a, b, c

vec3 trinormal(vec3 a, vec3 b, vec3 c) {
    return normalize( cross(b-a, c-a) );
}

//////////////////////////////////////////////////////////////////////
// compute a smoothed square wave

float smoothsquare(float t) {
    
	const float t_rise = 0.25;
    
    const float t_hi = 1.0;
    const float t_lo = 2.0;
    
    const float t_total = t_hi + t_lo;
    
    t = mod(t, t_total);
    
    if (t < t_hi) {
        
        return smoothstep(t_hi, t_hi-t_rise, t);
        
    } else {
        
        t -= t_hi;
        
        return smoothstep(t_lo - t_rise, t_lo, t);
    }
    
}

//////////////////////////////////////////////////////////////////////
// given a point p inside a half-dart or half-kite with vertices abc, 
// compute the surface normal of the triangle containing p

vec3 get_normal(vec2 p, int type, vec3 a, vec3 b, vec3 c) {
    
    // x is the point along line segment ac that bisects angle abc
    // see ASCII art above for location
    vec3 x = mix(a, c, INVPHI);
    
    vec3 lbx = line(b.xy, x.xy);
    
    float scl = length(a.xy-b.xy);
    float s = sign(side(a.xy, lbx));
    
	// we can treat triangles abx and xbc the same -- if p is on the
    // same side of line bx as c is, we replace a with c and flip 
    // the sign of the triangle determinant
    if (side(p, lbx) * s <= 0.) {
        a = c;
        s = -s;
    }
    
    // at this point just assume we are dealing with triangle abx 
    
    ////////////////////////////////////////////////////////////
    // now select some parameters affecting triangle geometry
    // 3 types of tile: flat, pyramid, folded    
    
    float t = iTime * 0.0625;
    
    for (int i=0; i<3; ++i) {
        if (true || texelFetch(iChannel1, ivec2(49+i, 0), 0).x > 0.) {
            t = float(i);
        }
    }
    
    // size of v-groove border between tiles (bigger for flat)
    float border =  0.05*scl * (1.0 + smoothsquare(t));

    // height of midpoint for "pyramid" type tiles
    float pyramid = 0.08*scl * smoothsquare(t - 1.0);

    // height of corners for "folding" type tiles
    float fold = 0.3*scl * smoothsquare(t - 2.0);

    
    ////////////////////////////////////////////////////////////
    // handle lifting up corners. 
    // note kites and darts are "folded" in opposite directions

    if (type == KITE) {
        a.z += fold;
        x.z += fold;
    } else {
        b.z += fold;
    }

    ////////////////////////////////////////////////////////////
    // compute the line that separates the border from the 
    // interior of the tile, and compute its intersections
    // with segments ax and bx
   
    vec3 lab = line(a.xy, b.xy);
    vec3 oab = offset_line(lab, s*border);
    
    vec3 d = intersect(oab, a, x);
    vec3 e = intersect(oab, b, x);
    
    ////////////////////////////////////////////////////////////
    // now raise up the interior points in z height
    
	d.z += border;
    e.z += border;
    x.z += border + pyramid;
    
    ////////////////////////////////////////////////////////////
	// finally, depending upon which side of the line we are on
    // compute the normal
    
    if (side(p, oab)*s >= 0.0) { 
        // inside border
        return s*trinormal(d, e, x);
    } else { 
        // outside border
        return s*trinormal(a, b, d);
    }

}

//////////////////////////////////////////////////////////////////////
// from Dave_Hoskins' hash without sine: 
// https://www.shadertoy.com/view/4djSRW

#define HASHSCALE3 vec3(.1031, .1030, .0973)

vec2 hash22(vec2 p) {
	vec3 p3 = fract(vec3(p.xyx) * HASHSCALE3);
    p3 += dot(p3, p3.yzx+19.19);
    return fract((p3.xx+p3.yz)*p3.zy);
}


//////////////////////////////////////////////////////////////////////
// compute wood grain texture for a tile whose centerline in world
// coordinates is given by ac

vec3 wood_grain(vec2 p, int type, vec2 a, vec2 c) {
       
    vec2 cs = normalize(c-a);
    mat2 R = mat2(cs.x, -cs.y, cs.y, cs.x);
    
    vec2 mid = 0.5*(a+c);
    
    p = R * (p - mid);
    
    p += hash22(mid * 8192.);
    
    vec3 color = vec3(0.7, 0.7, 0.7); //texture(iChannel0, 12. * p).xyz;
    
    if (type == KITE) {
        color = vec3(color.x + color.y, color.x - color.y, color.z) * 0.3;
    } else {
        color = mix(color, vec3(.7, .6, .2), 0.2);
    }
    
    return pow(color, vec3(GAMMA));

}

//////////////////////////////////////////////////////////////////////
// compute position along winding path

vec2 get_path_pos(float i) {
       
    float theta = i * 6.283185307179586;
    
    float r = 8.0*sin(6.0*theta) + 2.0;
    
    return 0.07 * r * vec2(cos(theta), sin(theta));
    
}

//////////////////////////////////////////////////////////////////////
// compute time-varying rotation matrix

mat2 get_rotation(float f) {

    float t = cos(f + 0.13) + 0.5 * cos(f*2.);
    vec2 fwd = vec2(cos(t), sin(t));
    
    return mat2(fwd.x, -fwd.y, fwd.y, fwd.x); 
    
}

/////////////////////////////////////////////////////////////////////////////
// our main function

void mainImage( out vec4 fragColor, in vec2 fragCoord ) {    


    ////////////////////////////////////////////////////////////
    // compute light coords
    
    vec2 mouse;
    
    if (max(iMouse.x, iMouse.y) > 20.0) {
        mouse = (iMouse.xy - 0.5*iResolution.xy)*1.25 + 0.5*iResolution.xy;
    } else {
        mouse = 0.5*iResolution.xy;
        float t = 0.25*iTime;
        mouse += vec2(cos(t+1.5707), cos(3.*t))*0.4*iResolution.xy;
    }
    
    vec3 l = normalize( vec3(mouse.xy - fragCoord.xy, 0.3*iResolution.y) );


    ////////////////////////////////////////////////////////////
    // compute 2D scene coords

#ifdef DEBUG_PATH
    float scl = 1.5 / iResolution.y;
    mat2 R = mat2(1.);
    vec2 path_pos = vec2(0);
#else
    float scl = (0.09 + cos(iTime*0.0815+0.3)*0.02) / iResolution.y;
    vec2 path_pos = get_path_pos(0.001*iTime);
    mat2 R = get_rotation(0.08*iTime);
#endif
    
    vec2 p = (fragCoord - 0.5*iResolution.xy) * scl;       

    p = (R * p) + path_pos;

    ////////////////////////////////////////////////////////////
    // get triangle type and vertices a, b, c
    
    vec2 a, b, c;
    
    int type = penrose(p, a, b, c);
    
    if (type < 0) {
        fragColor = vec4(0);
        return;
    }
       
    ////////////////////////////////////////////////////////////
    // compute triangle normal

    vec3 n = get_normal(p, type, vec3(a, 0), vec3(b, 0), vec3(c, 0));
    
    // rotate the normal back to counter camera rotation
    n.xy = n.xy * R;

    ////////////////////////////////////////////////////////////
    // per-pixel lighting
    
    vec3 color = wood_grain(p, type, a, c);
    
    color *= mix(dot(n, l), 1.0, 0.3);
    
    // specular
    vec3 v = normalize(vec3(fragCoord.xy - 0.5*iResolution.xy, 1.5*iResolution.y));
    vec3 r = reflect(l, n);
    color += 0.05*pow(clamp(-dot(r, v), 0.0, 1.0), 10.0);
    
    // gamma correct
    color = pow(color, vec3(INV_GAMMA));

#ifdef DEBUG_PATH    
    
    const float di = 1.0/200.0;
    
    for (float i=0.0; i<1.0; i+=di) {
        
        vec2 pi = get_path_pos(i);
        vec2 pj = get_path_pos(i+di);
      
        float u = dot(p-pi, pj-pi) / dot(pj-pi, pj-pi);
        vec2 pc = mix(pi, pj, clamp(u, 0.0, 1.0));
        
        color = mix(color, vec3(0.75,1,0), smoothstep(scl, 0.0, length(p-pc)-scl));
        
    }
    
#endif    
    
    fragColor = vec4(color, 1);
    
}