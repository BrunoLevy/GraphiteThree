//stage GL_FRAGMENT_SHADER
//import <GLUP/ShaderToy.h>

// "Rolling Forever" by dr2 - 2014
// License: Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License

const vec4 cHashA4 = vec4 (0., 1., 57., 58.);
const vec3 cHashA3 = vec3 (1., 57., 113.);
const float cHashM = 43758.54;

vec4 Hashv4f (float p)
{
  return fract (sin (p + cHashA4) * cHashM);
}

float Noisefv2 (vec2 p)
{
  vec2 i = floor (p);
  vec2 f = fract (p);
  f = f * f * (3. - 2. * f);
  vec4 t = Hashv4f (dot (i, cHashA3.xy));
  return mix (mix (t.x, t.y, f.x), mix (t.z, t.w, f.x), f.y);
}

float Fbm2 (vec2 p)
{
  float s = 0.;
  float a = 1.;
  for (int i = 0; i < 5; i ++) {
    s += a * Noisefv2 (p);
    a *= 0.5;
    p *= 2.;
  }
  return s;
}

float SmoothBump (float lo, float hi, float w, float x)
{
  return (1. - smoothstep (hi - w, hi + w, x)) * smoothstep (lo - w, lo + w, x);
}

float PrBoxDf (vec3 p, vec3 b)
{
  vec3 d = abs (p) - b;
  return min (max (d.x, max (d.y, d.z)), 0.) + length (max (d, 0.));
}

float PrSphDf (vec3 p, float r)
{
  return length (p) - r;
}

float PrCylDf (vec3 p, float r, float h)
{
  return max (length (p.xy) - r, abs (p.z) - h);
}

float PrTorusDf (vec3 p, float ri, float rc)
{
  vec2 q = vec2 (length (p.xy) - rc, p.z);
  return length (q) - ri;
}

int idObj;
const int nBall = 8;
mat3 vuMat;
vec3 bPos[nBall], bNv[nBall], trNv, qHit, ltDir;
float tCur, posLin, radSpir, dhSpir, ballRad, rollVel, fnSpir, dLinSeg,
   dRotSeg, lenPath, bAng[nBall], trAng;
bool showFlap;
const int nSpir = 3;
const float dstFar = 60.;
const float pi = 3.14159;

float TrackDf (vec3 p, float dHit)
{
  vec3 q, qq;
  vec2 b;
  float d, tw;
  float i2 = 1. / sqrt (2.);
  float hTrk = 0.25;
  float rTrk = 0.3;
  float dpTrk = 0.3;
  float wWid = 0.1;
  float wdTrk = 2.3;
  float posSpir = posLin + radSpir;
  vec3 sn = sign (p);
  q = p;
  q.xz -= posSpir * sn.xz;
  qq = q;
  qq.xz = sn.z * (qq.xz * vec2 (i2, i2) + sn.x * qq.zx * vec2 (- i2, i2));
  qq.y -= 0.5 * (atan (qq.z, - sn.z * sn.x * qq.x) / pi - fnSpir +
     0.25 * sn.z + 0.5) * dhSpir - hTrk;
  tw = (length (qq.xz) - radSpir) / wdTrk;
  for (int j = 0; j <= nSpir; j ++) {
    d = length (vec2 (tw, qq.y)) - rTrk;
    b = vec2 (abs (tw) - (rTrk - wWid), abs (qq.y - hTrk) - dpTrk);
    d = max (d, - (min (max (b.x, b.y), 0.) + length (max (b, 0.))));
    d = max (d, abs (q.y) - (0.5 * fnSpir + 0.2) * dhSpir);
    if (j == 0) d = max (d, - PrBoxDf (p + vec3 (0., 0.5 * fnSpir * dhSpir, 0.),
       vec3 (2. * posSpir, 2.2 * hTrk, posSpir)));
    else if (j == nSpir) d = max (d, - sn.x * q.x);
    if (d < dHit) {
      dHit = d;  idObj = 1;
    }
    qq.y -= dhSpir;
  }
  q = p;
  q.y += 0.1 * dhSpir;
  for (int k = 0; k <= 1; k ++) {
    qq = q;
    if (k == 0) {
      qq.y -= 0.5 * fnSpir * dhSpir;
    } else {
      qq.xz = qq.zx;
      qq.y += 0.5 * (fnSpir - 0.5) * dhSpir;
    }
    qq.z = abs (qq.z) - posLin;
    tw = qq.z / wdTrk;
    d = length (vec2 (tw, qq.y)) - rTrk;
    b = vec2 (abs (tw) - (rTrk - wWid), abs (qq.y - hTrk) - dpTrk);
    d = max (d, - (min (max (b.x, b.y), 0.) + length (max (b, 0.))));
    d = max (d, abs (qq.x) - posSpir);
    if (d < dHit) {
      dHit = d;  idObj = 1;
    }
  }
  q = p;
  q.xz = abs (q.xz) - posLin - radSpir;
  d = PrCylDf (q.xzy, 0.1 * radSpir, 0.5 * fnSpir * dhSpir);
  if (d < dHit) {
    dHit = d;  idObj = 2;
  }
  q.y -= 0.1 * dhSpir;
  q.y += 0.5 * (fnSpir - 0.5) * dhSpir;
  float cl = 0.5 * (radSpir - 0.25 * wdTrk);
  for (int j = 0; j < nSpir; j ++) {
    qq = q;
    qq.z -= cl;
    d = PrCylDf (qq, 0.03 * radSpir, cl);
    if (d < dHit) {
      dHit = d;  idObj = 2;
    }
    qq.y -= 0.5 * dhSpir;
    qq.z += 2. * cl;
    d = PrCylDf (qq, 0.03 * radSpir, cl);
    if (d < dHit) {
      dHit = d;  idObj = 2;
    }
    q.y -= dhSpir;
  }
  q = p;
  q.y += 0.5 * (fnSpir * dhSpir - wdTrk);
  q.x = abs (q.x) - posLin;
  d = PrTorusDf (q.yxz, 0.07 * radSpir, 0.5 * wdTrk);
  if (d < dHit) {
    dHit = d;  idObj = 3;
  }
  if (showFlap) {
    d = max (PrCylDf (q.yxz, 0.5 * wdTrk, 0.03 * radSpir), - q.y - 0.2 * wdTrk);
    if (d < dHit) {
      dHit = d;  idObj = 3;
    }
  }
  q = p - vec3 (0., - 0.5 * fnSpir * dhSpir - 0.2, 0.);
  d = PrBoxDf (q, vec3 (posLin + 2. * radSpir, 0.2, posLin + 2. * radSpir));
  if (d < dHit) {
    dHit = d;  idObj = 4;  qHit = q;
  }
  return dHit;
}

