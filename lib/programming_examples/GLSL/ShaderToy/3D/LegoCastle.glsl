//stage GL_FRAGMENT_SHADER
//import <GLUP/ShaderToy.h>

//these options seem to be too much for Windows: turn on for moar fun.
//#define MOAR_CSG
//#define BUILD_UP_AND_DOWN

//Please turn on!! looks soo much better!
//#define REFLECTIONS

//turn these off to be cheaper...
#define SHADOWS
#define COLORS
#define AO

//OMG!! text due to mmalex!!
#define LOGO

//select one of these
//#define NICE_NORMALS
#define MEDIUM_NORMALS
//#define NASTY_NORMALS

#define pi 3.1415927


float Box( vec3 p, vec3 b )
{
  vec3 d = abs(p) - b;
  return min(max(d.x,max(d.y,d.z)),0.0) + length(max(d,0.0));
}

vec2 BoxY( vec3 p, vec3 b ) //euclidean & directed y distance only
{
	vec3 d = abs(p) - b;
	float mxz = max(d.x,d.z);
	float c = min(max(d.y,mxz),0.) + length(max(d,0.0));
		
	return vec2(c, mxz < 0. ? d.y : 1e10 );
}

vec4 Box4( vec4 p_x, vec4 p_y, vec4 p_z, vec4 b_x, vec4 b_y, vec4 b_z )
{
	vec4 d_x = abs(p_x) - b_x;
	vec4 d_y = abs(p_y) - b_y;
	vec4 d_z = abs(p_z) - b_z;
	
	vec4 mdx = max(d_x,0.);
	vec4 mdy = max(d_y,0.);
	vec4 mdz = max(d_z,0.);
	
	vec4 len = sqrt( mdx*mdx+mdy*mdy+mdz*mdz );
		
	return min(max(d_x,max(d_y,d_z)),vec4(0)) + len;
}

vec4 Box4Y( vec4 p_x, vec4 p_y, vec4 p_z, vec4 b_x, vec4 b_y, vec4 b_z, out vec4 Y )
{
	vec4 d_x = abs(p_x) - b_x;
	vec4 d_y = abs(p_y) - b_y;
	vec4 d_z = abs(p_z) - b_z;
	
	vec4 mdx = max(d_x,0.);
	vec4 mdy = max(d_y,0.);
	vec4 mdz = max(d_z,0.);
	
	vec4 len = sqrt( mdx*mdx+mdy*mdy+mdz*mdz );
	
	vec4 max_dxz = max(d_x,d_z);

#if 0	
	Y = mix(vec4(1e10), d_y, vec4(lessThan(max_dxz, vec4(0.))) );	//select y dist if inside ow. far out
#else	
	Y.x = max_dxz.x < 0. ? d_y.x : 1e10;
	Y.y = max_dxz.y < 0. ? d_y.y : 1e10;
	Y.z = max_dxz.z < 0. ? d_y.z : 1e10;
	Y.w = max_dxz.w < 0. ? d_y.w : 1e10;
#endif
	
	return min( max(d_y,max_dxz), vec4(0)) + len;
}
	
float CylinderXZ( vec3 p, vec3 c ) {
	return length(p.xz-c.xy)-c.z;
}

vec3 RotX(vec3 p, float t) {
	float c = cos(t); float s = sin(t);
	return vec3(p.x,
				p.y*c+p.z*s,
				-p.y*s+p.z*c);
}

vec3 RotY(vec3 p, float t) {
	float c = cos(t); float s = sin(t);
	return vec3(p.x*c+p.z*s,
				p.y,
				-p.x*s+p.z*c);
}

vec3 RotZ(vec3 p, float t) {
	float c = cos(t); float s = sin(t);
	return vec3(p.x*c+p.y*s,
				-p.x*s+p.y*c,
				p.z);
}

