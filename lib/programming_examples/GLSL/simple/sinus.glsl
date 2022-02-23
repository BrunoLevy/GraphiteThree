//stage GL_FRAGMENT_SHADER
//import <GLUP/portable_fragment_shader.h>

// GLSL example: draws a square with colored stripes
//
// Usage: 
//   Objects->MeshGrob->New Mesh Object -> Name = foobar
//   Objects->MeshGrob->Send GLSL to...-> foobar

void main(void) {
     vec2 uv = tex_coord.xy;
     glup_FragColor = vec4(
        0.5*(1.0 + sin(uv.x*10.0)), 
        0.5*(1.0 + sin(uv.y*10.0)), 
        0.5*(1.0 + sin((uv.x+uv.y) * 10.0)), 
        1.0
     );
}









