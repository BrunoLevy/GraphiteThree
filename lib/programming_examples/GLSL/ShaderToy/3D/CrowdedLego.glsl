//stage GL_FRAGMENT_SHADER
//import <GLUP/ShaderToy.h>



#define T iTime
#define r(v,t) { float a = (t)*T, c=cos(a),s=sin(a); v*=mat2(c,s,-s,c); }
#define SQRT3_2  1.26
#define SQRT2_3  1.732

// --- using the base ray-marcher of Trisomie21: https://www.shadertoy.com/view/4tfGRB#

vec4 bg = vec4(0); // vec4(0,0,.3,0); // vec4(0,0,.4,0);

void mainImage( out vec4 f, vec2 w ) {
    vec4 p = vec4(w,0,1)/iResolution.yyxy-.5, d,c; p.x-=.4; // init ray
     r(p.xz,.13); r(p.yz,.2); r(p.xy,.1);   // camera rotations
    d = p;                                 // ray dir = ray0-vec3(0)
    p = vec4(.3,.5,5,0)*T+d;
    vec2 mouse = iMouse.xy/iResolution.xy;
    float closest = 999.0;
    f = bg;
    float x1,x2,x3,l,x=1e9, R=4.;
   
    for (float i=1.; i>0.; i-=.01)  {
       
         vec4 u = floor(p/8.), t = mod(p, 8.)-4., ta; // objects id + local frame
      
        // r(t.xy,u.x); r(t.xz,u.y); // r(t.yz,.1);    // objects rotations
        u = sin(78.17*(u+u.yzxw));                     // randomize ids
        // t -= 4.*u;                                  // jitter positions
        
        c = p/p*1.2;
        ta = abs(t);
        // x = max(ta.x,max(ta.y,ta.z))  -3.8*(sin(17.*(u.x+u.y+u.z))>.95 ? 1. : -1. );
        x=1e9; 
        if (sin(17.*(u.x+u.y+u.z))>.95) { // 10% of big blocks
            x = max(ta.x,max(ta.y,ta.z))  -3.8; 
            u = floor(p/2.), ta = abs(mod(p, 2.)-2./2.);  u = sin(78.17*(u+u.yzxw));
            if (sin(17.*(u.x+u.y+u.z+floor(.3*T+u.x)))>.0)  // 50% of small blocks
               	x = max(x,-(max(ta.x,max(ta.y,ta.z))  -1.1));
         }
        
        // artifacts: passed a object, we might be fooled about dist to next (hidden in next modulo-tile)
#if 0        // if dist augment with depth, skip to end of modulo-box.
        ta = abs(t+.01*d);x1 = max(ta.x,max(ta.y,ta.z))  -2.8*(sin(17.*(u.x+u.y+u.z))>.9?1.:0.1);
        if (x1>x) { l = -(max(ta.x,max(ta.y,ta.z))  -4.); p+= 2.*d*(l+0.01); _i+=.0; continue; }
#endif
#if 1        // if dist to box border is closest to object, go there.  <<< the working solution ! (at mod8 scale)
		// l = -(max(ta.x,max(ta.y,ta.z))  -4.);
        vec4 k, k1 = (R-t)/d ,k2 = (-R-t)/d, dd; 
        k = min (k1-1e5*sign(k1),k2-1e5*sign(k2))+1e5; // ugly trick to get the min only if positive.
        // 2 less ugly/costly formulations, but less robust close to /0 :
        // k = mix(k1,k2, .5+.5*sign(k2));
        // dd = d+.001*clamp(1.-d*d,.999,1.); k = (R*sign(dd)-t)/dd;
        l = min(k.x,min(k.y,k.z));

        if (l<x) { p+= 1.*d*(l+0.01); continue; }
#endif
#if 0        // if falling inside object, backtrack to box entry.
        if (x<0.) {  
            // if (x<0.) { f=f-f; return; }
            vec4 k, k1 = -(4.-t)/d ,k2 = -(-4.-t)/d; 
            k = min (k1-999.*sign(k1),k2-999.*sign(k2))+999.;
            x = min(k.x,min(k.y,k.z))-.01;
            
            p -= 2.*d*x;  continue; 
        }
#endif
        // if (x<.01) c = mix(c,u,.5);
        // if (x<.01) c = mix(c,u,.5)*(sin(15.*T)*sin(2.7*T)*sin(1.2*T)>.5 ? 3. : 1.);
        if (x<.01) c = mix(c,u,.5)*(1.+.1/(.6+.5*sin(1.*(.01*p.x-3.*T)))*clamp(cos(T/8.),0.,1.));
      
        if(x<.01) // hit !
            { f = mix(bg,c,i*i); break;  }  // color texture + black fog
       
        p += d*x;       // march ray
     }
}

