//stage GL_FRAGMENT_SHADER
//import <GLUP/ShaderToy.h>

// TorusJourney (shadertoy version) by @paulofalcao

//Util Start
#define PI 3.14159265
#define timeadd 25.0

vec2 ObjUnion(in vec2 obj0,in vec2 obj1){
  if (obj0.x<obj1.x)
    return obj0;
  else
    return obj1;
}

vec3 sim(vec3 p,float s){
  vec3 ret=p;
  ret=p+s/2.0;
  ret=fract(ret/s)*s-s/2.0;
  return ret;
}

vec2 rot(vec2 p,float r){
  vec2 ret;
  ret.x=p.x*cos(r)-p.y*sin(r);
  ret.y=p.x*sin(r)+p.y*cos(r);
  return ret;
}

vec2 rotsim(vec2 p,float s){
  vec2 ret=p;
  ret=rot(p,-PI/(s*2.0));
  ret=rot(p,floor(atan(ret.x,ret.y)/PI*s)*(PI/s));
  return ret;
}

vec2 revolve(vec2 p){
  p.x=length(p);
  return p;
}
//Util End

//Scene Start

//IQ signed box
float sdBox(vec3 p,vec3 b){
  vec3 d=abs(p)-b;
  return min(max(d.x,max(d.y,d.z)),0.0) +
         length(max(d,0.0));
}

vec2 obj0(in vec3 p){
  p.z=p.z-3.5;
  float c1=sdBox(p,vec3(0.25,0.25,0.25));
  float c2=sdBox(p,vec3(1.5,0.15,0.15));
  float c3=sdBox(p,vec3(0.15,1.5,0.15));
  return vec2(min(min(c1,c3),c2),0);
}

vec3 obj0_c(in vec3 p){
  return vec3(0.2,0.5,1.0);
}

vec2 obj1(vec3 p){
  p.z=p.z-6.0;
  float c1=sdBox(p,vec3(1.5,0.15,0.3));
  float c2=sdBox(p,vec3(0.15,1.5,0.3));
  return vec2(min(c1,c2),1);
}

vec3 obj1_c(in vec3 p){
  return vec3(1.0,0.7,0.6);
}

vec2 obj2(vec3 p){
  float time=iTime+timeadd;
  vec3 op=p;
  p.xz=revolve(p.xz);
  p.x-=32.0;
  op.xz=rot(op.xz,time*0.3);
  p.xy=rot(p.xy,atan(op.z,op.x)*24.0*sin(time*0.2));
  p.x-=sin(time*2.0);
  return vec2(length(max(abs(p.xy)-vec2(0.5,0.1),0.0))-0.2,2);
}

vec3 obj2_c(in vec3 p){
  return vec3(1.0,0.5,0.2);
}


vec2 inObj(in vec3 p){
  vec3 b=p;
  p.xz=rotsim(p.xz,64.0);
  p.z=p.z-32.0;
  p.yz=rotsim(p.yz,8.0);
  return ObjUnion(ObjUnion(obj0(p),obj1(p)),obj2(b));
}

//Scene End

//tetrahedron normal
vec3 objNormal(vec3 p){
  const float n_er=0.02;
  float v1=inObj(vec3(p.x+n_er,p.y-n_er,p.z-n_er)).x;
  float v2=inObj(vec3(p.x-n_er,p.y-n_er,p.z+n_er)).x;
  float v3=inObj(vec3(p.x-n_er,p.y+n_er,p.z-n_er)).x;
  float v4=inObj(vec3(p.x+n_er,p.y+n_er,p.z+n_er)).x;
  return normalize(vec3(v4+v1-v3-v2,v3+v4-v1-v2,v2+v4-v3-v1));
}

void cameraSetup(out vec3 prp, out vec3 scp, in vec2 fragCoord){
  float time=iTime+timeadd;
    
  vec2 vPos=-1.0+2.0*fragCoord.xy/iResolution.xy;
  
  //Camera animation
  vec3 vuv=vec3(sin(time*0.7),1.0,sin(time*0.9));
  float camSpeed=time*0.3;
  vec3 vrp=vec3(sin(camSpeed+0.2)*32.0,0,cos(camSpeed+0.2)*32.0);
  float camCenterDist=2.1+exp(-20.0*pow(sin(time*0.2)+1.0,20.0))*2.8;
  prp=vec3(sin(camSpeed)*32.0,0,cos(camSpeed)*32.0);
  mat3 camMov=mat3(normalize(prp),vec3(0,1.0,0),normalize(vrp-prp));
  camSpeed+=sin(time*0.3)*0.5;
  vrp=vec3(sin(camSpeed)*32.0,0,cos(camSpeed)*32.0);
  vec3 camMovP=vec3(sin(time)*camCenterDist,cos(time)*camCenterDist,0.0);
  prp=prp+camMovP*camMov;
  float vpd=1.5; 
    
  //Camera setup
  vec3 vpn=normalize(vrp-prp);
  vec3 u=normalize(cross(vuv,vpn));
  vec3 v=cross(vpn,u);
  vec3 scrCoord=prp+vpn*vpd+vPos.x*u*iResolution.x/iResolution.y+vPos.y*v;
  scp=normalize(scrCoord-prp);

}


void raymarching( out vec4 fragColor, in vec3 prp, in vec3 scp){

  //Raymarching
  const vec3 e=vec3(0.1,0,0);
  float maxd=48.0; //Max depth

  vec2 s=vec2(0.1,0.0);
  vec3 c,p,n;

  float f=1.0;
  for(int i=0;i<128;i++){
    if (abs(s.x)<.001||f>maxd) break;
    f+=s.x;
    p=prp+scp*f;
    s=inObj(p);
  }
  
  if (f<maxd){
    if (s.y==0.0)
      c=obj0_c(p);
    else if (s.y==1.0)
      c=obj1_c(p);
    else
      c=obj2_c(p);
 
    n=objNormal(p);
    float b=abs(dot(n,normalize(prp-p)));
    vec3 objColor=b*c+pow(b,8.0);
  
    //reflect
    float f0=f;
    if (s.y==2.0){
      prp=p-0.02*scp;
      scp=reflect(scp,n);
      f=0.0;
      s=vec2(0.1,0.0);
      maxd=16.0;
      for(int i=0;i<64;i++){
        if (abs(s.x)<.01||f>maxd) break;
        f+=s.x;
        p=prp+scp*f;
        s=inObj(p);
      }
      if (f<maxd){
        if (s.y==0.0)
          c=obj0_c(p);
        else if (s.y==1.0)
          c=obj1_c(p);
        else
          c=obj2_c(p);
      
        n=objNormal(p);
        b=abs(dot(n,normalize(prp-p)));
        vec3 objColor2=b*c+pow(b,8.0);
        fragColor=vec4((objColor*0.8+objColor2*0.2)*(1.0-f0*.03),1.0);
      } else fragColor=vec4(objColor*0.8*(1.0-f0*.03),1.0);

    } else fragColor=vec4(objColor*(1.0-f0*.03),1.0);
    
  } else fragColor=vec4(0,0,0,1);

}


void mainImage( out vec4 fragColor, in vec2 fragCoord ){
  vec3 prp;
  vec3 scp;
  cameraSetup(prp,scp,fragCoord);
  raymarching(fragColor,prp,scp);
}
