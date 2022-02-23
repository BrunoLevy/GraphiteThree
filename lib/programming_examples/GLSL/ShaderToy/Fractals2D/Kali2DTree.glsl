//stage GL_FRAGMENT_SHADER
//import <GLUP/ShaderToy.h>

#define PI 3.14159
#define R iResolution

#define IT 128

#define c vec2(-.25, -0.5)

#define r 0.45
#define s 1.3

vec2 formula (in vec2 p) {
    float m = dot(p, p);
    if (m < r) {
        p = abs(p) / (m*m);
    } else {
        p = abs(p) / m * s;
    }
    return p + c;
}

float map(in vec2 p) {
    for (int i = 0; i < IT; i++) {
        p = formula(p);
    }
    return length(p);
}


vec4 shade (float t) {
    const vec3 a = vec3(.5);
    const vec3 b = vec3(.5);
    const vec3 cc = vec3(1.);
    const vec3 d = vec3(0., 0.1, 0.2);

    vec3 color = a + b * cos( 2.*PI * (cc * t + d));
    return vec4(color, 1.);
}

void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
	vec2 uv = (2. * fragCoord - R.xy) / R.y; //via @coyote
    
    float t = map(uv);
    fragColor = shade(t);
}
