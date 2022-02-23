//stage GL_FRAGMENT_SHADER
//import <GLUP/ShaderToy.h>

// Created by SHAU - 2017
// License Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License.
//-----------------------------------------------------

#define T iTime * 2.0
#define PI 3.14159265359
#define FAR 50.0 
#define EPS 0.005

#define FLOOR 1.0
#define ORB 2.0
#define WHITE_GLOW 3.0
#define FLAT_L_BLUE 4.0
#define FLAT_D_BLUE 5.0
#define WALL 6.0
#define BLUE_GLOW 7.0
#define ARCH 8.0
#define ARCH_2 9.0

vec3 lp = vec3(0.0);

struct Scene {
    float t;
    vec3 n;
    float id;
    float wli;
    float bli;
};

mat2 rot(float x) {return mat2(cos(x), sin(x), -sin(x), cos(x));}
float rand(vec2 p) {return fract(sin(dot(p, vec2(12.9898,78.233))) * 43758.5453);}

float sdTorus(vec3 p, vec2 t) {
    vec2 q = vec2(length(p.xz) - t.x, p.y);
    return length(q) - t.y;
}

float sdSphere(vec3 p, float s) {
    return length(p) - s;
}

float fCylinder(vec3 p, float r, float height) {
    p.z *= 0.8;
	float d = length(p.xz) - r;
	d = max(d, abs(p.y) - height);
	return d;
}

float planeIntersection(vec3 ro, vec3 rd, vec3 n, vec3 o) {
    return dot(o - ro, n) / dot(rd, n);
}

vec2 nearest(vec2 a, vec2 b){ 
    float s = step(a.x, b.x);
    return s * a + (1. - s) * b;
}

vec4 archway(vec3 rp) {
    float rr = (mod(rp.z, 12.0) - 6.0) > 0.0 ? 1.0 : -1.0;
    rp.xy *= rot(T * 0.2 * rr);
    float a = atan(rp.y, rp.x) / 6.2831853;
    float ia = floor(a * 4.0) / 4.0 * 6.2831853;
    vec3 q = rp;
    q.xy *= rot(ia);
    q.z = mod(q.z, 6.0) - 3.0;
    float t1 = sdTorus(q.xzy, vec2(1.5, 0.15));
    float t2 = sdTorus(q.xzy, vec2(1.5, 0.22));
    float t3 = sdTorus(q.xzy, vec2(1.71, 0.02));
    float t4 = sdTorus(q.xzy - vec3(0.0, -0.22, 0.0), vec2(1.5, 0.05));
    t2 = max(t2, q.y - 0.5); 
    t4 = max(t4, q.y - 1.2);
    q = rp;
    q.x = abs(q.x);
    q.z = mod(q.z, 6.0) - 3.0;
    float t5 = sdTorus(q.zyx - vec3(0.0, 0.0, 1.5), vec2(0.26, 0.03));     
    vec2 near = vec2(t1, ARCH);
    near = nearest(near, vec2(t2, WALL));    
    near = nearest(near, vec2(t3, ARCH_2));    
    near = nearest(near, vec2(t4, BLUE_GLOW));    
    near = nearest(near, vec2(t5, WHITE_GLOW));    
    return vec4(near, t4, t5);
}

vec3 tower1(vec3 rp) {
    rp.x = abs(rp.x);
    rp.z = mod(rp.z, 12.0) - 6.0;
    vec2 core = vec2(fCylinder(rp - vec3(2.5, 0.0, 0.0), 0.2, 3.0), FLAT_L_BLUE);
    float sh = 2.0 + sin(T * 0.2) * 1.;
    vec2 sleeve = vec2(fCylinder(rp - vec3(2.5, 0.0, 0.0), 0.3, 3.0), WHITE_GLOW);
    float sleevecut = fCylinder(rp - vec3(2.5, 0.0, 0.0), 0.4, 3.0);
    sleeve.x = max(max(sleeve.x, sh - 0.4 - rp.y), rp.y - sh);
    sleevecut = max(max(sleevecut, sh - 0.3 - rp.y), rp.y - sh + 0.1);
    sleeve.x = max(sleeve.x, -sleevecut);
    vec2 base = vec2(fCylinder(rp - vec3(2.5, 0.0, 0.0), 0.3, 0.5), WALL);
    return vec3(nearest(nearest(base, core), sleeve), sleeve.x);
}

