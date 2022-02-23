//stage GL_FRAGMENT_SHADER
//import <GLUP/ShaderToy.h>

// Escher's prentententoonstelling. Reinder Nijhoff 2013
// @reindernijhoff
//
// https://www.shadertoy.com/view/Mdf3zM
//
// Study of the transformation of Escher in 'the prentententoonstelling'
//
// http://www.ams.org/notices/200304/fea-escher.pdf
// h(w) = wÎ± = w^((2Ï€i+log scale)/(2Ï€i))
//
// distancefield functions by inigo quilez.
//

//#define SHADOW
#define WOBBLE

float t;

float st = 0., zt = 0.;

float deformationScale, zoom;

vec2 escherDeformation( in vec2 uv ) {
	
// http://www.ams.org/notices/200304/fea-escher.pdf
// h(w) = wÎ± = w^((2Ï€i+log scale)/(2Ï€i))
	
	float lnr = log(length(uv));
	float th = atan( uv.y, uv.x )+(0.4/256.)*deformationScale;
	float sn = -log(deformationScale)*(1./(2.*3.1415926));
	float l = exp( lnr - th*sn ); 
	
	vec2 ret = vec2( l );
	
	ret.x *= cos( sn*lnr+th );
	ret.y *= sin( sn*lnr+th );
		
	return ret;
}

#define drostescale 256.

vec2 drosteTransformation( in vec2 uv ) {
	for( int i=0; i<2; i++ ) {
		if(any(greaterThan(abs(uv),vec2(1.)))) {
			uv *= (1./drostescale);
		}		
		if(all(lessThan(abs(uv),vec2(1./drostescale)))) {
			uv *= drostescale;
		}
	}
	return uv;
}

float hash( float n )
{
    return fract(sin(n)*43758.5453);
}

float sdPlane( vec3 p ) {
	return p.y+14.+0.05*cos(p.x+iTime*2.);
}

float sdBox( vec3 p, vec3 b ) {
  vec3 d = abs(p) - b;
  return min(max(d.x,max(d.y,d.z)),0.0) +
         length(max(d,0.0));
}

float udBox( vec3 p, vec3 b) {
  return length(max(abs(p)-b,0.0));
}
float sdTriPrism( vec3 p, vec2 h ) {
    vec3 q = abs(p);
    return max(q.x-h.y,max(q.z*0.4+p.y*0.5,-p.y)-h.x*0.5);
}

float sdCylinderXY( vec3 p, vec2 h ) {
  return max( length(p.xy)-h.x, abs(p.z)-h.y );
}
float sdCylinderYZ( vec3 p, vec2 h ) {
  return max( length(p.yz)-h.x, abs(p.x)-h.y );
}
float sdCylinderXZ( vec3 p, vec2 h ) {
  return max( length(p.xz)-h.x, abs(p.y)-h.y );
}


//----------------------------------------------------------------------

float opS( float d1, float d2 ) {
    return max(-d2,d1);
}
float opU( float d1, float d2 ) {
    return min(d2,d1);
}

vec2 opU( vec2 d1, vec2 d2 ) {
	return (d1.x<d2.x) ? d1 : d2;
}

float opI( float d1, float d2 ) {
    return max(d1,d2);
}

//----------------------------------------------------------------------


