//stage GL_FRAGMENT_SHADER
//import <GLUP/ShaderToy.h>

// "Gotthard Tunnel" by dr2 - 2015
// License: Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License

const float pi = 3.14159;
const vec4 cHashA4 = vec4 (0., 1., 57., 58.);
const vec3 cHashA3 = vec3 (1., 57., 113.);
const float cHashM = 43758.54;

vec2 Hashv2f (float p)
{
  return fract (sin (p + cHashA4.xy) * cHashM);
}

vec4 Hashv4f (float p)
{
  return fract (sin (p + cHashA4) * cHashM);
}

float Noiseff (float p)
{
  float i, f;
  i = floor (p);  f = fract (p);
  f = f * f * (3. - 2. * f);
  vec2 t = Hashv2f (i);
  return mix (t.x, t.y, f);
}

float Noisefv2 (vec2 p)
{
  vec2 i = floor (p);
  vec2 f = fract (p);
  f = f * f * (3. - 2. * f);
  vec4 t = Hashv4f (dot (i, cHashA3.xy));
  return mix (mix (t.x, t.y, f.x), mix (t.z, t.w, f.x), f.y);
}

float Noisefv3 (vec3 p)
{
  vec3 i = floor (p);
  vec3 f = fract (p);
  f = f * f * (3. - 2. * f);
  float q = dot (i, cHashA3);
  vec4 t1 = Hashv4f (q);
  vec4 t2 = Hashv4f (q + cHashA3.z);
  return mix (mix (mix (t1.x, t1.y, f.x), mix (t1.z, t1.w, f.x), f.y),
     mix (mix (t2.x, t2.y, f.x), mix (t2.z, t2.w, f.x), f.y), f.z);
}

vec3 Noisev3v2 (vec2 p)
{
  vec2 i = floor (p);
  vec2 f = fract (p);
  vec2 ff = f * f;
  vec2 u = ff * (3. - 2. * f);
  vec2 uu = 30. * ff * (ff - 2. * f + 1.);
  vec4 h = Hashv4f (dot (i, cHashA3.xy));
  return vec3 (h.x + (h.y - h.x) * u.x + (h.z - h.x) * u.y +
     (h.x - h.y - h.z + h.w) * u.x * u.y, uu * (vec2 (h.y - h.x, h.z - h.x) +
     (h.x - h.y - h.z + h.w) * u.yx));
}

float Fbm1 (float p)
{
  float f, a;
  f = 0.;
  a = 1.;
  for (int i = 0; i < 5; i ++) {
    f += a * Noiseff (p);
    a *= 0.5;  p *= 2.;
  }
  return f;
}

float Fbmn (vec3 p, vec3 n)
{
  vec3 s = vec3 (0.);
  float a = 1.;
  for (int i = 0; i < 5; i ++) {
    s += a * vec3 (Noisefv2 (p.yz), Noisefv2 (p.zx), Noisefv2 (p.xy));
    a *= 0.5;
    p *= 2.;
  }
  return dot (s, abs (n));
}

float PrBoxDf (vec3 p, vec3 b)
{
  vec3 d = abs (p) - b;
  return min (max (d.x, max (d.y, d.z)), 0.) + length (max (d, 0.));
}

float PrRoundBoxDf (vec3 p, vec3 b, float r)
{
  return length (max (abs (p) - b, 0.)) - r;
}

float PrCylDf (vec3 p, float r, float h)
{
  return max (length (p.xy) - r, abs (p.z) - h);
}

float VBlockRingDf (vec3 p, float r, float w)
{
  vec2 q = vec2 (length (p.yx) - r, p.z);
  q = q * q * q;
  return pow (dot (q, q), 1./6.) - w *
     (0.7 + 0.3 * pow (abs (sin (13. * atan (p.y, p.x))), 0.25));
}

float SmoothMin (float a, float b, float k)
{
  float h = clamp (0.5 + 0.5 * (b - a) / k, 0., 1.);
  return mix (b, a, h) - k * h * (1. - h);
}

mat3 vuMat;
vec3 vuPos, vuDir, sunDir;
float tCur, vuSpd, grDir;
int idObj;
const int nCar = 4;
const float carSep = 3.5;
const float dstFar = 250.;

const int idTun = 0, idLight = 1, idArch = 2, idRail = 3, idTie = 4,
   idBody = 5, idBase = 6, idWheel = 7, idCable = 8, idFlash = 9, idWin = 10,
   idFrm = 11;