vec3 tower2(vec3 rp) {
    vec3 q = rp;
    q.x = abs(q.x);
    q.z = mod(q.z, 24.0) - 12.0;
    vec2 near = vec2(fCylinder(q - vec3(9.0, 0.0, 0.0), 2.0, 1.0), WALL);
    vec2 core = vec2(fCylinder(q - vec3(9.0, 0.0, 0.0), 1.2, 6.0), WALL);
    q = rp;
    float rh = sin(T * 0.1) * 1.5;    
    q.y += rh;
    q.x = abs(q.x);
    q.z = mod(q.z, 24.0) - 12.0;
    vec2 ringcore = vec2(fCylinder(q - vec3(9.0, 0.0, 0.0), 1.3, 4.2), BLUE_GLOW);
    q.y = mod(q.y, 0.4) - 0.2;
    vec2 rings = vec2(fCylinder(q - vec3(9.0, 0.0, 0.0), 1.8, 0.1), WALL);
    rings.x = max(rings.x, rp.y + rh - 4.4);
    near = nearest(near, core);
    near = nearest(near, rings);
    near = nearest(near, ringcore);    
    return vec3(near, ringcore.x);
}

vec3 smallorbs(vec3 rp) {
    rp.z = mod(rp.z, 3.0) - 1.5;
    rp.x = abs(rp.x);
    vec2 orb = vec2(sdSphere(rp - vec3(1.2, 0.0, 0.0), 0.2), ORB);
    vec2 torus = vec2(sdTorus(rp - vec3(1.2, 0.0, 0.0), vec2(0.25, 0.02)), WHITE_GLOW);
    return vec3(nearest(orb, torus), torus.x);
}

vec3 mediumorbs(vec3 rp) {    
    vec3 q = rp;
    q.z = mod(q.z, 6.0) - 3.0;
    q.x = abs(q.x);
    vec2 near = vec2(sdSphere(q - vec3(3.4, 0.0, 0.0), 0.8), ORB);
    vec2 torus = vec2(sdTorus(q - vec3(3.4, 0.0, 0.0), vec2(1.0, 0.03)), WHITE_GLOW);
    torus.x = min(torus.x, sdTorus(q - vec3(1.5, 0.0, 0.0), vec2(0.35, 0.03)));
    float fins = sdSphere(q - vec3(3.4, 0.0, 0.0), 0.9);
    q.y = mod(q.y, 0.1) - 0.05;
    fins = max(fins, q.y - 0.01);    
    vec2 fins1 = vec2(max(fins, rp.y - 0.55), FLAT_L_BLUE);
    fins1 = max(fins1, 0.4 - rp.y);
    vec2 fins2 = vec2(max(fins, rp.y - 0.25), FLAT_D_BLUE);
    fins2 = max(fins2, 0.2 - rp.y);  
    near = nearest(near, torus);
    near = nearest(near, fins1);
    near = nearest(near, fins2);
    return vec3(near, torus.x);
}

vec3 largeorbs(vec3 rp) {
    vec3 q = rp;
    q.z = mod(q.z - 12.0, 24.0) - 12.0;
    q.x = abs(q.x);
    vec2 near = vec2(sdSphere(q - vec3(16.0, 0.0, 0.0), 6.0), ORB);
    vec2 torus = vec2(sdTorus(q - vec3(16.0, 0.0, 0.0), vec2(6.5, 0.1)), WHITE_GLOW);
    float fins = sdSphere(q - vec3(16.0, 0.0, 0.0), 6.3);
    q.y = mod(q.y, 0.4) - 0.2;
    fins = max(fins, q.y - 0.05);    
    vec2 fins1 = vec2(max(fins, rp.y - 1.8), FLAT_L_BLUE);
    fins1 = max(fins1, 1.1 - rp.y);
    vec2 fins2 = vec2(max(fins, rp.y - 0.7), FLAT_D_BLUE);
    fins2 = max(fins2, 0.3 - rp.y);  
    near = nearest(near, torus);
    near = nearest(near, fins1);
    near = nearest(near, fins2);
    return vec3(near, torus.x);
}

