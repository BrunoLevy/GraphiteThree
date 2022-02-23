//stage GL_FRAGMENT_SHADER
//import <GLUP/ShaderToy.h>

bool keyToggle(int ascii) {
	return texture(iChannel3,vec2((.5+float(ascii))/256.,.75)).x > 0.;
}

void mainImage( out vec4 fragColor, vec2 fragCoord )
{
    float t = iTime;
    vec2 uv0 = fragCoord / iResolution.y;
    vec2 m = iMouse.xy/iResolution.xy;
    if (iMouse.z<=0.) { // autodemo 
		    uv0 += vec2( 1.8*cos(.15*t)+.5*sin(.4*t) , sin(.22*t)-.5*cos(.3*t) );
			m = .5+.5*vec2(cos(t),sin(.6*t)); m.y*=m.x;
    }

    vec2 uv = uv0*8., iuv=floor(uv);
    float d = mod(iuv.x+iuv.y,2.), s = mod(iuv.y,2.);
    uv = fract (uv); if (d==1.) uv.x = 1.-uv.x; // checkered tile coordinates
    uv = uv + vec2(-uv.y,uv.x); // rotate 45deg
 
    float q = sign(s-.5)*sign(d-.5),
      size0 = m.x+m.y*cos(.5*3.1415927*uv.y) *q,
          l = abs(uv.x)-size0,
          v = smoothstep(0.,.1,abs(l)),
         v0 = step(0.,l);
    
    float size = m.x+m.y*cos(.5*3.1415927*uv.x), 
           ofs = (1.-size)*q; // corner distance
             l = (uv.y-1.)-ofs;
    float   v1 = step(0.,l),
            d0 =  mod(s+v1,2.),
            d1 =  mod(s+d+v1,2.); // corner area
    v0 = d1<1. ? v0 : 0.; // background
    v = (d1<1. ? v : 1.)*smoothstep(0.,.1,abs(l)); // contour

    float col = v0 *(cos(8.*31.4*uv0.x)*cos(8.*31.4*uv0.y))
          + (1.-v0)*( d1==1. ? cos(2.*31.4*( q>0. ? 2.-uv.y : uv.y )*m.x/size)  
                             : cos(2.*31.4*(uv.x*m.x/size0)) );
    if (keyToggle(32)) v/= 1.+.5*d;
	fragColor = vec4(col*v);
}