vec3 BallPos (int k)
{
  float ht = 0.5 * fnSpir * dhSpir;
  float htSeg = (2. * ht - 0.25 * dhSpir) / dRotSeg;
  float htd = ht - 0.25 * dhSpir;
  float posSpir = posLin + radSpir;
  float da = (2. * fnSpir - 0.5) * pi / dRotSeg;
  float dLRSeg = 0.25 * lenPath;
  float d = mod (tCur * rollVel + lenPath * float (k) / float (nBall), lenPath);
  trAng = mod (d / ballRad, 2. * pi);
  trNv = vec3 (0.);
  vec3 p = vec3 (0.);
  float a;
  if (d < 2. * dLRSeg) {
    if (d < dLinSeg) {
      p = vec3 (- posSpir + d, htd, posLin);
      trNv.z = -1.;
    } else if (d < dLRSeg) {
      d -= dLinSeg;
      a = d * da - 0.5 * pi;
      trNv.xz = vec2 (cos (a), sin (a));
      p.xz = vec2 (posSpir, posSpir) + radSpir * trNv.xz;
      p.y = htd - d * htSeg;
    } else if (d < dLinSeg + dLRSeg) {
      d -= dLRSeg;
      p = vec3 (posLin, - ht, posSpir - d);
      trNv.x = -1.;
    } else {
      d -= dLinSeg + dLRSeg;
      a = d * da + pi;
      trNv.xz = vec2 (cos (a), sin (a));
      p.xz = vec2 (posSpir, - posSpir) + radSpir * trNv.xz;
      p.y = - ht + d * htSeg;
    }
  } else {
    d -= 2. * dLRSeg;
    if (d < dLinSeg) {
      p = vec3 (posSpir - d, htd, - posLin);
      trNv.z = 1.;
    } else if (d < dLRSeg) {
      d -= dLinSeg;
      a = d * da - 0.5 * pi;
      trNv.xz = - vec2 (cos (a), sin (a));
      p.xz = - vec2 (posSpir, posSpir) + radSpir * trNv.xz;
      p.y = htd - d * htSeg;
    } else if (d < dLinSeg + dLRSeg) {
      d -= dLRSeg;
      p = vec3 (- posLin, - ht, - posSpir + d);
      trNv.x = 1.;
    } else {
      d -= dLinSeg + dLRSeg;
      a = d * da + pi;
      trNv.xz = - vec2 (cos (a), sin (a));
      p.xz = vec2 (- posSpir, posSpir) + radSpir * trNv.xz;
      p.y = - ht + d * htSeg;
    }
  }
  p.y += 1.75 * ballRad;
  return p;
}