vec4 map(vec3 rp) {
    vec4 arch = archway(rp);
    vec2 near = arch.xy;
    vec3 tow1 = tower1(rp);
    near = nearest(near, tow1.xy);
    vec3 tow2 = tower2(rp);
    near = nearest(near, tow2.xy);
    vec3 sorbs = smallorbs(rp);
    near = nearest(near, sorbs.xy);
    vec3 morbs =  mediumorbs(rp);
    near = nearest(near, morbs.xy);
    vec3 lorbs = largeorbs(rp);
    near = nearest(near, lorbs.xy);
    float blt = min(tow2.z, arch.z); //blue light
    float wlt = min(sorbs.z, morbs.z); //white light
    wlt = min(wlt, lorbs.z);
    wlt = min(wlt, tow1.z);
    wlt = min(wlt, arch.w);
    return vec4(near, blt, wlt);
}

vec3 normal(vec3 rp) {
    vec2 e = vec2(EPS, 0);
    float d1 = map(rp + e.xyy).x, d2 = map(rp - e.xyy).x;
    float d3 = map(rp + e.yxy).x, d4 = map(rp - e.yxy).x;
    float d5 = map(rp + e.yyx).x, d6 = map(rp - e.yyx).x;
    float d = map(rp).x * 2.0;
    return normalize(vec3(d1 - d2, d3 - d4, d5 - d6));
}

// Based on original by IQ.
// http://www.iquilezles.org/www/articles/raymarchingdf/raymarchingdf.htm
float AO(vec3 rp, vec3 n) {

    float r = 0.0;
    float w = 1.0;
    float d = 0.0;

    for (float i = 1.0; i < 5.0; i += 1.0){
        d = i / 5.0;
        r += w * (d - map(rp + n * d).x);
        w *= 0.5;
    }

    return 1.0 - clamp(r, 0.0, 1.0);
}

//IQ
//http://www.iquilezles.org/www/articles/fog/fog.htm
vec3 applyFog(vec3  rgb,      // original color of the pixel
              float d, // camera to point distance
              vec3  rayDir,   // camera to point vector
              vec3  sunDir,
              float b)  // sun light direction
{
    //float b = 0.07;
    float fogAmount = 1.0 - exp(-d * b);
    float sunAmount = max(dot(rayDir, sunDir), 0.0);
    vec3  fogColor  = mix(vec3(0.5, 0.3, 0.8), // purple
                          vec3(0.7, 0.7, 1.0), // blue
                          pow(sunAmount, 8.0));
    return mix(rgb, fogColor, fogAmount);
}

vec4 march(vec3 ro, vec3 rd) {
    
    float t = 0.0;
    float id = 0.0;
    float bli = 0.0;
    float wli = 0.0;
    
    for (int i = 0; i < 96; i++) {
        vec3 rp = ro + rd * t;    
        vec4 scene = map(rp);
        if (scene.x < EPS || t > FAR) {
            id = scene.y;
            break;   
        }
        
        bli += 0.05 / (1.0 + scene.z * scene.z * 30.0) * step(0.0, rp.y);
        wli += 0.05 / (1.0 + scene.w * scene.w * 100.0) * step(0.0, rp.y);
        
        t += scene.x;
    }
    
    return vec4(t, id, bli, wli);
}

Scene drawScene(vec3 ro, vec3 rd) {
    
    float mint = FAR;
    vec3 minn = vec3(0.0);
    float id = 0.0;
    float bli = 0.0;
    float wli = 0.0;
    
    vec3 fo = vec3(0.0, 0.0, 0.0);
    vec3 fn = vec3(0.0, 1.0, 0.0);
    float ft = planeIntersection(ro, rd, fn, fo);
    if (ft > 0.0 && ft < mint) {
        mint = ft;
        minn = fn;
        id = FLOOR;
    }

    vec4 scene = march(ro, rd);
    if (scene.x > 0.0 && scene.x < mint) {
        if (scene.x < mint) {
            mint = scene.x;
            minn = normal(ro + rd * scene.x);
            id = scene.y;
        }
    }
    
    bli = scene.z;
    wli = scene.w;

    return Scene(mint, minn, id, wli, bli);
}