vec3 SkyHrzCol (vec3 ro, vec3 rd, int refl)
{
  const float skyHt = 150.;
  vec3 col;
  vec2 p;
  float w, f, cloudFac, s;
  if (refl == 0 &&
     rd.y < max (0.015 * Fbm1 (20. * rd.z + 0.5 * tCur) - 0.01, 0.002))
     col = vec3 (0.03, 0.07, 0.03);
  else {
    if (rd.y > 0.) {
      ro.x += 1.5 * tCur;
      p = 0.02 * (rd.xz * (skyHt - ro.y) / rd.y + ro.xz);
      w = 0.8;
      f = 0.;
      for (int j = 0; j < 4; j ++) {
	f += w * Noisefv2 (p);
	w *= 0.5;
	p *= 2.;
      }
      cloudFac = clamp (5. * (f - 0.5) * rd.y + 0.1, 0., 1.);
    } else cloudFac = 0.;
    s = max (dot (rd, sunDir), 0.);
    col = vec3 (0.1, 0.1, 0.6) + 0.2 * pow (1. - max (rd.y, 0.), 5.) +
       (0.35 * pow (s, 6.) + 0.65 * min (pow (s, 256.), 0.3));
    col = mix (col, vec3 (0.75), cloudFac);
  }
  return col;
}

float WaterHt (vec3 p)
{
  const float wb = 1.414;
  float ht, w;
  p *= 0.02;
  ht = 0.;
  w = wb;
  for (int j = 0; j < 6; j ++) {
    w *= 0.5;
    p = wb * vec3 (p.y + p.z, p.z - p.y, 2. * p.x);
    ht += w * abs (Noisefv3 (p) - 0.5);
  }
  return ht;
}

vec3 WaterNf (vec3 p, float d)
{
  float h;
  vec2 e = vec2 (max (0.01, 0.001 * d * d), 0.);
  h = WaterHt (p);
  return normalize (vec3 (h - WaterHt (p + e.xyy), e.x,
     h - WaterHt (p + e.yyx)));
}

vec3 TrackPath (float t)
{
  return vec3 (0.3 * sin (0.3 * t) * cos (0.04 * t) + 0.9 * cos (0.017 * t),
     0.6 + 0.3 * cos (0.15 * t) * cos (0.04 * t) * sin (0.021 * t), t);
}

float GrndHt (vec2 p, int hiRes)
{
  const vec2 vRot = vec2 (1.4624, 1.6721);
  vec3 v;
  vec2 q, vyz;
  float h, w;
  p -= TrackPath (p.y).xy;
  q = p * 0.06;
  w = 0.75 * Noisefv2 (0.25 * q) + 0.15;
  w *= 36. * w;
  vyz = vec2 (0.);
  h = 0.;
  for (int j = 0; j < 10; j ++) {
    v = Noisev3v2 (q);
    vyz += v.yz;
    h += w * v.x / (1. + dot (vyz, vyz));
    if (j == 4 && hiRes == 0) break;
    w *= -0.4;      
    q *= mat2 (vRot.x, vRot.y, - vRot.y, vRot.x);
  }
  return h * smoothstep (0., 5., - grDir * p.x) + 0.1 * Noisefv2 (3. * p) - 1.;
}

float GrndRay (vec3 ro, vec3 rd)
{
  vec3 p;
  float dHit, h, s, sLo, sHi;
  s = 0.;
  sLo = 0.;
  dHit = dstFar;
  for (int j = 0; j < 200; j ++) {
    p = ro + s * rd;
    h = p.y - GrndHt (p.xz, 0);
    if (h < 0.) break;
    sLo = s;
    s += 0.4 * h + 0.008 * s;
    if (s > dstFar) break;
  }
  if (h < 0.) {
    sHi = s;
    for (int j = 0; j < 6; j ++) {
      s = 0.5 * (sLo + sHi);
      p = ro + s * rd;
      h = step (0., p.y - GrndHt (p.xz, 0));
      sLo += h * (s - sLo);
      sHi += (1. - h) * (s - sHi);
    }
    dHit = sHi;
  }
  return dHit;
}

vec3 GrndNf (vec3 p, float d)
{
  float h;
  vec2 e = vec2 (max (0.01, 0.00001 * d * d), 0.);
  h = GrndHt (p.xz, 1);
  return normalize (vec3 (h - GrndHt (p.xz + e.xy, 1), e.x,
     h - GrndHt (p.xz + e.yx, 1)));
}

