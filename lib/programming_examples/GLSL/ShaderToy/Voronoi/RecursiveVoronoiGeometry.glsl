//stage GL_FRAGMENT_SHADER
//import <GLUP/ShaderToy.h>


const float pi=acos(-1.);
const int n=3;
const float ssd=1.7;
const int maxdepth=7;

float depth=0.;
float index=0.;
float h0=0.,h1=-.1;

vec2 ps[n];
int j=0;

vec2 point(vec2 q, int k)
{
    return vec2(cos((float(k)+.5)/float(n)*pi*2.+float(index)*2.),sin((float(k)+.5)/float(n)*pi*2.))+vec2(0.,.5);
}

void populatePoints(vec2 p,float ss)
{
    float md=1e4;
    for(int k=0;k<n;++k)
    {
        ps[k]=point(p, k);
        ps[k]*=ss;
        float d = distance(ps[k], p);
        if(d<md)
        {
            md=d;
            j=k;
        }
    }
}

float shadow(vec3 hitp,vec3 ld,float softness)
{
    float shadt=(h0-hitp.z)/ld.z;
    vec3 shadp=hitp+ld*shadt;
    float r=softness*shadt;
    float dd=0.;
    float ss=1.;
    depth=0.;
    index=0.;

    for(int i=0;i<maxdepth;++i)
    {
        if(cos(float(index)*239.)<-.8)break;

        populatePoints(hitp.xy, ss);

        for(int k=0;k<n;++k)
            if(k!=j)
                dd+=pow(min(0.,-r/2.+dot(shadp.xy-(ps[j]+ps[k])/2.,normalize(ps[j]-ps[k]))),2.);   

            shadp.xy-=ps[j];
        hitp.xy-=ps[j];
        ss/=ssd;

        index=index*float(n)+float(j);
        depth+=1.;
    }
    return 1.-clamp(sqrt(dd)/r,0.,1.);
}

float light(vec3 hitp,vec3 ld,vec3 hitnorm,float softness)
{
    return max(0.,.5+.5*dot(ld,hitnorm))*pow(shadow(hitp,ld,softness),2.5);
}

void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
    vec2 uv = fragCoord.xy / iResolution.xy;
    vec2 t=uv*2.-1.;
   	t.y/=iResolution.x/iResolution.y;
    vec3 ro=vec3(cos(iTime/10.)+.1,0.3,1.1),oro=ro;
    vec3 rd=normalize(vec3(t.xy,-1.));

    float t0=(h0-ro.z)/rd.z;
    float t1=(h1-ro.z)/rd.z;

    ro+=rd*t0;

    float ppt=t1-t0;

    vec3 floornorm=vec3(0,0,1);
    vec3 hitnorm=floornorm;

    float ss=1.;
    depth=0.;
    index=0.;
    float dd=1e3;
    for(int i=0;i<maxdepth;++i)
    {
        if(cos(float(index)*239.)<-.8)break;

        populatePoints(ro.xy, ss);

        for(int k=0;k<n;++k)
        {
            if(k!=j)
            {
                vec4 plane;
                plane.xy=-normalize(ps[k]-ps[j]);
                plane.z=0.;
                float thickness=.0015;
                plane.w=dot((ps[j]+ps[k])/2., plane.xy)+thickness;
                dd=min(dd,-thickness+dot(ro.xy-(ps[j]+ps[k])/2.,plane.xy));
                float pt=(plane.w-dot(ro,plane.xyz))/dot(rd,plane.xyz);
                if(dot(rd,plane.xyz)<0.&&pt<ppt)
                {
                    ppt=pt;
                    hitnorm=plane.xyz;
                }
            }
        }      

        ro.xy-=ps[j];
        ss/=ssd;

        index=index*float(n)+float(j);
        depth+=1.;
        if(dd<0.){ppt=0.;hitnorm=floornorm;break;}

    }
    ppt+=t0;
    vec3 hitp=oro+rd*ppt;

    vec3 diff=mix(vec3(.5,.75,1),vec3(1,1.,.5),.5+.5*cos(index));

    diff=mix(diff,vec3(1./1.5),1.-clamp(dd/.001,0.,1.));
    
  	vec3 diffm=vec3(1);
    
    if(hitnorm.z>.0)
		diffm=texture(iChannel0,hitp.xy).rgb;
    else
		diffm=texture(iChannel0,2.*vec2(dot(hitp.xy,normalize(vec2(hitnorm.y,-hitnorm.x))),hitp.z/2.)).rgb;
    
    diff*=mix(diffm,vec3(1),.4);
    diff=mix(diff,vec3(dot(diff,vec3(1./3.))),.13);
    //diff=diffm;
    
    fragColor.rgb=vec3(0);
    fragColor.rgb+=diff*light(hitp,normalize(vec3(1.,.5,.7)),hitnorm,.2)*vec3(1.,1.,.8)*
        (.3+smoothstep(.05,.3,abs(fract((hitp.y*6.+hitp.x)*.7)-.5)));
    fragColor.rgb+=diff*pow(light(hitp,normalize(vec3(-.1,0.,7)),hitnorm,2.5)/1.,.5)/3.*vec3(.8,.9,1.);

    fragColor.rgb*=exp(-length(hitp.xy-oro.xy)/4.);
    fragColor.rgb*=1.5;
    fragColor.rgb+=.01*vec3(1,1,.7);
    fragColor.rgb=pow(fragColor.rgb*1.,vec3(1./2.4));
}