vec2 floorTexture(vec3 rp, float t) {

    float line = 0.0;
    float cut = 0.0;
        
    //small orbs
    vec3 q = rp;
    q.z = mod(q.z, 3.0) - 1.5;
    q.x = abs(q.x);
    line += smoothstep(0.33, 0.32, length(q.xz - vec2(1.2, 0.0))); 
    cut += smoothstep(0.3, 0.29, length(q.xz - vec2(1.2, 0.0)));
                
    //medium orbs and archway
    q = rp;
    q.z = mod(q.z, 6.0) - 3.0;
    q.x = abs(q.x);
    line += smoothstep(1.2, 1.18, length(q.xz - vec2(3.4, 0.0))); 
    cut += smoothstep(1.1, 1.08, length(q.xz - vec2(3.4, 0.0)));
    line += smoothstep(0.5, 0.48, length(q.xz - vec2(1.5, 0.0)));
    cut += smoothstep(0.42, 0.41, length(q.xz - vec2(1.5, 0.0)));
        
    //large orbs
    q = rp;
    q.z = mod(q.z - 12.0, 24.0) - 12.0;
    q.x = abs(q.x);
    line += smoothstep(7.6, 7.5, length(q.xz - vec2(16.0, 0.0))); 
    cut += smoothstep(7.4, 7.3, length(q.xz - vec2(16.4, 0.0)));
        
    q = rp;
    vec2 grid = fract(vec2(q.x * 0.25, q.z * 0.5));
    float gridline = (smoothstep(0.01, 0.005, grid.x) + smoothstep(0.998, 0.999, grid.x));
    gridline += smoothstep(0.02 + t / FAR * 0.2, 0.01, grid.y) / (t * 1.0);
    
    line = clamp(line + gridline, 0.0, 1.0);
    line /= t * 0.5;

    return vec2(line, cut);
}

float tex(vec2 uv) {
    vec2 mx = mod(uv, 0.1) - 0.05;     
    float tx = (mx.x * mx.y) > 0.0 ? 1.0 : 0.0;
    tx *= sin(uv.y * 100.0 + T) * 0.6 + 1.0;
    tx *= sin(uv.x * 100.0 + T) * 0.6 + 1.0;
    tx *= step(uv.y, 0.1) + step(-uv.y, -0.2); 
    return tx;
}        

vec3 colourScene(vec3 ro, vec3 rd, Scene scene) {
 
    vec3 pc = vec3(0.0);
    
    vec3 rp = ro + rd * scene.t;
    vec3 ld = normalize(lp - rp);
    float lt = length(lp - rp);
    
    float diff = 1.0;
    float spec = pow(max(dot(reflect(-ld, scene.n), -rd), 0.0), 64.0);
    float atten = 1.0;
    float fres = 0.0;
    
    if (scene.id == FLOOR) {
        
        vec2 flrTex = floorTexture(rp, scene.t);        
        pc += vec3(0.5, 0.3, 1.0) * clamp(flrTex.x, 0.0, 1.0);
        pc -= clamp(flrTex.y, 0.0, 1.0);
        
    } else if (scene.id == ORB) {
        
        diff = max(dot(ld, scene.n), 0.05);
        spec = pow(max(dot(reflect(-ld, scene.n), -rd), 0.0), 32.0);
        fres = pow(clamp(dot(scene.n, rd) + 1.0, 0.0, 1.0), 4.0);
        
        pc = vec3(0.1) * diff;
        pc += vec3(1.0) * spec;
        pc += vec3(0.5, 0.3, 1.0) * fres * diff;

    } else if (scene.id == WALL) {

        diff = max(dot(ld, scene.n), 0.05);
        fres = pow(clamp(dot(scene.n, rd) + 1.0, 0.0, 1.0), 4.0);

        pc = vec3(0.1) * diff;
        pc += vec3(1.0) * spec;
        pc += vec3(0.5, 0.3, 1.0) * fres * diff;

    } else if (scene.id == ARCH) {
        
        diff = max(dot(ld, scene.n), 0.05);
        fres = pow(clamp(dot(scene.n, rd) + 1.0, 0.0, 1.0), 4.0);

        vec3 q = rp;
        float rr = (mod(q.z, 12.0) - 6.0) > 0.0 ? 1.0 : -1.0;
        q.xy *= rot(T * 0.2 * rr);
        float a = atan(q.y, q.x) / 6.2831853;
        a = fract(a * 8.0);
        
        pc = vec3(0.1) * diff;
        pc += vec3(0.6, 0.3, 1.0) * step(a, 0.3) * tex(vec2(length(rp.xy), a));
        pc += vec3(1.0) * spec;
        pc += vec3(0.5, 0.3, 1.0) * fres * diff;

    } else if (scene.id == WHITE_GLOW) {
        
        diff = max(dot(ld, scene.n), 0.8);
        
        pc = vec3(1.0) * diff;
        
    } else if (scene.id == BLUE_GLOW) {
        
        diff = max(dot(ld, scene.n), 0.8);

        pc = vec3(0.3, 0.3, 1.0) * diff;
        pc += vec3(1.0) * spec;

    } else if (scene.id == FLAT_L_BLUE) {
        
        diff = max(dot(ld, scene.n), 0.8);

        pc = vec3(0.5, 0.8, 1.0) * diff;
        pc += vec3(1.0) * spec;
        
    } else if (scene.id == FLAT_D_BLUE) {

        diff = max(dot(ld, scene.n), 0.8);

        pc = vec3(0.3, 0.3, 1.0) * diff;
        pc += vec3(1.0) * spec;

    } else if (scene.id == ARCH_2) {
        
        diff = max(dot(ld, scene.n), 0.05);
        
        pc = vec3(0.5, 0.3, 1.0);
        pc *= diff;
        pc += vec3(1.0) * spec;
    }
    
    //pc *= AO(rp, scene.n);
    
    return pc;
}

