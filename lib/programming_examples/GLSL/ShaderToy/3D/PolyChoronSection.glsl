//stage GL_FRAGMENT_SHADER
//import <GLUP/ShaderToy.h>

////////////////////////////////////////////////////////////////////////////////
//
// Created 2018 by Matthew Arcus
//
// Sections of a Polychoron (an omnitruncated 120-cell, I believe).
//
// https://en.wikipedia.org/wiki/Runcinated_120-cells
//
////////////////////////////////////////////////////////////////////////////////

#define NOKEYS (__VERSION__ < 300)
//#define NOKEYS 1

bool doinvert = false;

const vec3 Face0Color = vec3(1,0,0);
const vec3 Face1Color = vec3(0,1,0);
const vec3 Face2Color = vec3(0,0,1);
const vec3 Face3Color = vec3(1,1,0);
const vec3 Face4Color = vec3(1,0,1);
const vec3 Face5Color = vec3(0,1,1);
const vec3 EdgeColor = vec3(0.1,0.1,0.1);

const float PI	= 3.14159265359;
const float TWOPI = 2.0 * PI;

//#define DEBUG
#if !defined DEBUG
#define assert(x) 0
#else
bool alert = false;
void assert(bool test) {
  if (!test) alert = true;
}
// Approximate equality. Used for assertion checks.
bool eq(float a, float b) {
  return abs(a-b) < 1e-4;
}
#endif

vec4 qmul(vec4 p, vec4 q) {
  vec3 P = p.xyz, Q = q.xyz;
  return vec4(p.w*q.w-dot(P,Q),p.w*Q+q.w*P+cross(P,Q));
}

// Given mirror planes PQRS, angles are PQ,QR,RP,PS,QS,RS
// Pick your symmetry group
#if 0
// Hypercube
const int PQ = 4, QR = 3, RP = 2;
const int NFOLDS = 4;
#elif 0
// 24-cell
const int PQ = 3, QR = 4, RP = 2;
const int NFOLDS = 6;
#else
// 120- & 600-cell
const int PQ = 5, QR = 3, RP = 2;
const int NFOLDS = 15;
#endif

const int PS = 2, QS = 2, RS = 3;

// Normals to mirror planes
vec4 planes[4];

// p is the "tetrahedron point"
vec4 tpoint;

// Dihedral angle between two faces with normals n,m
// Assume the normals are pointing inwards
float dihedral(vec4 n, vec4 m) {
  return PI - acos(dot(n,m));
}

float dihedral(int x) {
  return -cos(PI/float(x));
}

void init(vec4 quad) {
  // Convert parameters to actual dihedral angles
  float A = dihedral(PQ);
  float B = dihedral(QR);
  float C = dihedral(RP);
  float D = dihedral(PS);
  float E = dihedral(QS);
  float F = dihedral(RS);
  // Now construct 4 mirror planes satisfying those constraints
  vec4 P = vec4(1,0,0,0);
  // Q = [a,b,0,0], P.Q = A
  float a = A;
  assert(a*a < 1.0);
  float b = sqrt(1.0 - a*a);
  vec4 Q = vec4(a,b,0,0);
  // R = [c,d,e,0]
  // R.P = C = c
  // Q.R = B = ac + bd
  float c = C, d = (B-a*c)/b;
  assert(c*c + d*d < 1.0);
  float e = sqrt(1.0 - c*c - d*d);
  vec4 R = vec4(c,d,e,0);
#if __VERSION__ >= 300
  // The easy way
  mat3 mm = inverse(mat3(P.xyz,Q.xyz,R.xyz));
  vec3 S3 = vec3(D,E,F)*mm;
#else
  vec3 X = cross(Q.xyz,R.xyz);
  vec3 Y = cross(R.xyz,P.xyz);
  vec3 Z = cross(P.xyz,Q.xyz);

  // S3 = iX + jY + kZ
  // P.S = P.S3 = D = iP.X
  // Q.S = Q.S3 = E = jQ.Y
  // R.S = R.S3 = F = kR.Z
  
  float i = D/dot(P.xyz,X);
  float j = E/dot(Q.xyz,Y);
  float k = F/dot(R.xyz,Z);

  vec3 S3 = i*X+j*Y+k*Z;
#endif
  // We might have dot(u,u) = 1, in which case the fundamental region
  // is 3 dimensional and we have an infinite 3d honeycomb. We don't
  // do honeycombs here so exclude this case.
  assert(dot(S3,S3) < 1.0-1e-4);

  float w = sqrt(1.0 - dot(S3,S3));
  vec4 S = vec4(S3,w);
  assert(eq(dihedral(P,Q),PI/float(PQ)));
  assert(eq(dihedral(Q,R),PI/float(QR)));
  assert(eq(dihedral(R,P),PI/float(RP)));
  assert(eq(dihedral(P,S),PI/float(PS)));
  assert(eq(dihedral(Q,S),PI/float(QS)));
  assert(eq(dihedral(R,S),PI/float(RS)));

  planes[0] = P;
  planes[1] = Q;
  planes[2] = R;
  planes[3] = S;
  
  mat4 m = inverse(mat4(P,Q,R,S));

  // Now I want to solve the "quadriplanar" equations:
  // given vec4(x,y,z,w), find p with:
  // p.P = x, p.Q = y, p.R = z, p.S = w
  tpoint = normalize(quad*m); // Note order!
}

