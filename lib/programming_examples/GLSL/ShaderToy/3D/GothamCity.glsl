//stage GL_FRAGMENT_SHADER
//import <GLUP/ShaderToy.h>

// "Gotham City" by dr2 - 2015
// License: Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License

// Architectural motifs and idea of dda-based raymarching from
// Octavio Good's "Skyline"; with thanks.

const float pi = 3.14159;
const vec4 cHashA4 = vec4 (0., 1., 57., 58.);
const vec3 cHashA3 = vec3 (1., 57., 113.);
const float cHashM = 43758.54;

float Hashff (float p)
{
  return fract (sin (p) * cHashM);
}

float Hashfv2 (vec2 p)
{
  return fract (sin (dot (p, cHashA3.xy)) * cHashM);
}

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
  vec2 t;
  float ip, fp;
  ip = floor (p);
  fp = fract (p);
  fp = fp * fp * (3. - 2. * fp);
  t = Hashv2f (ip);
  return mix (t.x, t.y, fp);
}

float Noisefv2 (vec2 p)
{
  vec4 t;
  vec2 ip, fp;
  ip = floor (p);
  fp = fract (p);
  fp = fp * fp * (3. - 2. * fp);
  t = Hashv4f (dot (ip, cHashA3.xy));
  return mix (mix (t.x, t.y, fp.x), mix (t.z, t.w, fp.x), fp.y);
}

float IFbm1 (float p)
{
  float s, a;
  s = 0.;
  a = 1.;
  for (int j = 0; j < 4; j ++) {
    s += floor (10. * a * Noiseff (p));
    a *= 0.6;
    p *= 4.;
  }
  return 0.1 * s;
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
  float s;
  vec3 e = vec3 (0.1, 0., 0.);
  s = Fbmn (p, n);
  g = vec3 (Fbmn (p + e.xyy, n) - s,
     Fbmn (p + e.yxy, n) - s, Fbmn (p + e.yyx, n) - s);
  return normalize (n + f * (g - n * dot (n, g)));
}

float PrOBoxDf (vec3 p, vec3 b)
{
  return length (max (abs (p) - b, 0.));
}

float PrCylDf (vec3 p, float r, float h)
{
  return max (length (p.xy) - r, abs (p.z) - h);
}

float PrCapsDf (vec3 p, float r, float h)
{
  return length (p - vec3 (0., 0., h * clamp (p.z / h, -1., 1.))) - r;
}

