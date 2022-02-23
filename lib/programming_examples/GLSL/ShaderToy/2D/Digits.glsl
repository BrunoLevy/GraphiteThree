//stage GL_FRAGMENT_SHADER
//import <GLUP/ShaderToy.h>

// compact version of  patriciogv shader https://www.shadertoy.com/view/XlXSz2
//  with the help of 834144373

#define R(v) mod(4e4*sin(dot(ceil(v),vec2(12,7))),10.)  // random

void mainImage( out vec4 o, vec2 u ) {
    o-=o;o.a=1.; // o = vec4(0,0,0,1);
    vec2 p = 6.*fract(u *= 24./iResolution.y) - .5;
    u.y += ceil(iDate.w*2.*R(u.xx));

    // char font decoding - see https://www.shadertoy.com/view/MlXXzH
    int i=int(p.y);
    i = ( abs(p.x-1.5)>1.1 ? 0:
    i==5? 972980223: i==4? 690407533: i==3? 704642687: i==2? 696556137:i==1? 972881535: 0 
        ) / int(exp2(30.-ceil(p.x)-3.*floor(R(u)))); // n=int(10.*R(u));
    i > i/2*2 ?  // 1:ink 0:paper 
        R(++u)<9.9 ? o++ : o=o.arrr : o;  // white, red or black
}


/* // 373

#define R(v) fract(4e4*sin(v.x*12.+v.y*7.))  // random

void mainImage( inout vec4 o, vec2 u ) {

    vec2 p = 6.*fract(u *= 24./iResolution.y) - vec2(1.5,.5);
    u = ceil(u); u.y += ceil(iTime*20.*R(u.xx));

    // char font decoding - see https://www.shadertoy.com/view/MlXXzH
    int i=int(p.y), b=int(exp2(float(29-int(p.x)-int(10.*R(u))*3))); // n=int(10.*R(u))
    i = ( p.x<0.||p.x>3.? 0:
    i==5? 972980223: i==4? 690407533: i==3? 704642687: i==2? 696556137:i==1? 972881535: 0 )/b;
    i > i/2*2 ?  // 1:ink 0:paper 
        R(++u)<.99 ? o++ : o=o.arrr : o;  // white, red or black
}

*/



/* // 413

#define R(v) fract(sin(v.x*12.+v.y*7.) * 4e4)


int D(vec2 p, int n) {
    int i=int(p.y), b=int(exp2(float(29-int(p.x)-n*3)));
    i = ( p.x<0.||p.x>3.? 0:
    i==5? 972980223: i==4? 690407533: i==3? 704642687: i==2? 696556137:i==1? 972881535: 0 )/b;
 	return i-i/2*2; 
}


void mainImage( inout vec4 o, vec2 i ) {

    vec2 p = fract(i*= 24./iResolution.y)*6. - vec2(1.5,.5);
    i = ceil(i);
    i.y += ceil(iTime*20.*R(i.xx));
    
    D(p, int(10.*R(i))) > 0 ? R((i+.1))<.99 ? o++ : o=o.arrr : o;
}

*/