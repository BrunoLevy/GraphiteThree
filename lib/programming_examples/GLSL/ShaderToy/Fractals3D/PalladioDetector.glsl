//stage GL_FRAGMENT_SHADER
//import <GLUP/ShaderToy.h>

// "Palladio's Detector" by dr2 - 2018
// License: Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License

#define AA  0   // optional antialiasing

float PrBoxDf (vec3 p, vec3 b);
float PrRoundBox2Df (vec2 p, vec2 b, float r);
vec3 HsvToRgb (vec3 c);
float SmoothBump (float lo, float hi, float w, float x);
float Minv3 (vec3 p);
float Maxv3 (vec3 p);
vec2 Rot2D (vec2 q, float a);
float Noisefv3 (vec3 p);
vec3 VaryNf (vec3 p, vec3 n, float f);

vec3 ltPos[2], ltAx;
float tCur, dstFar;
const float pi = 3.14159;
const float itMax = 12.;

float ObjDf (vec3 p)
{
  vec4 p4;
  float d;
  p = mod (p + 3., 6.) - 3.;
  p4 = vec4 (p, 1.);
  for (float j = 0.; j < itMax; j ++) {
    p4.xyz = 2. * clamp (p4.xyz, -1., 1.) - p4.xyz;
    p4 = 2.8 * p4 / clamp (dot (p4.xyz, p4.xyz), 0.25, 1.) + vec4 (p, 1.);
  }
  d = max (max (length (p4.xyz) / p4.w, - PrBoxDf (p, vec3 (0.33))),
     - Minv3 (vec3 (PrRoundBox2Df (p.xy, vec2 (0.05), 0.03),
     PrRoundBox2Df (p.yz, vec2 (0.05), 0.03), PrRoundBox2Df (p.zx, vec2 (0.05), 0.03))));
  return d;
}

