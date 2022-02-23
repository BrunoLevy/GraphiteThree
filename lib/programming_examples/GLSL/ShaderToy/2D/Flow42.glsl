//stage GL_FRAGMENT_SHADER
//import <GLUP/ShaderToy.h>


#define M_PI 3.14159265

#define SCALE 1.0
#define K 7
#define F 0.55


// LogPolar transform
vec2 c2p(vec2 coord) {
	float th = atan(coord.y, coord.x);
    float r = log(sqrt(coord.x*coord.x+coord.y*coord.y)); 
    
    return vec2(th, r);
}

// Colorize. See:
// http://www.iquilezles.org/www/articles/palettes/palettes.htm
vec4 colorize(float t, vec3 a, vec3 b, vec3 c, vec3 d) {
    vec3 col = 2.5 * a * b * (cos(0.4*M_PI*(c*t+d))); 
    return vec4(col, 1.0);
}

vec4 colorize2(float v) {
 	vec3 col = vec3(v, v, v);
    return vec4(col, 1.0);
}

float v(vec2 coord, float k, float s, float rot) {
    float cx = cos(rot);
    float sy = sin(rot);
    
	return 0.0 + 0.5 * cos((cx * coord.x + sy * coord.y) * k + s);
}

void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
   	float t = 2.12 * iTime;
    
    vec2 xy = fragCoord.xy - (0.5 * iResolution.xy); // center
    vec2 uv = c2p(xy); // Polar (opt.)
    uv = xy / SCALE; // Scale
        
    float vt = 0.0;
  
	    
    for(int i = 0; i < K; i++) {
        float s = float(i) * M_PI / float(K);
    	float w = v(uv, F, t, s);
        
        vt += w / 0.5;
    }
    
    
    vec4 col = colorize(vt, vec3(0.5, 0.5, 0.5),
                        	vec3(0.5, 0.5, 0.5),
                        	vec3(1.0, 1.0, 1.0),
                        	vec3(0.00, 0.33, 0.67));
    /*
    vec4 col = colorize(vt, vec3(0.5, 0.5, 0.5),
                        	vec3(0.5, 0.5, 0.5),
                        	vec3(1.0, 1.0, 0.5),
                        	vec3(0.80, 0.90, 0.30));
    */
   // col = colorize2(vt);
    
    // Mask center (a bit)
    float m =  3.0*(distance((fragCoord.xy / iResolution.xy), vec2(0.5, 0.5)));
    //col = vec4( m, m, m, 1.0);
    // col = clamp(m, 0.0, 1.0) * col;
    
    // Output to screen
    fragColor = col;
}
