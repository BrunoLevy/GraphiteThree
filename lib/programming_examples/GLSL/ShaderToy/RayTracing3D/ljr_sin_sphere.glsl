//stage GL_FRAGMENT_SHADER
//import <GLUP/ShaderToy.h>

// Largely based on inigo quilez's Sculpture III
// https://www.shadertoy.com/view/XtjSDK

// License Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License.

//#define SS

const float epsilon = 0.0001;
const int maxSteps = 512;
const float maxDistance = 20.;
const int samples = 2;
const vec4 background = vec4(vec3(.05), 1.);

const vec3 lightPos = vec3(0, 0, 5.);

const float radius = 7.;

float sphereDist (in vec3 p, float radius) {
    return length(p) - radius;
}

vec4 grow = vec4(1.);
vec3 mapP( vec3 p ) {
    p.xyz += 1.000*sin(  2.*p.yzx )*grow.x;
    p.xyz += 0.500*sin(  4.*p.yzx )*grow.y;
    p.xyz += 0.250*sin(  8.*p.yzx )*grow.z;
    p.xyz += 0.050*sin( 16.*p.yzx )*grow.w;
    return p;
}

float map (in vec3 p) {
    vec3 q = mapP(p);
	return sphereDist(q, radius) * 0.05;
}

float march (in vec3 rayOrigin, in vec3 rayDirection) {
    vec4 color = vec4(0.);
    
    float t  = 0.;
    for (int i = 0; i < maxSteps; i++) {
        vec3 p = rayOrigin + rayDirection * t;
        float d = map(p);
        if (d < epsilon || t > maxDistance) {
	    	break;
        }
        t += d;
    }
    if (t > maxDistance) t = -1.;
    return t;
}

vec3 getNormal( in vec3 p ) {
	vec3 dlt = vec3(0.005, 0., 0.);
    return normalize(vec3(
        map(p + dlt.xyy) - map(p - dlt.xyy),
        map(p + dlt.yxy) - map(p - dlt.yxy),
        map(p + dlt.yyx) - map(p - dlt.yyx) ));
}

vec4 distanceToMat( in vec3 rayOrigin, in vec3 rayDirection, in float t ) {
    vec4 color = background;
    if (t>0.) {
    	vec3 pos = rayOrigin + rayDirection * t;
        vec3 nor = getNormal(pos);
        // Basic Diffusion
        color = vec4(vec3(
            	clamp(dot(nor, lightPos) / length(lightPos), 0., 1.)
        	), 1.);
        // Fog
        color = mix(background, color, (maxDistance-t) / maxDistance);
    }
    return color;
}
                      
void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
    const float d = 10.;
    const vec3 up = vec3(0, 1, 0);
    const float f = .5;
    
    const vec4 delay = vec4(2.);
    grow = smoothstep( 0.0, 1.0, (iTime-vec4(0.0,1.0,2.0,3.0))/3.0 - delay );

    // Adjust coordinates to (-1, -1) -> (1, 1)
    vec2 uv = fragCoord.xy / iResolution.y;
    uv *= 2.;
    uv -= vec2(iResolution.x / iResolution.y, 1.);
        
	float an = 0.05*iTime;

	vec3 ro = vec3(d*cos(an),0.,d*sin(an));
    vec3 ta = vec3(0.);
    // camera
    vec3 ww = normalize( ta - ro );
    vec3 uu = normalize( cross(ww,up) );
    vec3 vv = normalize( cross(uu,ww));
    
    #ifdef SS
    // Antialias by averaging all adjacent values
    vec4 color = vec4(0.);
    float t = 0.;
    for (int x = - samples / 2; x < samples / 2; x++) {
        for (int y = - samples / 2; y < samples / 2; y++) {
			vec3 rd = normalize(
                (float(x) / iResolution.y + uv.x)*uu +
                (float(y) / iResolution.y + uv.y)*vv +
                f*ww );
            t = march(ro, rd);
            color += distanceToMat(ro, rd, t);
        }
    }
	fragColor = color / float(samples * samples);

    #else
    // Non-antialias
	vec3 rd = normalize(
    	uv.x*uu +
        uv.y*vv +
         f*ww );
    float t = march(ro, rd);
    fragColor = distanceToMat(ro, rd, t);
    #endif
}