float Rep(float x, float t) { return mod(x,t)-0.5*t; }
vec2 Rep(vec2 x, vec2 t) { return mod(x,t)-0.5*t; }
vec3 Rep(vec3 x, vec3 t) { return mod(x,t)-0.5*t; }
float U(float a,float b) { return min(a,b); }
float U4(vec4 da) {
	da.xy = min(da.xy,da.zw);
	return U(da.x,da.y);	
}
float U8(vec4 da, vec4 db) {
	da = min(da,db);
	return U4(da);
}

float I(float a,float b) { return max(a,b); }
vec2 IY(vec2 a, vec2 b) { return max(a,b); }

float S(float a,float b) { return max(a,-b); }

vec2 UY(vec2 a,vec2 b) { return min(a,b); }
vec2 U4Y(vec4 da, vec4 daY) {
	da.xy = min(da.xy,da.zw);
	daY.xy = min(daY.xy,daY.zw);
	return UY(vec2(da.x,da.y),vec2(daY.x,daY.y));	
}

vec2 SY(vec2 a,vec2 b) { return max(a,-b); }

float ClipX(float d, vec3 p, float x) { return I(d,p.x-x); }
float ClipXX(float d, vec3 p, float x) { return I(d,abs(p.x)-x); }

float ClipY(float d, vec3 p, float x) { return I(d,p.y-x); }

vec2 ClipY(vec2 d, vec3 p, float x) { return vec2( I(d.x,p.y-x), I(d.y,p.y-x)) ; }

float ClipNY(float d, vec3 p, float x) { return I(d,-p.y-x); }

float ClipYY(float d, vec3 p, float x) { return I(d,abs(p.y)-x); }

float ClipZ(float d, vec3 p, float x) { return I(d,p.z-x); }
float ClipZZ(float d, vec3 p, float x) { return I(d,abs(p.z)-x); }

float floor_height = -1.0;
float floor_mat_bodge = 0.05;

vec3 ss_grad(vec3 X)
{
	return cross(dFdx(X),dFdy(X));
}

//http://dept-info.labri.fr/~schlick/DOC/gem2.ps.gz
float bias(float x, float b) {
	return  x/((1./b-2.)*(1.-x)+1.);
}

float gain(float x, float g) {
	float t = (1./g-2.)*(1.-(2.*x));	
	return x<0.5 ? (x/(t+1.)) : (t-x)/(t-1.);
}

vec2 gain(vec2 x, float g) {
	vec2 t = (1./g-2.)*(1.-(2.*x));	
	return vec2(	x.x < 0.5 ? (x.x/(t.x+1.)) : (t.x-x.x)/(t.x-1.),
					x.y < 0.5 ? (x.y/(t.y+1.)) : (t.y-x.y)/(t.y-1.) );
}

//http://www.robertcailliau.eu/Lego/Dimensions/zMeasurements-en.xhtml
/*
There are five basic dimensions:
The horizontal pitch, or distance between knobs:  8mm.
The vertical pitch, or height of a classic brick:  9.6mm.
The horizontal tolerance:  0.1mm
This is half the gap between bricks in the horizontal plane.  The horizontal tolerance prevents friction between bricks during building.
The knob diameter:  4.8mm
This is also the diameter of axles and holes.  Actually a knob must be slightly larger and an axle slightly smaller (4.85 and 4.75 respectively, otherwise axles would not turn in bearing holes and knobs would not stick in them) but we will ignore this difference here.
The height of a knob:  1.8mm
*/
#define brick_h 9.6
#define brick_w	(8./brick_h)
#define knob_h	(1.8/brick_h)
#define knob_r	(2.4/brick_h)

//brick being +/-1 high is useful for floor fn.

