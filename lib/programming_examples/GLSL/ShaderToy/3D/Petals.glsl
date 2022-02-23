//stage GL_FRAGMENT_SHADER
//import <GLUP/ShaderToy.h>

// "Petals" by dr2 - 2018
// License: Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License

float PrSphDf (vec3 p, float r);
float PrCylDf (vec3 p, float r, float h);
float PrCylAnDf (vec3 p, float r, float w, float h);
float PrTorusDf (vec3 p, float ri, float rc);
float SmoothBump (float lo, float hi, float w, float x);
vec2 Rot2D (vec2 q, float a);
vec3 HsvToRgb (vec3 c);
vec3 VaryNf (vec3 p, vec3 n, float f);

vec3 qHit;
float dstFar, tCur;
int idObj;
const float pi = 3.14159;

#define DMINQ(id) if (d < dMin) { dMin = d;  idObj = id;  qHit = q; }

float ObjDf (vec3 p)
{
  vec3 q, qq;
  float dMin, d, h, r, y, s;
  dMin = dstFar;
  h = 2.;
  q = p;
  q.xz = Rot2D (q.xz, 2. * pi * (floor (6. * ((length (q.xz) > 0.) ? atan (q.z, - q.x) /
     (2. * pi) : 0.) + 0.5) / 6.));
  q.x += 3.1;
  q.xy = Rot2D (q.xy, -0.33 * pi);
  r = length (q.xz);
  qq = q;
  for (float k = 0.; k < 2.; k ++) {
    q = qq;
    q.y -= 0.1 * step (0.5, k);
    q.xz = Rot2D (q.xz, sign (k - 0.5) * pi / 6.);
    q.xz = Rot2D (q.xz, 2. * pi * (floor (3. * ((r > 0.) ?
       atan (q.z, - q.x) / (2. * pi) : 0.) + 0.5) / 3.));
    y = 0.5 * (1. + q.y / h);
    s = 1. - 0.9 * cos (0.5 * pi * y);
    d = max (PrCylAnDf (q.xzy, 0.3 + 0.5 * y + s * (1. + 0.5 * s) - 0.01 * step (0.5, k), 0.01, h),
       abs (q.z) - 1.55 + 1.4 * (1. - (0.12 + y) * (0.88 - y)));
    DMINQ (1);
  }
  dMin *= 0.5;
  q = qq;  q.y += 1.75;
  d = max (PrSphDf (q, 0.6), q.y - 0.4);
  DMINQ (2);
  q = qq;
  q.xz = Rot2D (q.xz, 2. * pi * (floor (12. * ((r > 0.) ? atan (q.z, - q.x) /
     (2. * pi) : 0.) + 0.5) / 12.));
  q.y += 1.;
  q.x = abs (q.x + 0.1 + 0.2 * q.y * (1. + q.y)) - 0.04;
  d = 0.3 * PrCylDf (q.xzy, 0.01, 0.4);
  DMINQ (3);
  q = qq;  q.xy += vec2 (0.5, 2.35);
  d = max (PrTorusDf (q, 0.05, 0.5), q.y);
  q.xy = Rot2D (q.xy, 0.25 * pi);
  d = max (d, - q.y);
  DMINQ (4);
  q.xy -= vec2 (0.5, -1.3);
  d = PrCylDf (q.xzy, 0.05, 1.3);
  DMINQ (4);
  q = p;  q.y += 4.4;
  d = PrCylDf (q.xzy, 1.5, 0.5 * (1. - 0.1 * length (q.xz) / 1.5));
  DMINQ (5);
  return dMin;
}

float ObjRay (vec3 ro, vec3 rd)
{
  float dHit, d;
  dHit = 0.;
  for (int j = 0; j < 200; j ++) {
    d = ObjDf (ro + rd * dHit);
    if (d < 0.0005 || dHit > dstFar) break;
    dHit += d;
  }
  return dHit;
}

vec3 ObjNf (vec3 p)
{
  vec4 v;
  vec2 e = vec2 (0.0001, -0.0001);
  v = vec4 (ObjDf (p + e.xxx), ObjDf (p + e.xyy), ObjDf (p + e.yxy), ObjDf (p + e.yyx));
  return normalize (vec3 (v.x - v.y - v.z - v.w) + 2. * v.yzw);
}

