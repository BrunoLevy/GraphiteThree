//stage GL_FRAGMENT_SHADER
//import <GLUP/ShaderToy.h>

const int MAX_STEPS = 256;
const float CLOSENESS = 0.00001;
const float EPSILON = 0.005;

float noise(vec3 x) {
    vec3 p = floor(x);
    vec3 f = fract(x);
    f = f*f*(3.-2.*f);
	
    float n = p.x + p.y*157. + 113.*p.z;
    
    vec4 v1 = fract(753.5453123*sin(n + vec4(0., 1., 157., 158.)));
    vec4 v2 = fract(753.5453123*sin(n + vec4(113., 114., 270., 271.)));
    vec4 v3 = mix(v1, v2, f.z);
    vec2 v4 = mix(v3.xy, v3.zw, f.y);
    return mix(v4.x, v4.y, f.x);
}

float field(vec3 p) {
   // random rotation reduces artifacts
   mat3 M = mat3(0.28862355854826727, 0.6997227302779844, 0.6535170557707412,
                 0.06997493955670424, 0.6653237235314099, -0.7432683571499161,
                 -0.9548821651308448, 0.26025457467376617, 0.14306504491456504);
   vec3 p1 = M*p;
   vec3 p2 = M*p1;
   float n1 = noise(p1*5.);
   float n2 = noise(p2*10.);
   float n3 = noise(p1*20.);
   float n4 = noise(p1*40.);
   float rocky = 0.1*n1*n1 + 0.05*n2*n2 + 0.02*n3*n3 + 0.01*n4*n4;
   float sph_dist = length(p) - 1.0;
   return sph_dist + (sph_dist < 0.1 ? rocky*0.2 : 0.);
}

float field_lores(vec3 p) {
   // random rotation reduces artifacts
   mat3 M = mat3(0.28862355854826727, 0.6997227302779844, 0.6535170557707412,
                 0.06997493955670424, 0.6653237235314099, -0.7432683571499161,
                 -0.9548821651308448, 0.26025457467376617, 0.14306504491456504);
   vec3 p1 = M*p;
   float n1 = noise(p1*5.);
   float rocky = 0.1*n1*n1;
   return length(p) - 1.0 + rocky*0.2;
}


vec3 getNormal(vec3 p, float value, mat3 rot) {
    vec3 n = vec3(field(rot*vec3(p.x+EPSILON,p.y,p.z)),
                  field(rot*vec3(p.x,p.y+EPSILON,p.z)),
                  field(rot*vec3(p.x,p.y,p.z+EPSILON)));
    return normalize(n - value);
}

void mainImage( out vec4 fragColor, in vec2 fragCoord ) {
    vec3 src = vec3(3. * (fragCoord.xy - 0.5*iResolution.xy) / iResolution.yy, 2.0);
    vec3 dir = vec3(0., 0., -1.);
    
    float ang = iTime*0.2;
    mat3 rot = mat3(-sin(ang),0.0,cos(ang),0.,1.,0.,cos(ang),0.,sin(ang));

    
    float t = 0.0;
    float atmos = 0.0;

    vec3 loc = src;
    float value;
    int steps = 0;
    for (int i=0; i < MAX_STEPS; i++) {
        steps++;
        loc = src + t*dir;
        if (loc.z < -1.) break;
        value = field(rot*loc);
        if (value <= CLOSENESS) break;
                if (value > 0.00001)
            atmos += 0.03;


        t += value*0.5;
    }
    // attempt at self-occlusion
    float shad1 = max(0.,field_lores(rot*(loc+normalize(vec3(0.,5.,1.))*0.1)))/0.1;
    float shad2 = max(0.,field_lores(rot*(loc+normalize(vec3(0.,5.,1.))*0.15)))/0.15;
    float shad3 = max(0.,field_lores(rot*(loc+normalize(vec3(0.,5.,1.))*0.2)))/0.2;
    float shad = clamp(shad1*0.333333 + shad2*0.333333 + shad3*0.333333, 0., 1.);
    shad = mix(shad, 1.0, 0.3);
    // attempt at some sort of ambient "glow"
    float ambient = clamp(field(rot*(loc - 0.5 * dir))/0.5*1.2, 0., 1.);
        

    if (value > CLOSENESS) fragColor = vec4(0., 0., 0., 1.);
    else {
      vec3 normal = getNormal(loc, value, rot);
      float light = dot(normal, normalize(vec3(0.,3.,1.)));

      float totalLight = mix(ambient, 1.0*max(0.,shad*light), 0.7);

    
      vec3 color = mix(vec3(0.1,.3,0.5), vec3(0.5,.6,1.2), 1.-(1.0-length(loc))*20.);
      fragColor = vec4(1.0*color*totalLight, 1.0);
    }
	vec2 p = 2.0* (fragCoord.xy / iResolution.yy - vec2(0.5 /  iResolution.y * iResolution.x, 0.5));
	float q = max(0.1, min(1.0, dot(vec3(p, sqrt(1. - dot(p,p))), vec3(0., 2., 1.))));
        fragColor += q * vec4(shad * max(0.0, pow(dot(normalize(src), normalize(vec3(0.,2., 1))),1.0)) * pow(atmos,1.5) * vec3(0.5, 0.5, 0.9), 1.0);

	//fragColor += vec4(q, 0.0, 0.0, 1.0);
}