vec2 BrickSDFY(vec3 p)
{
#if 0	
	//bricks	
	vec3 r = p;
	r.x = Rep(r.x-brick_w,6.0*brick_w);
	vec2 d = BoxY(r,vec3(brick_w,1.,brick_w*3.));
	
	d = UY(d,BoxY(p,vec3(brick_w,5.,brick_w*3.)));
	d = SY(d,BoxY(p-vec3(0.,2.,0.),vec3(brick_w*2.,1.,brick_w*1.)));
#endif
	
#if 1	
	float h = 13.;
		
	p.x += brick_w;
	p.x = abs(p.x);
	
				//bastion		column	main	tower
	vec4 px = p.x - vec4(15.0, 	8.,		20., 	22.)*brick_w;
	vec4 py = vec4(p.y);// + vec4(0., 	0.,		0., 	0.);
	vec4 pz = p.z + vec4(0.0, 	1.,		28., 	21.)*brick_w;
	vec4 bx = 		vec4(5.,	2.,		8.,		6.)*brick_w;
	vec4 by = 		vec4(h,		19.,	h,		25.);
	vec4 bz = 		vec4(5.,	2.,		27.,	6.)*brick_w;	
		
	vec4 daY;
	vec4 da = Box4Y(px,py,pz,bx,by,bz,daY);
	
		px = p.x - vec4(15.0, 	20.,	17., 	22.)*brick_w;
	//	py = vec3(p.y);// + vec4(0., 	0.,		0., 	0.);
		pz = p.z + vec4(1.0, 	21.,	26., 	21.)*brick_w;
		bx = 		vec4(3.,	6.,		9.,		4.)*brick_w;
		by = 		vec4(h+2.,	13.,	h+2.,	11.);
		bz = 		vec4(4.,	2.,		23.,	7.)*brick_w;	
		
	vec4 dbY;
	vec4 db = Box4Y(px,py,pz,bx,by,bz,dbY);

	vec2 d = U4Y(da, daY);
	
	d = SY(d,vec2(db.x,dbY.x));
	d = SY(d,vec2(db.z,dbY.z));
	d = SY(d,vec2(db.w,dbY.w));

					  
	//bast+main - bast-main + tower - tower ? 
	d = SY( UY(vec2(da.x,daY.x),vec2(da.z,daY.z)), UY(vec2(db.x,dbY.x),vec2(db.z,dbY.z)) );
	d = UY(d, vec2(da.y,daY.y) );
			
	vec3 q;
	q.x = Rep(p.x+0.*brick_w,10.*brick_w);
	q.y = Rep(p.y-1.,8.);
	q.z = Rep(p.z-1.*brick_w,6.*brick_w);
	
		//front windows&crenels		side crenels 
	px = vec4(q.x,p.xxx) -  	vec4(0.0, 	23.,	22., 	22.)*brick_w;
	py = vec4(q.yy,p.yy) -  	vec4(0., 	0.,		18., 	18.);
	pz = vec4(p.z,q.z,p.zz) + 	vec4(1.0, 	2.,		21., 	21.)*brick_w;
	bx = 						vec4(1.,	7.,		7.,		4.)*brick_w;
	by = 						vec4(2.,	2.,		3.,		5.);
	bz = 						vec4(8.,	1.,		2.,		7.)*brick_w;	
	
	vec4 holesY;
	vec4 holes = Box4Y( px, py, pz, bx, by, bz, holesY );
		
	d = SY(d,vec2(holes.x,holesY.x));
	
	d = SY(d,vec2(holes.y,holesY.y));

	//add towers now to not cut holes in them!
	d = UY(d, SY(vec2(da.w,daY.w),vec2(db.w,dbY.w)));	

	d = SY( d, vec2(db.y,dbY.y) );
#ifdef MOAR_CSG	
	d = SY( d, vec2(holes.w,holesY.w) );
	d = SY( d, vec2(holes.z,holesY.z) );
#endif	
#endif	
	
	return d;
}

vec2 RedBrickSDFY(vec3 p)
{
	vec2 d = BoxY(p+vec3(1.*brick_w,-1./3.,-11.*brick_w),vec3(6.*brick_w,2./3.,14.*brick_w));
	p.x = abs(p.x+1.*brick_w);
	d = UY(d, BoxY(p-vec3(5.*brick_w,1.,0.),vec3(brick_w*1.,2.,brick_w*3.) ));
	return d;
}