vec3 ShowScene (vec3 ro, vec3 rd)
{
  vec3 ltPos[3], ltCol[3], col, vn, ltDir, dfSum, spSum;
  vec2 vf;
  float dstObj, spec, cv, at, r, a;
  for (int k = 0; k < 3; k ++) {
    a = 2. * pi * float (k) / 3.;
    ltPos[k] = 10. * vec3 (cos (a), 0.5, sin (a));
  }
  ltCol[0] = vec3 (1., 1., 0.4);
  ltCol[1] = vec3 (1., 0.4, 0.4);
  ltCol[2] = vec3 (0.4, 0.4, 1.);
  dstObj = ObjRay (ro, rd);
  if (dstObj < dstFar) {
    ro += dstObj * rd;
    vn = ObjNf (ro);
    vf = vec2 (0.);
    spec = 0.;
    if (idObj == 1) {
      cv = (1. - 0.3 * (1. - 0.4 * (SmoothBump (0.4, 0.6, 0.05, mod (6. * (abs (qHit.z) -
         0.3 * abs (qHit.x) - 0.6 * qHit.x * qHit.x), 1.)) + 1. - smoothstep (0., 0.02,
         abs (qHit.z))))) * (1. - 0.3 * smoothstep (0.4, 0.5, abs (qHit.z) +
         0.1 * abs ((qHit.x + 0.3) * (1. - (qHit.x + 0.3))))) *
         (1. - 0.3 * smoothstep (-0.8, -0.5, qHit.x - 0.5 * abs (qHit.z)));
      col = HsvToRgb (vec3 (floor (6. * (atan (ro.z, - ro.x) / (2. * pi) + 0.5) + 0.5) / 6., 0.7, cv));
      spec = 0.1;
      vf = vec2 (128., 0.1);
    } else if (idObj == 2) {
       if (qHit.y < 0.35) {
        col = vec3 (0.3, 0.9, 0.2) * (1. - 0.3 * SmoothBump (0.4, 0.6, 0.05,
           mod (12. * ((length (qHit.xz) > 0.) ? atan (qHit.z, - qHit.x) / (2. * pi) : 0.), 1.)));
        spec = 0.1;
        vf = vec2 (64., 0.5);
      } else {
        r = length (qHit.xz);
        qHit.xz = Rot2D (qHit.xz, 5. * tCur);
        col = mix (vec3 (0.5, 0.5, 0.1), vec3 (0.4, 0.2, 0.1), SmoothBump (0.2, 0.8, 0.1,
           mod (16. * r + ((r > 0.) ? atan (qHit.z, - qHit.x) / (2. * pi) : 0.), 1.)));
        spec = 0.05;
        vf = vec2 (32., 2.);
      }
    } else if (idObj == 3) {
      col = mix (vec3 (0.9, 0.9, 0.2), vec3 (1., 0.1, 0.1), step (0.35, qHit.y));
      spec = 0.1 - step (0.35, qHit.y);
    } else if (idObj == 4) {
      col = mix (vec3 (0.2, 0.3, 0.2), vec3 (0.2, 0.7, 0.1), smoothstep (-1., 0., qHit.y));
      spec = 0.1;
      vf = vec2 (64., 0.5);
    } else if (idObj == 5) {
      if (length (ro.xz) < 1.4) {
        col = vec3 (0.25, 0.15, 0.1) * (0.5 + 0.5 * step (0.1, length (Rot2D (ro.xz, 2. * pi *
           (floor (6. * atan (ro.z, - ro.x) / (2. * pi) + 0.5) / 6.)) + vec2 (0.25, 0.))));
        vf = vec2 (32., 1.);
      } else {
        col = vec3 (0.7, 0.7, 0.9);
        vf = vec2 (128., 0.2);
      }
    }
    if (vf.x > 0.) vn = VaryNf (vf.x * ro, vn, vf.y);
    if (spec >= 0.) {
      dfSum = vec3 (0.);
      spSum = vec3 (0.);
      for (int k = 0; k < 3; k ++) {
        ltDir = normalize (ltPos[k]);
        at = smoothstep (0.7, 0.8, dot (normalize (ltPos[k] - ro), ltDir));
        dfSum += ltCol[k] * at * max (dot (vn, ltDir), 0.);
        spSum += ltCol[k] * at * pow (max (dot (normalize (ltDir - rd), vn), 0.), 128.);
      }
      col = col * (0.2 + 0.7 * dfSum) + spec * spSum;
    }
  } else {
    col = vec3 (1., 1., 0.8) * (0.2 + 0.2 * (rd.y + 1.) * (rd.y + 1.));
  }
  return clamp (col, 0., 1.);
}

