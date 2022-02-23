//stage GL_FRAGMENT_SHADER
//import <GLUP/ShaderToy.h>

/* greetings to my friends */
/* no textures were used to create this IFS based experiment */

mat2 rot(float x) {
    return mat2(cos(x), sin(x), -sin(x), cos(x));
}

float map(vec3 p, float k) {
    p.xz *= rot(k * 3.141592);
	vec3 a = abs(p);
    float d = max(a.x, max(a.y, a.z)) - 1.0;
    float s = 0.9;
    d = max(d, s - max(a.y, a.z));
    d = max(d, s - max(a.x, a.z));
    d = max(d, s - max(a.x, a.y));
    return d;
}

float trace(vec3 o, vec3 r, float k) {
    float t = 0.0;
    for (int i = 0; i < 16; ++i) {
        t += map(o + r * t, k) * (1.0 - k * 0.75);
    }
    return t;
}

void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
    vec2 uv = fragCoord / iResolution.xy;
	uv = uv * 2.0 - 1.0;
    uv.x *= iResolution.x / iResolution.y;
    
    vec3 r = normalize(vec3(uv, 1.0 - dot(uv, uv) * 0.1));
    vec3 o = vec3(4.0, 3.0, iTime * 3.0);
    
    r.xy *= rot(iTime * 0.125);
    r.yz *= rot(3.141592 * -0.5);

    vec3 fc = vec3(0.0);
    
    const int n = 7;
    for (int i = 0; i < n; ++i) {
        float u = float(i) / float(n - 1);
        
        float h = -u;
        float pt = (h - o.y) / r.y;
        vec3 w = o + r * pt;

        float s = 8.0;
        vec2 a = w.xz;
        w.xz = (fract(w.xz / s) - 0.5) * s;

        for (int j = 0; j <= i; ++j) {
            w.xz = abs(w.xz) - 0.5;
            w.xz *= rot(3.141592 * 0.125);
        }
        
        vec3 tr = normalize(vec3(w.xz, 1.0));
        vec3 to = vec3(0.0, 0.0, -2.0 - sin(a.x + a.y));
        float t = trace(to, tr, u);
        float d = map(to + tr * t, u);
        
        float tc = 1.0 / (1.0 + pt * pt * t * 0.01 + d * 100.0);
        
        vec3 fs = vec3(0.5);
        vec3 fd = vec3(0.25);
        
        vec3 gc = vec3(1.0, 0.5, 0.0);
        fd = mix(fd, fs * 20.0 * gc, floor(u));

        vec3 uc = mix(fs, fd, tc) * tc;
        
        if (pt > 0.0) {
            float b = float(i + 1) / float(n);
        	fc += uc * b * 2.0;
        }
    }
    
    fragColor = vec4(fc, 1.0);
}