float AddStuds(vec2 D, vec3 p)
{
	float d = D.x;
	
	//studs
	vec3 q = p;
	q.xz = Rep(q.xz-brick_w,vec2(2.*brick_w));
	float c = CylinderXZ(q,vec3(0.,0.,knob_r*2.));

	if (D.y > -0.0) //outside in Y dir
	{		
		c = ClipY(c,p,floor((p.y-D.y))+knob_h*2.); //clip knob off at knob height above surface height
		d = U(d,c);
	}
	
	return d;
}

float sdf(vec3 p)
{
	//floor!
	float f = p.y-floor_height;
	vec2 D = vec2(f);

	//bricks
	//vec2 D = BrickSDFY(p) );
 	D = UY(D,BrickSDFY(p) ); 
	D = UY(D,RedBrickSDFY(p));
#ifdef BUILD_UP_AND_DOWN	
	//build up and down
	float t = mod(iTime,20.);
	t = t > 10. ? 20.-t : t;
	t *= 2.;
	t = floor(t);
	float bh = 1.+2.*floor(t);
	D.x = ClipY(D.x,p,bh-0.08);
	D.y = ClipY(D.y,p,bh);
#endif	
	return AddStuds(D,p);
}

float bb( vec2 p, vec2 b )
{
  return length(max(abs(p)-b,0.0));
}

//lego logo by mmalex!!
//https://www.shadertoy.com/view/MsX3W2
float lego(vec2 uv) { // x is -2 to 2
	uv.x+=uv.y*-0.1; // italic
	float oldx=uv.x;
	uv.x=fract(uv.x)-0.5; // letter repeat
	if (abs(oldx)>2.0) return 0.0; // clip!
	float l;
	if( oldx<0.0) {
		// l and e
		float e0=bb(uv-vec2(-0.15,0.0),vec2(0.2,0.0)); // cross of e 
		if (oldx>-1.0) uv.y=-abs(uv.y); else e0=1.0;
		float l0=bb((uv)-vec2(0.0,-0.75),vec2(0.35,0.0)); // bottom of l
		float l1=bb((uv)-vec2(-0.35,0.0),vec2(0.0,0.75)); // left of l                                              
		l0=min(l0,e0);
		l=min(l0,l1);                                        
	} else {
		l=abs(bb(uv,vec2(0.2,0.6))-0.15); // round o
		if (oldx<1.0) {
			// g - ugh nasty
			if (uv.x>0.0 && uv.y>0.0 && uv.y<0.5)                                    
				l=bb((uv)-vec2(0.35,0.6),vec2(0.0,0.1));
			float e0=bb(uv-vec2(0.2,0.0),vec2(0.15,0.0));
			l=min(l,e0);
		}
	}              
	return smoothstep(0.2,0.05,l);   
}

float nsdf(vec3 p)
{
//	return sdf(p); 
	
	//for normals, add small bump displacements
	float d = sdf(p);
	
	float stripe = mod(p.y,2.);
	d -= smoothstep(0.,0.04,abs(stripe-1.)*0.5)*0.01;
		
#ifdef LOGO	
 	d -= lego(10.*(fract(p.xz*vec2(0.5/brick_w)+0.5)-0.5))*.05;
#endif
	
	return d;
}

vec3 ss_nor(vec3 X)
{
	return normalize(cross(dFdx(X),dFdy(X)));
}
	
vec3 nor(vec3 X)
{
#ifdef NICE_NORMALS	
	//expensive... kills windows shader compilers
	vec2 e = vec2(0.01,0.0); //fatter filter looks like bevelled edges on hard CSG shapes
	vec3 N = vec3(nsdf(X-e.xyy),nsdf(X-e.yxy),nsdf(X-e.yyx)) -
			 vec3(nsdf(X+e.xyy),nsdf(X+e.yxy),nsdf(X+e.yyx));
	return -normalize(N);
#endif	
	
#ifdef MEDIUM_NORMALS
	//less expensive	
	float e = 0.01;
	float d = nsdf(X);
	vec3 D;
	D.x = nsdf(vec3(X.x+e,X.y,X.z));
	D.y = nsdf(vec3(X.x,X.y+e,X.z));
	D.z = nsdf(vec3(X.x,X.y,X.z+e));
	return normalize(D-vec3(d));	
#endif	

#ifdef NASTY_NORMALS	
	return -ss_nor(X); //cheap and nasty
#endif	
}