vec3 GrndCol (vec3 p, vec3 rd, vec3 n)
{
  const vec3 gCol1 = vec3 (0.6, 0.7, 0.7), gCol2 = vec3 (0.2, 0.1, 0.1),
     gCol3 = vec3 (0.4, 0.3, 0.3), gCol4 = vec3 (0.1, 0.2, 0.1),
     gCol5 = vec3 (0.7, 0.7, 0.8), gCol6 = vec3 (0.05, 0.3, 0.03),
     gCol7 = vec3 (0.02, 0.1, 0.02), gCol8 = vec3 (0.1, 0.08, 0.);
  vec3 col, c;
  vec2 q;
  float f, d, cSpec;
  q = p.xz;
  cSpec = 0.;
  f = 0.5 * (clamp (Noisefv2 (0.1 * q), 0., 1.) +
      0.8 * Noisefv2 (0.2 * q + 2.1 * n.xy + 2.2 * n.yz));
  col = f * mix (f * gCol1 + gCol2, f * gCol3 + gCol4, 0.65 * f);
  if (n.y < 0.5) {
    f = 0.4 * (Noisefv2 (0.4 * q + vec2 (0., 0.57 * p.y)) +
       0.5 * Noisefv2 (6. * q));
    d = 4. * (0.5 - n.y);
    col = mix (col, vec3 (f), clamp (d * d, 0.1, 1.));
    cSpec += 0.1;
  }
  if (p.y > 22.) {
    if (n.y > 0.25) {
      f = clamp (0.07 * (p.y - 22. - Noisefv2 (0.2 * q) * 15.), 0., 1.);
      col = mix (col, gCol5, f);
      cSpec += f;
    }
  } else {
    if (n.y > 0.45) {
      c = (n.y - 0.3) * (gCol6 * vec3 (Noisefv2 (0.4 * q),
         Noisefv2 (0.34 * q), Noisefv2 (0.38 * q)) + gCol7);
      col = mix (col, c, smoothstep (0.45, 0.65, n.y) *
         (1. - smoothstep (15., 22., p.y - 1.5 + 1.5 * Noisefv2 (0.2 * q))));
    }
    if (p.y < 0.65 && n.y > 0.4) {
      d = n.y - 0.4;
      col = mix (col, d * d + gCol8, 2. * clamp ((0.65 - p.y -
         0.35 * (Noisefv2 (0.4 * q) + 0.5 * Noisefv2 (0.8 * q) +
         0.25 * Noisefv2 (1.6 * q))), 0., 0.3));
      cSpec += 0.1;
    }
  }
  return col * (0.2 + max (dot (n, sunDir), 0.)) +
     cSpec * pow (max (0., dot (sunDir, reflect (rd, n))), 128.);
}