float SmoothMin (float a, float b, float r)
{
  float h = clamp (0.5 + 0.5 * (b - a) / r, 0., 1.);
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

vec3 HsvToRgb (vec3 c)
{
  vec3 p = abs (fract (c.xxx + vec3 (1., 2./3., 1./3.)) * 6. - 3.);
  return c.z * mix (vec3 (1.), clamp (p - 1., 0., 1.), c.y);
}

mat3 vuMat;
vec3 vuPos, qHit, sunDir;
vec2 iqBlk, cTimeV;
float dstFar, tCur, scnCount, flyDir, fDayLt, qcCar, cDir;
int idObj, flyMode;
const float flrHt = 0.05;
const int idBldgF = 1, idBldgC = 2, idRoad = 3, idSWalk = 4, idCarWhl = 5,
   idCarBdy = 6, idTrLight = 7, idTwr = 8, idTwrTop = 9;

vec3 SkyCol (vec3 ro, vec3 rd)
{
  vec3 dyCol, ntCol, rds;
  vec2 p;
  float cloudFac, a, s, sd;
  if (rd.y > 0.) {
    ro.xz += 2. * tCur;
    p = 0.005 * (rd.xz * (100. - ro.y) / rd.y + ro.xz);
    a = 0.8;
    s = 0.;
    for (int j = 0; j < 4; j ++) {
      s += a * Noisefv2 (p);
      a *= 0.5;
      p *= 2.;
    }
    cloudFac = clamp (2. * s * sqrt (rd.y) - 0.1, 0., 1.);
  } else cloudFac = 0.;
  sd = max (dot (rd, sunDir), 0.);
  dyCol = vec3 (0.05, 0.15, 0.3) + 0.25 * pow (1. - max (rd.y, 0.), 8.) +
     (0.35 * pow (sd, 6.) + 0.65 * min (pow (sd, 256.), 0.3));
  dyCol = mix (dyCol, vec3 (1.), cloudFac);
  ntCol = vec3 (0.02, 0.02, 0.04);
  rds = rd + vec3 (1.);
  for (int j = 0; j < 10; j ++)
     rds = 11. * abs (rds) / dot (rds, rds) - 3.;
  ntCol += vec3 (0.7, 0.6, 0.6) * max (1. - 8. * fDayLt, 0.) *
     min (1., 1.5e-6 * pow (min (16., length (rds)), 5.));
  return mix (ntCol, dyCol, fDayLt);
}

float BldgDf (vec3 p, float dMin)
{
  vec3 q, qq;
  vec2 ip;
  float d, bWid, bWidU, bHt, bHtU, bShape, tWid, hiMid, twHt;
  bool bTall;
  ip = floor (p.xz);
  hiMid = dot (ip, ip);
  bTall = (hiMid == 0.);
  hiMid = 0.75 * clamp (4. / max (sqrt (hiMid), 1.), 0., 1.);
  d = p.y;
  if (d < dMin) { dMin = d;  idObj = idRoad;  qHit = p;  iqBlk = ip; }
  q = p;
  q.xz = fract (q.xz) - vec2 (0.5);
  bWid = floor ((0.2 + Hashfv2 (11. * ip) * 0.1) / flrHt + 0.5) * flrHt;
  bWidU = floor (bWid * (0.5 + 0.3 * Hashfv2 (12. * ip)) / flrHt + 0.5) * flrHt;
  bHt = (0.5 * Hashfv2 (13. * ip) + 0.05) * hiMid *
     (1.5 + (bWid - 0.15) / flrHt) + 0.1;
  bHtU = 0.25 * bHt + 0.75 * max (0., Hashfv2 (15. * ip) - 0.5) * hiMid + 0.05;
  bHt = (floor (bHt / flrHt) + 0.2) * flrHt;
  bHtU = floor (bHtU / flrHt) * flrHt;
  if (bHtU > 0.) bHtU += 0.2 * flrHt;
  if (bTall) {
    bHt = max (bHt, 40.2 * flrHt);
    bHtU = max (bHtU, 20.2 * flrHt);
  }
  tWid = ((bHtU > 0.) ? bWidU : bWid) - 0.0125;
  bShape = Hashfv2 (17. * ip);
  q.y -= 0.0015;
  d = PrOBoxDf (q, vec3 (0.35, 0.0015, 0.35));
  if (d < dMin) { dMin = d;  idObj = idSWalk;  qHit = p; }
  q.y -= 0.0015;
  qq = q;
  qq.xz = abs (qq.xz) - vec2 (0.345);
  qq.y -= 0.02;
  d = PrCylDf (qq.xzy, 0.002, 0.02);
  if (d < dMin) { dMin = d;  idObj = idTrLight;  qHit = qq; }
  qq = q;
  qq.y -= bHt - 0.2 * flrHt - 0.001;
  if (bShape > 0.25) {
    d = PrOBoxDf (qq, vec3 (bWid, bHt, bWid));
    if (d < dMin) { dMin = d;  idObj = idBldgF;  qHit = qq;  iqBlk = ip; }
  } else {
    d = PrCylDf (qq.xzy, bWid, bHt);
    if (d < dMin) { dMin = d;  idObj = idBldgC;  qHit = qq;  iqBlk = ip; }
  }
  qq.y -= bHt + bHtU - 0.2 * flrHt - 0.001;
  if (bHtU > 0.) {
    if (bShape > 0.5) {
      d = max (PrOBoxDf (qq, vec3 (bWidU, bHtU, bWidU)),
         - PrOBoxDf (qq - vec3 (0., bHtU, 0.),
	 vec3 (tWid, 0.1 * flrHt, tWid)));
      if (d < dMin) { dMin = d;  idObj = idBldgF;  qHit = qq;  iqBlk = ip; }
    } else {
      d = max (PrCylDf (qq.xzy, bWidU, bHtU),
	 - PrCylDf ((qq - vec3 (0., bHtU, 0.)).xzy, tWid, 0.1 * flrHt));
      if (d < dMin) { dMin = d;  idObj = idBldgC;  qHit = qq;  iqBlk = ip; }
    }
  }
  if (flyMode > 0) {
    qq.y -= bHtU - 0.2 * flrHt - 0.001;
    if (bShape < 0.1) {
      d = PrCapsDf (qq.xzy, 0.4 * bWidU, 1.25 * flrHt);
      if (d < dMin) { dMin = d;  idObj = idBldgC;  qHit = qq;  iqBlk = ip; }
    } else if (bShape > 0.7) {
      d = PrOBoxDf (qq, vec3 (0.25 * bWidU, 1.25 * flrHt, 0.25 * bWidU));
      if (d < dMin) { dMin = d;  idObj = idBldgF;  qHit = qq;  iqBlk = ip; }
    }
    if (bHt + bHtU > 30. * flrHt) {
      twHt = 0.1 * (bHt + bHtU);
      qq.y -= twHt;
      d = PrCapsDf (qq.xzy, 0.3 * flrHt, twHt);
      if (d < dMin) {
	dMin = d;  qHit = qq;  iqBlk = ip;
	idObj = (qq.y > 0.9 * twHt) ? idTwrTop : idTwr;  
      }
    }
    if (bTall) {
      qq = q;
      qq.y -= 2. * (bHt + bHtU) + 0.2 * flrHt;
      d = PrCylDf (qq.xzy, 0.3, 1.2 * flrHt);
      if (d < dMin) { dMin = d;  idObj = idBldgC;  qHit = qq;  iqBlk = ip; }
    }
  }
  return dMin;
}

float CarDf (vec3 p, float dMin)
{
  vec3 q;
  float d, bf, vDir, cCar;
  if (cDir == 0. && abs (fract (p.z) - 0.5) > 0.35 ||
     cDir == 1. && abs (fract (p.x) - 0.5) < 0.35) {
    p.xz = vec2 (- p.z, p.x);
    vDir = 0.;
  } else {
    vDir = 1.;
  }
  q = p;
  q.y -= -0.003;
  q.z += 3. * floor (q.x);
  q.x = fract (q.x) - 0.5;
  q.z *= sign (q.x);
  q.z -= cTimeV.x + ((cDir == vDir) ? vDir + cTimeV.y : 1.);
  cCar = floor (20. * q.z);
  q.z = fract (q.z) - 0.5;
  q.x = abs (q.x) - 0.395 - 0.06 * step (0.7, Hashff (11. * cCar)) -
     0.03 * Hashff (13. * cCar);
  bf = PrOBoxDf (q + vec3 (0., 0., -0.1), vec3 (0.015, 0.05, 0.2));
  q.z = mod (q.z, 0.05) - 0.025;
  d = SmoothMin (PrOBoxDf (q + vec3 (0., -0.008, 0.), vec3 (0.007, 0.002, 0.015)),
     PrOBoxDf (q + vec3 (0., -0.015, 0.003), vec3 (0.0035, 0.0003, 0.005)), 0.02);
  d = max (d, bf);
  if (d < dMin) { dMin = d;  idObj = idCarBdy;  qHit = q;  qcCar = cCar; }
  q.xz = abs (q.xz) - vec2 (0.0085, 0.01);
  q.y -= 0.006;
  d = max (PrCylDf (q.yzx, 0.003, 0.0012), bf);
  if (d < dMin) { dMin = d;  idObj = idCarWhl;  qHit = q; }
  return 0.8 * dMin;
}

float ObjDf (vec3 p)
{
  float dMin;
  dMin = dstFar;
  dMin = BldgDf (p, dMin);
  dMin = CarDf (p, dMin);
  return dMin;
}

float ObjRay (vec3 ro, vec3 rd)
{
  vec3 p;
  vec2 srd, dda, h;
  float dHit, d;
  srd = - sign (rd.xz);
  dda = - srd / (rd.xz + 0.0001);
  dHit = 0.;
  for (int j = 0; j < 220; j ++) {
    p = ro + dHit * rd;
    h = fract (dda * fract (srd * p.xz));
    d = ObjDf (p);
    dHit += min (d, 0.2 + max (0., min (h.x, h.y)));
    if (d < 0.0002 || dHit > dstFar) break;
  }
  return dHit;
}

vec3 ObjNf (vec3 p)
{
  vec4 v;
  const vec3 e = 0.0001 * vec3 (1., -1., 0.);
  v = vec4 (ObjDf (p + e.xxx), ObjDf (p + e.xyy),
     ObjDf (p + e.yxy), ObjDf (p + e.yyx));
  return normalize (vec3 (v.x - v.y - v.z - v.w) + 2. * v.yzw);
}

float ObjSShadow (vec3 ro, vec3 rd)
{
  float sh, d, h;
  sh = 1.;
  d = 0.05;
  for (int j = 0; j < 20; j ++) {
    h = ObjDf (ro + rd * d);
    sh = min (sh, 30. * h / d);
    d += 0.3 + 0.02 * d;
    if (h < 0.001) break;
  }
  return max (sh, 0.);
}

vec4 ObjCol (vec3 ro, vec3 rd, vec3 vn)
{
  vec3 col;
  vec2 g;
  float wFac, f, ff, spec;
  wFac = 1.;
  col = vec3 (0.);
  spec = 0.;
  if (idObj == idBldgF || idObj == idBldgC) {
    col = HsvToRgb (vec3 (0.7 * Hashfv2 (19. * iqBlk), 0.2,
       0.4 + 0.2 * Hashfv2 (21. * iqBlk)));
    if (abs (vn.y) < 0.05) {
      f = mod (qHit.y / flrHt - 0.2, 1.) - 0.5;
      wFac = 1. - 0.5 * sign (f) * step (abs (abs (f) - 0.24), 0.02) -
         0.801 * step (abs (f), 0.22);
      if (wFac < 0.2) {
        f = (idObj == idBldgF) ? 1.5 * dot (qHit.xz, normalize (vn.zx)) :
           length (qHit.xz) * (atan (qHit.z, qHit.x) + 0.5 * pi);
  	wFac = min (0.2 + 0.8 * floor (fract (f / flrHt + 0.25) *
	   (1. + Hashfv2 (51. * iqBlk))), 1.);
      }
      col *= wFac;
      spec = 0.3;
    } else if (vn.y > 0.95) {
      g = step (0.05, fract (qHit.xz * 70.));
      col *= mix (0.7, 1., g.x * g.y);
    }
    if (wFac > 0.5)
       col *= (0.8 + 0.2 * Noisefv2 (512. * vec2 (qHit.x + qHit.z, qHit.y)));
  } else if (idObj == idTwr) {
    col = vec3 (0.3);
    spec = 0.3;
  } else if (idObj == idTwrTop) {
     col = vec3 (1., 0., 0.);
     spec = -1.;
  } else if (idObj == idSWalk) {
    g = step (0.05, fract (qHit.xz * 35.));
    col = vec3 (0.2) * mix (0.7, 1., g.x * g.y);
  } else if (idObj == idTrLight) {
    f = 2. * (atan (qHit.z, qHit.x) / pi + 1.) + 0.5;
    ff = floor (f);
    if (abs (qHit.y - 0.014) < 0.004 && abs (f - ff) > 0.3) {
      col = mix (vec3 (0., 1., 0.), vec3 (1., 0., 0.),
         (mod (ff, 2.) == 0.) ? cDir : 1. - cDir);
      spec = -2.;
    } else {
      col = vec3 (0.4, 0.2, 0.1);
      spec = 0.5;
    }
  } else if (idObj == idCarBdy) {
    col = HsvToRgb (vec3 (Hashff (qcCar * 37.), 0.9,
       0.4 + 0.6 * vec3 (Hashff (qcCar * 47.))));
    f = abs (qHit.z + 0.003);
    wFac = max (max (step (0.001, f - 0.005) * step (0.001, abs (qHit.x) - 0.0055),
       step (f, 0.001)), step (0.0015, abs (qHit.y - 0.0145)));
    col *= wFac;
    spec = 1.;
    if (abs (qHit.z) > 0.015) {
      g = vec2 (qHit.x, 3. * (qHit.y - 0.008));
      if (qHit.z > 0. && dot (g, g) < 3.6e-5) col *= 0.3;
      g = vec2 (abs (qHit.x) - 0.005, qHit.y - 0.008);
      f = dot (g, g);
      if (qHit.z > 0. && f < 2.2e-6) {
	col = vec3 (1., 1., 0.3);
	spec = -2.;
      } else if (qHit.z < 0. && f < 1.1e-6) {
	col = vec3 (1., 0., 0.);
	spec = -2.;
      }
    }
  } else if (idObj == idCarWhl) {
    if (length (qHit.yz) < 0.0015) {
      col = vec3 (0.7);
      spec = 0.8;
    } else {
      col = vec3 (0.03);
    } 
  } else if (idObj == idRoad) {
    g = abs (fract (qHit.xz) - 0.5);
    if (g.x < g.y) g = g.yx;
    col = mix (vec3 (0.05), vec3 (0.08), step (g.x, 0.355));
    f = (step (abs (g.x - 0.495), 0.0015) + step (abs (g.x - 0.365), 0.0015)) *
       step (g.y, 0.29);
    col = mix (col, vec3 (1., 0.8, 0.3), f);
    f = step (abs (g.x - 0.44), 0.0015) * step (g.y, 0.29) *
       step (fract (g.y * 18. + 0.25), 0.7) +
       step (0.6, fract (g.x * 30. + 0.25)) * step (0.36, g.x) *
       step (abs (g.y - 0.32), 0.02);
    col = mix (col, vec3 (0.8), f);
  }
  if (wFac < 0.5) {
    rd = reflect (rd, vn);
    g = Rot2D (rd.xz, 5.1 * atan (20. + iqBlk.y, 20. +  iqBlk.x));
    col = 0.8 * (0.2 + 0.8 * (step (1., ro.y + 5. * rd.y -
       0.2 * floor (5. * IFbm1 (0.3 * atan (g.y, g.x) + pi) + 0.05)))) *
       SkyCol (ro, rd);
    spec = -1.;
  }
  return vec4 (col, spec);
}

vec3 ShowScene (vec3 ro, vec3 rd)
{
  vec4 col4;
  vec3 col, sCol, vn;
  float dstHit, sh, f, s, fHaze, spec;
  int idObjT;
  sCol = SkyCol (ro, rd);
  idObj = -1;
  dstHit = ObjRay (ro, rd);
  if (dstHit == dstFar) idObj = -1;
  if (dstHit < dstFar) {
    ro += rd * dstHit;
    idObjT = idObj;
    vn = ObjNf (ro);
    idObj = idObjT;
    col4 = ObjCol (ro, rd, vn);
    col = col4.rgb;
    spec = col4.a;
    if (spec >= 0.) {
      if (idObj == idRoad) vn = VaryNf (500. * qHit, vn, 2.);
      else if (idObj == idBldgF || idObj == idBldgC)
         vn = VaryNf (500. * qHit, vn, 0.5);
      sh = mix (1., 0.2 + 0.8 * ObjSShadow (ro, sunDir), fDayLt);
      col = col * (0.1 + 0.1 * max (vn.y, 0.) * sh +
         0.8 * fDayLt * sh * max (dot (vn, sunDir), 0.)) +
	 fDayLt * sh * spec * pow (max (0., dot (sunDir, reflect (rd, vn))), 64.);
    }
    if (spec == -1.) {
      if (idObj == idBldgF || idObj == idBldgC) {
        s = Hashfv2 (37. * iqBlk);
        f = step (fDayLt, 0.2 + 0.3 * Hashfv2 (47. * iqBlk));
        col = mix (0.2 * col, vec3 (0.8 + 0.2 * s, 0.75 - 0.4 * s, 0.), f);
      } else if (idObj == idCarBdy) col = mix (vec3 (0.1, 0.05, 0.), col, fDayLt);
      else if (idObj == idTwrTop) col *= 1. - 0.8 * fDayLt;
    }
    if (spec >= 0.) col *= 0.1 + 0.9 * fDayLt;
    fHaze = (flyMode > 0) ? clamp (8. * (dstHit / dstFar - 0.8), 0., 1.) :
       clamp (4. * (dstHit / dstFar - 0.2), 0., 1.);
    col = mix (col, sCol, fHaze * fHaze);
  } else col = sCol;
  return pow (clamp (col, 0., 1.), vec3 (0.4));
}

vec3 TrackPath (float t)
{
  vec3 p;
  float ti[5], tLin, tCyc, pLen;
  tLin = 1.;
  tCyc = 4. * tLin;
  ti[0] = 0.;
  ti[1] = ti[0] + tLin;
  ti[2] = ti[1] + tLin;
  ti[3] = ti[2] + tLin;
  ti[4] = ti[3] + tLin;
  pLen = 4.;
  p = vec3 (0.);
  p.y = 0.3 + 2.5 * SmoothBump (0.3, 0.8, 0.07, mod (0.11 * t, 1.));
  t = mod (t + scnCount * tLin, tCyc);
  if (t < ti[1]) {
    p.x = pLen;
    p.z = - pLen + 2. * pLen * (t - ti[0]) / (ti[1] - ti[0]);
  } else if (t < ti[2]) {
    p.x =   pLen - 2. * pLen * (t - ti[1]) / (ti[2] - ti[1]);
    p.z = pLen;
  } else if (t < ti[3]) {
    p.x = - pLen;
    p.z =   pLen - 2. * pLen * (t - ti[2]) / (ti[3] - ti[2]);
  } else if (t < ti[4]) {
    p.x = - pLen + 2. * pLen * (t - ti[3]) / (ti[4] - ti[3]);
    p.z = - pLen;
  }
  p.x *= flyDir;
  p.x += (scnCount - 3.) * pLen;
  return p;
}

void VuPM (float t)
{
  vec3 fpF, fpB, vel;
  float a, ca, sa, dt;
  dt = 0.05;
  fpF = TrackPath (t + dt);
  fpB = TrackPath (t - dt);
  vuPos = 0.5 * (fpF + fpB);
  vuPos.y = fpB.y;
  vel = (fpF - fpB) / (2. * dt);
  a = atan (vel.z, vel.x) - 0.5 * pi;
  ca = cos (a);  sa = sin (a);
  vuMat = mat3 (ca, 0., - sa, 0., 1., 0., sa, 0., ca);
}

void mainImage (out vec4 fragColor, in vec2 fragCoord)
{
  vec4 mPtr;
  vec3 ro, rd, col;
  vec2 canvas, uv, ori, ca, sa;
  float el, az, zmFac, tScene, cTime, cTimeI, t, tRep, sunAz, sunEl, asp;
  canvas = iResolution.xy;
  uv = 2. * fragCoord.xy / canvas - 1.;
  uv.x *= canvas.x / canvas.y;
  tCur = iTime;
  mPtr = iMouse;
  mPtr.xy = mPtr.xy / canvas - 0.5;
  tRep = 40.;
  scnCount = floor (tCur / (3. * tRep));
  tScene = tCur - 3. * tRep * scnCount;
  scnCount = mod (scnCount, 10.);
  if (tScene < tRep) flyMode = 0;
  else if (tScene < 2. * tRep) flyMode = 1;
  else flyMode = 2;
  flyDir = sign (mod (tCur, 6. * tRep) - 3. * tRep);
  t = mod (0.013 * tCur + 0.2, 1.);
  fDayLt = SmoothBump (0.2, 0.8, 0.05, t);
  sunAz = 0.8 * pi * (2. * t - 1.);
  sunEl = 0.97 * (1. - 0.7 * sin (abs (0.5 * sunAz)));
  sunDir = vec3 (0., 0., - 1.);
  sunDir.xz = Rot2D (sunDir.xz, sunAz);
  sunDir.yz = Rot2D (sunDir.yz, sunEl);
  cTime = 0.15 * tScene;
  cTimeI = floor (cTime);
  cDir = mod (cTimeI, 2.);
  cTimeV = vec2 (floor (0.5 * cTimeI), cTime - cTimeI);
  if (flyMode == 0) dstFar = 50.;
  else dstFar = 70.;
  if (flyMode == 0) {
    az = 0.;
    el = -0.05;
    zmFac = 4.8;
    if (mPtr.z > 0.) {
      az -= 2. * pi * mPtr.x;
      el = clamp (el - mPtr.y, -1.4, 0.3);
    }
  } else if (flyMode == 1) {
    az = 0.;
    el = 0.;
    zmFac = 2.2;
    if (mPtr.z > 0.) zmFac = clamp (zmFac + 3. * mPtr.y, 0.7, 4.);
  } else if (flyMode == 2) {
    az = 0.033 * tCur * flyDir;
    t = SmoothBump (0.3, 0.8, 0.2, mod (0.025 * tCur, 1.));
    el = 0.1 + 0.8 * t;
    zmFac = 3.6 + 11. * t;
  }
  if (flyMode == 0) {
    ro = vec3 (-0.03, 0.1, scnCount + cTimeV.x + ((cDir == 1.) ? cTimeV.y : 0.));
  } else if (flyMode == 1) {
    VuPM (0.1 * tCur);
    ro = vuPos;
  } else if (flyMode == 2) {
    ori = vec2 (el, az);
    ca = cos (ori);  sa = sin (ori);
    vuMat = mat3 (1., 0., 0., 0., ca.x, - sa.x, 0., sa.x, ca.x) *
       mat3 (ca.y, 0., - sa.y, 0., 1., 0., sa.y, 0., ca.y);
    ro = vec3 (0., 1., -25.) * vuMat;
    if (mPtr.z > 0.) {
      az -= 0.5 * pi * mPtr.x;
      el = clamp (el - 2. * mPtr.y, -0.1, 1.5);
    }
  }
  if (flyMode == 0 || flyMode == 2) {
    ori = vec2 (el, az);
    ca = cos (ori);  sa = sin (ori);
    vuMat = mat3 (1., 0., 0., 0., ca.x, - sa.x, 0., sa.x, ca.x) *
       mat3 (ca.y, 0., - sa.y, 0., 1., 0., sa.y, 0., ca.y);
  }
  rd = normalize (vec3 (uv, zmFac)) * vuMat;
  col = ShowScene (ro, rd);
  asp = canvas.y / canvas.x;
  t = step (50. * abs (mod (tCur / tRep + 0.5, 1.) - 0.5),
     max (abs (uv.x * asp), abs (uv.y)) / max (asp, 1.));
  col = mix (col, vec3 (0.1, 0.1, 0.2), t);
  fragColor = vec4 (col, 1.);
}

