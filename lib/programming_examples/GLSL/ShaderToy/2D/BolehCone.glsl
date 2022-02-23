//stage GL_FRAGMENT_SHADER
//import <GLUP/ShaderToy.h>

#define PI 3.14159265359
#define rot(a) mat2(cos(a + PI*0.25*vec4(0,6,2,0)))
#define FOV 0.7

const vec3 forward = normalize(vec3(1, 2, 3));

// Dave Hoskins hash
vec3 hash33( in vec3 p3 ) {
	p3 = fract(p3 * vec3(.1031, .1030, .0973));
    p3 += dot(p3, p3.yxz+19.19);
    return fract((p3.xxy + p3.yxx)*p3.zyx);
}

// cosine based hue to rgb conversion
vec3 hue( in float c ) {
    return cos(2.0*PI*c + 2.0*PI/3.0*vec3(3,2,1))*0.5+0.5;
}

// tonemapping from https://www.shadertoy.com/view/lslGzl
vec3 filmicToneMapping( vec3 col ) {
    col = max(vec3(0.), col - vec3(0.004));
    return (col * (6.2 * col + .5)) / (col * (6.2 * col + 1.7) + 0.06);
}

// main distance function, returns distance, color and normal
float de( in vec3 p, in float r, out vec3 color, out vec3 normal ) {
    float d = 9e9;
    color = vec3(0);
    vec3 center = floor(p)+0.5;
    float phase = 0.01*dot(p, forward);
    
    // 8 taps voronoi
    for (float x = -0.5 ; x < 1.0 ; x++)
    for (float y = -0.5 ; y < 1.0 ; y++)
    for (float z = -0.5 ; z < 1.0 ; z++) {
        vec3 sphere = center+vec3(x, y, z);
        vec3 rnd = hash33(sphere);
        sphere += (rnd-0.5)*0.3;
        vec3 inSphere = p-sphere;
        float len = length(inSphere);
        vec3 norm = inSphere/len;
        float R = 0.01+(rnd.x*rnd.y)*0.1;
        float dist = len-R;
        
        // found the closest sphere, update distance and normal
        if (dist < d) {
            d = dist;
            normal = norm;
        }
        
        // figure out how much light there is inside the radius
        float area = R*R*R;
    	float maxArea = r*r*r;
        float scale = smoothstep(r, -r, dist)*area/max(area, maxArea);
        
        // add lights
        vec3 lightColor = hue(rnd.z*0.2 + phase);
        color += scale*lightColor*100.0;
    }
    
    return d;
}

void mainImage( out vec4 fragColor, in vec2 fragCoord ) {
    vec3 rnd = hash33(vec3(fragCoord, iFrame));
    vec2 uv = (fragCoord - iResolution.xy*0.5)/iResolution.y;
    
    vec3 right = normalize(cross(forward, vec3(0, 1, 0)));
    vec3 up = cross(right, forward);
    vec3 dir = normalize(forward/tan(FOV*0.5)+right*uv.x+up*uv.y);
    vec3 from = forward*iTime*4.0;
    
    float focal = 4.0;
    float sinPix = sin(FOV / iResolution.y);
    vec4 acc = vec4(0, 0, 0, 1);
    vec3 dummy = vec3(0);
    float totdist = de(from, 0.0, dummy, dummy)*rnd.x;
    
    for (int i = 0 ; i < 50 ; i++) {
		vec3 p = from + totdist * dir;
        float r = max(totdist*sinPix, abs((totdist-focal)*0.03));
        vec3 color = vec3(0);
        vec3 normal = vec3(0);
        float dist = de(p, r, color, normal);
        
        // apply fog to the given color
        float fog = 1.0-exp(-totdist * 0.15);
        color = mix(color, vec3(0), fog);
        
        // find out the orientation of the light source with the aperture
        float theta = atan(dot(normal, right), dot(normal, up));
        // change the shape of the cone tracing radius according to that
        float R = r * (cos(theta*6.0)*0.2+0.8);
        
        // cone trace the surface
		float alpha = smoothstep(R, -R, dist);
        acc.rgb += acc.a * (alpha*color.rgb);
        acc.a *= (1.0 - alpha);
        
        // hit a surface, stop
        if (acc.a < 0.01) break;
        // continue forward
        totdist += max(abs(dist), r*0.5);
	}
    
    fragColor.rgb = filmicToneMapping(acc.rgb);
    fragColor.a = 1.0;
    
}