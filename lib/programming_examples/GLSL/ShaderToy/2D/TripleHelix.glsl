//stage GL_FRAGMENT_SHADER
//import <GLUP/ShaderToy.h>
// variant of "helix 5b ( triple helix )" https://shadertoy.com/view/XdtfD8
//                    and "helix 4 (DNA)" https://www.shadertoy.com/view/XddBD8

void mainImage(out vec4 O, vec2 U) {
    
    float t = iTime/2.+11., l,a,d,v, ds = 1.,  // ds=.5 for better look
          r0 = 200., r1 = 20., n = 3.82,       // n = 24/2pi
          A = 2.38, H = 4.*cos(A/2.);
    mat2  R = mat2( sin(t+vec4(0,33,11,0)) );  
    vec3  q = iResolution.xyx,
          D = normalize(vec3(.3*(U+U-q.xy)/q.y, -1)).zxy,  // ray direction
          c, p // = 30./q;                     // marching point along ray 
               = vec3(-30.*(iTime-2.),-2,-15.); D.yz*=R;
    O-=O;
    for ( O++; O.x > 0. && t > .01 ; O-=.015*ds )
        q = p, //q.xz *= R, q.yz *= R,         // rotation (could be factored out loop on p,D)

        c = q, c.z += r0, a = atan(c.y,c.z), 
        q.z = length(c.zy)-r0, q.y = r0*a, q.x = mod(q.x-a*r0/6.3,r0)-r0/2.,// large helix
        c = q, c.x += r1, a = atan(c.z,c.x), 
        q.x = length(c.xz)-r1, q.z = r1*a, q.y = mod(q.y-a*r1/6.3,r1)-r1/2.,// medium helix

        l = length(q.xy), a = atan(q.y,q.x), d = a-q.z,
        d = min( abs( mod(d  ,6.28) -3.14), 
                 abs( mod(d-A,6.28) -3.14) ),  // double strand (~2pi/3)
        t = length( vec3( l-4., d, fract(n*a)-.5 ) ) - .3, // spheres along spring
        d = a - round(q.z*n)/n -A/2. -3.14 +.5/n,
        d = ( length(vec2( (fract(q.z*n-.5)-.5)/n, l*cos(d)-H ))-.05 )/n, //rods
        t = min( t, v=max(l-4.,d) ),           // bounded rods
        p += ds*t*D;                           // step forward = dist to obj

    if (t==v) O.rg *= .9; else O.gb *= .9;     // colored rods vs spheres
}
