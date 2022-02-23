//stage GL_FRAGMENT_SHADER
//import <GLUP/ShaderToy.h>
// Created by Matthew Arcus, 2018
// Minimal animated Cairo tiling
void mainImage(out vec4 c, vec2 p) {
  p = 2.0*fract(2.5*p/iResolution.y)-1.0;
  if (p.x*p.y < 0.0) p = p.yx;
  p = 1.0-2.0*abs(abs(p)-0.5);
  vec2 n = vec2(sin(iTime),cos(iTime));
  vec2 m = vec2(-n.y,n.x);
  float d = dot(p,n), e = dot(p,m);
  float k = min(abs(d),abs(e));
  if (d < 0.0) k = min(k,abs(p.x-1.0));
  if (e < 0.0) k = min(k,abs(p.y-1.0));
  c = vec4(smoothstep(0.0,0.05,k)*vec3(1),1);
}
