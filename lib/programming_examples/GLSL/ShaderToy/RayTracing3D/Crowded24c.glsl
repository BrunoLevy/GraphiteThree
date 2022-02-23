//stage GL_FRAGMENT_SHADER
//import <GLUP/ShaderToy.h>

// using the base ray-marcher of Trisomie21: https://www.shadertoy.com/view/4tfGRB#
// variant from: https://www.shadertoy.com/view/MtKXDG

#define EPS .001 // smaller = sharper & more contrasted

#define r(v,t) { float a = (t)*T, c=cos(a),s=sin(a); v*=mat2(c,s,-s,c); }

void mainImage( out vec4 f, vec2 w ) {
    float T = iTime+20., 
        closest = 999.;
    vec2 R = iResolution.xy,
         m = iMouse.xy/R;
    vec4 p = vec4((w-.5*R)/R.y,-.5,0), d,c;      // init ray 
    p.z -= .4;                                   // view angle width / zoom
    if (length(m)>.01)  T = 30.*m.x; // mouse control preempts.
    r(p.xz,.13); r(p.yz,.2); r(p.xy,.1);         // camera rotations
  //d = p;                                       // ray dir = ray0-vec3(0) 
    d = normalize(p);
    //if (length(m)>.01) p.z = -30.*(1.-m.y), T = 30.*m.x; // mouse control preempts.
    //else p.z -= 60.;                             // camera distance.
    p.z = T; 
    f -= f;
   
    for (float i=1.; i>0.; i-=.01)  {
        
        vec4 //u = floor(p/18.+3.5), 
            t = p, ta;
            c = p/p*1.2;
	    float x=1e9,x1=1e9,x2=1e9;

#define smod(t,n)       ( mod(t+(n)/2., n) - (n)/2.  )
#define setbox(t,h,w,d)   abs(t) / vec4(h,w,d,1)
#define setboxU(t)        abs(t)
#define dbox(t,r)       ( max(t.x,max(t.y,t.z)) -(r) )
#define dsphere(t,r)    ( length((t).xyz) -(r)       )
#define dcyl(t,r)       ( length((t).xy ) -(r)       )
#define union(a,b)        min(a,b)
#define sub(a,b)          max(a,-(b))
        
        // r(t.xz,.13); r(t.yz,.2); r(t.xy,.1);         // object rotation

        // the object
        
        ta =  cos(t); x1 =  abs( .2+ (ta.x+ta.y+ta.z)/3.)-.005;// + smoothstep(1.,0.,p.z-T));      
        x1 = sub(x1,dsphere(smod(t/3.14    ,.5),.25) );
        x1 = sub(x1,dsphere(smod(t/3.14+.25,.5),.16) );

        ta =  sin(t); x2 =  abs( .2+ (ta.x+ta.y+ta.z)/3.)-.005;// + smoothstep(1.,0.,p.z-T));      
        x2 = sub(x2,dsphere(smod(t/3.14+.24,.5),.25) );
        x2 = sub(x2,dsphere(smod(t/3.14    ,.5),.16) );

        x = union(x1,x2);

             
        if(x<EPS)                           // hit ! 
            {   
                //c = .5+.5*cos(63.3*t);
 #define sort(v)  if (abs(v.z)>abs(v.y)) v.yz=v.zy;  if (v.y>v.x) v.xy=v.yx;  if (v.z>v.y) v.yz=v.zy;               
                if (x==x1) {
                	ta =  abs(cos(t)); sort(ta);  	
                	c = vec4(1.0, 0.2, 0.2, 1.0); // .5+.5*cos(63.3*t.xyxy);
                    //c = texture(iChannel0,ta.xy);
                } else {
                	ta =  abs(sin(t)); sort(ta);
                    // c = texture(iChannel1,ta.xy);
                    c = vec4(0.2, 0.2, 1.0, 1.0);
                }
                x = i+.01*(1.-abs(x)/EPS);  // decrease banding
                f = x*c; break;           // color texture + black fog 
            }
        p -= d*x;                           // march ray
     }

     //f = sqrt(f); // bright version
     f = pow(f, .3+.7*vec4(.5+.5*sin(iTime))); // I couldn't choose dark vs bright :-)
}

