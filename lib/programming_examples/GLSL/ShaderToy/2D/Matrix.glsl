//stage GL_FRAGMENT_SHADER
//import <GLUP/ShaderToy.h>

// the 2-tweets version of patriciogv's Matrix  https://www.shadertoy.com/view/MlfXzN 

// 255   ( -21 with the slight look-changing suggestions in comments )



#define r(s) fract(43.*sin(s.x*13.+s.y*78.))

void mainImage(out vec4 o, vec2 i){
    vec2 j = fract(i*=50./iResolution.x), 
         p = i-j+ vec2(2,floor(iTime*20.*fract(sin(i-j).x)));   // iDate.w: -4 chars
    i = abs(j-.5);
    o =  vec4(r(floor(p*23.+5.*j))>.5&&i.x<.3&&i.y<.45 ?   1. - r(p)*(2.-dot(i,i)*6.)  :  1.);
 // o +=  r(floor(p*23.+5.*j))>.5&&i.x<.3&&i.y<.45 ?   1. - r(p):  1.;  // -17 chars
}








/* // 258
#define r(s) fract(43.*sin(s.x*13.+s.y*78.))

void mainImage(inout vec4 o, vec2 i){
    vec2 p = floor(i*= 50./iResolution.x), j=i-p; i=abs(j-.5);
    p += vec2(2,floor(iTime*20.*fract(sin(p.x)))); 
    o +=  r(floor(p*23.+5.*j))>.5 && i.x<.3&&i.y<.45 ? 1. - r(p)*(2.-dot(i,i)*6.)  :  1.;
}
*/


/* // 270
#define r(s) fract(43.*sin(s.x*13.+s.y*78.))

void mainImage(inout vec4 o, vec2 i){
	i *= 50./iResolution.x;
    vec2 p = floor(i); i -= p; 
    p += vec2(2,floor(iTime*20.*fract(sin(p.x)))); 
	o +=  r((p*23.+floor(5.*i)))>.5 ? r(p) : 0.;
    i=abs(i-.5); 
    o = 1.- o *  (2.-dot(i,i)*6.) * (i.x<.3&&i.y<.45?1.:0.);
}
*/


/*  // 273
#define r(s) fract(43.*sin(s.x*13.+s.y*78.))

void mainImage(inout vec4 o, vec2 i){
	i *= 50./iResolution.x;
    vec2 p = floor(i); i -= p; 
    p += vec2(2,floor(iTime*20.*fract(sin(p.x)))); 
	o +=  r(p) * step(.5,r((p*23.+floor(5.*i))));
    i=abs(i-.5); 
    o = 1.- o *  (2.-dot(i,i)*6.) * (i.x<.3&&i.y<.45?1.:0.);
}
*/