float ObjRay (vec3 ro, vec3 rd)
{
  float dHit, h, s, sLo, sHi, eps;
  eps = 0.0005;
  s = 0.;
  sLo = 0.;
  dHit = dstFar;
  for (int j = 0; j < 120; j ++) {
    h = ObjDf (ro + s * rd);
    if (h < eps || s > dstFar) {
      sHi = s;
      break;
    }
    sLo = s;
    s += h;
  }
  if (h < eps) {
    for (int j = 0; j < 6; j ++) {
      s = 0.5 * (sLo + sHi);
      if (ObjDf (ro + s * rd) > eps) sLo = s;
      else sHi = s;
    }
    dHit = sHi;
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

float ObjSShadow (vec3 ro, vec3 rd, float dMax)
{
  float sh, d, h;
  sh = 1.;
  d = 0.02;
  for (int j = 0; j < 50; j ++) {
    h = ObjDf (ro + rd * d);
    sh = min (sh, smoothstep (0., 0.05 * d, h));
    d += clamp (3. * h, 0.02, 0.1);
    if (sh < 0.05 || d > dMax) break;
  }
  return 0.6 + 0.4 * sh;
}

float ObjAO (vec3 ro, vec3 rd)
{
  float ao, d;
  ao = 0.;
  for (float j = 1.; j < 4.; j ++) {
    d = 0.02 * j;
    ao += max (0., d - ObjDf (ro + d * rd));
  }
  return 0.5 + 0.5 * clamp (1. - 5. * ao, 0., 1.);
}

vec4 ObjCol (vec3 p)
{
  vec3 p3, col;
  float pp, ppMin, cn, s;
  p = mod (p + 3., 6.) - 3.;
  p3 = p;
  cn = 0.;
  ppMin = 1.;
  for (float j = 0.; j < itMax; j ++) {
    p3 = 2. * clamp (p3, -1., 1.) - p3;
    pp = dot (p3, p3);
    if (pp < ppMin) {
      cn = j;
      ppMin = pp;
    }
    p3 = 2.8 * p3 / clamp (pp, 0.25, 1.) + p;
  }
  s = mod (cn, 2.);
  col = HsvToRgb (vec3 (mod (0.6 + 1.5 * cn / itMax, 1.), mix (0.6, 0., s), 1.));
  return vec4 (col, 0.05 + 0.4 * s);
}

vec3 ShowScene (vec3 ro, vec3 rd)
{
  vec4 col4;
  vec3 col, vn, vnn, ltDir, rds;
  float dstObj, atten, dfSum, spSum, sh;
  dstObj = ObjRay (ro, rd);
  if (dstObj < dstFar) {
    ro += dstObj * rd;
    vn = ObjNf (ro);
    vnn = VaryNf (256. * ro, vn, 0.2);
    dfSum = 0.;
    spSum = 0.;
    for (int k = 0; k < 2; k ++) {
      ltDir = ltPos[k] - ro;
      atten = 1. / (1. + 0.1 * dot (ltDir, ltDir));
      ltDir = normalize (ltDir);
      atten *= smoothstep (0.3, 0.5, dot (ltAx, - ltDir));
      dfSum += atten * max (dot (vnn, ltDir), 0.);
      spSum += atten * pow (max (0., dot (ltDir, reflect (rd, vn))), 16.);
    }
    ltDir = normalize (0.5 * (ltPos[0] + ltPos[1]) - ro);
    sh = ObjSShadow (ro, ltDir, max (dstObj - 0.2, 0.));
    col4 = ObjCol (ro);
    col = (0.1 + 0.4 * sh * dfSum) * col4.rgb + col4.a * sh * spSum * vec3 (1., 1., 0.9);
    col *= ObjAO (ro, vn);
    col += vec3 (0., 0.2, 0.) * max (dot (- rd, vn), 0.) *
       (1. - smoothstep (0., 0.05, abs (dstObj - mod (0.5 * tCur, 3.))));
    col *= mix (1., smoothstep (0., 1., Maxv3 (col)), 0.3);
  } else {
    if (rd.y < 0.) {
      rd.y = - rd.y;
      rd.xz = vec2 (- rd.z, rd.x);
    }
    rds = floor (2000. * rd);
    rds = 0.00015 * rds + 0.1 * Noisefv3 (0.0005 * rds.yzx);
    for (int j = 0; j < 19; j ++) rds = abs (rds) / dot (rds, rds) - 0.9;
    col = vec3 (0.02, 0.02, 0.05) + 0.5 * vec3 (1., 1., 0.7) * min (1., 0.5e-3 *
       pow (min (6., length (rds)), 5.));
  }
  return col;
}

vec3 TrackPath (float t)
{
  vec3 p;
  p = vec3 (6. * floor (t / 18.));
  t = mod (t, 18.);
  if (t < 6.) p += vec3 (0., 0., t);
  else if (t < 12.) p += vec3 (t - 6., 0., 6.);
  else p += vec3 (6., t - 12., 6.);
  return p;
}

void mainImage (out vec4 fragColor, in vec2 fragCoord)
{
  mat3 vuMat;
  vec4 mPtr;
  vec3 ro, rd, vd, col;
  vec2 canvas, uv, cs;
  float el, az, spd, t, tm, ts;
  canvas = iResolution.xy;
  uv = 2. * fragCoord.xy / canvas - 1.;
  uv.x *= canvas.x / canvas.y;
  tCur = iTime;
  mPtr = iMouse;
  mPtr.xy = mPtr.xy / canvas - 0.5;
  spd = 0.2;
  ro = TrackPath (spd * tCur);
  vd = normalize (TrackPath (spd * tCur + 0.7) - TrackPath (spd * tCur - 0.7));
  az = 0.;
  el = 0.;
  if (mPtr.z > 0.) {
    az -= 2. * pi * mPtr.x;
    el -= pi * mPtr.y;
  }
  t = spd * tCur / 18.;
  tm = mod (3. * t, 1.);
  ts = 2. * step (1., mod (t, 2.)) - 1.;
  if (max (abs (vd.x), abs (vd.z)) > 0.5) {
    az += 0.5 * pi * SmoothBump (0.3, 0.7, 0.15, tm) * ts;
    if (abs (vd.z) > 0.5) vd.yz = Rot2D (vd.yz, el);
    else if (abs (vd.x) > 0.5) vd.yx = Rot2D (vd.yx, el);
  }
  if (abs (vd.y) < 1. - 1e-5) {
    vd.xz = Rot2D (vd.xz, az);
    vuMat = mat3 (vec3 (vd.z, 0., - vd.x) / sqrt (1. - vd.y * vd.y),
       vec3 (- vd.y * vd.x, 1. - vd.y * vd.y, - vd.y * vd.z) / sqrt (1. - vd.y * vd.y), vd);
  } else {
    az += 2. * pi * smoothstep (0.3, 0.7, tm) * ts;
    cs = sin (az + vec2 (0.5 * pi, 0.));
    vuMat = mat3 (vec3 (cs.x, 0., - cs.y), vec3 (cs.y, 0., cs.x), vec3 (0., vd.y, 0.));
  }
  ltPos[0] = ro + vuMat * vec3 (-0.3, 0.2, -0.05);
  ltPos[1] = ro + vuMat * vec3 (0.3, 0.2, -0.05);
  ltAx = vuMat * vec3 (0., 0., 1.);
  dstFar = 80.;
#if ! AA
  const float naa = 1.;
#else
  const float naa = 4.;
#endif  
  col = vec3 (0.);
  for (float a = 0.; a < naa; a ++) {
    rd = normalize (vec3 (uv + step (1.5, naa) * Rot2D (vec2 (0.71 / canvas.y, 0.),
       0.5 * pi * (a + 0.5)), 2.));
    rd = vuMat * rd;
    col += (1. / naa) * ShowScene (ro, rd);
  }
  col = pow (clamp (col, 0., 1.), vec3 (0.9));
  fragColor = vec4 (col, 1.);
}

float PrBoxDf (vec3 p, vec3 b)
{
  vec3 d;
  d = abs (p) - b;
  return min (max (d.x, max (d.y, d.z)), 0.) + length (max (d, 0.));
}

float PrRoundBox2Df (vec2 p, vec2 b, float r)
{
  return length (max (abs (p) - b, 0.)) - r;
}

vec3 HsvToRgb (vec3 c)
{
  return c.z * mix (vec3 (1.), clamp (abs (fract (c.xxx + vec3 (1., 2./3., 1./3.)) * 6. - 3.) - 1., 0., 1.), c.y);
}

float SmoothBump (float lo, float hi, float w, float x)
{
  return (1. - smoothstep (hi - w, hi + w, x)) * smoothstep (lo - w, lo + w, x);
}

float Minv3 (vec3 p)
{
  return min (p.x, min (p.y, p.z));
}

float Maxv3 (vec3 p)
{
  return max (p.x, max (p.y, p.z));
}

vec2 Rot2D (vec2 q, float a)
{
  vec2 cs;
  cs = sin (a + vec2 (0.5 * pi, 0.));
  return vec2 (dot (q, vec2 (cs.x, - cs.y)), dot (q.yx, cs));
}

const float cHashM = 43758.54;

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



