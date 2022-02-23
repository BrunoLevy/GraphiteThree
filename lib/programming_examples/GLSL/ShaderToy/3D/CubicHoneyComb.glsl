//stage GL_FRAGMENT_SHADER
//import <GLUP/ShaderToy.h>

 ////////////////////////////////////////////////////////////////////////////////
//
// Created by Matthew Arcus, 2018.
//
// Cubic honeycombs using the Wythoff construction.
//
// 'm': smooth transition between Wythoff symbols
// 'a': control Wythoff symbol with keys '1'-'4'
// 'i': invert in origin
// 'c': translate Wythoff point to origin (before inversion)
//
////////////////////////////////////////////////////////////////////////////////

bool invert = false;
bool centre = false;
bool morph = true;

const float scale = 1.0;

vec4 tri0 = vec4(1,1,1,1);

const float fudge = 0.5; // Try to cope with non-linearity of inversion

const float sscale = 1.0;
const float swidth = sscale*0.05;
const float swidth2 = swidth*swidth;
const float twidth = sscale*0.02;
const float twidth2 = twidth*twidth;

const float PI =  3.141592654;
const float TWOPI =  2.0*PI;

//-------------------------------------------------
//From https://www.shadertoy.com/view/XtXGRS#
vec2 rotate(in vec2 p, in float t) {
  return p * cos(-t) + vec2(p.y, -p.x) * sin(-t);
}

// Setup folding planes and vertex

// Vertices of fundamental region
const vec3 p0 = vec3(0,0,0);
const vec3 q0 = vec3(1,0,0);
const vec3 r0 = vec3(1,1,0);
const vec3 s0 = vec3(1,1,1);

// The reflecting planes
// Also the directions of the edges from the fundamental point
const float K = 1.41421356;
const vec3 P = vec3(-1, 0, 0); // Distance 1 from origin
const vec3 Q = 0.5*K*vec3(1,-1, 0);
const vec3 R = 0.5*K*vec3(0, 1,-1);
const vec3 S = vec3(0, 0, 1);

vec4 tri2bary = vec4(1,K,K,1);
vec3 a; // Wythoff point

void init(vec4 tri) {
  tri *= tri2bary;
  a = tri[0]*p0 + tri[1]*q0 + tri[2]*r0 + tri[3]*s0;
  a /= dot(tri,vec4(1));
}

vec3 fold(vec3 p, out float iscale) {
  iscale = 1.0;
  if (invert) {
    float r2 = dot(p,p);
    p /= r2;
    iscale = fudge*r2;
  }
  if (centre) p = p+a; // Centre on Wythoff point rather than origin (ie. one of the polyhedra)

  // Fold in to unit cube
  // Make sure origin goes to origin
  p = mod(1.0+p,2.0)-1.0;
  p = abs(p);
  // Now flip into main tetrahedron
  p -= 2.0*min(0.0,dot(p,Q))*Q;
  p -= 2.0*min(0.0,dot(p,R))*R;
  p -= 2.0*min(0.0,dot(p,Q))*Q;
  return p;
}

float scene(vec3 p) {
  float d = 1e8;
  p -= a; // Centre on Wythoff point
  float dP = dot(p,P), dQ = dot(p,Q), dR = dot(p,R), dS = dot(p,S);
  d = min(d,length(p)-swidth); // The vertex
  d = min(d,max(dP,length(p-dP*P)-twidth)); // The four edges
  d = min(d,max(dQ,length(p-dQ*Q)-twidth));
  d = min(d,max(dR,length(p-dR*R)-twidth));
  d = min(d,max(dS,length(p-dS*S)-twidth));
  return d;
}

float eval(vec3 p) {
  float iscale;
  p *= scale;
  p = fold(p,iscale);
  float d = scene(p);
  d *= iscale;
  d /= scale;
  return d;
}

vec3 calcNormal(in vec3 p) {
  const vec2 e = vec2(0.0001, 0.0);
  return normalize(vec3(eval(p + e.xyy) - eval(p - e.xyy),
                        eval(p + e.yxy) - eval(p - e.yxy),
                        eval(p + e.yyx) - eval(p - e.yyx)));
}

float march(in vec3 ro, in vec3 rd) {
  const float maxd = 25.0;
  const float precis = 0.0002;
  float h = precis * 2.0;
  float t = 0.0;
  float res = 1e8;
  for (int i = 0; i < 200; i++) {
    if (h < precis || t > maxd) break;
    h = eval(ro + rd * t);
    // When inverting, limit step size to prevent
    // overshoot when coming in from a distance.
    if (invert) h = min(h,0.5);
    t += h;
  }
  if (t < maxd) res = t;
  return res;
}

