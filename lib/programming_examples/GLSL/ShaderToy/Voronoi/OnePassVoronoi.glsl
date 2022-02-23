//stage GL_FRAGMENT_SHADER
//import <GLUP/ShaderToy.h>

// "One-Pass Voronoi" by dr2 - 2018
// License: Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License

vec2 PixToHex (vec2 p);
vec2 HexToPix (vec2 h);
void HexVorInit ();
vec4 HexVor (vec2 p);
vec3 HsvToRgb (vec3 c);
float SmoothMin (float a, float b, float r);
float SmoothBump (float lo, float hi, float w, float x);
vec2 Rot2D (vec2 q, float a);
float Hashfv2 (vec2 p);
vec2 Hashv2v2 (vec2 p);

float tCur;
const float pi = 3.14159, sqrt3 = 1.7320508;

vec3 ShowScene (vec2 p)
{
  vec4 vc, col4;
  vec3 col, ltDir, vn;
  ltDir = normalize (vec3 (0., 1., 0.7));
  ltDir.xz = Rot2D (ltDir.xz, -0.2 * tCur);
  HexVorInit ();
  vc = HexVor (p - 0.2 * tCur);
  vc.yz *= smoothstep (0.1, 0.15, vc.x);
  vn = normalize (vec3 (vc.y, 0.6, vc.z));
  col4 = vec4 (HsvToRgb (vec3 (mod (0.1 * tCur + atan (vn.z, - vn.x) / (2. * pi), 1.), 0.9, 1.)), 0.2);
  col4 = mix (vec4 (0.3, 0.5, 0.3, 0.5), col4, smoothstep (0.1, 0.2, vc.x));
  col4 *= 0.6 + 0.4 * mix (SmoothBump (0.25, 0.75, 0.05, mod (12. * vc.x, 1.)),
     SmoothBump (0.25, 0.75, 0.05, mod (2. * vc.x / (1. - 0.8 * vn.y), 1.)), smoothstep (0.1, 0.2, vc.x));
  col = col4.rgb * (0.2 + 0.8 * max (dot (vn, ltDir), 0.)) +
     col4.a * pow (max (dot (normalize (ltDir + vn), vn), 0.), 32.);
  return col;
}

void mainImage (out vec4 fragColor, vec2 fragCoord)
{
  vec3 col;
  vec2 canvas, uv, p;
  float pSize;
  canvas = iResolution.xy;
  uv = 2. * fragCoord.xy / canvas - 1.;
  uv.x *= canvas.x / canvas.y;
  tCur = iTime;
  tCur += 20.;
  pSize = canvas.x / 200.;
  p = pSize * uv;
  fragColor = vec4 (ShowScene (p), 1.);
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
  amp = 0.65;
  ip = PixToHex (p);
  fp = p - HexToPix (ip);
  sd = vec4 (4.);
  udm = vec4 (4.);
  for (int k = 0; k < 7; k ++) {
    u = Hashv2v2 (ip + gVec[k]);
    a = 0.5 * pi * (u.y - 0.5) * tCur;
    d = hVec[k] + amp * (0.4 + 0.6 * u.x) * vec2 (cos (a), sin (a)) - fp;
    sd.w = dot (d, d);
    if (sd.w < sd.x) {
      sd = sd.wxyw;
      udm = vec4 (d, u);
    } else sd = (sd.w < sd.y) ? sd.xwyw : ((sd.w < sd.z) ? sd.xyww : sd);
  }
  sd.xyz = sqrt (sd.xyz);
  return vec4 (SmoothMin (sd.y, sd.z, 0.2) - sd.x, udm.xy, Hashfv2 (udm.zw));
}

vec3 HsvToRgb (vec3 c)
{
  vec3 p;
  p = abs (fract (c.xxx + vec3 (1., 2./3., 1./3.)) * 6. - 3.);
  return c.z * mix (vec3 (1.), clamp (p - 1., 0., 1.), c.y);
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
  return q * cos (a) + q.yx * sin (a) * vec2 (-1., 1.);
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
