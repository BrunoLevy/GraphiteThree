//stage GL_FRAGMENT_SHADER
//import <GLUP/ShaderToy.h>

#define N 10.
void mainImage( out vec4 o, vec2 u ) {
    u = (u+u-(o.xy=iResolution.xy))/o.y;
    //u = 2.*(u / iResolution.y -vec2(.9,.5));
    float t = iTime,
          r = length(u), a = atan(u.y,u.x),
          i = floor(r*N);
    a *= floor(pow(128.,i/N)); 	 a += 20.*sin(.5*t)+123.34*i-100.*r*cos(.5*t); // (r-0.*i/N)
    r +=  (.5+.5*cos(a)) / N;    r = floor(N*r)/N;
	o = (1.-r)*vec4(.5,1,1.5,1);
}