//stage GL_FRAGMENT_SHADER
//import <GLUP/ShaderToy.h>

// 168 chars
#define L  ;for(float i=0.; i<=2.; i+=.1) f+= (i*i-i+.5)/2e2/abs(i*(u.y-u.x-i)+i-u.y);

void mainImage( out vec4 f, vec2 u ) {
    f-=f;
    u = fract(abs(2.*u/iResolution.y -vec2(1.8,1)))  // also interesting without either fract or abs
    L
    u = 1.-u 
    L

  // f*= abs(sin(u.y*1e2)); f.rb*=0.;   // old raster-screen fashion 
} 









/* // 179 chars
#define L(u)   (i*i-i+.5)/2e2/abs(i*(u.y-u.x-i)+i-u.y)

void mainImage( inout vec4 f, vec2 u ) {
    u = fract(abs(2.*u/iResolution.y -vec2(1.8,1)));  // also interesting without either fract or abs
    for (float i=0.; i<=2.; i+=.1)
        f += L(u) + L((1.-u));

  // f*= abs(sin(u.y*1e2)); f.rb*=0.;   // old raster-screen fashion 
} 
*/





/*    // 185 chars

//#define L(u)  f+=            dot(V,V)/4e2/abs(-i*(u-V).x-(u).y*V.x);
  #define L(u)  f+=     (2.*i*i-i-i+1.)/4e2/abs(i*(u.y-u.x-i)+i-u.y);

void mainImage( inout vec4 f, vec2 u ) {
    u = fract(abs(2.*u/iResolution.y -vec2(1.8,1)));

    for (float i=0.; i<=2.; i+=.1) {
        L(u) L((1.-u))
	}
    
    // f*= abs(sin(u.y*1e2)); f.rb*=0.;   // old raster screen fashion 
}
*/




/* //  252 - 237 - 198 chars

//#define L(u)  f+= v*(l-v)<0. ? 0. : smoothstep(.01,.0,abs(-i*(u-V).x-(u).y*V.x)/l);  // 252 chars 
  #define L(u)  f+= v*(l-v)<0. ? 0. : l/4e2/abs(-i*(u-V).x-(u).y*V.x);      //237 chars 
//#define L(u)  f+=            dot(V,V)/4e2/abs(-i*(u-V).x-(u).y*V.x);    // 198 chars (+ 1 comment below)

void mainImage( inout vec4 f, vec2 u ) {
    u = fract(abs(2.*u/iResolution.y -vec2(1.8,1)));

    for (float i=0.; i<=2.; i+=.1) {
      	vec2  V = vec2(1,0)-i;   // +.1*fract(iDate.w); // if you want some anim
        float l = dot(V,V), v = dot(u,V)+i*i;  // -- comment for the 198 chars version
        L(u) L((1.-u))
	}
    
    // f*= abs(sin(u.y*1e2)); f.rb*=0.;   // old raster screen fashion 
}
*/




/* // 269 chars

float D(vec2 u, vec2 A) {   // draw line A
    u.y += A.y; 
    float v = dot(u,A/=dot(A,A));
    return v-v*v<0. ? 0. :
       smoothstep(.01,.0,abs(u.x*A.y-u.y*A.x));  // should be /sqrt(l) but look same
    // 8e-4/abs(u.x*A.y-u.y*A.x)/l;  // should be /sqrt(l) but look same
}
 
void mainImage( inout vec4 f, vec2 u ) {
    u = fract(abs(2.*u/iResolution.y -vec2(1.8,1)));

    for (float i=0.; i<=2.; i+=.1) {
      	vec2  V = vec2(1,0)-i;   // +.1*fract(iDate.w); // if you want some anim
        f +=  D(u, V)+ D(1.-u, V);
    }

    // f*= abs(sin(u.y*1e2)); f.rb*=0.;   // old raster screen fashion 
}
*/

