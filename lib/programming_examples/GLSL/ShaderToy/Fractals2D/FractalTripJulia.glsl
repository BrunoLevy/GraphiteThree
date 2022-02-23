//stage GL_FRAGMENT_SHADER
//import <GLUP/ShaderToy.h>
//This function convert color from hsv to rgb
vec3 hsv2rgb(vec3 c)
{
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
    // Normalized pixel coordinates (from 0 to 1)
    vec2 uv = fragCoord/iResolution.xy;

    int numSteps = 1024;
    float scale = 2.0;
    vec2 translate = vec2(-0.11,-0.15554);
    vec2 z, c;
    c = vec2(-0.182,0.664);
    c.x += abs(pow(0.6*sin(iTime*0.2),6.));
    
    scale = 0.0005;
    scale += abs(pow(sin(iTime*0.2),6.)*3.0);
    //scale = 2.9;
    
    z.x = 1.3333 * (uv.x - 0.5) * scale + translate.x;
    z.y = (uv.y - 0.5) * scale + translate.y;

    int i;
    //z = c;
    
    for(i=0; i<numSteps; i++) {
        float x = (z.x * z.x - z.y * z.y) + c.x;
        float y = (z.x * z.y + z.y * z.x) + c.y;

        if((x * x + y * y) > 4.0) break;
        z.x = x;
        z.y = y;
    }
    float r = (i == numSteps ? 0.0 : float(i)) / 100.0;
    r = mix(r, r+1., c.y*c.x);
    float hue = mix(0.2, 0.5, sqrt(r)*50.);
    vec3 q = hsv2rgb(vec3(hue,1,1));

    // Output to screen
    fragColor = vec4(sqrt(q), 1);
}