float objPrentenTentoonstelling( in vec3 pos ) {
	vec3 tpos;// = pos;
	tpos.x = min( abs(pos.x), abs(pos.z) );
	tpos.y = pos.y;
	tpos.z = max( abs(pos.x), abs(pos.z) );
	
	float res = opU(opU(opU(opU(opU(
			opS(opS(opS( // main building
				opS(
					udBox( tpos, vec3( 5.5, 24.0, 5.5 ) ),
					sdBox( vec3(tpos.x, tpos.y-24.0, tpos.z), vec3( 5.25, 0.5, 5.25) ) 
				),
				sdBox( vec3( mod(tpos.x+1.75, 3.5)-1.75, tpos.y-21.5, tpos.z-5.), vec3( 1.,1.,4.) )
			),
				sdBox( vec3( mod(tpos.x+1.75, 3.5)-1.75, tpos.y-15.5, tpos.z-5.), vec3( 1.,2.,4.) )
			),
				sdCylinderXY( vec3( mod(tpos.x+1.75, 3.5)-1.75, tpos.y-17.5, tpos.z-5.), vec2( 1.,4.) )
			),
			opI( // main building windows
				udBox( tpos, vec3( 5.5, 23., 5.5 ) ),
				opU(
					udBox(  vec3( mod(tpos.x+1.75, 3.5)-1.75, tpos.y, tpos.z-5.2), vec3( 0.05, 24., 0.05 ) ),
					udBox(  vec3( tpos.x, mod(tpos.y+0.425, 1.75)-0.875, tpos.z-5.2), vec3( 10.0, 0.05, 0.05 ) )
				)
			)
		),
		opS( // gallery
			opU(opU(opU(		
				opS(opS( 
						udBox( tpos, vec3( 8.375, 8.75, 8.375 ) ),
						sdCylinderXY( vec3( mod(tpos.x, 2.75)-1.375, tpos.y-6.5, tpos.z-8.75), vec2( 1.25,2.75) )
					),
					sdBox( vec3(  mod(tpos.x, 2.75)-1.375, tpos.y-4.5, tpos.z-8.75), vec3( 1.25,2.0,2.75) )			
				),
				udBox(  vec3( mod(tpos.x-8.375/18., 8.375/9.)-8.375/18., tpos.y, tpos.z-8.3), vec3( 0.025, 8.5, 0.025 ) )
			),
				udBox(  vec3( tpos.x, tpos.y-4.3, tpos.z-8.3), vec3( 8.5, 0.025, 0.025 ) ) 
			),
				udBox(  vec3( tpos.x, tpos.y-6.3, tpos.z-8.3), vec3( 8.5, 0.025, 0.025 ) ) 
			),
			opU(opU(opU(
				sdCylinderYZ( vec3( pos.x-8.75, pos.y-6.5, mod(pos.z, 13.75)-6.875), vec2( 1.25,20.) ),
				sdBox( vec3(  pos.x-8.75, pos.y-2.5, mod(pos.z,  13.75)-6.875), vec3( 20.,4.0,1.25) )			
			),
				sdCylinderXY( vec3( mod(pos.x,13.75)-6.875, pos.y-6.5, pos.z-8.75), vec2( 1.25,20.) )
			),
				sdBox( vec3(  mod(pos.x, 13.75)-6.875, pos.y-2.5, pos.z-8.75), vec3( 1.25,4.0,20.) )	
			)
		) ),
			sdTriPrism( vec3(tpos.x, tpos.y-9.3, tpos.z-5.2), vec2(2.0, 10. ) ) // roof
		),
			sdTriPrism( vec3(tpos.x, tpos.y-2.8, tpos.z-5.2), vec2(0.75, 8. ) )
		),
		udBox( tpos, vec3( 6.5, 2.5, 6.5 ) )
	);
	
	return res;
}

float objB1( in vec3 pos ) {
	float res =
		opU(opS(			
			opS(
				udBox( pos, vec3( 20., 30.0, 10. ) ),				
				sdBox( pos+vec3(0., -30., 0.), vec3( 19.75, 1., 9.75 ) )
			),
			sdBox( vec3( mod(pos.x+1.75, 3.5)-1.75, mod(pos.y+3.5, 7.)-2., pos.z-10.), vec3( 1.,1.,4.) )
		),
			opI( // main building windows
				udBox( pos, vec3( 18., 30.0, 10. ) ),
				opU(
					udBox(  vec3( mod(pos.x+1.75, 3.5)-1.75, pos.y, pos.z-9.8), vec3( 0.05, 30., 0.05 ) ),
					udBox(  vec3( pos.x, mod(pos.y+0.425, 1.75)-0.875, pos.z-9.8), vec3( 50.0, 0.05, 0.05 ) )
				)
			)
		);
	return res;	
}

float objB2( in vec3 pos ) {
	vec3 tpos;// = pos;
	tpos.x = min( abs(pos.x), abs(pos.z) );
	tpos.y = pos.y;
	tpos.z = max( abs(pos.x), abs(pos.z) );
	
	float res = opU(
			opS(opS( // main building
				opS(
					udBox( tpos, vec3( 8.75, 31.0, 8.75 ) ),
					sdBox( vec3(tpos.x, tpos.y-31.0, tpos.z), vec3( 8.5, 1.0, 8.5) ) 
				
			),
				sdBox( vec3( mod(tpos.x+1.75, 3.5)-1.75, mod(tpos.y+4.5, 9.)-2.5, tpos.z-5.), vec3( 1.,2.,4.) )
			),
				sdCylinderXY( vec3( mod(tpos.x+1.75, 3.5)-1.75, mod(tpos.y+4.5, 9.)-4.5, tpos.z-5.), vec2( 1.,4.) )
			),
			opI( // main building windows
				udBox( tpos, vec3( 8.75, 31.0, 8.75 ) ),
				opU(
					udBox(  vec3( mod(tpos.x+1.75, 3.5)-1.75, tpos.y, tpos.z-8.45), vec3( 0.05, 31., 0.05 ) ),
					udBox(  vec3( tpos.x, mod(tpos.y+0.425, 1.75)-0.875, tpos.z-8.45), vec3( 10.0, 0.05, 0.05 ) )
				)
			)
		);
	return res;	
}

