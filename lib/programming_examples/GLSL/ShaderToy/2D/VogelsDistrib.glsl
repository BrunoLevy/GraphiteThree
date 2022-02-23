//stage GL_FRAGMENT_SHADER
//import <GLUP/ShaderToy.h>

// compacification of  fernlightning's shader https://www.shadertoy.com/view/XtsSWM
// with the help of coyote and fernlightning (see fernlightning's shader forum for compaction steps)

void mainImage(out vec4 o, vec2 i) {

    o = iResolution.xyyy;
    i += i-o.xy;
    float r = length(i/=o.y/1e2)-2.6, 
          v = floor(r*r*.06);  // floor(...-.5) to add one circle at center
          r+=5.2; 

    for (int k = 0; k < 128; k++)
        v++ < r*r*.06 ? 
          o = min(o, length( sqrt(v/.06) * cos(v*2.4+vec2(1.57,0)) -i ) )
        : o ;
     o = 1.-abs(o-2.);   // or o = abs(o-2.); for reverse video (-3 chars)
}




/* // 248 chars
#define I(r) (r)*(r)*6e2-.5

void mainImage(out vec4 o, vec2 i) {

    o = iResolution.xyzz;
    i += i-o.xy;
    float r = length(i/=o.y), v=floor(I(r-.026))+.5;
    
    for (int k = 0; k < 128; k++)
        v++ < I(r+.026) ?
          o = min(o, length( sqrt(v/6e2) * cos(v*2.4+vec2(1.57,0)) -i ) )
        : o ;
     o = 1.-abs(1e2*o-2.);
}
*/