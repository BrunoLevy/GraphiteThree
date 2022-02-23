//stage GL_FRAGMENT_SHADER
//import <GLUP/ShaderToy.h>

// "Rock Garden" by dr2 - 2018
// License: Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License

float PrBoxDf (vec3 p, vec3 b);
float PrBox2Df (vec2 p, vec2 b);
float PrBoxAn2Df (vec2 p, vec2 b, float w);
float PrSphDf (vec3 p, float r);
float SmoothMin (float a, float b, float r);
float SmoothBump (float lo, float hi, float w, float x);
vec2 Rot2D (vec2 q, float a);
vec2 PixToHex (vec2 p);
vec2 HexToPix (vec2 h);
void HexVorInit ();
vec4 HexVor (vec2 p);
float Hashfv2 (vec2 p);
vec2 Hashv2v2 (vec2 p);
float Noisefv2 (vec2 p);
float Noisefv3 (vec3 p);
float Fbm2 (vec2 p);
float Fbm3 (vec3 p);
vec3 VaryNf (vec3 p, vec3 n, float f);

vec3 sunDir;
float tCur, dstFar;
int idObj;
const float pi = 3.14159, sqrt3 = 1.7320508;

#define DMIN(id) if (d < dMin) { dMin = d;  idObj = id; }

float ObjDf (vec3 p)
{
  vec4 vc;
  vec3 q;
  float dMin, d, h, s;
  dMin = dstFar;
  q = p;
  h = SmoothMin (4.8 - abs (q.x), 2.8 - abs (q.z), 0.2);
  h = SmoothMin (h, SmoothMin (length (q.xz - vec2 (3., 1.)),
    length (q.xz - vec2 (-3., -1.)), 0.1), 0.1);
  q.xz = Rot2D (q.xz, pi / 4.);
  h = SmoothMin (h, SmoothMin (length (q.xz - vec2 (-1.5, 2.)),
    length (q.xz - vec2 (1.5, -2.)), 0.1), 0.1);
  q = p;
  q.xz *= q.xz;
  h = SmoothMin (h, pow (dot (q.xz, q.xz), 0.25), 0.1);
  h *= 12. * pi;
  s = abs (sin (h));
  h = 0.01 + 0.02 * (1. - s) * (s + sqrt (1. - s * s));
  q = p;
  d = max (PrBoxDf (q, vec3 (4., h, 2.)), - p.y);
  DMIN (1);
  d = PrSphDf (q, 0.35 + 0.07 * Noisefv3 (8. * p));
  q.xz -= vec2 (3., 1.) * sign (q.zx);
  d = min (d, PrSphDf (q, 0.25 + 0.05 * Noisefv3 (8. * p)));
  q = p;
  q.xz -= vec2 (1.35, 1.2) * sign (q.zx);
  d = max (min (d, PrSphDf (q, 0.2 + 0.04 * Noisefv3 (8. * p))), - p.y);
  DMIN (2);
  q = p;
  d = max (PrBoxAn2Df (q.xz, vec2 (4.02, 2.02), 0.02), abs (q.y - 0.02) - 0.02);
  DMIN (3);
  vc = HexVor (8. * q.zx);
  h = 0.006 * smoothstep (0.05, 0.14, vc.x - 0.03 * vc.w);
  d = max (PrBoxAn2Df (q.xz, vec2 (4.55, 2.55), 0.51), abs (q.y - h) - h);
  DMIN (4);
  d = max (max (PrBoxAn2Df (q.xz, vec2 (5.04, 3.04), 0.05), abs (q.y - 0.3) - 0.3),
     0.3 - min (abs (q.x - 0.4 * sign (q.z)), abs (q.z + 1.6 * sign (q.x))));
  DMIN (5);
  q.xz = abs (q.xz) - vec2 (4.9, 2.9);
  q.y -= 0.1;
  d = max (max (PrSphDf (q, 0.25 + 0.03 * Noisefv3 (64. * p)),
     PrBox2Df (p.xz, vec2 (5., 3.))), - p.y);
  DMIN (6);
  return 0.7 * dMin;
}

float ObjRay (vec3 ro, vec3 rd)
{
  float dHit, d;
  dHit = 0.;
  for (int j = 0; j < 150; j ++) {
    d = ObjDf (ro + dHit * rd);
    if (d < 0.0005 || dHit > dstFar) break;
    dHit += d;
  }
  return dHit;
}

vec3 ObjNf (vec3 p)
{
  vec4 v;
  vec2 e = vec2 (0.0002, -0.0002);
  v = vec4 (ObjDf (p + e.xxx), ObjDf (p + e.xyy), ObjDf (p + e.yxy), ObjDf (p + e.yyx));
  return normalize (vec3 (v.x - v.y - v.z - v.w) + 2. * v.yzw);
}

