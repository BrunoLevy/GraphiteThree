//stage GL_FRAGMENT_SHADER
//import <GLUP/ShaderToy.h>

float random (in float x) {
    return fract(sin(x)*1e5);
}

float random (in vec2 st) {
    return fract(cos(dot(st.xy, vec2(12.9898,78.233)))* 43758.5453123);
}

float pattern(vec2 st, vec2 v, float t) {
    vec2 p = floor(st + v);
    return step(t, random(100. + p *.0001) + random(p.y) * 0.5 );
}

void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
    vec2 st = fragCoord.xy / iResolution.xy;
    st.x *= iResolution.x / iResolution.y;

    vec2 grid = vec2(50.0, 25.);
    st *= grid;

    vec2 ipos = floor(st);
    vec2 fpos = fract(st);

    // Time.
    vec2 vel = vec2(iTime * 0.1 * max(grid.y, grid.x)); 
    
    // Direction.
    vel *= vec2(0., -1.2) * random(1.0 + ipos.x);
    
    // Backward moving.
    if(fpos.x * 0.5 < 0.25)
    {
       vel.y *= -1.;
    }

    // Assign a random value base on the integer coord.
    vec2 offset = vec2(0.1, 0.1);

    vec3 color = vec3(0.);
    color.r = pattern(st + offset, vel, 0.3 + clamp(abs(sin(iTime* 0.4)), 0.2, 0.8));
    color.g = pattern(st , vel, 0.3 + clamp(abs(sin(iTime * 0.4)), 0.2, 0.8));
    color.b = pattern(st - offset, vel, 0.3 + clamp(abs(sin(iTime* 0.4)), 0.2, 0.8));

    // Margins
    color *= step(0.1, fpos.x);

    fragColor = vec4(1.0 - color, 1.0);
}
