//stage GL_FRAGMENT_SHADER
//import <GLUP/ShaderToy.h>

// Cheap 2D Humanoid SDF for dropping into scenes to add a sense of scale.
// Hazel Quantock 2018
// This work is licensed under a Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License. http://creativecommons.org/licenses/by-nc-sa/4.0/

float RoundMax( float a, float b, float r )
{
    a += r; b += r;
    
    float f = ( a > 0. && b > 0. ) ? sqrt(a*a+b*b) : max(a,b);
    
    return f - r;
}

float RoundMin( float a, float b, float r )
{
    return -RoundMax(-a,-b,r);
}

// Humanoid, feet placed at <0,0>, with height of ~1.8 units on y
float Humanoid( in vec2 uv, in float phase )
{
    #define Rand(idx) fract(phase*pow(1.618,float(idx)))
    float n3 = sin((uv.y-uv.x*.7)*11.+phase)*.014; // "pose"
    float n0 = sin((uv.y+uv.x*1.1)*23.+phase*2.)*.007;
    float n1 = sin((uv.y-uv.x*.8)*37.+phase*4.)*.004;
    float n2 = sin((uv.y+uv.x*.9)*71.+phase*8.)*.002;
    //uv.x += n0+n1+n2; uv.y += -n0+n1-n2;
    
    float head = length((uv-vec2(0,1.65))/vec2(1,1.2))-.15/1.2;
    float neck = length(uv-vec2(0,1.5))-.05;
    float torso = abs(uv.x)-.25;
    //torso += .2*(1.-cos((uv.y-1.)*3.));
    //torso = RoundMax( torso, abs(uv.y-1.1)-.4, .2*(uv.y-.7)/.8 );
    torso = RoundMax( torso, uv.y-1.5, .2 );
    torso = RoundMax( torso, -(uv.y-.5-.4*Rand(3)), .0 );

    float f = RoundMin(head,neck,.04);
    f = RoundMin(f,torso,.02);
    
    float leg =
        Rand(1) < .3 ?
        abs(uv.x)-.1-.1*uv.y : // legs together
    	abs(abs(uv.x+(uv.y-.8)*.1*cos(phase*3.))-.15+.1*uv.y)-.05-.04*Rand(4)-.07*uv.y; // legs apart
    leg = max( leg, uv.y-1. );
    
    f = RoundMin(f,leg,.2*Rand(2));
    
    f += (-n0+n1+n2+n3)*(.1+.9*uv.y/1.6);
    
    return max( f, -uv.y );
}


void mainImage( out vec4 fragColour, in vec2 fragCoord )
{
    vec2 uv = (fragCoord-iResolution.xy*vec2(.5,0))/iResolution.y;
    uv = uv*2.-vec2(0,0);
//    float phase = floor(iTime*3.)*2.; // randomise
    uv.x += iTime;
    float w = .7;
    float sss = floor(uv.x/w);
    uv.x = uv.x - (sss + .5)*w;
    float phase = 2.*sss;
    float sdf = Humanoid( uv, phase );
    
    fragColour = vec4(.5+.5*sdf/(abs(sdf)+.002)); // map SDF to slightly soft black & white image
}