//thanks again iq http://www.iquilezles.org/www/articles/rmshadows/rmshadows.htm
float shadow( in vec3 X, in vec3 n, in vec3 L )
{
	float mint = 0.001;
	float maxt = 20.0;
	
	X += n*.01;
	
	float h=0.4;
	float sharpness = 25.;
	float soft=1.0;
	float t = mint;
	for (int i=0; i<32; i++)
    {
        float d = sdf(X + L*t);
        if( d<-0.1 )
            return h; //t*h;
		
		soft = min( soft, (sharpness*d)*(1./t));
		
		if (t > maxt) break;
        t += d * 0.9;
    }
    return clamp(soft,h,1.0);
}

float Ao(vec3 p, vec3 n) {
	float vis = 0.0;
	p += n*0.035;
	float t = 0.;//0.035;
	for (int i=0; i<9; i++)
	{
		float d = sdf(p);

	//	h^2 = d^2+L^2
		float h = inversesqrt( d*d + t*t );
		vis += abs(d) * h;	//cos angle of clear area?

		p += n * d * 0.9;
		t += d * 0.9;
	}
	vis *= 0.1;
	vis = bias(vis,0.125)*4.;
	return vis; 
}

void MakeViewRay(out vec3 eye, out vec3 ray, in vec2 fragCoord)
{
	vec2 ooR = 1./iResolution.xy;
    vec2 q = fragCoord.xy * ooR;
    vec2 p =  2.*q -1.;
    p.x *= iResolution.x * ooR.y;
	
    vec3 lookAt = vec3(0.,0.,0.);
	float t = iTime*0.1;
//	t=0.;
	t = mod(t,3.);
	if (t < 1.)
	{
		eye = vec3(10.,10.,10)*4.;
		eye = RotY(eye,smoothstep(0.,1.,t)*-0.5*pi);
	
	}
	else if (t < 2.)
	{
		t -= 1.;
		float s = smoothstep(0.,1.,t);
		eye = vec3(50.+s*10.,90.*(1.-s*0.5),50.);
		lookAt = vec3(5.,0.,-10.);		
	}
	else
	{
		t -= 2.;
		float s = smoothstep(0.,1.,t);
		eye = vec3(-10.*(s*2.-1.),60. - s*30. ,-25.);
		lookAt = vec3(-20.*s,s,s*10.);
	}
	
    // camera frame
    vec3 fo = normalize(lookAt-eye);
    vec3 ri = normalize(vec3(fo.z, 0., -fo.x ));
    vec3 up = normalize(cross(fo,ri));
     
    float fov = .25;
	
    ray = normalize(fo + fov*p.x*ri + fov*p.y*up);
}

float Trace(vec3 viewP, vec3 viewD, float max_t)
{
	float t = 0.;
	float d;

	float floor_intersect_t = (-viewP.y + floor_height) / (viewD.y);
	
	vec3 neighViewD = dFdy(viewD)+viewD;	
				
	for (int i=0; i<48; i++)
	{
		vec3 X = viewP + viewD * t;
		d = sdf(X);
		

		vec3 nX = viewP + neighViewD*t;
		float r = length(X-nX);				
		if (abs(d) < r*(0.25)) break; //less sparkly crap on silhouette edges?
				
//		if (abs(d) < 0.00001) break; //near enough surface for normals to look OK.
	
#if 1	
		if (t>max_t) //too far - won't converge: just go to ground plane.
		{
			t = floor_intersect_t;
			break;
		}
#endif		
		t += d*0.9; //bounding volumes make the distance a bit wrong so slow down
	}

	return t;	
}

