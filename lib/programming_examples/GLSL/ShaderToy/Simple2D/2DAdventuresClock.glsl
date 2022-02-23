//stage GL_FRAGMENT_SHADER
//import <GLUP/ShaderToy.h>


//// ABSTRACT
//
//	The purpose of this shader was to bang my head against the wall until I
//	understoon the methodology behind 2D shaders. In particular I wanted to
//	learn how to devise a solution to a shader rather than follow from anothers
//	example. The idea clicked when I started thinking of the functions as point
//	collision detection, which allowed me to discern the line segment from the
//	line function.
//
//	I enjoyed this endevour however small. I think I'll be making a small series
//	of shaders following my fiddling with 2D shaders, I hope at some point to be
//	able to devise shaders as beautiful as this one by Shane;
//		https://www.shadertoy.com/view/MtlBDs
//	I suppose it makes a decent goal.
//
//// AUTHOR
//
//	Scott R Howell (Bombshell93)

#define PI 3.14
#define PI2 6.28

//// sat
//
//	a saturate function, returns the given float clamped between 0.0 amd 1.0

float sat(float a) {
    
    return clamp(a, 0.0, 1.0);
}

//// circle
//
//	a circle distance function, if negative the point lies within the circle

float circle(vec2 uv, vec2 position, float radius) {
    
    return length(uv - position) - radius;
}

//// lineSegment
//
//	a line segment distance function, finding the nearest point and performing 
//	a circle distance function at that point, effectively a capsule point
//	collision detection function

float lineSegment(vec2 uv, vec2 start, vec2 end, float width) {
    
    vec2 stoe = end - start;
    vec2 nstoe = normalize(stoe);
    vec2 near = start + nstoe * sat(dot(nstoe, uv) / length(stoe)) * length(stoe);
    return length(uv - near) - width;
}

void mainImage( out vec4 outColor, in vec2 coord ) {
    
    vec2 uv = (coord - iResolution.xy * 0.5) * 2.0 / iResolution.y;
    
    vec3 color = vec3(uv, 0.0);
    
    //	clock face
    //
    //	dist is set so the outline can be done without another shape
    
    float faceDist = circle(uv, vec2(0.0), 1.0);
    
    //	digits
    //
    //	simple, 12 dots distributed around a circles edge, like a clock
    //	EDIT: previously this was a for loop, I had considered a for loop
    //	would be inefficient but not thought to use atan in its place to
    //	determine the nearest digit, thanks to FabriceNeyret2
    
    float i = round(atan(uv.y, uv.x) * 12.0 / PI2);
    float angle = (PI2 / 12.0) * i;
    float digitDist = circle(uv, vec2(cos(angle), sin(angle)) * 0.8, 0.05);
    
    //	hands
    //
    //	line segments with different timed cos and sin functions determining
    //	the segment end to be a revolving circle
    //	EDIT: I've moved from iTime to iDate and added an extra hand for seconds
    
    float seconds = iDate.w * PI2 / 60.0;
    float minutes = seconds / 60.0;
    float hours = minutes / 12.0;
    float handDist = lineSegment(uv, vec2(0.0), vec2(sin(seconds), cos(seconds)) * 0.8, 0.025);
    handDist = min(handDist, lineSegment(uv, vec2(0.0), vec2(sin(minutes), cos(minutes)) * 0.8, 0.05));
    handDist = min(handDist, lineSegment(uv, vec2(0.0), vec2(sin(hours), cos(hours)) * 0.6, 0.05));
    
    //	EDIT: compositing
    //
    //	thanks to some advice from FabriceNeyret2 in the comments I learned some
    //	better shader etiquette, so for the sake of a final addition before I
    //	move on to 2D Adventures #2 distance values are stored and used to
    //	composite colors at the end using mix and smooth step for anti-aliasing
    
    color = mix(color, vec3(0.0), smoothstep(0.0, -2.0 / iResolution.y, faceDist));
    color = mix(color, vec3(1.0), smoothstep(0.0, -2.0 / iResolution.y, faceDist + 0.1));
    color = mix(color, vec3(1.0, 0.0, 0.0), smoothstep(0.0, -2.0 / iResolution.y, digitDist));
    color = mix(color, vec3(0.0), smoothstep(0.0, -2.0 / iResolution.y, handDist));
    
    outColor = vec4(color, 1.0);
}

