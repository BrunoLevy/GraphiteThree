//stage GL_FRAGMENT_SHADER
//import <GLUP/ShaderToy.h>

// "Smiley Tutorial" by Martijn Steinrucken aka BigWings - 2017
// License Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License.
//
// This Smiley is part of my ShaderToy Tutorial series on YouTube:
// Part 1 - Creating the Smiley - https://www.youtube.com/watch?v=ZlNnrpM0TRg
// Part 2 - Animating the Smiley - https://www.youtube.com/watch?v=vlD_KOrzGDc&t=83s

#define S(a, b, t) smoothstep(a, b, t)
#define B(a, b, blur, t) S(a-blur, a+blur, t)*S(b+blur, b-blur, t)
#define sat(x) clamp(x, 0., 1.)

float remap01(float a, float b, float t) {
	return sat((t-a)/(b-a));
}

float remap(float a, float b, float c, float d, float t) {
	return sat((t-a)/(b-a)) * (d-c) + c;
}

vec2 within(vec2 uv, vec4 rect) {
	return (uv-rect.xy)/(rect.zw-rect.xy);
}

vec4 Brow(vec2 uv, float smile) {
    float offs = mix(.2, 0., smile);
    uv.y += offs;
    
    float y = uv.y;
    uv.y += uv.x*mix(.5, .8, smile)-mix(.1, .3, smile);
    uv.x -= mix(.0, .1, smile);
    uv -= .5;
    
    vec4 col = vec4(0.);
    
    float blur = .1;
    
   	float d1 = length(uv);
    float s1 = S(.45, .45-blur, d1);
    float d2 = length(uv-vec2(.1, -.2)*.7);
    float s2 = S(.5, .5-blur, d2);
    
    float browMask = sat(s1-s2);
    
    float colMask = remap01(.7, .8, y)*.75;
    colMask *= S(.6, .9, browMask);
    colMask *= smile;
    vec4 browCol = mix(vec4(.4, .2, .2, 1.), vec4(1., .75, .5, 1.), colMask); 
   
    uv.y += .15-offs*.5;
    blur += mix(.0, .1, smile);
    d1 = length(uv);
    s1 = S(.45, .45-blur, d1);
    d2 = length(uv-vec2(.1, -.2)*.7);
    s2 = S(.5, .5-blur, d2);
    float shadowMask = sat(s1-s2);
    
    col = mix(col, vec4(0.,0.,0.,1.), S(.0, 1., shadowMask)*.5);
    
    col = mix(col, browCol, S(.2, .4, browMask));
    
    return col;
}

vec4 Eye(vec2 uv, float side, vec2 m, float smile) {
    uv -= .5;
    uv.x *= side;
    
	float d = length(uv);
    vec4 irisCol = vec4(.3, .5, 1., 1.);
    vec4 col = mix(vec4(1.), irisCol, S(.1, .7, d)*.5);		// gradient in eye-white
    col.a = S(.5, .48, d);									// eye mask
    
    col.rgb *= 1. - S(.45, .5, d)*.5*sat(-uv.y-uv.x*side); 	// eye shadow
    
    d = length(uv-m*.4);									// offset iris pos to look at mouse cursor
    col.rgb = mix(col.rgb, vec3(0.), S(.3, .28, d)); 		// iris outline
    
    irisCol.rgb *= 1. + S(.3, .05, d);						// iris lighter in center
    float irisMask = S(.28, .25, d);
    col.rgb = mix(col.rgb, irisCol.rgb, irisMask);			// blend in iris
    
    d = length(uv-m*.45);									// offset pupile to look at mouse cursor
    
    float pupilSize = mix(.4, .16, smile);
    float pupilMask = S(pupilSize, pupilSize*.85, d);
    pupilMask *= irisMask;
    col.rgb = mix(col.rgb, vec3(0.), pupilMask);		// blend in pupil
    
    float t = iTime*3.;
    vec2 offs = vec2(sin(t+uv.y*25.), sin(t+uv.x*25.));
    offs *= .01*(1.-smile);
    
    uv += offs;
    float highlight = S(.1, .09, length(uv-vec2(-.15, .15)));
    highlight += S(.07, .05, length(uv+vec2(-.08, .08)));
    col.rgb = mix(col.rgb, vec3(1.), highlight);			// blend in highlight
    
    return col;
}