void setupCamera(vec2 fragCoord, inout vec3 ro, inout vec3 rd) {

    vec2 uv = (fragCoord.xy - iResolution.xy * 0.5) / iResolution.y;

    vec3 lookAt = vec3(0.0, 0.0, T * 0.5);
    lp = lookAt + vec3(4.0, 4.0, -2.0);
    ro = lookAt + vec3(0.0, 0.8, -4.0);
    
    float FOV = PI / 3.0;
    vec3 forward = normalize(lookAt - ro);
    vec3 right = normalize(vec3(forward.z, 0.0, -forward.x)); 
    vec3 up = cross(forward, right);

    rd = normalize(forward + FOV * uv.x * right + FOV * uv.y * up);
}

void mainImage(out vec4 fragColor, in vec2 fragCoord) {
    
    vec3 pc = vec3(0.0);
    float mint = FAR;
    
    vec3 ro, rd;
    setupCamera(fragCoord, ro, rd);
    
    Scene scene = drawScene(ro, rd);
    if (scene.t > 0.0 && scene.t < FAR) {
        mint = scene.t;
        pc = colourScene(ro, rd, scene);
        
        if (scene.id == FLOOR || 
            scene.id == ORB || 
            scene.id == WALL ||
            scene.id == ARCH) {
            
            float lt = scene.t;
            vec3 rc = vec3(0.5, 0.3, 1.0) * 0.2;
            float ra = 0.1;
            
            vec3 rpb = ro + rd * (scene.t - EPS);
            vec3 rrd = reflect(rd, scene.n);
            Scene refl = drawScene(rpb, rrd);
            
            if (refl.t > 0.0 && refl.t < FAR) {
                rc = colourScene(rpb, rrd, refl); 
                lt += refl.t;
                if (refl.id == WHITE_GLOW ||
                    refl.id == BLUE_GLOW) {
                    ra = 0.02;
                } else if (refl.id == FLAT_L_BLUE ||
                           refl.id == FLAT_D_BLUE) {
                    ra = 0.06;
                }
            }
            float rt = 1.0 / (1.0 + lt * lt * ra);
            pc += rc * rt * 0.5;
            pc += vec3(1.0) * refl.wli * rt * 0.5;
        }
    }

    pc = applyFog(pc, mint, rd, normalize(vec3(2.0, 2.0, 4.0)), 0.01);

    pc += vec3(1.0) * scene.wli * 0.8;
    pc += vec3(0.3, 0.3, 1.0) * scene.bli;
    
	fragColor = vec4(pc, 1.0);
    //fragColor = vec4(sqrt(clamp(pc, 0.0, 1.0)), 1.0);
}