vec2 map( in vec3 pos ) {
    vec2 res = opU( vec2( sdPlane( pos), 3.0 ),
	                vec2( udBox( pos+vec3(0.0, 9.0, 85.0), vec3( 200., 10.0, 100. ) ), 1. ) );

	res = opU( res, vec2( udBox( pos+vec3(0.0, 20.0, 75.0), vec3( 200., 10.0, 100. ) ), 1. ) );
 	res = opU( res, vec2( udBox( pos+vec3(0.0, 6.5, -15.0), vec3( 200., 10.0, 0.25 ) ), 1. ) );

	res = opU( res, vec2( udBox( pos+vec3( 220.0, 14.0, 0.0), vec3( 100., 10.0, 200. ) ), 1. ) );
		
	res = opU( res, vec2( udBox( (pos+vec3(3.20, -4.95, -5.55)), vec3( 0.55, 0.9, 0.01 ) ), 2. ) );
	res = opU( res, vec2( sdCylinderXZ( vec3(mod(pos.x+8., 16.)-8., pos.y+10., pos.z-24.), vec2( 0.4, 1.5)), 1.) );

	if( pos.z > 20. ) {
		return res;
	}
	
	res = opU( res, vec2( objPrentenTentoonstelling( vec3(mod(pos.x+40.,80.)-40., pos.y, mod(pos.z+40.,80.)-40.) ), 1. ) );
	
	pos += vec3(3.25, -4.60, -5.55);
	res = opU( res, vec2( opI(
		udBox( vec3(mod(pos.x+0.8, 1.6)-0.8, pos.y, pos.z), vec3( 0.7, 0.9, 0.1 ) ),
		udBox( pos-vec3(3.25, -4.60, -5.55), vec3( 5.5, 5.5, 8.5 ) )
		), 4. ) );
	pos -= vec3(3.25, -4.60, -5.55);
	
	pos += vec3( 15.5, 8., 10.);
	res = opU( res, vec2( objB1( vec3(mod(pos.x+27.,54.)-27., pos.y, mod(pos.z+50.,100.)-50.) ), 1. ) );
	pos += vec3( 20.5, -8., 5.);
	res = opU( res, vec2( objB2( vec3(mod(pos.x+23.,46.)-23., pos.y, mod(pos.z+35.,70.)-35.) ), 1. ) );
	pos += vec3( 20., -10., 10.);
	res = opU( res, vec2( objB1( vec3(mod(pos.x+77.,144.)-77., pos.y, mod(pos.z+66.,132.)-66.) ), 1. ) );
		
	return res;
}


// fast castfunctions to detect if droste picture is hit by ray

float fastObjPrentenTentoonstelling( in vec3 pos ) {
	return opU(	udBox(  vec3( pos.x, pos.y-6.3, pos.z-8.3), vec3( 8.5, 0.025, 0.025 ) ),
				udBox(  vec3( mod(pos.x-8.375/18., 8.375/9.)-8.375/18., pos.y, pos.z-8.3), vec3( 0.025, 8.5, 0.025 ) )
	);
}
vec2 fastMap( in vec3 pos ) {
    return opU( vec2( fastObjPrentenTentoonstelling( pos), 1.0 ),
	            vec2( udBox( (pos+vec3(3.30, -4.55, -5.55)), vec3( 0.55, 0.7, 0.01 ) ), 2. ) );
}

vec2 fastCastRay( in vec3 ro, in vec3 rd, in float maxd )
{
	float precis = 0.001;
    float h=precis*2.0;
    float t = 0.0;
    float m = -1.0;
    for( int i=0; i<60; i++ )
    {
		if( abs(h)<precis || t>maxd ) break;  {
			t += h;
			vec2 res = fastMap( ro+rd*t );
			h = res.x;
			m = res.y;
		}
    }

    if( t>maxd ) m=-1.0;
    return vec2( t, m );
}

vec2 castRay( in vec3 ro, in vec3 rd, in float maxd )
{
	float precis = 0.001;
    float h=precis*2.0;
    float t = 0.0;
    float m = -1.0;
    for( int i=0; i<60; i++ )
    {
		if( abs(h)<precis || t>maxd ) break;  {
			t += h;
			vec2 res = map( ro+rd*t );
			h = res.x;
			m = res.y;
		}
    }

    if( t>maxd ) m=-1.0;
    return vec2( t, m );
}


float softshadow( in vec3 ro, in vec3 rd, in float mint, in float maxt, in float k )
{
	float res = 1.0;
    float t = 0.1;
    for( int i=0; i<15; i++ )
    {
		if( t<maxt )
		{
        float h = map( ro + rd*t ).x;			
		res = min( res, k*h/t );
        t += 0.005+h;
		} 
    }
    return clamp( res, 0.0, 1.0 );

}