float BallDf (vec3 p, float dHit)
{
  vec3 q;
  float d;
  q = p - bPos[0];  d = PrSphDf (q, ballRad);
  if (d < dHit) {
    dHit = d;  idObj = 5;  trNv = bNv[0];  trAng = bAng[0];  qHit = q;
  }
  q = p - bPos[1];  d = PrSphDf (q, ballRad);
  if (d < dHit) {
    dHit = d;  idObj = 5;  trNv = bNv[1];  trAng = bAng[1];  qHit = q;
  }
  q = p - bPos[2];  d = PrSphDf (q, ballRad);
  if (d < dHit) {
    dHit = d;  idObj = 5;  trNv = bNv[2];  trAng = bAng[2];  qHit = q;
  }
  q = p - bPos[3];  d = PrSphDf (q, ballRad);
  if (d < dHit) {
    dHit = d;  idObj = 5;  trNv = bNv[3];  trAng = bAng[3];  qHit = q;
  }
  q = p - bPos[4];  d = PrSphDf (q, ballRad);
  if (d < dHit) {
    dHit = d;  idObj = 5;  trNv = bNv[4];  trAng = bAng[4];  qHit = q;
  }
  q = p - bPos[5];  d = PrSphDf (q, ballRad);
  if (d < dHit) {
    dHit = d;  idObj = 5;  trNv = bNv[5];  trAng = bAng[5];  qHit = q;
  }
  q = p - bPos[6];  d = PrSphDf (q, ballRad);
  if (d < dHit) {
    dHit = d;  idObj = 5;  trNv = bNv[6];  trAng = bAng[6];  qHit = q;
  }
  q = p - bPos[7];  d = PrSphDf (q, ballRad);
  if (d < dHit) {
    dHit = d;  idObj = 5;  trNv = bNv[7];  trAng = bAng[7];  qHit = q;
  }
  return dHit;
}

float ObjDf (vec3 p)
{
  float dHit = dstFar;
  dHit = TrackDf (p, dHit);
  dHit = BallDf (p, dHit);
  return dHit;
}

float ObjRay (vec3 ro, vec3 rd)
{
  const float dTol = 0.001;
  float d;
  float dHit = 0.;
  for (int j = 0; j < 150; j ++) {
    d = ObjDf (ro + dHit * rd);
    dHit += d;
    if (d < dTol || dHit > dstFar) break;
  }
  return dHit;
}

vec3 ObjNf (vec3 p)
{
  const vec3 e = vec3 (0.001, -0.001, 0.);
  float v0 = ObjDf (p + e.xxx);
  float v1 = ObjDf (p + e.xyy);
  float v2 = ObjDf (p + e.yxy);
  float v3 = ObjDf (p + e.yyx);
  return normalize (vec3 (v0 - v1 - v2 - v3) + 2. * vec3 (v1, v2, v3));
}

float ObjSShadow (vec3 ro, vec3 rd)
{
  float sh = 1.;
  float d = 0.05;
  for (int i = 0; i < 100; i++) {
    float h = ObjDf (ro + rd * d);
    sh = min (sh, 20. * h / d);
    d += 0.05;
    if (h < 0.001) break;
  }
  return clamp (sh, 0., 1.);
}

float ObjAO (vec3 ro, vec3 rd)
{
  float ao = 0.;
  for (int i = 0; i < 8; i ++) {
    float d = 0.1 + float (i) / 8.;
    ao += max (0., d - 3. * ObjDf (ro + rd * d));
  }
  return clamp (1. - 0.1 * ao, 0., 1.);
}

vec4 BallCol ()
{
  vec3 col;
  vec3 q = qHit;
  float aa = atan (trNv.x, trNv.z);
  q.xz = q.xz * cos (aa) * vec2 (1., 1.) + q.zx * sin (aa) * vec2 (-1., 1.);
  if (q.z * (mod (pi + atan (q.x, q.y) + trAng, 2. * pi) - pi) > 0.)
     col = vec3 (0., 0.6, 0.);
  else col = vec3 (0.6, 0., 0.);
  return vec4 (col, 2.);
}

vec3 WoodCol (vec3 p, vec3 n)
{
  p *= 4.;
  float f = dot (vec3 (Fbm2 (p.yz * vec2 (1., 0.1)),
     Fbm2 (p.zx * vec2 (1., 0.1)), Fbm2 (p.yx * vec2 (1., 0.1))), abs (n));
  return 0.5 * mix (vec3 (0.8, 0.4, 0.2), vec3 (0.45, 0.25, 0.1), f);
}

