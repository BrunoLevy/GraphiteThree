//stage GL_FRAGMENT_SHADER
//import <GLUP/ShaderToy.h>

// Credits: S.Guillitte  for the color distribution  and Stefan Gustavson for the cellurarl noise 

float rand(vec2 co){
    return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

float randomInRange(in float lower, in float higher, in float period, in float seedDifference){
     float ranVal = rand (vec2(seedDifference + floor(iTime / (period))));
     return lower + ranVal * (higher-lower);
}

vec4 permute(in vec4 x)
{
 return mod ((34.0 * x + 1.0) * x , 289.0);   
}

vec2 celluar2x2( vec2 P, float  jitterng) {
    
    float pp =  1.0 + floor(jitterng* 2.0) *7.0; 
    
 	float K = 1.0/pp;
	float K2 = 0.5/pp;
	float jitter = jitterng /*abs( fract(iTime * 0.1)* 2.0 - 1. )*/; // jitter 1.0 makes F1 wrong more often
	vec2 Pi = mod(floor(P), 289.0) ;
	vec2 Pf = fract(P);
	vec4 Pfx = Pf.x + vec4(-0.5, -1.5, -0.5, -1.5);
	vec4 Pfy = Pf.y + vec4(-0.5, -0.5, -1.5, -1.5);
	vec4 p = permute (Pi.x + vec4(0.0 , 1.0, 0.0, 1.0));
	p = permute (p + Pi.y + vec4(0.0 , 0.0, 1.0, 1.0));
	vec4 ox = mod(p, pp) * K + K2;
	vec4 oy = mod(floor(p * K) ,pp) * K + K2;
	vec4 dx = Pfx + jitter * ox;
	vec4 dy = Pfy + jitter * oy;
	vec4 d = dx * dx + dy * dy; // distances squared
	// Cheat and pick only F1 for the return value
	d.xy = min(d.xy, d.zw);
	d.x = min(d.x, d.y);
	return d.xx; // F1 duplicated , F2 not computed
 
    
}

void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
    
    float period =20.;
    float time= mod(iTime*1.0, period);
    vec2 uv = (fragCoord.xy / iResolution.y)*8.0;
    vec2 uv0=uv;
    
    float Seed = randomInRange(0.7, 1.7, period, 2232.);
    float i0=Seed;
    float i1=1.0;
    float i2=1.0;
    float i4=0.0;
    float firstSeed = randomInRange(0.0, 1., period, 1323.);

    vec2 F = celluar2x2(uv0 , firstSeed);
    float secondSeed = randomInRange(0.0, 100., period, 242.);
    for(int s=0;s<7;s++)
    {
        vec2 r;
        r=vec2(cos(uv.y*i0-i4+time * pow(F.x,2.0)/i1  +  secondSeed),sin(uv.x*i0-i4+time * F.x/i1  +  secondSeed))/i2;
        r+=vec2(-r.y,r.x)*0.3 ;
        uv.xy+=r;

        i0*=1.93;
        i1*=1.15;
        i2*=1.7;
        i4+=0.05+0.1*time*i1;
    }
    float r=sin(uv.x + secondSeed-time *0.5* F.x )*0.5+0.5;
    float b=sin(uv.y+ secondSeed+time *0.4*F.x )*0.5+0.5;
    float g=sin((uv.x+ secondSeed +uv.y+sin(time *F.x*0.5 ))*0.5)*0.5+0.5;


    fragColor = vec4(r,g,b,1.0);
}
