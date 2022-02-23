//stage GL_FRAGMENT_SHADER
//import <GLUP/ShaderToy.h>

// "Toy Train" by dr2 - 2014
// License: Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License

const vec4 cHashA4 = vec4 (0., 1., 57., 58.);
const vec3 cHashA3 = vec3 (1., 57., 113.);
const float cHashM = 43758.54;

vec4 Hashv4f (float p)
{
  return fract (sin (p + cHashA4) * cHashM);
}

float Hashfv2 (vec2 p)
{
  return fract (sin (dot (p, cHashA3.xy)) * cHashM);
}

float Noisefv2 (vec2 p)
{
  vec2 i = floor (p);
  vec2 f = fract (p);
  f = f * f * (3. - 2. * f);
  vec4 t = Hashv4f (dot (i, cHashA3.xy));
  return mix (mix (t.x, t.y, f.x), mix (t.z, t.w, f.x), f.y);
}

vec3 BrickSurfCol (vec2 p) {
  vec2 q = p * (1. / 20.);
  vec2 i = floor (q);
  if (2. * floor (i.y / 2.) != i.y) {
    q.x += 0.5;
    i = floor (q);
  }
  q = smoothstep (0.015, 0.025, abs (fract (q + 0.5) - 0.5));
  return (1. + Noisefv2 (10. * p)) * (0.3 + 0.7 * q.x * q.y) *
     (0.3 + 0.2 * sin (2. * Hashfv2 (i) + vec3 (1., 1.2, 1.4)));
}

