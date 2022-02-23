//stage GL_FRAGMENT_SHADER
//import <GLUP/ShaderToy.h>

// "The Inversion Machine" by Kali

const float width=.22;
const float scale=4.;
const float detail=.002;

vec3 lightdir=-vec3(.2,.5,1.);

mat2 rot;

float rand(vec2 co){
	return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

float de(vec3 p) {
	float t=iTime;
	float dotp=dot(p,p);
	p.x+=sin(t*40.)*.007;
	p=p/dotp*scale;
	p=sin(p+vec3(sin(1.+t)*2.,-t,-t*2.));
	float d=length(p.yz)-width;
	d=min(d,length(p.xz)-width);
	d=min(d,length(p.xy)-width);
	d=min(d,length(p*p*p)-width*.3);
	return d*dotp/scale;
}

vec3 normal(vec3 p) {
	vec3 e = vec3(0.0,detail,0.0);
	
	return normalize(vec3(
			de(p+e.yxx)-de(p-e.yxx),
			de(p+e.xyx)-de(p-e.xyx),
			de(p+e.xxy)-de(p-e.xxy)
			)
		);	
}

float light(in vec3 p, in vec3 dir) {
	vec3 ldir=normalize(lightdir);
	vec3 n=normal(p);
	float sh=1.;
	float diff=max(0.,dot(ldir,-n))+.1*max(0.,dot(normalize(dir),-n));
	vec3 r = reflect(ldir,n);
	float spec=max(0.,dot(dir,-r))*sh;
	return diff+pow(spec,20.)*.7;	
		}

float raymarch(in vec3 from, in vec3 dir, in vec2 fragCoord) 
{
	vec2 uv = fragCoord.xy / iResolution.xy*2.-1.;
	uv.y*=iResolution.y/iResolution.x;
	float st,d,col,totdist=st=0.;
	vec3 p;
	float ra=rand(uv.xy*iTime)-.5;
	float ras=max(0.,sign(-.5+rand(vec2(1.3456,.3573)*floor(30.+iTime*20.))));
	float rab=rand(vec2(1.2439,2.3453)*floor(10.+iTime*40.))*ras;
	float rac=rand(vec2(1.1347,1.0331)*floor(40.+iTime));
	float ral=rand(1.+floor(uv.yy*300.)*iTime)-.5;
	for (int i=0; i<60; i++) {
		p=from+totdist*dir;
		d=de(p);
		if (d<detail || totdist>2.) break;
		totdist+=d; 
		st+=max(0.,.04-d);
	}
	vec2 li=uv*rot;
	float backg=.45*pow(1.5-min(1.,length(li+vec2(0.,-.6))),1.5);
	if (d<detail) {
		col=light(p-detail*dir, dir); 
	} else { 
		col=backg;
	}
	col+=smoothstep(0.,1.,st)*.8*(.1+rab);
	col+=pow(max(0.,1.-length(p)),8.)*(.5+10.*rab);
	col+=pow(max(0.,1.-length(p)),30.)*50.;
	col = mix(col, backg, 1.0-exp(-.25*pow(totdist,3.)));
	if (rac>.7) col=col*.7+(.3+ra+ral*.5)*mod(uv.y+iTime*2.,.25);
	col = mix(col, .5+ra+ral*.5, max(0.,3.-iTime)/3.);
	return col+ra*.03+(ral*.1+ra*.1)*rab;
}

void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
	float t=iTime*.2;
	vec2 uv = fragCoord.xy / iResolution.xy*2.-1.;
	uv.y*=iResolution.y/iResolution.x;
	vec3 from=vec3(0.,0.1,-1.2);
	vec3 dir=normalize(vec3(uv,1.));
	rot=mat2(cos(t),sin(t),-sin(t),cos(t));
	dir.xy=dir.xy*rot;
	float col=raymarch(from,dir,fragCoord); 
	col=pow(col,1.25)*clamp(60.-iTime,0.,1.);
	fragColor = vec4(col);
}