float ObjSShadow (vec3 ro, vec3 rd)
{
  float sh, d, h;
  sh = 1.;
  d = 0.05;
  for (int j = 0; j < 30; j ++) {
    h = ObjDf (ro + d * rd);
    sh = min (sh, smoothstep (0., 0.1 * d, h));
    d += clamp (3. * h, 0.05, 0.5);
    if (sh < 0.05) break;
  }
  return 0.3 + 0.7 * sh;
}

vec3 ShowScene (vec3 ro, vec3 rd)
{
  vec4 vc;
  vec3 col, vn;
  vec2 vf;
  float dstObj, sh, spec;
  HexVorInit ();
  dstObj = ObjRay (ro, rd);
  if (dstObj < dstFar) {
    ro += dstObj * rd;
    vn = ObjNf (ro);
    vf = vec2 (0.);
    if (idObj == 1) {
      col = mix (vec3 (0.3), vec3 (0.5), smoothstep (0.45, 0.55, Fbm2 (256. * ro.xz)));
      spec = 0.02;
      vf = vec2 (64., 1.);
    } else if (idObj == 2) {
      col = mix (vec3 (0.4, 0.45, 0.4), vec3 (0.6, 0.6, 0.65),
         smoothstep (0.4, 0.6, Noisefv3 (256. * ro)));
      spec = 0.1;
      vf = vec2 (16., 5.);
    } else if (idObj == 3) {
      col = vec3 (0.15, 0.2, 0.15);
      spec = 0.05;
    } else if (idObj == 4) {
      vc = HexVor (8. * ro.zx);
      col = mix (vec3 (0.2, 0.2, 0.), vec3 (0.45, 0.4, 0.4),
         step (0.06 + 0.03 * vc.w, vc.x)) * (1. - 0.1 * Noisefv2 (128. * ro.xz));
      spec = 0.1;
      vf = vec2 (32., 1.);
    } else if (idObj == 5) {
      col = vec3 (0.5, 0.4, 0.3);
      if (ro.y > 0.59) col *= 0.3;
      else col *= 1. - 0.15 * smoothstep (0.3, 0.7,
         Fbm2 (4. * vec2 (2. * dot (abs (vn.zx), ro.xz), ro.y)));
      vf = vec2 (64., 0.5);
      spec = 0.05;
    } else if (idObj == 6) {
      col = vec3 (0.1, 0.4, 0.1) * (1. - 0.5 * Fbm3 (32. * ro.yzx));
      spec = 0.;
      vf = vec2 (16., 5.);
    }
    if (vf.x > 0.) vn = VaryNf (vf.x * ro, vn, vf.y);
    sh = ObjSShadow (ro, sunDir);
    col = col * (0.2 + 0.8 * sh * max (dot (vn, sunDir), 0.)) +
       spec * sh * pow (max (dot (normalize (sunDir - rd), vn), 0.), 32.);
  } else {
    col = vec3 (0.6, 0.6, 0.7) * (0.05 + 0.245 * (rd.y + 1.) * (rd.y + 1.));
  }
  return pow (clamp (col, 0., 1.), vec3 (0.7));
}

void mainImage (out vec4 fragColor, in vec2 fragCoord)
{
  mat3 vuMat;
  vec4 mPtr;
  vec3 ro, rd;
  vec2 canvas, uv, ori, ca, sa;
  float el, az, zmFac;
  canvas = iResolution.xy;
  uv = 2. * fragCoord.xy / canvas - 1.;
  uv.x *= canvas.x / canvas.y;
  tCur = iTime;
  mPtr = iMouse;
  mPtr.xy = mPtr.xy / canvas - 0.5;
  if (mPtr.z > 0.) {
    az = 3. * pi * mPtr.x;
    el = pi * (-0.2 + mPtr.y);
  } else {
    az = 0.1 * pi * (floor (0.3 * tCur) + smoothstep (0.9, 1., mod (0.3 * tCur, 1.)));
    el = - pi * (0.2 + 0.05 * sin (0.2 * az));
  }
  el = clamp (el, -0.4 * pi, -0.02 * pi);
  ori = vec2 (el, az);
  ca = cos (ori);
  sa = sin (ori);
  vuMat = mat3 (ca.y, 0., - sa.y, 0., 1., 0., sa.y, 0., ca.y) *
          mat3 (1., 0., 0., 0., ca.x, - sa.x, 0., sa.x, ca.x);
  ro = vuMat * vec3 (0., 0., -20.);
  zmFac = 6. + 2. * SmoothBump (0.25, 0.75, 0.15, mod (az / pi, 1.));
  rd = vuMat * normalize (vec3 (uv, zmFac));
  dstFar = 50.;
  sunDir = normalize (vec3 (1., 1.5, -1.));
  fragColor = vec4 (ShowScene (ro, rd), 1.);
}

float PrBoxDf (vec3 p, vec3 b)
{
  vec3 d;
  d = abs (p) - b;
  return min (max (d.x, max (d.y, d.z)), 0.) + length (max (d, 0.));
}

float PrBox2Df (vec2 p, vec2 b)
{
  vec2 d;
  d = abs (p) - b;
  return min (max (d.x, d.y), 0.) + length (max (d, 0.));
}

