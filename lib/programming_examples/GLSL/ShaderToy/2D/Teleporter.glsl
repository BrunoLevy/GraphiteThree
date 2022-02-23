//stage GL_FRAGMENT_SHADER
//import <GLUP/ShaderToy.h>
#define pi 3.14159
#define tau (2.0 * pi)

// Gold Noise ©2017-2018 dcerisano@standard3d.com
// - based on the Golden Ratio, PI and the Square Root of Two
// - fastest noise generator function
// - works with all chipsets (including low precision)

precision lowp    float;

float PHI = 1.61803398874989484820459 * 00000.1; // Golden Ratio   
float PI  = 3.14159265358979323846264 * 00000.1; // PI
float SQ2 = 1.41421356237309504880169 * 10000.0; // Square Root of Two

float gold_noise(in vec2 coordinate, in float seed){
    return fract(sin(dot(coordinate*(seed+PHI), vec2(PHI, PI)))*SQ2);
}



vec3 getColor(vec2 fragCoord, float time, vec3 color, float theta, float rad, float len, float life, float dist, float ang) {
       
    float beamWidth = 2.0;
    float timeLived = mod(time, life);

    rad -= timeLived * rad / life;			// Reduce radius as we spiral towards center


    float angdiff = theta - ang;			// Angle between pixel and head of tracker
    if(angdiff < 0.0) 
        angdiff += tau;

    float alpha = 0.0;

    if(dist > (rad - beamWidth * (1.0 - 2.0 * angdiff / len)) && dist < rad + beamWidth)
        alpha = (1.0 - angdiff / len) * smoothstep(0.1, 1.0, timeLived/life + .4);
    
  	return vec3(color * alpha);
}


vec3 getBaseColor(int index) {
    
    if(index == 1)
     	return vec3( 0.0, 0.25, 0.8 );
    if(index == 2)
     	return vec3( 0.0, 0.5,  1.0 );
    if(index == 3)
     	return vec3( 0.0, 0.0,  1.0 );
    if(index == 4)
     	return vec3( 0.0, 1.0,  1.0 );
    if(index == 5)
     	return vec3( 0.0, 0.5,  0.5 );
    if(index == 6)
     	return vec3( 0.0, 0.0,  1.0 );
}


void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
    
    float rad =  80.0;
    
    
    const int trackers = 80;
    #define baseColorCount 6
    
    // Normalized pixel coordinates (from 0 to 1)
    //vec2 uv = fragCoord/iResolution.xy;
    vec2 cen = iResolution.xy / 2.0;
    float dist = distance(fragCoord, cen);

       
    vec3 color = vec3(0.0);	// Start with black; we'll add up colors as we go
    
    if(dist < rad) {
    	float ang = -atan(fragCoord.y - cen.y, fragCoord.x - cen.x);

        for(int i = 0; i < trackers; i++) {

            float fi = float(i);

            float life = 5.0 + 4.0 * gold_noise(vec2(fi),fi);
            float len = 0.25 + 1.75 * gold_noise(vec2(fi)*3.0, fi); // in radians
            float theta = mod(iTime * (0.5 +  2.0 * gold_noise(vec2(fi)*2.0,fi)), tau) - pi;

            int colorIndex = int(mod(fi,float(baseColorCount)));

            vec3 baseColor = getBaseColor(colorIndex);

            vec3 col = getColor(fragCoord, iTime, baseColor, theta, rad, len, life, dist, ang);

            color += col;
        }
    }


    // Output to screen
    fragColor = vec4(color, 1.0);
}