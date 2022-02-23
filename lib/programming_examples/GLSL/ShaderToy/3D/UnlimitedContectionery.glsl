//stage GL_FRAGMENT_SHADER
//import <GLUP/ShaderToy.h>

#define fbm2(g) fbm3(vec3(g, 0.0))

vec3 gum_colours[4];
vec3 gum_ramps[4];

vec2 tc=vec2(0.0);
float time = 0.0;
float colour = 0.0;
float ss = 1.0;
float is_choc = 0.0;
float t_per_target = 3.0;
vec3 l = normalize(vec3(10.0,30.0,0.0));
float icing_factor = 0.0;

vec3 gumColour(float i)
{
	if(i < 1.0)
		return vec3(0.11,0,0.002);
	else if(i < 2.0)
		return vec3(0.002, 0.06, 0.0);
	else if(i < 3.0)
		return vec3(0.0,0.02, 0.11);
	else
		return vec3(0.11,0.012, 0);
}

vec3 gumRamp(float i)
{
	if(i < 1.0)
		return vec3(0.8,1,1);
	else if(i < 2.0)
		return vec3(0.8,0.8,1);
	else if(i < 3.0)
		return vec3(1,0.8,1);
	else
		return vec3(0.8,0.8,1);
}

// Noise functions from IQ.
float hash( float n ) { return fract(sin(n)*43758.5453123); }
float noise( in vec3 x )
{
    vec3 p = floor(x);
    vec3 f = fract(x);
    f = f*f*(3.0-2.0*f);
	
    float n = p.x + p.y*157.0 + 113.0*p.z;
    return mix(mix(mix( hash(n+  0.0), hash(n+  1.0),f.x),
                   mix( hash(n+157.0), hash(n+158.0),f.x),f.y),
               mix(mix( hash(n+113.0), hash(n+114.0),f.x),
                   mix( hash(n+270.0), hash(n+271.0),f.x),f.y),f.z);
}


float smN2(vec2 p)
{
	return noise(vec3(p,0.0));
}

float smN3(vec3 p)
{
	return noise(p);
}

float fbm3(vec3 p)
{
	float f = 0.0, x;
	for(int i = 1; i <= 9; ++i)
	{
		x = exp2(float(i));
		f += (noise(p * x) - 0.5) / x;
	}
	return f;
}

vec2 rotate(float a, vec2 v)
{
	return vec2(cos(a) * v.x + sin(a) * v.y,
				cos(a) * v.y - sin(a) * v.x);
}

float sugarybit(vec2 p)
{
	p = clamp(p, vec2(-1.0), vec2(1.0));
	vec2 o = vec2(0.0, 0.0);
	float a = 1.0 - pow((1.0 - pow(abs(p.x), 8.0)) * (1.0 - pow(abs(p.y + o.y), 8.0)) * 0.95, 0.5);
	float b = pow((1.0 - pow(abs(p.x), 8.0)) * (1.0 - pow(abs(p.y), 8.0)), 4.0);
	return a * b * 3.0;
}

float sugarlayer(vec2 t, float ndotv)
{
	vec2 t2 = t.xy * 8.0;
	vec2 p = fract(t2) - vec2(0.5);
	vec2 c = floor(t2);
	float a = c.x + c.y * 5.0;
	vec2 o = vec2(cos(c.y * 53.0), sin(c.x * 125.0)) * 2.5;
	vec2 s = 1.0 + vec2(smN2(c), smN2(c + vec2(100.0)));
	
	float fres = mix(1.0, pow(1.0 - ndotv, 4.0), 0.9) * 25.0;
	
	return sugarybit(rotate(a, p * s * 4.0 + o)) * max(0.0, smN2(t2.xy * 5.0) - 0.5) * fres;
}

vec3 saturatecol(vec3 c)
{
	return pow(c, gumRamp(colour));
}

float sprinkles2(vec2 coord, float ndotv)
{
	float sprinkle = 0.0;
	
	for(int i = 0; i < 4; ++i)
		sprinkle += sugarlayer((coord + vec2(float(i) * 10.45)) * (1.0 + float(i) * 0.2), ndotv) * pow(1.0 - float(i) / 4.0, 4.0);
	
	return sprinkle;
}

float sprinkles(vec2 coord, float ndotv)
{
	return sprinkles2(coord * 0.75, ndotv) + sprinkles2(coord * 2.0, ndotv) * 0.3;
}

