//stage GL_FRAGMENT_SHADER
//import <GLUP/ShaderToy.h>

vec2 scale = vec2(-4.0, -4.0);
vec2 offset = vec2(0.5, 0.5);
vec2 tscale = vec2(0.3333, 0.5);
vec4 tint = vec4(1.0, 1.0, 1.0, 1.0);
vec4 multi = vec4(2.0, 3.0, 5.0, 2.0);
vec4 phase = vec4(0.5, 0.1, 0.2, 0.0);

float speed = 2.0;

void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
    float time = iTime * speed;

    // Normalized pixel coordinates (from 0 to 1)
    vec2 uv = fragCoord/iResolution.xy;
    uv *= scale;
    uv += offset;
    
	vec2 vtime = tscale * time;
    float sytime = sin(vtime.y);
    float yftime = uv.y * sytime;
	
    float xtime = sin(time * 0.2);
    float xytime = (uv.x * sin(vtime.x)) + yftime;
    
    vec2 r_uv = uv * multi.r;
    vec2 g_uv = uv * multi.g;
    vec2 b_uv = uv * multi.b;
    vec2 a_uv = uv * multi.a;

    // Time varying pixel color
    float r = sin(r_uv.y + r_uv.x + time + phase.r);
    float g = sin(g_uv.y + g_uv.x + time + phase.g);
    float b = sin(b_uv.y + b_uv.x + time + phase.b);
    float a = sin(a_uv.y + a_uv.x + time + phase.a);
    vec4 v1 = vec4(r,g,b,a);
       
    float r2 = sin(multi.r * xytime + time + phase.r); 
    float g2 = sin(multi.g * xytime + time + phase.g); 
    float b2 = sin(multi.b * xytime + time + phase.b); 
    float a2 = sin(multi.a * xytime + time + phase.a); 
    vec4 v2 = vec4(r2, g2, b2, a2);

    float cy2 = pow(uv.y + (sytime * 0.5), 2.0);
    float cx2 = pow(uv.x + sin(xtime), 2.0);
    float cxcy = cx2 + cy2;
    
    vec4 mu = multi * cxcy;
        
    float r3 = clamp(sqrt(mu.r), 0.0, 1.0); 
    float g3 = clamp(sqrt(mu.g), 0.0, 1.0);
    float b3 = clamp(sqrt(mu.b), 0.0, 1.0);
    float a3 = clamp(sqrt(mu.a), 0.0, 1.0);
    vec4 v3 = vec4(r3, g3, b3, a3);
    
    vec4 v = (v1 + v2 + v3 + tint) * 0.25;
    v *= tint;
    
    // Output to screen
    fragColor = v;
    //fragColor = vec4(v.r, 0., 0., 1.0);
}