vec3 calcNormal( in vec3 pos )
{
	vec3 eps = vec3( 0.001, 0.0, 0.0 );
	vec3 nor = vec3(
	    map(pos+eps.xyy).x - map(pos-eps.xyy).x,
	    map(pos+eps.yxy).x - map(pos-eps.yxy).x,
	    map(pos+eps.yyx).x - map(pos-eps.yyx).x );
	return normalize(nor);
}

void getRoAndRd( in vec2 uv, out vec3 ro, out vec3 rd ) {
#ifdef WOBBLE
	ro = vec3( 20.2+(1.0+cos((t+42.)/48.*2.*3.1415926))*cos(iTime), 36.0, 47.0  );
#else
	ro = vec3( 20.2, 36.0, 47.0  );
#endif	
	vec3 ta = vec3( -3.1, 4.8,  5.5 );
	
	// camera tx
	vec3 cw = normalize( ta-ro );
	vec3 cp = vec3( 0.0, 1.0, 0.0 );
	vec3 cu = normalize( cross(cw,cp) );
	vec3 cv = normalize( cross(cu,cw) );
	rd = normalize( uv.x*cu + uv.y*cv + cw*zoom);
}

bool hitDrostePicture( vec2 uv ) {
	vec3 ro, rd;
	getRoAndRd( uv, ro, rd );	
	
	vec2 res = fastCastRay(ro,rd,200.0);
	return (res.y == 2. );
}

vec4 trace( vec2 uv ) {
	vec3 ro, rd;
	getRoAndRd( uv, ro, rd );
	
    vec3 col = vec3(0.);
		
    vec2 res = castRay(ro,rd,400.0);
    float t = res.x;
	float m = res.y;
    if( m>-0.5 )
    {
        vec3 pos = ro + t*rd;
        vec3 nor = calcNormal( pos );

		col = vec3(0.7);
		if( m == 3. ) col = vec3(0.6,0.71,1.0);
		if( m == 4. ) col = vec3( 1. );
		
		if( m == 1. && all(lessThan(abs(pos), vec3( 5.65, 10., 5.65 ) ) ) ) {
			col = vec3( 0.6 ); // inside gallery
		}
		
		vec3 lig = normalize( vec3(-0.4, 0.4, 0.8) );
		float amb = clamp( 0.5+0.5*nor.y, 0.0, 1.0 );
        float dif = clamp( dot( nor, lig ), 0.0, 1.0 );

		float sh = 1.0;
#ifdef SHADOW		
		if( dif>0.05 ) { sh = softshadow( pos, lig, 0.1, 30.0, 5.0 ); dif *= (0.8+0.2*sh); }
#endif		
		vec3 brdf = vec3(0.0);
		brdf += 0.80*amb*vec3(0.6,0.71,0.85);
        brdf += 1.30*dif*vec3(1.00,0.90,0.70);

		col = col*brdf;
		
	} else {
		col = 1.2*vec3(0.6,0.71,0.85) - rd.y*0.2*vec3(1.0,0.5,1.0);
	}

	return vec4( clamp(col,0.0,1.0), m );
}

void init() {
	t = mod( t+11., 48. );

	if( t < 8. ) st = t;
	else if( t < 24. ) st = 8.;
	else if( t < 32. ) st = 32.-t;
		
	t = mod( t+12., 48. );
		
	if( t < 8. ) zt = t;
	else if( t < 24. ) zt = 8.;
	else if( t < 32. ) zt = 32.-t;
		
	deformationScale = clamp(pow(2.0,st), 1., 256.);
	zoom = 2.8*clamp(pow(2.0,zt), 1.0, 256. );
}

void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
    t = mod(iTime, 48.);
	vec2 uv = fragCoord.xy / iResolution.xy;
	
	uv = 2.*uv - vec2(1.);
    uv.x *= iResolution.x/ iResolution.y;
		
	init();
	
	vec3 col = vec3(0.);

	bool band = abs(uv.x)>1.?true:false;
	
	// the  gallerymodel is a factor 1./0.7 too high to match Eschers painting, so I cheat :(
	uv.x *= 0.7;
	uv = escherDeformation(uv);	
	uv = drosteTransformation(uv);
	
	if( hitDrostePicture(uv) ) uv*=256.;
	if( hitDrostePicture(uv) ) uv*=256.;
	
	
	vec4 tr = trace( uv );
	col = tr.xyz;
	
	if( band ) {
		col = mix( col, vec3(0.), st/8. );	
	}		
	
	fragColor = vec4( col,1.0);
}
