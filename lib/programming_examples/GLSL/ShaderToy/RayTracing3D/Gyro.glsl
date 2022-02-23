//stage GL_FRAGMENT_SHADER
//import <GLUP/ShaderToy.h>

// using the base ray-marcher of Trisomie21: https://www.shadertoy.com/view/4tfGRB#

#define T iTime
#define r(v,t) { float a = (t)*T, c=cos(a),s=sin(a); v*=mat2(c,s,-s,c); }
#define SQRT3_2  1.26
#define SQRT2_3  1.732
#define smin(a,b) (1./(1./(a)+1./(b)))

void mainImage( out vec4 f, vec2 w ) {
    vec4 p = vec4(w,0,1)/iResolution.yyxy-.5, d,c; p.x-=.3; // init ray 
    //    r(p.xz,.13); r(p.yz,.2); r(p.xy,.1);   // camera rotations
    d = p;                                 // ray dir = ray0-vec3(0)
    p.z -= 5. ; // *T;
    vec2 mouse = iMouse.xy/iResolution.xy;
    float closest = 999.0;
    f = vec4(0);
    
    for (float i=1.; i>0.; i-=.01)  {
        
        //vec4 u = floor(p/8.), t = mod(p, 8.)-4., ta; // objects id + local frame
        vec4 u=floor(p/18.+3.5), t = p, ta,v;
        // r(t.xy,u.x); r(t.xz,u.y); r(t.yz,1.);    // objects rotations
        u = sin(78.*(u+u.yzxw));                    // randomize ids
        // t -= u;                                  // jitter positions
        c = p/p*1.2;
 
        float x1,x2,x=1e9;
        for (float j=2.5; j>1.; j-= .1) {
            r(t.xy,u.x); r(t.xz,u.y); r(t.yz,u.z);
            // t -= 2.*u;   // try ;-)
            ta = abs(t);
            x1 = length(t.xyz) -j*SQRT3_2*1.1;       // inside carving sphere
            x2 = max(ta.x,max(ta.y,ta.z)) -j;        // cube
            x2 = max(-x1,x2);                        // cube-sphere
            x1 = length(t.xyz) -j*SQRT3_2*1.1-.03;   // outside carving sphere
            if (p.z<-5.5+9.*mouse.x) break; // mouse.cut
            if (p.z>-1. && x1 > .1) break;  // culling. save 50% cost.
            x2 = max(x1,x2);                         // shape inter sphere
#define cyl(a,b,c) {v = vec4(a,b,c,0)/SQRT2_3; x1 = length(t.xyz-dot(t,v)*v.xyz)-.2*j; x2 = max(-x1,x2);}            
            cyl( 1, 1, 1); cyl( 1, 1,-1);  
            cyl( 1,-1, 1); cyl( 1,-1,-1);
            x =  min(x,x2);                          // union with the others
        }

        x1 = length(t.xyz) -.6;                      // central sphere
        closest = min(closest, x1); 
        x = min(x1,x);
        if (x==x1) c  = vec4(2.,.3,0,0);
        if (cos(.25*T)>0.) c += vec4(2.,.3,0,0)*pow(abs((x-x1)),.2)*(.5+.5*cos(.5*T));  // thanks squid !
        
        // f = i*i*c;      // color texture + black fog 

        if(x<.01) // hit !
            { f = i*i*c; break;  }  // color texture + black fog 
        
        p -= d*x;           // march ray
     }
     f += vec4(1,0,0,0) * exp(-closest)*(.5+.5*cos(.5*T)); // thanks kuvkar ! 
}

