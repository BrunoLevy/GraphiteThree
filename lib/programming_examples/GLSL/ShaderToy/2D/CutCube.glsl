//stage GL_FRAGMENT_SHADER
//import <GLUP/ShaderToy.h>

/// CIS566 cut cube
/// SDF reference: http://www.iquilezles.org/www/articles/distfunctions/distfunctions.htm

#define DEPTH 30
#define END 1000.0
#define EPSILON 0.0075
#define EPSILON2 0.0005
#define PI 3.14159265359

#define WIREFRAME


// signed distance function of cube

vec2 unionSDF(vec2 v1, vec2 v2) {
    if(v1.x < v2.x) {
        return v1;
    } else {
        return v2;
    }
}


vec2 boxSDF(vec3 p, vec3 b) {
    p /= 1.5;
    //b /= 1.5;
    vec3 d = abs(p) - b;
  	float dist = min(max(d.x,max(d.y,d.z)),0.0) + length(max(d,0.0));
    
    float e = 0.0;
    
    float c1 = step(b.x - EPSILON, abs(p.x));
    float c2 = step(b.y - EPSILON, abs(p.y));
    float c3 = step(b.z - EPSILON, abs(p.z));
    
    if(c1 + c2 + c3 > 1.5)
    {
        e = 1.0;
    }    
    dist *= 1.5;
    return vec2(dist, e);
}


mat4 rotate(vec3 axis, float angle) {
    axis = normalize(axis);
    angle = radians(angle);
    float c = cos(angle);
    float s = sin(angle);
    float t = 1.0 - c;
    
    return transpose(mat4(
        t * pow(axis.x, 2.0) + c, t * axis.x * axis.y - s * axis.z, t * axis.x * axis.z + s * axis.y, 0.0,
        t * axis.x * axis.y + s * axis.z, t * pow(axis.y, 2.0), t * axis.y * axis.z - s * axis.x, 0.0,
        t * axis.x * axis.z - s * axis.y, t * axis.y * axis.z + s * axis.x, t * pow(axis.z, 2.0), 0.0,
        0.0, 0.0, 0.0, 1.0));
}

mat4 rotateY(float a) {
    float c = cos(radians(a));
    float s = sin(radians(a));
    return mat4(
        c, 0.0, -s, 0.0,
        0.0, 1.0, 0.0, 0.0,
        s, 0.0, c, 0.0,
        0.0, 0.0, 0.0, 1.0);  
}

mat4 rotateX(float t) {
	float cost = cos(radians(t));
	float sint = sin(radians(t));
	return mat4(
		1.0, 0.0, 0.0, 0.0,   // first column
		0.0, cost, sint, 0.0, // second column
		0.0, -sint, cost, 0.0, // third column
		0.0, 0.0, 0.0, 1.0
	);
}

mat4 rotateZ(float t) {
	float cost = cos(radians(t));
	float sint = sin(radians(t));
	return mat4(
		cost, sint, 0.0, 0.0,   // first column
		-sint, cost, 0.0, 0.0, // second column
		0.0, 0.0, 1.0, 0.0, // third column
		0.0, 0.0, 0.0, 1.0
	);
}

mat4 translate(vec3 t) {
    return mat4(
        1.0, 0.0, 0.0, 0.0,
        0.0, 1.0, 0.0, 0.0,
        0.0, 0.0, 1.0, 0.0,
        t.x, t.y, t.z, 1.0
        );
}


// scene builder
vec2 scene(vec3 p) {
    
    float offset = floor(0.75 * iTime) + max(0.0, 4.0 * fract(0.75 * iTime) - 3.0);
    
    mat4 m1 = rotateY(45.0 + offset * 90.0);
    vec3 p1 = vec3(inverse(m1) * vec4(p, 1.0));
    
    vec2 dist = boxSDF(p1, vec3(0.1, 0.1, 0.1));
    
    mat4 m2 = rotateY(45.0) * translate(vec3(0.0, 0.0, 0.3));
    vec3 p2 = vec3(inverse(m2) * vec4(p, 1.0));
    
    float bounce = 0.075 * pow(sin(iTime * PI * 2.0 * 0.75) + 1.0, 1.0 / 3.0);
    
    int num = 6;
    
    for(int i = 0; i < num; ++i) {
        for(int j = 0; j < 6; ++j) {
            
            float lag = float(i) / 10.0;
            
            offset = floor(0.75 * (iTime - lag)) + max(0.0, 4.0 * fract(0.75 * (iTime - lag)) - 3.0) + floor(0.75 * lag);
            
            m1 = rotateY(45.0 + 90.0 * offset);
            
            mat4 rotation;
            if(j < 4) {
                float angle = 90.0 * float(j);
                rotation = rotateY(angle);
            } else if(j == 4) {
                float angle = -90.0;
                rotation = rotateX(angle);
            } else {
                float angle = 90.0;
                rotation = rotateX(angle);
            }
                
            rotation = m1 * rotation;

            vec3 trans = vec3(0.0, 0.0, 0.3 + pow(float(i), 1.0 / 1.5) * 0.2 + bounce);

            mat4 transform = rotation * translate(trans);

            vec3 pos = vec3(inverse(transform) * vec4(p, 1.0));

            vec2 d = boxSDF(pos, vec3(0.1, 0.1, 0.001));

            dist = unionSDF(dist, d);
                                 
        }
    }

    return dist;
}

vec2 rayMarching(vec3 origin, vec3 dir, float start) {
    float t = start;
	for(int i = 0; i < DEPTH; ++i) {
		vec3 curP = origin + dir * t;
		vec2 step = scene(curP);
		t += step.x;
		if(step.x < EPSILON2) {
			return vec2(t, step.y);
		}
		if(t >= END){
			return vec2(END, 0.0);
		}	
	}
	return vec2(END, 0.0);
}

vec3 getNormal(vec3 p) {
    return normalize(vec3(
        scene(vec3(p.x + EPSILON2, p.y, p.z)).x - scene(vec3(p.x - EPSILON2, p.y, p.z)).x,
		scene(vec3(p.x, p.y + EPSILON2, p.z)).x - scene(vec3(p.x, p.y - EPSILON2, p.z)).x,
		scene(vec3(p.x, p.y, p.z + EPSILON2)).x - scene(vec3(p.x, p.y, p.z - EPSILON2)).x
	));
}


void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
    vec2 spos = (2.0 * fragCoord - iResolution.xy) / iResolution.y;
    vec3 eye = vec3(0.0, 5.0, 8.0);
    vec3 target = vec3(0.0, 0.0, 0.0);
    vec3 dir = normalize(target - eye);
    vec3 up = vec3(0.0, 1.0, 0.0);
    vec3 right = cross(dir, up);
    up = cross(right, dir);
    
    vec3 lightpos = vec3(0.0, 2.0, 1.0);
    
    vec3 pos = eye + right * spos.x + up * spos.y;
    
    vec2 final_t = rayMarching(pos, dir, 0.001);
    
    if(final_t.x > END) {
        fragColor = vec4(0.0);
        return;
    } 
    
    #ifdef WIREFRAME
    fragColor = vec4(vec3(final_t.y), 1.0);
    #endif
    
    #ifndef WIREFRAME
    vec3 intersection = pos + final_t.x * dir;
    vec3 normal = getNormal(intersection);
    float diffuse = dot(normalize(lightpos - intersection),normal);
    fragColor = vec4(vec3(diffuse), 1.0);
    #endif
    
      
}
