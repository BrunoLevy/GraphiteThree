//stage GL_FRAGMENT_SHADER
//import <GLUP/ShaderToy.h>

float m_dist = 1.0;

vec2 random2( vec2 p ) 
{
    return fract(sin(vec2(dot(p,vec2(127.1,311.7)),dot(p,vec2(269.5,183.3))))*43758.5453);
}

void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
    vec2 st = fragCoord.xy / iResolution.xy;
    
    float aspect = iResolution.x / iResolution.y;
    
    // Normalize aspect.
    st.x *= iResolution.x / iResolution.y;
    
    // Scale
    st *= 10.;
    
    // Tile the space
    vec2 i_st = floor(st);
    vec2 f_st = fract(st);

    vec3 color = vec3(.0);

    for (int y= -1; y <= 1; y++) {
        for (int x= -1; x <= 1; x++) {
            // Neighbor place in the grid
            vec2 neighbor = vec2(float(x),float(y));

            // Random position from current + neighbor place in the grid
            vec2 point = random2(i_st + neighbor);
            
			// Animate the point
            point = 0.5 + 0.5 * cos(0.8831 * point * iTime);

			// Vector between the pixel and the point
            vec2 diff = neighbor + point - f_st;

            // Distance to the point
            float dist = length(diff);

            // Keep the closer distance
            m_dist = min(m_dist, dist);
        }
    }

    // Draw the min distance (distance field)
    color += vec3(0., m_dist - 0.2, m_dist - 0.1);

    // Draw cell center
    // color += 1.-step(.02, m_dist);

    // Draw grid
    // color.r += step(.98, f_st.x) + step(.98, f_st.y);

    // Show isolines
    // color -= step(.7,abs(sin(27.0*m_dist)))*.5;

    fragColor = vec4(color, 1.0);
}