vec4 ObjCol (vec3 n)
{
  vec4 col4;
  if (idObj == 1) col4 = vec4 (0., 0.6, 0.6, 1.);
  else if (idObj == 2) col4 = vec4 (0.5, 0.3, 0., 3.);
  else if (idObj == 3) col4 = vec4 (0.5, 0.5, 0., 3.);
  else if (idObj == 4) col4 = vec4 (WoodCol (qHit, n), 1.);
  else if (idObj == 5) col4 = BallCol ();
  return col4;
}

void BallPM ()
{
  float dGap = lenPath / float (nBall);
  float db = tCur * rollVel;
  showFlap = true;
  for (int nb = 0; nb < nBall; nb ++) {
    if (abs (abs (mod (db, lenPath) - (2.5 * dLinSeg + 2. * dRotSeg)) -
       (dLinSeg + dRotSeg)) < 3. * ballRad) showFlap = false;
    db += dGap;
  }
  bPos[0] = BallPos (0);  bNv[0] = trNv;  bAng[0] = trAng;
  bPos[1] = BallPos (1);  bNv[1] = trNv;  bAng[1] = trAng;
  bPos[2] = BallPos (2);  bNv[2] = trNv;  bAng[2] = trAng;
  bPos[3] = BallPos (3);  bNv[3] = trNv;  bAng[3] = trAng;
  bPos[4] = BallPos (4);  bNv[4] = trNv;  bAng[4] = trAng;
  bPos[5] = BallPos (5);  bNv[5] = trNv;  bAng[5] = trAng;
  bPos[6] = BallPos (6);  bNv[6] = trNv;  bAng[6] = trAng;
  bPos[7] = BallPos (7);  bNv[7] = trNv;  bAng[7] = trAng;
}

vec3 ShowScene (vec3 ro, vec3 rd)
{
  vec3 vn, objCol;
  float dstHit;
  vec3 col = vec3 (0., 0., 0.02);
  BallPM ();
  idObj = -1;
  dstHit = ObjRay (ro, rd);
  int idObjT = idObj;
  if (idObj < 0) dstHit = dstFar;
  if (dstHit < dstFar) {
    ro += rd * dstHit;
    vn = ObjNf (ro);
    idObj = idObjT;
    vec4 col4 = ObjCol (vn);
    objCol = col4.xyz;
    float spec = col4.w;
    float dif = max (dot (vn, ltDir), 0.);
    float ao = ObjAO (ro, vn);
    col = objCol * (0.2 * ao * (1. +
       max (dot (vn, - normalize (vec3 (ltDir.x, 0., ltDir.z))), 0.)) +
       max (0., dif) * ObjSShadow (ro, ltDir) *
       (dif + ao * spec * pow (max (0., dot (ltDir, reflect (rd, vn))), 64.)));
  }
  col = sqrt (clamp (col, 0., 1.));
  return col;
}

void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
  vec2 uv = 2. * fragCoord.xy / iResolution.xy - 1.;
  uv.x *= iResolution.x / iResolution.y;
  float zmFac = 4.2;
  tCur = iTime;
  fnSpir = float (nSpir);
  posLin = 1.5;
  radSpir = 3.;
  dhSpir = 2.5;
  rollVel = 2.;
  dLinSeg = 2. * (posLin + radSpir);
  dRotSeg = length (vec2 (2. * pi * (fnSpir - 0.25) * radSpir,
     (fnSpir - 0.25) * dhSpir));
  lenPath = 4. * (dLinSeg + dRotSeg);
  ballRad = 0.6;
  ballRad = lenPath / (2. * pi * floor (lenPath / (2. * pi * ballRad)));
  float dist = 15. + 7.5 * fnSpir;
  float el = 0.1;
  float az = 0.;
  el += 0.4 * SmoothBump (5., 25., 5., mod (tCur, 40.));
  az += mod (0.1 * tCur, 2. * pi);
  float cEl = cos (el);
  float sEl = sin (el);
  float cAz = cos (az);
  float sAz = sin (az);
  vuMat = mat3 (1., 0., 0., 0., cEl, - sEl, 0., sEl, cEl) *
     mat3 (cAz, 0., sAz, 0., 1., 0., - sAz, 0., cAz);
  vec3 rd = normalize (vec3 (uv, zmFac)) * vuMat;
  vec3 ro = - vec3 (0., 0., dist) * vuMat;
  ro.y -= 0.03 * dist;
  ltDir = normalize (vec3 (0.3, 0.5, -1.)) * vuMat;
  vec3 col = ShowScene (ro, rd);
  fragColor = vec4 (col, 1.);
}