float ObjDf (vec3 p)
{
  vec3 q;
  float dMin, dTun, d, tw1, tw2;
  dMin = dstFar;
  p.xy -= TrackPath (p.z).xy;
  d = length (max (abs (vec2 (abs (p.x) - 0.25, p.y + 0.51)) -
     vec2 (0.01, 0.03), 0.));
  if (d < dMin) { dMin = d; idObj = idRail; }
  tw1 = 0.5 + 0.1 * sin (2.5 * p.z) * sin (31.4 * p.z);
  tw2 = 0.8 + 0.02 * sin (6.5 * p.z) * sin (7.4 * p.z);
  dTun = length (p.xy - vec2 (0., tw1 * clamp (p.y / tw1, -0.5, 0.5))) - tw2;
  if (dTun < 0.1)
     dTun = SmoothMin (- dTun, p.y + 0.6 + 0.02 * sin (4.1 * p.z), 0.05);
  else dTun -= 0.2;
  q = vec3 (p.x * sign (mod (p.z, 10.) - 5.1) + 0.9, p.y - 0.15,
     mod (p.z, 5.) - 1.5);
  d = max (dTun, - PrBoxDf (q, vec3 (0.6, 0.4, 1.)));
  if (d < dMin) { dMin = d; idObj = idTun; }
  d = PrBoxDf (vec3 (q.x + 0.1, q.yz), vec3 (0.1, 0.4, 1.));
  if (d < dMin) { dMin = d; idObj = idWin; }
  d = max (PrBoxDf (q, vec3 (0.13, 0.4, 1.)),
     - PrBoxDf (q, vec3 (0.14, 0.38, 0.98)));
  if (d < dMin) { dMin = d; idObj = idFrm; }
  q = vec3 (p.x, p.y + 0.58, mod (p.z, 0.8) - 0.4);
  d = PrBoxDf (q, vec3 (0.37, 0.02, 0.07));
  if (d < dMin) { dMin = d; idObj = idTie; }
  d = VBlockRingDf (vec3 (p.xy, mod (p.z, 5.) - 4.5), 0.9, 0.16);
  if (d < dMin) { dMin = d; idObj = idArch; }
  d = PrBoxDf (vec3 (p.x, p.y - 0.92, mod (p.z, 5.) - 2.),
     vec3 (0.02, 0.03, 0.2));
  if (d < dMin) { dMin = d; idObj = idLight; }
  d = length (max (abs (vec2 (p.x, p.y - 0.92)) - vec2 (0.01), 0.));
  if (d < dMin) { dMin = d; idObj = idCable; }
  float ppz = p.z - vuSpd * tCur - 2.;
  for (int nc = nCar - 1; nc >= 0; nc --) {
    p.z = ppz - carSep * float (nc);
    q = p;
    q.y -= -0.12;
    d = max (PrCylDf (q, 0.28, 0.4), - PrCylDf (q, 0.27, 0.38));
    d = max (d, - min (PrCylDf (q, 0.23, 0.41),
      PrBoxDf (q, vec3 (0.3, 0.1, 0.35))));
    if (d < dMin) { dMin = d; idObj = idBody; }
    q.y -= -0.28;
    d = PrRoundBoxDf (q, vec3 (0.2, 0.02, 0.38), 0.01);
    if (d < dMin) { dMin = d; idObj = idBase; }
    q.y -= 0.57;
    d = PrCylDf (q.xzy, 0.04, 0.03);
    if (d < dMin) { dMin = d; idObj = idFlash; }
    q = vec3 (abs (p.x) - 0.23, p.y + 0.41, abs (p.z + 0.2) - 0.2);
    d = PrCylDf (q.yzx, 0.06 - sign (q.x) * 0.01, 0.02);
    if (d < dMin) { dMin = d; idObj = idWheel; }
  }
  return dMin;
}

vec3 ObjNf (vec3 p)
{
  vec4 v;
  const vec3 e = vec3 (0.001, -0.001, 0.);
  v = vec4 (ObjDf (p + e.xxx), ObjDf (p + e.xyy),
     ObjDf (p + e.yxy), ObjDf (p + e.yyx));
  return normalize (vec3 (v.x - v.y - v.z - v.w) + 2. * vec3 (v.y, v.z, v.w));
}

vec3 VaryNf (vec3 p, vec3 n, float f)
{
  const vec3 e = vec3 (0.2, 0., 0.);
  vec3 g;
  float s;
  s = Fbmn (p, n);
  g = vec3 (Fbmn (p + e.xyy, n) - s, Fbmn (p + e.yxy, n) - s,
     Fbmn (p + e.yyx, n) - s);
  return normalize (n + f * (g - n * dot (n, g)));
}

vec3 ObjCol (vec3 ro, vec3 rd, float dist)
{
  vec4 col4;
  vec3 ltPos, ltDir, ltAx;
  float ltDiff, ltSpec, amb, di, atten, t;
  int idObjT;
  vec3 col;
  idObjT = idObj;
  col4 = vec4 (0.);
  if      (idObjT == idTun) col4 = vec4 (0.05, 0.05, 0.055, 0.5);
  else if (idObjT == idFrm) col4 = vec4 (0.1, 0.1, 0.11, 0.6);
  else if (idObjT == idLight) col4 = vec4 (0.);
  else if (idObjT == idArch) col4 = vec4 (0.3, 0.2, 0.1, 0.5);
  else if (idObjT == idRail) col4 = vec4 (0.5, 0.5, 0.55, 1.);
  else if (idObjT == idTie) col4 = vec4 (0.15, 0.15, 0.1, 0.5);
  else if (idObjT == idBody) col4 = vec4 (0.15, 0.25, 0.3, 0.2);
  else if (idObjT == idBase) col4 = vec4 (0.1, 0.2, 0.2, 0.2);
  else if (idObjT == idWheel) col4 = vec4 (0.15, 0.12, 0.05, 0.3);
  else if (idObjT == idCable) col4 = vec4 (1., 1., 0., 1.);
  else if (idObjT == idFlash) col4 = vec4 ((mod (2. * tCur, 1.) < 0.5) ?
     vec3 (2., 0., 0.) : vec3 (0., 0., 2.), 0.);
  vec3 vn = ObjNf (ro);
  if (idObjT == idTun || idObjT == idFrm) {
    vn = VaryNf (20. * ro, vn, 10.);
  } else if (idObjT == idArch || idObjT == idTie) {
    vn = VaryNf (40. * ro, vn, 2.);
  }
  if (idObjT == idLight) col = vec3 (1., 1., 0.9);
  else {
    amb = 0.05;
    ltDiff = 0.;
    ltSpec = 0.;
    for (int nc = nCar; nc >= 0; nc --) {
      t = vuSpd * tCur + carSep * float (nc);
      ltPos = TrackPath (t);
      ltAx = normalize (TrackPath (t + 0.1) - ltPos);
      ltPos.y += 0.2;
      ltDir = ro - ltPos;
      di = 1. / max (length (ltDir), 0.01);
      ltDir *= di;
      atten = pow (min (di, 1.), 2.) * max (dot (ltAx, ltDir), 0.);
      ltDiff += atten * max (dot (- ltDir, vn), 0.);
      ltSpec += atten * pow (max (dot (reflect (- ltDir, vn), rd), 0.), 128.);
    }
    col = col4.rgb * (amb + (1. - amb) * ltDiff) + col4.a * ltSpec;
  }
  return clamp (col, 0., 1.);
}