float TraceR(vec3 viewP, vec3 viewD, float max_t)
{
	float t = 0.;
	float d;

	float floor_intersect_t = (-viewP.y + floor_height) / (viewD.y);
	
	vec3 neighViewD = dFdy(viewD)+viewD;	
				
	for (int i=0; i<32; i++)
	{
		vec3 X = viewP + viewD * t;
		d = sdf(X);
		

		vec3 nX = viewP + neighViewD*t;
		float r = length(X-nX);				
		if (abs(d) < r*(0.25)) break; //less sparkly crap on silhouette edges?
				
//		if (abs(d) < 0.001) break; //near enough surface for normals to look OK.
	
#if 1	
		if (t>max_t) //too far - won't converge: just go to ground plane.
		{
			t = floor_intersect_t;
			//t = viewD.y < 0. ? floor_intersect_t : max_t;
			break;
		}
#endif		
		t += d*0.9; //bounding volumes make the distance a bit wrong so slow down
	}

	return t;	
}

vec3 Light(vec3 X, vec3 n, vec3 V)
{
	vec3 lightDir = normalize(vec3(-3,8,-5));

	vec3 c = vec3( max(dot(lightDir, n), 0.) );

	c = pow(vec3(1.0, 1.0, 1.0),vec3(2.2));
	
		
#ifdef SHADOWS	
	float sha = shadow(X,n,lightDir);
	c *= sha;
#endif

	c += 0.3;
	
#ifdef AO	
	float ao = Ao(X, normalize(n) );
//	return vec3(ao);
	c *= ao;
#endif	
	
#ifdef COLORS	
	vec2 aX = abs(X.xz+vec2(1.*brick_w,23.*brick_w));
	float mX = max(aX.x,aX.y);
	
	const float mat_test = 0.08;
	if (AddStuds(BrickSDFY(X),X) < mat_test)
		c *= pow(vec3(245./255.,205./255.,47./255.),vec3(2.2));  //yellow
	else if (AddStuds(RedBrickSDFY(X),X) < mat_test)
		c *= pow(vec3(196./255.,40./255.,27./255.),vec3(2.2)); //red
	else if (mX < 30.*brick_w)
		c *= pow(vec3(161./255.,165./255.,162./255.),vec3(2.2)); //gray
	else if (mX < 44.*brick_w)		
		c *= pow(vec3(13./255.,105./255.,171./255.),vec3(2.2));  //blue
	else
		c *= pow(vec3(40./255.,127./255.,70./255.),vec3(2.2));  //green
#endif	
		
	//vec3 h = normalize(V+lightDir);
	//c += ao * sha *pow(max(dot(n,h),0.),80.0); //*2.5;

	return c;
}

void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
	vec3 viewP, viewD;
	MakeViewRay(viewP, viewD, fragCoord);
	
	float t = Trace(viewP,viewD, 200.);
			
	vec3 X = viewP + viewD * t;
	vec3 n = nor(X);
	
	vec3 c = Light(X,n, viewD)*.7;
	
#ifdef REFLECTIONS	
	vec3 R = reflect(viewD,n);
	R = normalize(R);
	t = TraceR(X,R, 200.);
			
	X = X + R * t;
	n = nor(X);
//	n = ss_nor(X);
	
	c += Light(X,n, R)*clamp(1.-t*0.0125,0.,1.)*.1;
#endif
	
//	c += Light(X,n);
	
//	c = vec3(glo);
				
	float gamma = 2.2;
	c = pow(c, vec3(1./gamma));
		
#if 0	
//	float debug_height = sin(iTime)*10.0;
	float debug_height = sin(iTime);
	float debug_intersect_t = (-viewP.y + debug_height) / (viewD.y);	
	if (debug_intersect_t < t)
	{
		vec3 X = viewP + viewD * debug_intersect_t;
		float d = sdf(X);
		c = vec3(max(d,0.),max(-d,0.),0.);
	}
#endif
	
//	c = n*0.5+0.5;
//	c = vec3(sha);
//	c = vec3(ao);
	fragColor = vec4(c,1.0);
}

