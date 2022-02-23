//stage GL_FRAGMENT_SHADER
//import <GLUP/ShaderToy.h>

//
// Simple toon fire example. Use on particles/ribbons/etc
//
// Jeremy Mitchell
// twitter: @floatvoid
//



float hash2D(vec2 x) {
	return fract(sin(dot(x, vec2(13.454, 7.405)))*12.3043);
}

//voronoi borrowed from someone. Probably iq? Sorry I forgot :(

float voronoi2D(vec2 uv) {
    vec2 fl = floor(uv);
    vec2 fr = fract(uv);
    float res = 1.0;
    for( int j=-1; j<=1; j++ ) {
        for( int i=-1; i<=1; i++ ) {
            vec2 p = vec2(i, j);
            float h = hash2D(fl+p);
            vec2 vp = p-fr+h;
            float d = dot(vp, vp);
            
            res +=1.0/pow(d, 8.0);
        }
    }
    return pow( 1.0/res, 1.0/16.0 );
}

void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
	vec2 uv = fragCoord.xy / iResolution.xy;


    //two scrolling voronoi maps. Could be a texture also
    float up0 = voronoi2D(uv * vec2(6.0, 4.0) + vec2(0,-iTime * 2.0)  );
	float up1 = 0.5 + voronoi2D(uv * vec2(6.0, 4.0) + vec2(42,-iTime * 2.0) + 30.0 );
	float finalMask = up0 * up1 + (1.0-uv.y);
   
    
    //vertical gradient. In a game use vertex color or something.
    finalMask += (1.0-uv.y)* 0.5;
    
    //horizontal gradient.
    finalMask *= 0.7-abs(uv.x - 0.5);
    
    
    vec3 dark = mix( vec3(0.0), vec3( 1.0, 0.4, 0.0),  step(0.8,finalMask) ) ;
    vec3 light = mix( dark, vec3( 1.0, 0.8, 0.0),  step(0.95, finalMask) ) ;
    
    
	fragColor.xyz = light;
}
