//stage GL_FRAGMENT_SHADER
//import <GLUP/ShaderToy.h>

// "Light and Motion" by dr2 - 2018
// License: Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License

// Playing around with ideas from Kali's "Generators" shader resulted in this...  

#define AA  0    // optional antialiasing

float PrRoundBoxDf (vec3 p, vec3 b, float r);
float SmoothBump (float lo, float hi, float w, float x);
vec2 Rot2D (vec2 q, float a);
float Hashfv2 (vec2 p);
vec3 VaryNf (vec3 p, vec3 n, float f);

vec3 ballPos, ltDir;
float tCur, dstFar, frctAng;
int idObj;
const float pi = 3.14159;

#define DMIN(id) if (d < dMin) { dMin = d;  idObj = id; }

float ObjDf (vec3 p) 
{
  float dMin, d, s, f;
  dMin = dstFar;
  d = 0.47 - abs (p.y - 3.5);
  DMIN (1);
  p.xz = abs (0.5 - mod ((2./3.) * p.xz, 1.));
  s = 1.;
  for (int j = 0; j < 9; j ++) {
    p = abs (p) - vec3 (-0.02, 1.98, -0.02);
    f = 2. / clamp (dot (p, p), 0.4, 1.);
    p = f * p - vec3 (0.5, 1., 0.4);
    s *= f;
    p.xz = Rot2D (p.xz, frctAng);
  }
  d = PrRoundBoxDf (p, vec3 (0.1, 5., 0.1), 0.1) / s;
  DMIN (2);
  return dMin;
}

vec3 ObjNf (vec3 p)
{
  vec4 v;
  vec2 e = vec2 (0.0001, -0.0001);
  v = vec4 (ObjDf (p + e.xxx), ObjDf (p + e.xyy), ObjDf (p + e.yxy), ObjDf (p + e.yyx));
  return normalize (vec3 (v.x - v.y - v.z - v.w) + 2. * v.yzw);
}

float ObjAO (vec3 ro, vec3 rd)
{
  float ao, d;
  ao = 0.;
  for (float j = 1.; j < 5.; j ++) {
    d = 0.002 * j * j;
    ao += max (0., d - ObjDf (ro + rd * d));
  }
  return 0.2 + 0.8 * clamp (1. - 20. * ao, 0., 1.);
}

float BallHit (vec3 ro, vec3 rd)
{
  float bRad, b, d;
  bRad = 0.025;
  b = dot (rd, ro);
  d = b * b + bRad * bRad - dot (ro, ro);
  if (d > 0.) {
    d = - b - sqrt (d);
    if (d < 0.) d = dstFar;
  } else d = dstFar;
  return d;
}

float TxPattern (vec3 p)
{
  float t, tt, c;
  p = abs (0.5 - fract (4. * p));
  c = 0.;
  t = 0.;
  for (float j = 0.; j < 6.; j ++) {
    p = abs (p + 3.) - abs (p - 3.) - p;
    p /= clamp (dot (p, p), 0., 1.);
    p = 3. - 1.5 * p;
    if (mod (j, 2.) == 0.) {
      tt = t;
      t = length (p);
      c += exp (-1. / abs (t - tt));
    }
  }
  return c;
}