float PrBoxAn2Df (vec2 p, vec2 b, float w)
{
  return max (PrBox2Df (p, vec2 (b + w)), - PrBox2Df (p, vec2 (b - w)));
}

float PrSphDf (vec3 p, float r)
{
  return length (p) - r;
}

float SmoothMin (float a, float b, float r)
{
  float h;
  h = clamp (0.5 + 0.5 * (b - a) / r, 0., 1.);
  return mix (b, a, h) - r * h * (1. - h);
}

float SmoothBump (float lo, float hi, float w, float x)
{
  return (1. - smoothstep (hi - w, hi + w, x)) * smoothstep (lo - w, lo + w, x);
}

vec2 Rot2D (vec2 q, float a)
{
  return q * cos (a) * vec2 (1., 1.) + q.yx * sin (a) * vec2 (-1., 1.);
}

vec2 PixToHex (vec2 p)
{
  vec3 c, r, dr;
  c.xz = vec2 ((1./sqrt3) * p.x - (1./3.) * p.y, (2./3.) * p.y);
  c.y = - c.x - c.z;
  r = floor (c + 0.5);
  dr = abs (r - c);
  r -= step (dr.yzx, dr) * step (dr.zxy, dr) * dot (r, vec3 (1.));
  return r.xz;
}

vec2 HexToPix (vec2 h)
{
  return vec2 (sqrt3 * (h.x + 0.5 * h.y), (3./2.) * h.y);
}

vec2 gVec[7], hVec[7];

void HexVorInit ()
{
  vec3 e = vec3 (1., 0., -1.);
  gVec[0] = e.yy;
  gVec[1] = e.xy;
  gVec[2] = e.yx;
  gVec[3] = e.xz;
  gVec[4] = e.zy;
  gVec[5] = e.yz;
  gVec[6] = e.zx;
  for (int k = 0; k < 7; k ++) hVec[k] = HexToPix (gVec[k]);
}

vec4 HexVor (vec2 p)
{
  vec4 sd, udm;
  vec2 ip, fp, d, u;
  float amp, a;
  amp = 0.7;
  ip = PixToHex (p);
  fp = p - HexToPix (ip);
  sd = vec4 (4.);
  udm = vec4 (4.);
  for (int k = 0; k < 7; k ++) {
    u = Hashv2v2 (ip + gVec[k]);
    a = 2. * pi * (u.y - 0.5);
    d = hVec[k] + amp * (0.4 + 0.6 * u.x) * vec2 (cos (a), sin (a)) - fp;
    sd.w = dot (d, d);
    if (sd.w < sd.x) {
      sd = sd.wxyw;
      udm = vec4 (d, u);
    } else sd = (sd.w < sd.y) ? sd.xwyw : ((sd.w < sd.z) ? sd.xyww : sd);
  }
  sd.xyz = sqrt (sd.xyz);
  return vec4 (SmoothMin (sd.y, sd.z, 0.3) - sd.x, udm.xy, Hashfv2 (udm.zw));
}

const float cHashM = 43758.54;

float Hashfv2 (vec2 p)
{
  return fract (sin (dot (p, vec2 (37., 39.))) * cHashM);
}

vec2 Hashv2f (float p)
{
  return fract (sin (p + vec2 (0., 1.)) * cHashM);
}

vec2 Hashv2v2 (vec2 p)
{
  vec2 cHashVA2 = vec2 (37., 39.);
  return fract (sin (vec2 (dot (p, cHashVA2), dot (p + vec2 (1., 0.), cHashVA2))) * cHashM);
}

vec4 Hashv4v3 (vec3 p)
{
  vec3 cHashVA3 = vec3 (37., 39., 41.);
  vec2 e = vec2 (1., 0.);
  return fract (sin (vec4 (dot (p + e.yyy, cHashVA3), dot (p + e.xyy, cHashVA3),
     dot (p + e.yxy, cHashVA3), dot (p + e.xxy, cHashVA3))) * cHashM);
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

float Noisefv3 (vec3 p)
{
  vec4 t;
  vec3 ip, fp;
  ip = floor (p);
  fp = fract (p);
  fp *= fp * (3. - 2. * fp);
  t = mix (Hashv4v3 (ip), Hashv4v3 (ip + vec3 (0., 0., 1.)), fp.z);
  return mix (mix (t.x, t.y, fp.x), mix (t.z, t.w, fp.x), fp.y);
}

float Fbm2 (vec2 p)
{
  float f, a;
  f = 0.;
  a = 1.;
  for (int j = 0; j < 5; j ++) {
    f += a * Noisefv2 (p);
    a *= 0.5;
    p *= 2.;
  }
  return f * (1. / 1.9375);
}

float Fbm3 (vec3 p)
{
  float f, a;
  f = 0.;
  a = 1.;
  for (int i = 0; i < 5; i ++) {
    f += a * Noisefv3 (p);
    a *= 0.5;
    p *= 2.;
  }
  return f * (1. / 1.9375);
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

