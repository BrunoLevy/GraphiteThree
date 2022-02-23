//stage GL_FRAGMENT_SHADER
//import <GLUP/ShaderToy.h>

//no license, use it as you wish

//can undef both
#define text 
#define AA

vec3 colorMinutes=vec3(.98222,.98222,.98222);
vec3 colorHours=vec3(.98222,.98222,.98222);
vec3 colorTextM=vec3(.98222,.98222,.98222);
vec3 colorTextH=vec3(.98222,.98222,.98222);
vec3 colorTri=vec3(.98222,.98222,.98222)/2.5;


#ifdef text
#ifdef AA
//https://www.shadertoy.com/view/Xsy3zG
float segment(vec2 uv)
{
    uv = abs(uv);
	return (1.0-smoothstep(-0.17,0.40,uv.x))
         * (1.0-smoothstep(0.25,0.66,uv.y));
}

float sevenSegment(vec2 uv,int num)
{
	float seg= 0.0;
    if (num>=2 && num!=7 || num==-2)
        seg = max(seg,segment(uv.yx));
    if (num==0 || 
            (uv.y<0.?((num==2)==(uv.x<0.) || num==6 || num==8):
            (uv.x>0.?(num!=5 && num!=6):(num>=4 && num!=7) )))
        seg = max(seg,segment(abs(uv)-0.5)); 
    if (num>=0 && num!=1 && num!=4 && (num!=7 || uv.y>0.))
        seg = max(seg,segment(vec2(abs(uv.y)-1.0,uv.x)));
	return seg;
}

float showNum(vec2 uv,float nr)
{
        float digit = floor(-uv.x / 1.5);
    	nr /= pow(10.,digit);
        nr = mod(floor(nr+0.000001),10.0);
    	if (uv.x<-2.90 )
            return 0.;
		return sevenSegment(uv+vec2( 0.75 + digit*1.5,0.0),int(nr));
}

float drawminutes(vec2 uv){
    float t;
    t=floor(mod((uv.x-0.0035)/.01,12.)/2.)*10.+10.;
    uv.y+=0.0652;
    uv.y/=0.25;
    uv.x+=0.0573;
    uv/=2.25;
    uv.x=-uv.x;
    return showNum(vec2(-mod(uv.x/.01,2./2.25)/0.15,uv.y*80.),t);
}

float drawhours(vec2 uv){
    float t;
    uv.y-=0.12052;
    uv.y/=0.221;
    uv.x+=0.0313024;
    uv/=2.;
    t=(floor(mod((uv.x-0.0035)/.01,144.)/6.)*10.)/10.;
    uv/=2.5;
    uv.x=-uv.x;
    return showNum(vec2(-mod((uv.x)/(.01),2./2.5)/(0.15),uv.y*80.),t);
}
#else
//https://www.shadertoy.com/view/ldKGRR
float D(vec2 p, float n) {  
    int i=int(p.y), b=int(exp2(floor(30.-p.x-n*3.)));
    i = ( p.x<0.||p.x>3.? 0:
    i==5? 972980223: i==4? 690407533: i==3? 704642687: i==2? 696556137:i==1? 972881535: 0 )/b;
 	return float(i-i/2*2);
}
float N(vec2 p, float v) { 
    for (float n=1.; n>=0.; n--)  
        if ((p.x-=4.)<3.) return D(p,floor(mod(v/pow(10.,n),10.))); 
    return 0.;
}

float drawminutes(vec2 uv){
    float t;
    t=floor(mod((uv.x-0.0035)/.01,12.)/2.)*10.+10.;
    uv.y+=0.0752;
    uv.y/=0.25;
    uv.x+=0.053;
    return N(vec2(mod(uv.x/.01,2.)/0.15,uv.y*80.), t );
}

float drawhours(vec2 uv){
    float t;
    uv.y-=0.1052;
    uv.y/=0.221;
    uv.x+=0.024;
    uv/=2.;
    t=(floor(mod((uv.x-0.0035)/.01,144.)/6.)*10.)/10.;
    return N(vec2(mod((uv.x)/(.01),2.)/(0.15),uv.y*80.), t );
}
#endif
#endif

//https://www.shadertoy.com/view/XlBBWt
#define PI 3.14159265359
#define TWO_PI 6.28318530718
float drawtriangle(vec2 uv){
    uv+=0.5;
    uv.y-=0.18;
    uv = uv *2.-1.;
    uv.x/=1.7;
    uv*=20.;
    int N = 3;
    float a = atan(uv.x,uv.y);
    float r = TWO_PI/float(N);
    return 1.0-smoothstep(.4,.541,cos(floor(.5+a/r)*r-a)*length(uv));
}

float moveto(){
    float timeval=iDate.w;
    //timeval=iMouse.x/iResolution.x*86400.;//test1
    //timeval=iTime*30.;//test2
    float h = floor(timeval/3600.);
    float m = floor(mod(timeval/60.,60.));
    float s = floor(mod(timeval,60.));
    float size=1./50.;//<screen res>/<num of 10min marks>
    return h*6.*size+m/60.*size*6.+s/60./10.*size;
}

void mainImage( out vec4 fragColor, vec2  fragCoord)
{
    vec2 uv = (fragCoord-.5*iResolution.xy) / iResolution.y*vec2(9./16.,1.);//vec2 uv = fragCoord/iResolution.xy-0.5;
    float lgradient=smoothstep(0.31,0.,uv.x);//gradient to black on left
    float rgradient=smoothstep(-0.31,0.,uv.x);//right
    float tri=drawtriangle(uv);//triangle
    uv.x+=moveto();
    float hline1=1.-step(1.8,mod(uv.y/.002,iResolution.y)); //horizontal line
    #ifdef AA
    float vline2=smoothstep(1.665495,1.9665495,mod(uv.x/.06,2.)); //hour vertical lines
    float aavline2=smoothstep(1.665495,1.9665495,mod((-uv.x-0.0041)/.06,2.));
    vline2=min(vline2,aavline2);
    float vline1=max(smoothstep(1.58,1.8,mod(uv.x/.01,2.)),hline1); //minutes vertical lines
    float aavline1=max(smoothstep(1.58,1.8,mod((-uv.x-0.0040)/.01,2.)),hline1);
    vline1=min(aavline1,vline1);
    #else
    float vline2=step(1.9665495,mod(uv.x/.06,2.)); //hour vertical lines
    float vline1=max(step(1.8,mod(uv.x/.01,2.)),hline1); //minutes vertical lines
    #endif
    float tmpvline2=step(1.59665495,mod((uv.x-0.0081)/.06,2.));//for numbers
    //lgradient=rgradient=1.;//test
    fragColor = vec4(min(rgradient,lgradient)*(colorMinutes*(1.-vline2)+colorHours*vline2)*vline1*(step(-0.051-0.05*vline2,uv.y))*(+1.-step(0.051+0.05*vline2,uv.y)),1. );
    fragColor += vec4(tri*colorTri,1.);
    #ifdef text
    fragColor += vec4(drawminutes(uv)*colorTextM*min(rgradient,lgradient)*(1.-tmpvline2),1.);
    fragColor+=vec4(drawhours(uv)*colorTextH*tmpvline2*min(rgradient,lgradient),1.);
    #endif
}