float ObjRay (vec3 ro, vec3 rd)
{
  float dHit, d;
  dHit = 0.;
  for (int j = 0; j < 150; j ++) {
    d = ObjDf (ro + dHit * rd);
    dHit += d;
    if (d < 0.001 || dHit > dstFar) break;
  }
  return dHit;
}

vec3 ShowScene (vec3 ro, vec3 rd)
{
  float dstHit, reflFac, dw;
  int refl;
  vec3 col;
  sunDir = normalize (vec3 (0.8, 1., 1.));
  grDir = -1.;
  idObj = -1;
  dstHit = ObjRay (ro, rd);
  reflFac = 1.;
  refl = -1;
  if (idObj != idWin) {
    ro += rd * dstHit;
    col = ObjCol (ro, rd, dstHit);
  } else {
    ro += rd * dstHit;
    if (grDir * (ro.x - TrackPath (ro.z).x) > 0.) {
      if (rd.y < 0.) {
        dw = - ro.y / rd.y;
        ro += dw * rd;
        rd = reflect (rd, WaterNf (ro, dw));
        ro += 0.01 * rd;
	reflFac *= 0.7;
	refl = 1;
      } else refl = 0;
    } else {
      dstHit = GrndRay (ro, rd);
      if (dstHit < dstFar || rd.y < 0.) {
        ro += rd * dstHit;
        col = GrndCol (ro, rd, GrndNf (ro, dstHit));
      } else refl = 0;
    }
  }
  if (refl >= 0) col = reflFac * SkyHrzCol (ro, rd, refl);
  return sqrt (clamp (col, 0., 1.));
}

void VuPM (float t)
{
  vec3 vuF, vuB, vel, acc, va, ort, cr, sr;
  float dt;
  dt = 0.2;
  vuPos = TrackPath (t);
  vuF = TrackPath (t + dt);
  vuB = TrackPath (t - dt);
  vel = (vuF - vuB) / (2. * dt);
  vuDir = normalize (vel);
  vel.y = 0.;
  acc = (vuF - 2. * vuPos + vuB) / (dt * dt);
  acc.y = 0.;
  va = cross (acc, vel) / length (vel);
  ort = vec3 (0., atan (vel.z, vel.x) - 0.5 * pi,
     0.5 * length (va) * sign (va.y));
  cr = cos (ort);
  sr = sin (ort);
  vuMat = mat3 (cr.z, - sr.z, 0., sr.z, cr.z, 0., 0., 0., 1.) *
     mat3 (1., 0., 0., 0., cr.x, - sr.x, 0., sr.x, cr.x) *
     mat3 (cr.y, 0., - sr.y, 0., 1., 0., sr.y, 0., cr.y);
}

void mainImage (out vec4 fragColor, in vec2 fragCoord)
{
  vec2 uv = 2. * (fragCoord - 0.5 * iResolution.xy) / iResolution.y;
  tCur = iTime;
  vec3 rd, ro;
  vuSpd = 3.;
  VuPM (vuSpd * tCur);
  rd = normalize (vec3 (uv, 2.2)) * vuMat;
  ro = vuPos;
  ro.y += 0.1;
  fragColor = vec4 (ShowScene (ro, rd), 1.);
}

