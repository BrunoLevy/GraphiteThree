//stage GL_FRAGMENT_SHADER
//import <GLUP/ShaderToy.h>

// "Vortex Street" by dr2 - 2015
// License: Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License

// Motivated by implementation of van Wijk's IBFV by eiffie (lllGDl) and andregc (4llGWl) 

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
  for (int i = 0; i < 6; i ++) {
    s += a * Noisefv2 (p);
    a *= 0.5;
    p *= 2.;
  }
  return s;
}

float tCur;

vec2 VortF (vec2 q, vec2 c)
{
  vec2 d = q - c;
  return 0.25 * vec2 (d.y, - d.x) / (dot (d, d) + 0.05);
}

vec2 FlowField (vec2 q)
{
  vec2 vr, c;
  float dir = 1.;
  c = vec2 (mod (tCur, 10.) - 20., 0.6 * dir);
  vr = vec2 (0.);
  for (int k = 0; k < 30; k ++) {
    vr += dir * VortF (4. * q, c);
    c = vec2 (c.x + 1., - c.y);
    dir = - dir;
  }
  return vr;
}

void mainImage (out vec4 fragColor, in vec2 fragCoord)
{
  vec2 uv = gl_FragCoord.xy / iResolution.xy - 0.5;
  uv.x *= iResolution.x / iResolution.y;
  tCur = iTime;
  vec2 p = uv;
  for (int i = 0; i < 10; i ++) p -= FlowField (p) * 0.03;
  vec3 col = Fbm2 (5. * p + vec2 (-0.1 * tCur, 0.)) *
     vec3 (0.5, 0.5, 1.);
  fragColor = vec4 (col, 1.);
}