vec3 gummy(vec3 no, vec3 vo, vec3 v)
{
	float ndotv = dot(no,-v);
	
	float s0 = sprinkles(vec2(atan(no.z, no.x), asin(no.y)) * 0.75, ndotv);
	float s1 = sprinkles(vo.xz, ndotv);
	
	float sprinkle = mix(s0, s1, smoothstep(0.3, 0.5, no.y));
	
	float ss = clamp((no.y -0.3) * 5.0, 0.0, 1.0);
	
	vec3 tex = saturatecol(mix(mix(vec3(0.5),gumColour(colour),0.98) * (0.1 + pow((1.0 - abs(tc.x)) * 0.5, 0.2)), vec3(2.0),
							   sprinkle * ss));
	
	tex *= 1.05 - ndotv;
	tex+=vec3(pow(1.0 - ndotv, 8.0)) * 0.01;
	
	return tex;
}

mat3 rotateXMat(float a)
{
	return mat3(1.0, 0.0, 0.0, 0.0, cos(a), -sin(a), 0.0, sin(a), cos(a));
}

mat3 rotateYMat(float a)
{
	return mat3(cos(a), 0.0, -sin(a), 0.0, 1.0, 0.0, sin(a), 0.0, cos(a));
}

float de(vec3 p)
{
	p.y*=1.3;
	
	vec3 fp=floor(p/3.0);
	
	is_choc=step(0.0,cos(fp.x*10.0-fp.z*103.0)+0.1);
	
	float ff=smN3(p*30.0)*0.001*is_choc-pow(max(0.0,smN3(p*5.0)),5.0)*(0.01+0.05*(1.0-is_choc));
	
	colour=mod(fp.x+sin(fp.z)*3.0,4.0);
	p.xz=mod(p.xz,vec2(3.0))-vec2(1.5);
	
	ss=mix(0.7,1.0,0.5+0.5*cos(fp.z*7.0+fp.x*9.0));
	
	float sp=length(p)-ss;
	float pl=-p.y;
	
	float d=(length(vec2(max(0.0,sp),max(0.0,pl)))-0.1+ff)*0.6;
	
	return d;
}

vec3 marble(vec2 p)
{
	p.x+=2.0;
	float border_size=0.015;
	float corner_size=0.015;
	
	vec2 c0=floor(p);
	vec2 c1=(p-c0)-vec2(0.5);
	vec2 rc1=(c1.x*vec2(1.0)+c1.y*vec2(1.0,-1.0))*0.6;
	
	vec3 ccol=mix(vec3(1.0,1.0,0.5)*0.1,vec3(max(0.0,fbm2(p)*0.5)),0.75);
	vec3 pat=mix(vec3(1.0,1.0,0.6)*0.4,vec3(1.0,1.0,0.8)*0.7,1.0-smoothstep(0.4,0.8,0.5+fbm2(c0*2.0+p*1.0+cos(p.yx*2.0)*0.4)))+
		vec3(max(0.0,fbm2(p*0.7)*1.0))+vec3(smoothstep(0.2,0.3,fbm2(-p)))*0.2;
	vec3 bcol=mix(pat,vec3(1.0,1.0,0.5)*0.1,0.5);
	
	float br=max(smoothstep(0.5-border_size,0.5,abs(c1.y)),smoothstep(0.5-border_size,0.5,abs(c1.x)));
	float cr=max(smoothstep(0.5-corner_size,0.5,abs(rc1.y)),smoothstep(0.5-corner_size,0.5,abs(rc1.x)));
	return mix(pat,mix(bcol,ccol,cr),max(cr,br))*0.8;
}

vec3 cameraPos(float t)
{
	return vec3(t*0.6,6.0+cos(t*4.0)*0.03,0.0);
}

vec3 targetPos(float ti)
{
	vec3 target=cameraPos(ti*t_per_target)*vec3(1.0,0.0,0.0)+vec3(cos(ti*20.0)*4.0,0.0,-7.0+cos(ti*14.0)*3.0);
	target.xz=floor(target.xz/3.0)*3.0+vec2(1.5);
	return target;
}

float cameraZoom(float ti)
{
	return mix(3.0,3.5,0.5+0.5*cos(ti*30.0))*1.4;
}

