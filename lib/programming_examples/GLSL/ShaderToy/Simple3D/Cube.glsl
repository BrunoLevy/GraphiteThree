//stage GL_FRAGMENT_SHADER
//import <GLUP/ShaderToy.h>

// reduction by Coyote: 262
                                                 // draw segment [a,b]
#define L  *I ; o+= 3e-3 / length( clamp( dot(u-a,v=b-a)/dot(v,v), 0.,1.) *v - u+a )
#define P  ; b=c= vec2(r.x,1)/(4.+r.y) L;   b=a L;   a=c L;   a=c; r= I*r.yx;

void mainImage(out vec4 o, vec2 v) {
	vec2 I=vec2(1,-1), a,b,c=iResolution.xy, 
         u = (v+v-c)/c.y,
         r = sin(iTime-.8*I); r += I*r.yx  // .8-.8*I = vec2(0,1.6)
    P  o-=o        // just to initialize a
	P P P P        // 4*3 segments
}