vec4 fold(vec4 pos) {
  for (int i = 0; i < NFOLDS; i++) {
    //pos -= 2.0*min(0.0,dot(pos,P))*P;
    pos.x = abs(pos.x);
    for (int j = 1; j < 4; j++) {
      pos -= 2.0*min(0.0,dot(pos,planes[j]))*planes[j];
    }
  }
  return pos;
}

// Perpendicular distance from p to segment a in direction r
// r should be normalized
float segment(vec4 p, vec4 a, vec4 r) {
  vec4 pa = p - a;
  float h = min(0.0,dot(pa, r));
  float d = length(pa - h*r);
  return d;
}

// Perpendicular distance from p to plane spanned by R and S
float pdist(vec4 p, vec4 R, vec4 S) {
  // p = aR + bS + X where R.X = S.X = 0
  // p.R = aR.R + bS.R
  // p.S = aR.S + bS.S
  mat2 m = inverse(mat2(1,dot(R,S),dot(R,S),1));
  vec2 t = vec2(dot(p,R),dot(p,S))*m;
  float a = min(0.0,t.x), b = min(0.0,t.y);
  vec4 X = p - a*R - b*S;
  return length(X);
}

vec4 lquat, rquat;

void qinit() {
  vec3 laxis = vec3(1,0.5,1);
  vec3 raxis = vec3(1,1,0.5);
  float t = iTime + 1.0;
  float ltheta = 0.1*t;
  float rtheta = 0.123*t;
  lquat = normalize(vec4(sin(ltheta)*laxis,cos(ltheta)));
  rquat = normalize(vec4(sin(rtheta)*raxis,cos(rtheta)));
}

vec3 transform(in vec3 p);

vec4 iproject(vec3 pos) {
  float t = iTime;
  // Slice parallel to z-plane.
  float k = 0.95*sin(0.1*t);
  vec4 pos4 = vec4(pos,k);
  // Arbitrary rotations just look a mess so just return
  // the slice position. More can be done here I expect.
  return pos4;
  // Two sided quaternion multiplication
  // All R4 rotations can be constructed from
  // a quaternion pair.
  //return qmul(lquat,qmul(pos4,rquat));
  // One sided quaternion multiplication
  //return qmul(pos4,rquat);
}

float face(vec4 p, vec4 q, vec4 R, vec4 S) {
  // Perpendicular distance from p to the plane containing q
  // and spanned by R and S, ie. points q + aR + bS
  // Want perp distance to (R,S) plane, then subtract perp distance of q
  return pdist(p-q,R,S);
}

float scene(vec4 pos4) {
  float d = 1e8;
  pos4 = fold(pos4);
  float fwidth = 0.01;
  float ewidth = 0.02;
  float pwidth = 0.04;
  for (int i = 0; i < 4; i++) {
    d = min(d,segment(pos4,tpoint,planes[i])-ewidth);
    for (int j = i+1; j < 4; j++) {
      d = min(d,face(pos4,tpoint,planes[i],planes[j])-fwidth);
    }
  }
  return d;
}

vec3 color(vec4 pos4) {
  float k,d = 1e8;
  pos4 = fold(pos4);
  float fwidth = 0.01;
  float ewidth = 0.02;
  float pwidth = 0.04;
  int col = -1;
  for (int i = 0; i < 4; i++) {
    d = min(d,segment(pos4,tpoint,planes[i])-ewidth);
  }
  int icol = 0;
  for (int i = 0; i < 4; i++) {
    for (int j = i+1; j < 4; j++,icol++) {
      float k = face(pos4,tpoint,planes[i],planes[j])-fwidth;
      if (k < d) { d = k; col = icol; }
    }
  }
  if (col == 0) return Face0Color;
  if (col == 1) return Face1Color;
  if (col == 2) return Face2Color;
  if (col == 3) return Face3Color;
  if (col == 4) return Face4Color;
  if (col == 5) return Face5Color;
  return EdgeColor;
}