vec3 trace(vec3 ro, vec3 rd, inout float t, float max_t)
{
	
	for(int i=0;i<100;i+=1)
	{
		float d=de(ro+rd*t);
		if(t>max_t)
			return vec3(0.0);
		if(abs(d)<1e-4)
			break;
		t=min(max_t+1e-3,t+d);
	}
	
	vec3 rp=ro+rd*t;
	vec3 col=vec3(0.2,0.1,0.05)*0.1;
	
	float e=1e-3, c=de(rp);
	vec3 n=normalize(vec3(de(rp+vec3(e,0.0,0.0))-c,de(rp+vec3(0.0,e,0.0))-c,de(rp+vec3(0.0,0.0,e))-c));
	vec3 v=rd;
	
	vec3 h=normalize(l-rd);
	
	if(is_choc<0.5)
	{
		vec3 chocolour;
		
		if(mod(floor(rp.x/3.0),2.0)>0.5)
		{
			float r=pow(distance(rp.xz,floor(rp.xz/3.0)*3.0+vec2(1.5))*0.6,2.0);
			chocolour=mix(vec3(2.0,2.0,1.3)*2.0,vec3(1.3,0.6,0.2)*0.6,smoothstep(0.5+r,0.6+r,0.4+fbm2(rp.xz*10.0)));
		}
		else
		{
			chocolour=mix(vec3(1.3,0.6,0.2),vec3(2.0,2.0,1.3)*2.0,smoothstep(0.7,0.9,0.5+0.5*cos(rp.x*10.0+sin(rp.z*5.0))));
		}
		
		col=(0.1*chocolour*vec3(0.5+0.5*dot(n,l))+0.06*vec3(1.0,1.0,0.5)*vec3(pow(clamp(0.5+0.5*dot(h,n),0.0,1.0),20.0)))*
			(1.0+0.6*pow(dot(n,-rd),2.0));
	}
	else
	{
		col = gummy(n, rp, v) * 5.0 + 0.1*vec3(smoothstep(0.5,0.6,pow(clamp(0.5+0.5*dot(h,n),0.0,1.0),256.0)));
	}
	
	col *= mix(1.0,pow(max(0.0,rp.y),0.5),0.6);
	
	
	return col;
}

void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
	time=iTime;
	
	tc=fragCoord.xy/iResolution.xy*2.0-vec2(1.0);
	
	float ti=floor(time/t_per_target);
	float tf=fract(time/t_per_target);
	
	vec3 camo=cameraPos(time);
	vec3 camt=mix(targetPos(ti-1.0),targetPos(ti),smoothstep(0.3,0.7,tf));
	vec3 camd=normalize(camt-camo);
	
	vec3 camu=normalize(cross(camd,vec3(0.0,1.0,0.0)));
	vec3 camv=normalize(cross(camu,camd));
	camu=normalize(cross(camd,camv));
	
	mat3 m=mat3(camu,camv,camd);
	
	vec2 q=tc.xy*0.5+vec2(0.5);
	vec2 p=tc.xy;
	vec3 ro=camo;
	
	p.x*=iResolution.x/iResolution.y;
	
	float zoom=mix(cameraZoom(ti-1.0),cameraZoom(ti),smoothstep(0.7,0.9,tf));
	
	vec3 rd=m*normalize(vec3(p,zoom));
	
	float t=max(0.0,(1.0-ro.y)/rd.y);
	
	float t2=max(0.0,(-0.01-ro.y)/rd.y);
	
	vec3 col = trace(ro, rd, t, t2);
	
	vec3 rp=ro+rd*t;	

	icing_factor=clamp(rp.y*2.0,0.0,1.0);
	
	if(t > 0.0 && t2 < t)
	{
		vec2 c=ro.xz+rd.xz*t2;
		vec2 xc=c*0.8;
		float x=step(0.5,fract(xc.x+0.5*step(0.5,fract(xc.y))));
		vec3 cc=mix(vec3(1.0),vec3(1.0,1.0,0.5)*0.5,x);
		cc=mix(cc,vec3(0.9,0.9,0.5),pow(0.3+0.5*smN2(xc*10.0+vec2(cos(xc.y*2.0)*3.0,cos(xc.x*1.0)*4.0)),4.0));
		
		vec3 h=normalize(l-rd);
		vec3 r=reflect(rd,vec3(0.0,1.0,0.0));
		float rt=0.0;
		cc=marble(rotate(2.0,xc*0.5));
		
		c=fract(c/3.0);
		col=cc*0.8*mix(gumColour(colour)*2.0*is_choc,vec3(1.0),smoothstep(0.33,0.53,distance(vec2(0.5),c)/ss));
		icing_factor=1.0;
	}

	float icing=1.0-iMouse.x/iResolution.x*icing_factor;
	col=mix(col,vec3(1.0),
		0.9*smoothstep(0.5,0.7,-0.25*icing+0.35+fbm2(rp.xz*4.2))+0.9*smoothstep(0.5,0.7,-0.25*icing+0.3+fbm2(rp.xz*10.1)));
	
	fragColor.rgb=sqrt(col * 1.4);
	
	// IQ's vignet.
	fragColor.rgb *= pow( 16.0*q.x*q.y*(1.0-q.x)*(1.0-q.y), 0.1 );
}