vec4 Mouth(vec2 uv, float smile) {
    uv -= .5;
	vec4 col = vec4(.5, .18, .05, 1.);
    
    uv.y *= 1.5;
    uv.y -= uv.x*uv.x*2.*smile;
    
    uv.x *= mix(2.5, 1., smile);
    
    float d = length(uv);
    col.a = S(.5, .48, d);
    
    vec2 tUv = uv;
    tUv.y += (abs(uv.x)*.5+.1)*(1.-smile);
    float td = length(tUv-vec2(0., .6));
    
    vec3 toothCol = vec3(1.)*S(.6, .35, d);
    col.rgb = mix(col.rgb, toothCol, S(.4, .37, td));
    
    td = length(uv+vec2(0., .5));
    col.rgb = mix(col.rgb, vec3(1., .5, .5), S(.5, .2, td));
    return col;
}

vec4 Head(vec2 uv) {
	vec4 col = vec4(.9, .65, .1, 1.);
    
    float d = length(uv);
    
    col.a = S(.5, .49, d);
    
    float edgeShade = remap01(.35, .5, d);
    edgeShade *= edgeShade;
    col.rgb *= 1.-edgeShade*.5;
    
    col.rgb = mix(col.rgb, vec3(.6, .3, .1), S(.47, .48, d));
    
    float highlight = S(.41, .405, d);
    highlight *= remap(.41, -.1, .75, 0., uv.y);
    highlight *= S(.18, .19, length(uv-vec2(.21, .08)));
    col.rgb = mix(col.rgb, vec3(1.), highlight);
    
    d = length(uv-vec2(.25, -.2));
    float cheek = S(.2,.01, d)*.4;
    cheek *= S(.17, .16, d);
    col.rgb = mix(col.rgb, vec3(1., .1, .1), cheek);
    
    return col;
}

vec4 Smiley(vec2 uv, vec2 m, float smile) {
	vec4 col = vec4(0.);
    
    if(length(uv)<.5) {					// only bother about pixels that are actually inside the head
        float side = sign(uv.x);
        uv.x = abs(uv.x);
        vec4 head = Head(uv);
        col = mix(col, head, head.a);

        if(length(uv-vec2(.2, .075))<.175) {
            vec4 eye = Eye(within(uv, vec4(.03, -.1, .37, .25)), side, m, smile);
            col = mix(col, eye, eye.a);
        }

        if(length(uv-vec2(.0, -.15))<.3) {
            vec4 mouth = Mouth(within(uv, vec4(-.3, -.43, .3, -.13)), smile);
            col = mix(col, mouth, mouth.a);
        }

        if(length(uv-vec2(.185, .325))<.18) {
            vec4 brow = Brow(within(uv, vec4(.03, .2, .4, .45)), smile);
            col = mix(col, brow, brow.a);
        }
    }
    
    return col;
}

void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
	float t = iTime;
    
    vec2 uv = fragCoord.xy / iResolution.xy;
    uv -= .5;
    uv.x *= iResolution.x/iResolution.y;
    
    vec2 m = iMouse.xy / iResolution.xy;
    m -= .5;
    
    if(m.x<-.49 && m.y<-.49) {			// make it that he looks around when the mouse hasn't been used
    	float s = sin(t*.5);
        float c = cos(t*.38);
        
        m = vec2(s, c)*.4;
    }
    
    if(length(m) > .707) m *= 0.;		// fix bug when coming back from fullscreen
    
    float d = dot(uv, uv);
    uv -= m*sat(.23-d);
    
    float smile = sin(t*.5)*.5+.5;
	fragColor = Smiley(uv, m, smile);
}