void mainImage (out vec4 fragColor, in vec2 fragCoord)
{
  mat3 vuMat;
  vec4 mPtr;
  vec3 ro, rd, vd, col;
  vec2 canvas, uv, uvs, ori, ca, sa;
  float el, az, zmFac;
  canvas = iResolution.xy;
  uv = 2. * fragCoord.xy / canvas - 1.;
  uvs = uv;
  uv.x *= canvas.x / canvas.y;
  tCur = iTime;
  mPtr = iMouse;
  mPtr.xy = mPtr.xy / canvas - 0.5;
  az = 0.;
  el = 0.1 * pi;
  if (mPtr.z > 0.) {
    az -= 2. * pi * mPtr.x;
    el -= pi * mPtr.y;
  } else {
    az += 0.02 * pi * tCur;
  }
  el = clamp (el, 0.05 * pi, 0.4 * pi);
  ro = 20. * vec3 (cos (el) * cos (az), sin (el), cos (el) * sin (az));
  vd = normalize (vec3 (0., -1., 0.) - ro);
  vuMat = mat3 (vec3 (vd.z, 0., - vd.x) / sqrt (1. - vd.y * vd.y),
     vec3 (- vd.y * vd.x, 1. - vd.y * vd.y, - vd.y * vd.z) / sqrt (1. - vd.y * vd.y), vd);
  zmFac = 4.4 - 0.8 * abs (el);
  rd = vuMat * normalize (vec3 (uv, zmFac));
  dstFar = 40.;
  col = ShowScene (ro, rd);
  uvs *= uvs * uvs;
  col *= 0.8 + 0.2 * pow (1. - 0.5 * length (uvs * uvs), 4.);
  fragColor = vec4 (col, 1.);
}

float PrSphDf (vec3 p, float r)
{
  return length (p) - r;
}

float PrCylDf (vec3 p, float r, float h)
{
  return max (length (p.xy) - r, abs (p.z) - h);
}

float PrCylAnDf (vec3 p, float r, float w, float h)
{
  return max (abs (length (p.xy) - r) - w, abs (p.z) - h);
}

float PrTorusDf (vec3 p, float ri, float rc)
{
  return length (vec2 (length (p.xy) - rc, p.z)) - ri;
}

float SmoothBump (float lo, float hi, float w, float x)
{
  return (1. - smoothstep (hi - w, hi + w, x)) * smoothstep (lo - w, lo + w, x);
}

vec2 Rot2D (vec2 q, float a)
{
  return q * cos (a) + q.yx * sin (a) * vec2 (-1., 1.);
}

vec3 HsvToRgb (vec3 c)
{
  vec3 p;
  p = abs (fract (c.xxx + vec3 (1., 2./3., 1./3.)) * 6. - 3.);
  return c.z * mix (vec3 (1.), clamp (p - 1., 0., 1.), c.y);
}

const float cHashM = 43758.54;

vec2 Hashv2v2 (vec2 p)
{
  vec2 cHashVA2 = vec2 (37., 39.);
  return fract (sin (vec2 (dot (p, cHashVA2), dot (p + vec2 (1., 0.), cHashVA2))) * cHashM);
}

float Noisefv2 (vec2 p)
{
  vec2 t, ip, fp;
  ip = floor (p);  
  fp = fract (p);
  fp = fp * fp * (3. - 2. * fp);
  t = mix (Hashv2v2 (ip), Hashv2v2 (ip + vec2 (0., 1.)), fp.y);
  return mix (t.x, t.y, fp.x);
}

float Fbmn (vec3 p, vec3 n)
{
  vec3 s;
  float a;
  s = vec3 (0.);
  a = 1.;
  for (int j = 0; j < 5; j ++) {
    s += a * vec3 (Noisefv2 (p.yz), Noisefv2 (p.zx), Noisefv2 (p.xy));
    a *= 0.5;
    p *= 2.;
  }
  return dot (s, abs (n));
}

vec3 VaryNf (vec3 p, vec3 n, float f)
{
  vec3 g;
  vec2 e = vec2 (0.1, 0.);
  g = vec3 (Fbmn (p + e.xyy, n), Fbmn (p + e.yxy, n), Fbmn (p + e.yyx, n)) - Fbmn (p, n);
  return normalize (n + f * (g - n * dot (n, g)));
}

