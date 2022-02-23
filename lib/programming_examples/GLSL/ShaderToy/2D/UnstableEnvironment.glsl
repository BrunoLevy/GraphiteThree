//stage GL_FRAGMENT_SHADER
//import <GLUP/ShaderToy.h>

#define zoom 0.45
#define time iTime * 0.7

// Didn't change the hash at all, so taken directly from here https://www.shadertoy.com/view/ltK3DR
vec3 hash33(vec3 p3)
{
	p3 = fract(p3 * vec3(.1031, .1030, .0973));
    p3 += dot(p3, p3.yxz+19.19);
    return fract(vec3((p3.x + p3.y)*p3.z, (p3.x+p3.z)*p3.y, (p3.y+p3.z)*p3.x));
}

void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
    vec2 uv = (fragCoord - .5 * iResolution.xy ) / iResolution.y;

    // 'Look at' vector.
    vec3 la = vec3(1., .25, 1.) + vec3(.5 + cos(time * .7), .25, .5 + sin(time * .5));
    
    // Uncomment for mouse look.
    //la = vec3(1., .25, 1.0) + vec3(-0.5 + iMouse.x / 50., -0.5 + iMouse.y / 50., 0.5);
    
    vec3 forward = normalize(la);
    vec3 right = normalize(vec3(forward.z, 0., -forward.x )); 
    vec3 up = cross(forward, right);

    // rd - Final ray direction.
    vec3 rd = normalize(forward + uv.x * right + uv.y * up);

    // Points movement speed.
    vec3 p = time + vec3(1.0);
    float l = .0;
    
    // Main marching loop.
    for(int i = 0; i < 48; i++)
    {
        float dis = 1.0;
        
        // 3D voronoi loop.
        for (int y= -1; y <= 1; y++) 
        {
            for (int x= -1; x <= 1; x++) 
            {
                 for (int z= -1; z <= 1; z++) 
                {
                    vec3 np = ceil(p + l*rd) + vec3(x,y,z);
                    
                    // Here we distribute points in space. (comment to see it).
                    np += (hash33(np) - .5);
                    dis = min(dis, length(p + l * rd - np));
                }
            }
        }
        
        // Adjusting sphere size and zoom.
        l += zoom * dis - l / (350. * length(dis) /* texture(iChannel0, vec2(.19, .21)).x */) ;
    }
    
    // Adding more impact on color pallete. Try multiplying by say 25 or more ;)
    l += 5.5 * texture(iChannel0, vec2(.19, .21)).x;

    // Output to screen.
    vec3 col = .25 + (.4 * sin(.75 * l * vec3(.75, .41, .33) + time));
    
    // Other version
    //vec3 col = (0.7 * (vec3(0.15, 0.63, 0.81) + (0.35 * (sin(0.25 * l + time)))));
    
    // Additional color mix.
    col = mix(col, col * col * col * 3.5, col);

    fragColor = vec4(col, 1.0);
}
