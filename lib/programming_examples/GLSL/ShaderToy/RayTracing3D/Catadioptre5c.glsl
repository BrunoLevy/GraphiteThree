//stage GL_FRAGMENT_SHADER
//import <GLUP/ShaderToy.h>

// variant of https://shadertoy.com/view/MscfDr

#define C      max(max(a.x,a.y),a.z)
#define R(x,y) t = max(t, max(abs(q.x+q.y),abs(q.x-q.y)) /1.41 )  
#define O(a)   q=q0, q.a=b.a, t-=t, R(x,y), R(y,z), R(x,z) -7., v = max(v,-t)

void mainImage(out vec4 O, vec2 U) {
    
    float t = iTime,v;
    mat2  R = mat2( sin(t+vec4(0,33,11,0)) );
    vec3  q = iResolution,
          D = vec3(.3*(U+U-q.xy)/q.y, -1),    // ray direction
          p = 30./q, q0,a,b;                  // marching point along ray 
    O-=O;
    for ( O++; O.x > 0. && t > .01 ; O-=.015 )
        q = p,
        q.xz *= R, q.yz *= R,                 // rotation
        q0 = q,
        a = abs(q), v = C - 5.,               // cube
        a = abs(mod(q-1.5,3.)-1.5),
        v = max(v, 1.-C),                     // grid of hollow cubes
        b = mod(q0,20.)-10.,
        v = max(v,-t),
        O(x), O(y), O(z),                     // - octaedron pairs along x,y,z axis
        t = v,
        p += t*D;                             // step forward = dist to obj
}