vec3 ShowScene (vec3 ro, vec3 rd)
{
  vec3 vn, rg, col, bgCol, amb;
  float dstBall, dstObj, dstGlow, d, eGap, bGlow, eGlow, eFlash, eMid, eVib, rnd;
  int idObjT;
  bool isRefl;
  frctAng = 0.18172 + pi * (2. * SmoothBump (0.25, 0.75, 0.25, mod (0.01 * tCur, 1.)) - 1.);
  dstBall = BallHit (ro - ballPos, rd);
  isRefl = false;
  if (dstBall < dstFar) {
    isRefl = true;
    ro += dstBall * rd;
    rd = reflect (rd, normalize (ro - ballPos));
  }
  dstObj = 0.;
  for (int j = 0; j < 150; j ++) {
    d = ObjDf (ro + dstObj * rd);
    dstObj += d;
    if (d > 0.03) dstGlow = dstObj;
    if (d < 0.001 || dstObj > dstFar) break;
  }
  bgCol = vec3 (0.9, 0.9, 1.) * (0.1 + 0.1 * max (0., dot (rd, normalize (vec3 (0.5, 0.2, 1.)))));
  if (dstObj < dstFar) {
    if (idObj == 2) {
      rnd = Hashfv2 (11. + 77. * floor ((ro + dstObj * rd).xz / 1.5));
      bGlow = 0.;
      eGlow = 0.;
      eGap = mod (0.0625 * (tCur + 7. * rnd), 1.);
      eMid = 3.35;
      eVib = eMid + 0.005 * rnd * sin ((64. + rnd) * tCur); 
      for (int j = 0; j < 30; j ++) {
        rg = ro + dstGlow * rd;
        d = ObjDf (rg);
        dstGlow += d;
        bGlow += max (0., 0.02 - d) * exp (- 0.5 * dstGlow);
        eGlow += max (0., 0.03 - d) * (pow (max (0., 1. - 7. * abs (eVib - rg.y)), 8.) +
           pow (max (0., 1. - 20. * min (abs (eMid - rg.y - eGap), abs (eMid - rg.y + eGap))), 4.));
        if (d < 0.001 || dstGlow > dstFar) break;
      }
      eFlash = 0.6 + 0.4 * sin (8. * (tCur + 7. * rnd + rg.z));
    }
    ro += dstObj * rd;
    idObjT = idObj;
    vn = ObjNf (ro);
    amb = vec3 (0.2);
    if (idObjT == 1) {
      col = vec3 (0.1) * (1. + TxPattern ((ro.y < 3.5) ? ro : 2. * ro));
      if (ro.y < 3.2) col *= vec3 (0.9, 1., 0.9);
      vn = VaryNf (128. * ro, vn, 0.2);
    } else if (idObjT == 2) {
      col = vec3 (0.7, 0.7, 0.6);
      amb += 1.5 * eFlash * vec3 (1., 0.2, 0.1) * pow (max (0., 1. - 2.5 * abs (eMid - ro.y)), 2.) *
         max (0.5, sign (eMid - ro.y) * vn.y);
    }
    col = ObjAO (ro, vn) * col * (amb + max (0., dot (ltDir, vn)) +
       pow (max (dot (normalize (ltDir - rd), vn), 0.), 8.));
    col = mix (col, bgCol, 1. - exp (- dstObj));
    if (idObjT == 2) col += mix (vec3 (0., 0.3, 1.), vec3 (0., 1., 0.3), rnd) *
       (bGlow + 20. * eGlow * eGlow * eFlash) * (1. - smoothstep (0.4, 1., dstObj / dstFar));
  } else col = bgCol;
  if (isRefl) col = mix (mix (0.9 * col, vec3 (0.7, 0.7, 0.8), 0.2), bgCol, 1. - exp (- dstBall));
  return col;
}

vec3 TrackPath (float t)
{
  return vec3 (0.75 * sin (t), 3.35 + 0.15 * sin (0.8 * t), 0.75 * cos (0.5 * t));
}

void mainImage (out vec4 fragColor, in vec2 fragCoord)
{
  mat3 vuMat;
  vec4 mPtr, dateCur;
  vec3 ro, rd, vd, col;
  vec2 canvas, uv, ori, ca, sa;
  float el, az, t;
  canvas = iResolution.xy;
  uv = 2. * fragCoord.xy / canvas - 1.;
  uv.x *= canvas.x / canvas.y;
  tCur = iTime;
  dateCur = iDate;
  mPtr = iMouse;
  mPtr.xy = mPtr.xy / canvas - 0.5;
  tCur = mod (tCur + 30., 36000.) + floor (dateCur.w / 7200.);
  az = 0.;
  el = 0.02 * pi;
  if (mPtr.z > 0.) {
    az += 2. * pi * mPtr.x;
    el += 0.7 * pi * mPtr.y;
  }
  t = 0.1 * tCur;
  ballPos = TrackPath (t + 0.4);
  ro = TrackPath (t);
  vd = normalize (ballPos - ro);
  az += 1.1 * (0.5 * pi + atan (- vd.z, vd.x));
  el += 0.8 * asin (vd.y);
  el = clamp (el, -0.25 * pi, 0.25 * pi);
  ori = vec2 (el, az);
  ca = cos (ori);
  sa = sin (ori);
  vuMat = mat3 (ca.y, 0., - sa.y, 0., 1., 0., sa.y, 0., ca.y) *
          mat3 (1., 0., 0., 0., ca.x, - sa.x, 0., sa.x, ca.x);
  dstFar = 5.;
  ltDir = normalize (vec3 (1., 1.5, -1.));
#if ! AA
  const float naa = 1.;
#else
  const float naa = 4.;
#endif  
  col = vec3 (0.);
  for (float a = 0.; a < naa; a ++) {
    rd = vuMat * normalize (vec3 (uv + step (1.5, naa) * Rot2D (vec2 (0.71 / canvas.y, 0.),
       0.5 * pi * (a + 0.5)), 1.2));
    col += (1. / naa) * ShowScene (ro, rd);
  }
  fragColor = vec4 (pow (clamp (col, 0., 1.), vec3 (0.9)), 1.);
}

float PrRoundBoxDf (vec3 p, vec3 b, float r)
{
  return length (max (abs (p) - b, 0.)) - r;
}

float SmoothBump (float lo, float hi, float w, float x)
{
  return (1. - smoothstep (hi - w, hi + w, x)) * smoothstep (lo - w, lo + w, x);
}

vec2 Rot2D (vec2 q, float a)
{
  vec2 cs;
  cs = sin (a + vec2 (0.5 * pi, 0.));
  return vec2 (dot (q, vec2 (cs.x, - cs.y)), dot (q.yx, cs));
}

const float cHashM = 43758.54;

float Hashfv2 (vec2 p)
{
  return fract (sin (dot (p, vec2 (37., 39.))) * cHashM);
}

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
