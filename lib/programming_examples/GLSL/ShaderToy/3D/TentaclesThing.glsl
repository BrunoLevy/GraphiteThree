//stage GL_FRAGMENT_SHADER
//import <GLUP/ShaderToy.h>

// Created by inigo quilez - iq/2014
// License Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License.

float sdSegment( vec3 p, vec3 a, vec3 b, float r )
{
    vec3 pa = p - a;
    vec3 ba = b - a;
    float h = clamp( dot(pa,ba)/dot(ba,ba), 0.0, 1. );	
    return length( pa - ba*h ) - r;
}

const float slab = 0.05;

vec3 map( vec3 pos ) 
{
    float an = 2.0*sin( 0.5*iTime - 0.5*length(pos) );
    float co = cos(an);
    float si = sin(an);
    mat2 gRot = mat2( co, -si, si, co );
    
    pos.xz = gRot*pos.xz;
    
    pos.x -= 0.2*an;
    
    float rad = length(pos);
    float the = atan(pos.x,pos.z)+3.1416;
    float kid = floor(the/6.2831/slab);
    float phi = acos(pos.y/rad) + 3.0*kid;
    
    vec2 id = floor( vec2(the,phi)/6.2831/slab );

    the = mod( the/6.2831, slab ) - slab*0.5;
    phi = mod( phi/6.2831, slab ) - slab*0.5;

    float anph = dot(id,vec2(3171.15,2317.34));
    the += rad*0.002*cos(-rad*8.0 + 2.0*iTime + anph);
    phi += rad*0.002*sin(-rad*8.0 + 2.0*iTime + anph);
    
    float len =      1.50 * (0.7+0.3*sin(dot(id,vec2(1213.15,1317.34))));
    float thi = slab*0.25 * (0.6+0.4*sin(6.2831*rad/len));

    float d = sdSegment( vec3(the*8.0,phi*8.0,rad), 
                         vec3(0.0,0.0,0.0), 
                         vec3(0.0,0.0,len), thi*8.0 )/8.0;
    
    return vec3( d, rad/len, dot(id,vec2(217.2,311.3)) );
}

vec3 calcNormal( in vec3 pos )
{
    const vec2 e = vec2(1.0,-1.0)*0.001;

    return normalize( e.xyy*map( pos + e.xyy ).x + 
					  e.yyx*map( pos + e.yyx ).x + 
					  e.yxy*map( pos + e.yxy ).x + 
					  e.xxx*map( pos + e.xxx ).x );
}

vec4 intersect( in vec3 ro, in vec3 rd, float tmin, float tmax )
{
    float t = tmin;

    vec4 res = vec4(-1.0);
    for( int i=0; i<200; i++ ) 
    {
		vec3 d = map( ro + t*rd );
        
		if( d.x < 0.002 )
        {
            res = vec4( t, d.yz, float(i)/200.0 );
            break;
        }
        
        t += d.x;

        if( t>tmax ) break;
    }
    
    return res;
}

vec2 iSphere( in vec3 ro, in vec3 rd, in vec4 sph )
{
	vec3 oc = ro - sph.xyz;
	float b = dot( oc, rd );
	float c = dot( oc, oc ) - sph.w*sph.w;
	float h = b*b - c;
	if( h<0.0 ) return vec2(-1.0);
    h = sqrt(h);
	return vec2( -b - h, -b + h );
}

void mainImage( out vec4 fragColor, in vec2 fragCoord ) 
{
    vec2  p = (-iResolution.xy+2.0*fragCoord.xy)/iResolution.y;
    vec2  q = fragCoord.xy/iResolution.xy;

    vec3 ta = vec3(-0.9,0.0,0.0);
    vec3 ro = vec3(-0.9,0.0,2.5);        
    
    vec3  ww = normalize( ta - ro);
    vec3  uu = normalize( cross( normalize(vec3(0.2,1.0,0.0)), ww ) );
    vec3  vv = normalize( cross(ww,uu) );
    vec3  rd = normalize( p.x*uu + p.y*vv + 2.0*ww );
    
    vec3 col = vec3( 0.15, 0.17, 0.15 ) + 0.1*p.y;
    
    col += 0.3*pow( texture(iChannel0, 0.05*p).yxx, vec3(4.0) );
        
    vec2 sp = iSphere( ro, rd, vec4(0.0,0.0,0.0,1.9) );
    if( sp.y>0.0 )
    {
        vec4 res = intersect( ro, rd, max(sp.x,0.0), sp.y );

        if( res.x>0.0 )
        {
            float t = res.x;
            float m = res.z;
            float g = res.y;
            vec3  pos = ro + t*rd;
            vec3  nor = calcNormal( pos );
            float id = 0.5 + 0.5*sin(m);

            col = 0.5 + 0.5*cos( 0.0 + id*6.2831*0.3 + vec3(0.0,1.0,1.5) );
            col += vec3(0.3,0.2,0.1)*smoothstep(0.9,1.0,g);
            col *= 1.3;
            
            col *= 0.05 + 0.95*smoothstep( -0.9, 0.0, smoothstep(0.9,1.0,g)+sin( 1.0 + g*30.0  ) );

            col *= 0.5 + 0.5*nor.y;
            col += col*vec3(3.0,2.0,1.0)*clamp(1.0+dot(rd,nor),0.0,1.0);
            col *= g*g;
            col *= 2.5*pow(1.0-res.w,4.0);
        }
    }
    
    col = pow( col, vec3(0.4545) );
    
    col *= vec3(1.0,1.2,1.4);

    col = mix( col, vec3(dot(col,vec3(0.333))), -0.25 );
    
    col *= pow( 16.0*q.x*q.y*(1.0-q.x)*(1.0-q.y), 0.1 );
    
	fragColor = vec4( col, 1.0 );
}