vec3 BrickCol (vec3 p, vec3 n)
{
  n = abs (n);
  p *= 150.;
  return BrickSurfCol (p.zy) * n.x + BrickSurfCol (p.xz) * n.y +
     BrickSurfCol (p.xy) * n.z;
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

float PrCapsDf (vec3 p, float r, float h)
{
  return length (p - vec3 (0., 0., h * clamp (p.z / h, -1., 1.))) - r;
}

int idObj;
mat3 trainMat[3], trMat;
vec3 trainPos[3], trPos, qHit, ltDir;
float tCur, tRun, trVel, trkRad, trkLin, trkWid;
const float dstFar = 40., sFac = 0.2;
bool onTrk, sigStop;
const float pi = 3.14159;

const int idEng = 1, idCabin = 2, idCoal = 3, idBase = 4, idBand = 4,
   idAxle = 4, idCar = 5, idRoof = 6, idWheel = 7, idSpoke = 7, idCrod = 8,
   idFunl = 9, idFunt = 10, idStripe = 10, idLamp = 11, idRail = 12,
   idTie = 13, idPlat = 14, idSig = 15, idCpl = 16, idGrnd = 17;

float EngDf (vec3 p, float dHit)
{
  vec3 q;
  float d, aw, a;
  float wRad = 0.8;
  q = p;
  q -= vec3 (0., -0.2, 0.5);
  d = PrCapsDf (q, 1., 2.);
  d = max (d, - (q.z + 1.7));
  if (d < dHit) { dHit = d;  idObj = idEng; }
  q = p;
  q.z = abs (q.z - 0.85);
  q -= vec3 (0., -0.2, 1.8);
  d = PrCylDf (q, 1.05, 0.05);
  if (d < dHit) { dHit = d;  idObj = idBand; }
  q = p;
  q -= vec3 (0., -1.3, -0.25);
  d = PrBoxDf (q, vec3 (1., 0.1, 3.2));
  if (d < dHit) { dHit = d;  idObj = idBase; }
  q = p;
  q -= vec3 (0., -1.4, 3.);
  d = PrBoxDf (q, vec3 (1.1, 0.2, 0.07));
  if (d < dHit) { dHit = d;  idObj = idBase; }
  q.x = abs (q.x);
  q -= vec3 (0.6, 0., 0.1);
  d = PrCylDf (q, 0.2, 0.1);
  if (d < dHit) { dHit = d;  idObj = idRoof; }
  q = p;
  q -= vec3 (0., 0.01, -1.75);
  d = max (max (PrBoxDf (q, vec3 (1., 1.4, 0.6)),
     - PrBoxDf (q - vec3 (0., 0., -0.2), vec3 (0.95, 1.3, 0.65))),
     - PrBoxDf (q - vec3 (0., 0.7, 0.), vec3 (1.1, 0.4, 0.5)));
  q.x = abs (q.x);
  q -= vec3 (0.4, 1., 0.4);
  d = max (d, - PrBoxDf (q, vec3 (0.35, 0.15, 0.3)));
  if (d < dHit) { dHit = d;  idObj = idCabin;  qHit = q; }
  q = p;
  q -= vec3 (0., -2.4, -1.75);
  d = max (PrCylDf (q, 4., 0.65), - (q.y - 3.75));
  if (d < dHit) { dHit = d;  idObj = idRoof; }
  q = p;
  q -= vec3 (0., -0.5, -3.15);
  d = PrBoxDf (q, vec3 (1., 0.7, 0.3));
  if (d < dHit) { dHit = d;  idObj = idCoal;  qHit = q; }
  q = p;
  q -= vec3 (0., -1.4, -3.5);
  d = PrCylDf (q.xzy, 0.4, 0.03);
  if (d < dHit) { dHit = d;  idObj = idCpl; }
  q = p;
  q.xz = abs (q.xz);
  q -= vec3 (trkWid - 0.12, -1.4, 1.1);
  d = min (max (min (PrCylDf (q.zyx, wRad, 0.1),
     PrCylDf (q.zyx - vec3 (0.,0., -0.07), wRad + 0.05, 0.03)),
     - PrCylDf (q.zyx, wRad - 0.1, 0.12)), PrCylDf (q.zyx, 0.15, 0.10));
  if (d < dHit) { dHit = d;  idObj = idWheel; }
  q = p;
  q.x = abs (q.x);
  q -= vec3 (trkWid - 0.17, -1.4, 1.1 * sign (q.z));
  aw = - (trVel / wRad) * tRun;
  q.yz = q.yz * cos (aw) * vec2 (1., 1.) + q.zy * sin (aw) * vec2 (-1., 1.);  
  a = floor ((atan (q.y, q.z) + pi) * 8. / (2. * pi) + 0.5) / 8.;
  q.yz = q.yz * cos (2. * pi * a) * vec2 (1., 1.) +
     q.zy * sin (2. * pi * a) * vec2 (-1., 1.);
  q.z += 0.5 * wRad;
  d = PrCylDf (q, 0.05, 0.5 * wRad);
  if (d < dHit) { dHit = d;  idObj = idSpoke; }
  q = p;
  float sx = sign (q.x);
  q.x = abs (q.x);
  q -= vec3 (trkWid + 0.08, -1.4, 0.);
  aw -= 0.5 * pi * sx; 
  q.yz -= 0.3 * vec2 (cos (aw), - sin (aw));
  d = PrCylDf (q, 0.04, 1.2);
  if (d < dHit) { dHit = d;  idObj = idCrod; }
  q.z = abs (q.z);
  q -= vec3 (-0.1, 0., 1.1);
  d = PrCylDf (q.zyx, 0.06, 0.15);
  if (d < dHit) { dHit = d;  idObj = idCrod; }
  q = p;
  q.z = abs (q.z);
  q -= vec3 (0., -1.4, 1.1);
  d = PrCylDf (q.zyx, 0.1, trkWid - 0.1);
  if (d < dHit) { dHit = d;  idObj = idAxle; }
  q = p;
  q -= vec3 (0., 1.1, 2.15);
  d = PrCylDf (q.xzy, 0.3, 0.5);
  if (d < dHit) { dHit = d;  idObj = idFunl; }
  q = p;
  q -= vec3 (0., 1.5, 2.15);
  d = max (PrCylDf (q.xzy, 0.4, 0.15), - PrCylDf (q.xzy, 0.3, 0.2));
  if (d < dHit) { dHit = d;  idObj = idFunt; }
  q = p;
  q -= vec3 (0., 0.8, 0.55);
  d = PrCapsDf (q.xzy, 0.3, 0.2);
  if (d < dHit) { dHit = d;  idObj = idFunt; }
  q = p;
  q.x = abs (q.x);
  q -= vec3 (1., -0.2, 0.85);
  d = PrBoxDf (q, vec3 (0.05, 0.1, 1.8));
  if (d < dHit) { dHit = d;  idObj = idStripe; }
  q = p;
  q.x = abs (q.x);
  q -= vec3 (1., -0.2, -1.75);
  d = PrBoxDf (q, vec3 (0.05, 0.1, 0.6));
  if (d < dHit) { dHit = d;  idObj = idStripe; }
  q = p;
  q.x = abs (q.x);
  q -= vec3 (1., -0.2, -3.15);
  d = PrBoxDf (q, vec3 (0.05, 0.1, 0.3));
  if (d < dHit) { dHit = d;  idObj = idStripe; }
  q = p;
  q -= vec3 (0., -0.2, 3.5);
  d = PrCylDf (q, 0.2, 0.1);
  if (d < dHit) { dHit = d;  idObj = idLamp; }
  return dHit;
}

float CarDf (vec3 p, float dHit)
{
  vec3 q;
  float d;
  float wRad = 0.35;
  q = p;
  d = max (max (PrBoxDf (q, vec3 (1.3, 1.4, 2.8)),
     - PrBoxDf (q, vec3 (1.2, 1.3, 2.7))),
     - PrBoxDf (q, vec3 (0.5, 1., 2.9)));
  q.z = abs (q.z);
  q -= vec3 (0., 0.6, 1.2);
  d = max (d, - PrBoxDf (q, vec3 (1.4, 0.7, 1.1)));
  if (d < dHit) { dHit = d;  idObj = idCar; qHit = q; }
  q = p; 
  q.y -= -2.35;
  d = max (PrCylDf (q, 4., 2.8), - (q.y - 3.75));
  if (d < dHit) { dHit = d;  idObj = idRoof; }
  q = p;
  q.z = abs (q.z);
  q -= vec3 (0., -0.2, 2.75);
  d = PrCylDf (q.zyx, 0.05, 0.5);
  if (d < dHit) { dHit = d;  idObj = idRoof;  qHit = q; }
  q = p;
  q.y -= -1.6;
  d = PrBoxDf (q, vec3 (0.8, 0.3, 2.));
  if (d < dHit) { dHit = d;  idObj = idBase; }
  q = p;
  q.z = abs (q.z);
  q -= vec3 (0., -1.4, 2.9);
  d = PrCylDf (q.xzy, 0.4, 0.03);
  if (d < dHit) { dHit = d;  idObj = idCpl; }
  q = p;
  q.xz = abs (q.xz);
  q -= vec3 (trkWid - 0.12, -1.85, 1.1);
  d = min (min (PrCylDf (q.zyx, wRad, 0.1),
     PrCylDf (q.zyx - vec3 (0.,0., -0.07), wRad + 0.05, 0.03)),
     PrCylDf (q.zyx, 0.15, 0.10));
  q.x -= 0.1;
  d = max (d, - (PrCylDf (q.zyx, 0.2, 0.05)));
  if (d < dHit) { dHit = d;  idObj = idWheel; }
  q = p;
  q.z = abs (q.z);
  q -= vec3 (0., -1.85, 1.1);
  d = PrCylDf (q.zyx, 0.1, trkWid - 0.15);
  if (d < dHit) { dHit = d;  idObj = idAxle; }
  q = p;
  q.x = abs (q.x);
  q -= vec3 (1.3, -0.2, 0.);
  d = PrBoxDf (q, vec3 (0.05, 0.1, 2.8));
  if (d < dHit) { dHit = d;  idObj = idStripe; }
  return dHit;
}

float TrackDf (vec3 p, float dHit)
{
  vec3 q;
  float d, nt;
  float gHt = 2.8;
  q = p;
  if (onTrk) {
    q.z -= 0.5 * trkLin * clamp (p.z / (0.5 * trkLin), -1., 1.);
    q.x = abs (length (q.xz) - trkRad);
    q -= vec3 (trkWid - 0.03, - gHt + 0.45, 0.);
    d = length (max (abs (q.xy) - vec2 (0.07, 0.13), 0.));
  } else {
    q.x = abs (q.x);
    q -= vec3 (trkWid - 0.03, - gHt + 0.45, 0.);
    d = max (length (max (abs (q.xy) - vec2 (0.07, 0.13), 0.)),
       abs (q.z) - 0.5 * trkLin);
  }
  if (d < dHit) { dHit = d;  idObj = idRail; }
  q = p;
  if (onTrk) q.x = abs (q.x) - trkRad;
  q -= vec3 (0., - gHt + 0.2, 0.);
  nt = 2.;
  float gap = trkLin / nt;
  q.z = mod (q.z + 0.5 * gap, gap) - 0.5 * gap;
  d = PrBoxDf (q, vec3 (trkWid + 0.5, 0.2, 0.4));
  q = p;
  q.y -= - gHt + 0.2;
  d = max (d, PrBoxDf (q, vec3 (trkRad + 3., 2., 0.5 * trkLin + 2.)));
  if (d < dHit) { dHit = d;  idObj = idTie; }
  if (onTrk) {
    q = p;
    q.x = abs (q.x);
    q -= vec3 (0., - gHt + 0.2, 0.5 * trkLin * sign (q.z));
    nt = 12.;
    float a = floor ((atan (q.x, q.z) + pi) * nt / (2. * pi) + 0.5) / nt;
    q.xz = q.xz * cos (2. * pi * a) * vec2 (1., 1.) +
       q.zx * sin (2. * pi * a) * vec2 (-1., 1.);
    q.z -= - trkRad;
    d = PrBoxDf (q, vec3 (0.4, 0.2, trkWid + 0.5));
    q = p;
    q.y -= - gHt + 0.2;
    d = max (d, - PrBoxDf (q, vec3 (trkRad + 3., 2., 0.5 * trkLin + 2.)));
    if (d < dHit) { dHit = d;  idObj = idTie; }
    q = p;
    q -= vec3 (trkRad - trkWid - 2., - gHt + 0.6, 0.);
    d = max (PrBoxDf (q, vec3 (trkWid, 0.4, 14.)), 0.5 * (abs (q.z) - 7.) + q.y);
    q -= vec3 (-1.2, 1.9, 0.);
    d = min (d, PrBoxDf (q, vec3 (0.2, 1.8, 5.)));
    q.z = abs (q.z) - 2.4;
    d = max (d,  - PrBoxDf (q, vec3 (0.3, 1.3, 1.7)));
    if (d < dHit) { dHit = d;  idObj = idPlat;  qHit = q; }
    q = p;
    q -= vec3 (trkRad - trkWid - 2.5, 0.8, 6.);
    d = PrCylDf (q.xzy, 0.15, 3.);
    if (d < dHit) { dHit = d;  idObj = idRail; }
    q.y -= 3.;
    d = PrSphDf (q, 0.35);
    if (d < dHit) { dHit = d;  idObj = idSig; } 
    q = p;
    q.y -= - gHt;
    d = PrBoxDf (q, vec3 (trkRad + trkWid + 2.5, 0.1,
       trkRad + 0.5 * trkLin + trkWid + 2.5));
    if (d < dHit) { dHit = d;  idObj = idGrnd;  qHit = q; } 
  }
  return dHit;
}

void TrainCarPM (float s)
{
  float a, ca, sa;
  if (onTrk) {
    s = mod (s, 2. * (pi * trkRad + trkLin));
    if (s < trkLin) {
      trPos = vec3 (trkRad, 0., s - 0.5 * trkLin);
      ca = 1.;  sa = 0.;
    } else if (s < trkLin + pi * trkRad) {
      a = (s - trkLin) / trkRad;
      ca = cos (a);  sa = sin (a);
      trPos = vec3 (trkRad * ca, 0., 0.5 * trkLin + trkRad * sa);
    } else if (s < 2. * trkLin + pi * trkRad) {
      trPos = vec3 (- trkRad, 0., 1.5 * trkLin + pi * trkRad - s);
      ca = -1.;  sa = 0.;
    } else {
      a = (s - (pi * trkRad + 2. * trkLin)) / trkRad + pi;
      ca = cos (a);  sa = sin (a);
      trPos = vec3 (trkRad * ca, 0., - 0.5 * trkLin + trkRad * sa);
    }
  } else {
    trPos = vec3 (0., 0., 0.3 * trkLin + s);
    ca = 1.;  sa = 0.;
  }
  trMat = mat3 (ca, 0., - sa, 0., 1., 0., sa, 0., ca);
}

float ObjDf (vec3 p)
{
  float dHit = dstFar / sFac;
  p /= sFac;
  dHit = EngDf (trainMat[0] * (p - trainPos[0]), dHit);
  dHit = CarDf (trainMat[1] * (p - trainPos[1]), dHit);
  dHit = CarDf (trainMat[2] * (p - trainPos[2]), dHit);
  dHit = TrackDf (p, dHit);
  return dHit * sFac;
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
  float d = 0.02;
  for (int i = 0; i < 30; i++) {
    float h = ObjDf (ro + rd * d);
    sh = min (sh, 20. * h / d);
    d += 0.02 + 0.01 * d;
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

vec4 ObjCol (vec3 n)
{
  const vec4 cLo = vec4 (0.4, 0.2, 0.1, 1.), cHi = vec4 (0., 1., 0., 1.),
     cBlk = vec4 (vec3 (0.01), 0.1), cCab = vec4 (0.2, 0.2, 0.8, 1.),
     cRf = vec4 (1., 0.1, 0.1, 1.);
  vec4 col4;
  if (idObj == idGrnd) col4 =
     vec4 (0., 0.2, 0., 0.) + 0.03 * Noisefv2 (10. * qHit.xz);
  else if (idObj == idEng) col4 = cHi;
  else if (idObj == idCabin) col4 = (qHit.y > -1.3) ? cCab : cLo;
  else if (idObj == idCoal)
     col4 = (qHit.y > 0.3) ? ((n.y > 0.9) ? cBlk : cCab) : cLo;
  else if (idObj == idBase || idObj == idBand || idObj == idAxle)
     col4 = vec4 (0.3, 0.2, 0.2, 0.3);
  else if (idObj == idCar) col4 = (qHit.y > -0.8) ? cHi : cLo;
  else if (idObj == idRoof) col4 = cRf;
  else if (idObj == idWheel || idObj == idSpoke) col4 = vec4 (0.5, 0.5, 0.6, 2.);
  else if (idObj == idCrod) col4 = vec4 (0.7, 0.7, 0.1, 1.);
  else if (idObj == idFunl) col4 = (n.y > 0.9) ? cBlk : cRf;
  else if (idObj == idStripe || idObj == idFunt) col4 = vec4 (1., 1., 0., 1.);
  else if (idObj == idLamp)
     col4 = (mod (tCur, 2.) < 1.) ? vec4 (5.) : vec4 (1.);
  else if (idObj == idRail) col4 = vec4 (0.8, 0.8, 0.8, 1.);
  else if (idObj == idTie) col4 = vec4 (0.4, 0.2, 0.2, 0.3);
  else if (idObj == idPlat)
     col4 = vec4 (vec3 (0.5, 0.3, 0.3) * BrickCol (0.3 * qHit, n), 1.);
  else if (idObj == idSig)
    col4 = sigStop ? vec4 (1., 0., 0., 0.5) : vec4 (0., 1., 0., 0.5);
  else if (idObj == idCpl) col4 = vec4 (0.2, 0.1, 0.2, 0.5);
  return col4;
}

vec3 ShowScene (vec3 ro, vec3 rd)
{
  vec3 vn, objCol;
  float dstHit;
  vec3 col = vec3 (0., 0., 0.04);
  idObj = -1;
  dstHit = ObjRay (ro, rd);
  if (idObj < 0) dstHit = dstFar;
  if (dstHit < dstFar) {
    ro += rd * dstHit;
    int idObjT = idObj;
    if (idObj != idGrnd) vn = ObjNf (ro);
    else vn = vec3 (0., 1., 0.);
    idObj = idObjT;
    vec4 col4 = ObjCol (vn);
    objCol = col4.xyz;
    float spec = col4.w;
    float dif = max (dot (vn, ltDir), 0.);
    float ao = ObjAO (ro, vn);
    float sh;
    if (idObj != idGrnd || length (qHit.xy) > trkRad - 3. * trkWid)
       sh = ObjSShadow (ro, ltDir);
    else sh = 1.;
    col = objCol * (0.2 * ao * (1. +
       max (dot (vn, - normalize (vec3 (ltDir.x, 0., ltDir.z))), 0.)) +
       max (0., dif) * sh * (dif + ao * spec *
       pow (max (0., dot (ltDir, reflect (rd, vn))), 64.)));
  }
  col = sqrt (clamp (col, 0., 1.));
  return col;
}

void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
  vec2 uv = 2. * fragCoord.xy / iResolution.xy - 1.;
  uv.x *= iResolution.x / iResolution.y;
  float zmFac = 6.;
  tCur = iTime;
  float dist = 15.;
  trkRad = 20.;
  trkLin = 20.;
  trkWid = 1.42;
  trVel = 6.;
  float tCyc = 2. * (pi * trkRad + trkLin) / trVel;
  float tPause = 0.1 * tCyc;
  tCyc += tPause;
  onTrk = (mod (floor (tCur / tCyc), 10.) != 5.);
  if (onTrk) {
    tRun = mod (tCur, tCyc);
    float tHalt = trkLin / trVel;
    sigStop = (tRun < tHalt + 0.8 * tPause);
    if (tRun > tHalt + tPause) tRun = tRun - tPause;
    else if (tRun > tHalt) tRun = tHalt;
  } else tRun = 0.;
  TrainCarPM (trVel * tRun);
  trainPos[0] = trPos;  trainMat[0] = trMat;
  TrainCarPM (trVel * tRun - 7.);
  trainPos[1] = trPos;  trainMat[1] = trMat;
  TrainCarPM (trVel * tRun - 13.4);
  trainPos[2] = trPos;  trainMat[2] = trMat;
  vec3 ro, rd, vd;
  if (onTrk) {
    ro = vec3 (- dist, 0.4 * dist, 0.25 * (trainPos[0] + trainPos[1]).z * sFac);
    vd = normalize (vec3 (- ro.xy, 0.));
    ro.y -= 0.04 * dist;
    vec3 u = - vd.y * vd;
    float f = 1. / sqrt (1. - vd.y * vd.y);
    mat3 scMat = mat3 (f * vec3 (vd.z, 0., - vd.x), f * vec3 (u.x, 1. + u.y, u.z), vd);
    rd = scMat * normalize (vec3 (uv, zmFac));
    ltDir = normalize (vec3 (-0.3, 1., -1.));
  } else {
    trVel = 1.;
    tRun = tCur;
    float ph = 2. * pi * tCur / tCyc;
    float el = 0.2 - 0.25 * sin (ph);
    float az = 0.5 * pi + ph;
    float cEl = cos (el);
    float sEl = sin (el);
    float cAz = cos (az);
    float sAz = sin (az);
    mat3 vuMat = mat3 (1., 0., 0., 0., cEl, - sEl, 0., sEl, cEl) *
       mat3 (cAz, 0., sAz, 0., 1., 0., - sAz, 0., cAz);
    rd = normalize (vec3 (uv, 1.8 * zmFac)) * vuMat;
    ro = - vec3 (0., 0.01 * dist, dist) * vuMat;
    ltDir = normalize (vec3 (0.3, 0.5, -1.)) * vuMat;
  }
  vec3 col = ShowScene (ro, rd);
  fragColor = vec4 (col, 1.);
}