vec3 transform(in vec3 p) {
  if (iMouse.x > 0.0) {
    float theta = -(2.0*iMouse.y-iResolution.y)/iResolution.y*PI;
    float phi = -(2.0*iMouse.x-iResolution.x)/iResolution.x*PI;
    p.yz = rotate(p.yz,phi);
    p.zx = rotate(p.zx,phi);
  }
  float t = iTime;
  p.yz = rotate(p.yz,t * 0.125);
  p.zx = rotate(p.zx,t * 0.2);
  return p;
}

vec3 getcolor(vec3 p) {
  float iscale;
  p *= scale;
  p = fold(p,iscale);
  p -= a; // Centre on Wythoff point
  float dP = dot(p,P), dQ = dot(p,Q), dR = dot(p,R), dS = dot(p,S);
  float d0 = length(p)-swidth; // The vertex
  float d1 = max(dP,length(p-dP*P)-twidth); // The four edges
  float d2 = max(dQ,length(p-dQ*Q)-twidth);
  float d3 = max(dR,length(p-dR*R)-twidth);
  float d4 = max(dS,length(p-dS*S)-twidth);
  int colindex = 0;
  float d = d0;
  if (d1 < d) { colindex = 1; d = d1; }
  if (d2 < d) { colindex = 2; d = d2; }
  if (d3 < d) { colindex = 3; d = d3; }
  if (d4 < d) { colindex = 4; d = d4; }
  if (colindex == 1) return 0.3*vec3(0,1,0);
  if (colindex == 2) return 0.6*vec3(0,0,1);
  if (colindex == 3) return 0.6*vec3(1,1,0);
  if (colindex == 4) return 0.3*vec3(1,0,1);
  return vec3(1,0,0);
}

const int CHAR_0 = 48;
const int CHAR_1 = 49;
const int CHAR_2 = 50;
const int CHAR_3 = 51;
const int CHAR_4 = 52;
const int CHAR_A = 65;
const int CHAR_C = 67;
const int CHAR_I = 73;
const int CHAR_M = 77;

bool keypress(int code) {
#if __VERSION__ < 300
  return false;
#else
  return texelFetch(iChannel0, ivec2(code,2),0).x != 0.0;
#endif
}

int imod(int n,int m) {
#if __VERSION__ >= 300
  return n%m;
#else
  return n - n/m*m;
#endif
}

vec4 gettri0(int i) {
#if __VERSION__ >= 300
  i += 9;
  i = 1+i%15;
  i = i^(i/2);
#else
  i += 14;
  i = 1+imod(i,15);
#endif
  return vec4(imod(i,2),imod(i/2,2),imod(i/4,2),imod(i/8,2));
}

vec4 gettri() {
  float t = 0.5*iTime;
  int i = int(t);
  float k = 0.0;
  if (morph) k = fract(t);
  return mix(gettri0(i),gettri0(i+1),k);
}

void mainImage( out vec4 fragColor, in vec2 fragCoord ) {
  invert = !keypress(CHAR_I);
  centre = !keypress(CHAR_C);
  morph = !keypress(CHAR_M);
  vec2 p = (2.0*fragCoord.xy - iResolution.xy) / iResolution.y;
  float t = iTime/TWOPI;
  if (keypress(CHAR_A)) {
    tri0 = vec4(int(!keypress(CHAR_1)),int(!keypress(CHAR_2)),
                int(!keypress(CHAR_3)),int(!keypress(CHAR_4)));
  } else {
    tri0 = gettri();
  }
  init(tri0);
  vec3 col = vec3(0.3 + p.y*0.1);
  vec3 ro = vec3(0, 0, 2.0);
  vec3 rd = normalize(vec3(p, -2.0));
  vec3 li = normalize(vec3(0.5, 0.8, 3.0));
  ro = transform(ro);
  if (iMouse.y > 0.0) {
    ro *= 2.0*iMouse.y/iResolution.y;
  }
  rd = transform(rd);
  li = transform(li);

  float k = march(ro,rd);
  if (k < 1e8) {
    vec3 pos = ro + k*rd;
    vec3 n = calcNormal(pos);
    col = 0.3+0.7*getcolor(pos);
    float diffuse = clamp(dot(n, li), 0.0, 1.0);
    col *= diffuse;
    if (invert) {
      //col = mix(vec3(1,1,0.5), col, smoothstep(0.0,0.2,length(pos)));
      col = mix(vec3(0), col, smoothstep(0.0,0.2,length(pos)));
    }
  }
  //col = mix(col, 0.5*vec3(1,1,0.5), clamp(k/40.0,0.0,1.0));
  col = mix(col, 0.1*vec3(0,0,1), clamp(k/25.0,0.0,1.0));
  //col = pow(col, vec3(0.4545));
  fragColor = vec4(col, 1.0);
}