float polyhedron(vec3 pos) {
  float k = 1.0;
  if (doinvert) k = dot(pos,pos);
  pos /= k;
  vec4 pos4 = iproject(pos);
  float d = scene(pos4);
  return k*d;
}

vec3 getColor(vec3 pos){
  float k = 1.0;
  if (doinvert) k = dot(pos,pos);
  pos /= k;
  vec4 pos4 = iproject(pos);
  return 0.8*color(pos4)+0.2;
}

//-------------------------------------------------
//From https://www.shadertoy.com/view/XtXGRS#
vec2 rotate(in vec2 p, in float t) {
  return p * cos(-t) + vec2(p.y, -p.x) * sin(-t);
}

float map(vec3 p) {
  return polyhedron(p);
}

vec3 calcNormal(in vec3 p) {
  const vec2 e = vec2(0.001, 0.0);
  return normalize(vec3(map(p + e.xyy) - map(p - e.xyy),
                        map(p + e.yxy) - map(p - e.yxy),
                        map(p + e.yyx) - map(p - e.yyx)));
}

float march(in vec3 ro, in vec3 rd) {
  const float maxd = 20.0;
  const float precis = 0.001;
  float h = precis * 2.0;
  float t = 0.0;
  float res = -1.0;
  for(int i = 0; i < 256; i++) {
      if (h < precis || t > maxd) break;
      h = map(ro + rd * t);
      if (doinvert) h *= 0.5;
      t += h;
    }
  if (t < maxd) res = t;
  return res;
}

vec3 transform(in vec3 p) {
  if (iMouse.x > 0.0) {
    float theta = (2.0*iMouse.y-iResolution.y)/iResolution.y*PI;
    float phi = (2.0*iMouse.x-iResolution.x)/iResolution.x*PI;
    p.yz = rotate(p.yz,-theta);
    p.zx = rotate(p.zx,phi);
  }
  p.yz = rotate(p.yz,iTime * 0.125);
  p.zx = rotate(p.zx,iTime * 0.2);
  return p;
}

const int CHAR_0 = 48;
const int CHAR_A = 65;
const int CHAR_B = 66;
const int CHAR_C = 67;
const int CHAR_D = 68;
const int CHAR_E = 69;
const int CHAR_F = 70;
const int CHAR_G = 71;
const int CHAR_I = 73;
const int CHAR_M = 77;
const int CHAR_N = 78;
const int CHAR_O = 79;
const int CHAR_P = 80;
const int CHAR_Q = 81;
const int CHAR_R = 82;
const int CHAR_S = 83;
const int CHAR_X = 88;

const int KEY_LEFT = 37;
const int KEY_UP = 38;
const int KEY_RIGHT = 39;
const int KEY_DOWN = 40;

#if NOKEYS
bool keypress(int code) {
  return false;
}

int keycount(int key) {
  return 0;
}
#else
bool keypress(int code) {
    return texelFetch(iChannel0, ivec2(code,2),0).x != 0.0;
}

vec4 store(int i,int j) {
  return texelFetch(iChannel1, ivec2(i,j),0);
}

int keycount(int key) {
  return int(store(0,key).x);
}
#endif

void mainImage( out vec4 fragColor, in vec2 fragCoord ) {
  //if (NOKEYS == 0) doinvert = keypress(CHAR_I);

  init(vec4(1));
  qinit();
  
  vec2 p = fragCoord.xy / iResolution.xy;
  // Now both coords are in (0,1)
  p = 2.0*p-1.0;
  p *= iResolution.xy/iResolution.y;
  vec3 col = vec3(0.3 + p.y * 0.1);
  vec3 ro = vec3(0.0, 0.0, -2.0);
  vec3 rd = vec3(p, 2.0);
  vec3 li = vec3(0.5, 1.0, -3);
  ro.z *= 0.1*float(10+keycount(KEY_DOWN)-keycount(KEY_UP));
#if 1
  ro = transform(ro);
  rd = transform(rd);
  li = transform(li);
#endif
  rd = normalize(rd);
  li = normalize(li);
  float t = march(ro, rd);
  if (t > 0.0) {
    vec3 pos = ro + t * rd;
    vec3 n = calcNormal(pos);
    float diffuse = clamp(dot(n, li), 0.0, 1.0);
    col = getColor(pos) * diffuse;
    col = pow(col, vec3(0.4545));
  }
#if defined DEBUG
  if (alert) col = vec3(1,0,0);
#endif
  fragColor = vec4(col, 1.0);
}
 