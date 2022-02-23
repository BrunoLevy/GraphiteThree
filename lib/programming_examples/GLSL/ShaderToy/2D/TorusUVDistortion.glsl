//stage GL_FRAGMENT_SHADER
//import <GLUP/ShaderToy.h>
vec2 split_uv(vec2 uv) {
    return uv - vec2(0.5*sign(uv.x)*iResolution.x/iResolution.y, 0.0);
}

vec3 get_normal(vec2 uv) {
    // http://web.cs.ucdavis.edu/~amenta/s12/findnorm.pdf
 	uv = uv * (1.0 + 0.6*cos(iTime));
    
    float phi = 2.0*3.1416*uv.x;
    float theta = 2.0*3.1416*uv.y;
    
    vec3 dphi = vec3(-sin(phi), cos(phi), 0.0);
    vec3 dtheta = vec3(-sin(theta)*cos(phi), -sin(theta)*sin(phi), cos(theta));
    
    return normalize(cross(dphi, dtheta));  
}

float checkerboard(vec2 uv, float size) {
    // https://www.shadertoy.com/view/lt2XWK
	vec2 p = floor(uv / size);
    return mod(p.x + p.y, 2.0);
}

void mainImage( out vec4 fragColor, in vec2 fragCoord ) {
    float cell_size = 0.125*iResolution.x/iResolution.y;
    
    // uv coordinates, corrected for aspect ratio
    vec2 uv0 = (2.0*fragCoord.xy - iResolution.xy)/iResolution.yy;
    
    // uv coordinates split in half and duplicate left hand side
    vec2 uv1 = split_uv(uv0);
    
    // grab the normals from the torus using these uv coordinates
    vec3 n = get_normal(uv1.yx);
    
    // distort uv's based on normal
    uv1 = uv1 + 0.1*cell_size*n.xy;
    
    // apply a scrolling motion upwards
    uv1 = uv1 - vec2(0.0, 0.2*iTime);
    
    // create a checkerboard effect
    float s = checkerboard(uv1, cell_size);
    
    // output
    if (uv0.x > 0.0) {   
        fragColor = vec4(s, s, s, 1.0);
        //fragColor = texture(iChannel0, uv1/cell_size);
    } else {
    	fragColor = vec4(0.5 + 0.5*n, 1.0);
    }
}