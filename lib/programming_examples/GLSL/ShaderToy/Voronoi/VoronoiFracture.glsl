//stage GL_FRAGMENT_SHADER
//import <GLUP/ShaderToy.h>

/* voronoi fracture, by mattz - Voronoi brittle fracture simulation. 
   License Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License.

   Yet another use of fast spherical Voronoi computations. 
   See https://www.shadertoy.com/view/MtBGRD for technical details.

   Note the code below is not as nice or pretty as the linked shader MtBGRD.
   Please consider that one the "reference implementation".


*/


/* Magic angle that equalizes projected area of squares on sphere. */
#define MAGIC_ANGLE 0.883475248536 // radians

const float warp_theta = MAGIC_ANGLE;
float tan_warp_theta = tan(warp_theta);

const float farval = 1e5;
const vec3 tgt = vec3(0);
const vec3 cpos = vec3(0,0,8.0);
const float fovy = 0.6; 
vec3 L = normalize(vec3(-0.4, 0.4, 1.0));

const float plane_dist = 3.0;
vec3 plane_n = normalize(vec3(0, .2, .5));

const float eps = 0.05;

const float thickness = 0.015;

const float N = 3.0;
const float fragment_radius = 0.5;

const float period = 10.0;
const float appear_time = 0.8;

const float coll_time = 3.6;
const vec3 coll_normal = vec3(1.0, 0.0, 0.0);
const float r2 = 1.0;
const float r1 = 0.25;
const float m1 = 4.0;
const float m2 = 1.0;
const float u1 = 2.0;
const float v1 = u1*(m1-m2)/(m1+m2);
const float cr = 0.7;
const float v2 = cr*2.0*m1*u1 /(m1 + m2);

float abs_time, rel_time, seed;

mat3 Rview;

struct rayhit {
    vec4 ntmin;
    vec4 ntmax;
};

const rayhit miss = rayhit(vec4(farval), vec4(-farval));

/* Choose vector with smaller w. */
vec4 v4min(vec4 a, vec4 b) {
	return a.w < b.w ? a : b;
}

/* Choose vector with larger w. */
vec4 v4max(vec4 a, vec4 b) {
	return a.w > b.w ? a : b;
}

/* Intersection of two ray intervals. */
rayhit rayinter(rayhit a, rayhit b) {
	return rayhit(v4max(a.ntmin, b.ntmin),
                  v4min(a.ntmax, b.ntmax));
}

/* Union of two ray intervals. */
rayhit rayunion(rayhit a, rayhit b) {
	return rayhit(v4min(a.ntmin, b.ntmin),
                  v4max(a.ntmax, b.ntmax));
}

/* Difference of two ray intervals. */
rayhit raydiff(rayhit a, rayhit b) {

    if (b.ntmin.w <= a.ntmin.w) {
        return rayhit(v4max(a.ntmin, vec4(-b.ntmax.xyz, b.ntmax.w)), a.ntmax);
    } else {
        return rayhit(a.ntmin, v4min(a.ntmax, vec4(-b.ntmin.xyz, b.ntmin.w)));
    }
    
		    
}

/* Ray-sphere intersection. */
rayhit sphere(in vec3 o, in vec3 d, in float r) {
	
	
	float p = -dot(o, d);
	float q = dot(o, o) - r*r;
		
	float D = p*p - q;

	if (D > 0.0) {
		float sqrtD = sqrt(D);
        float t0 = p-sqrtD;
        float t1 = p+sqrtD;
        if (t1 < 0.0) { return miss; }
        return rayhit(vec4(normalize(o+t0*d), t0),
                      vec4(normalize(o+t1*d), t1));
	}
	
	return miss;
		
}

/* Ray-plane intersection. */
rayhit plane(vec3 o, vec3 d, vec3 n) {

	float t = -dot(n,o)/dot(n,d);    
    
    if (dot(d, n) > 0.0) {
        return rayhit(vec4(n, eps), vec4(n, t));
    } else {
        return rayhit(vec4(n, t), vec4(n, farval));
    }
        
    
}

/* Ray-box intersection. */
rayhit box(vec3 ro, vec3 rd, vec3 b) {
	
	vec3 rdi = 1.0/rd;	
		
	vec3 t1 = (-b - ro)*rdi;
	vec3 t2 = ( b - ro)*rdi;
	
	vec3 tmin = min(t1, t2);
	vec3 tmax = max(t1, t2);
	
	const vec3 x = vec3(1.0, 0.0, 0.0);
	const vec3 y = vec3(0.0, 1.0, 0.0);
	const vec3 z = vec3(0.0, 0.0, 1.0);
	
	vec4 vmin = v4max(v4max(vec4(x,tmin.x), vec4(y,tmin.y)), vec4(z,tmin.z));
	vec4 vmax = v4min(v4min(vec4(x,tmax.x), vec4(y,tmax.y)), vec4(z,tmax.z));
    
    if (vmin.w > 0.0 && vmin.w <= vmax.w) {
        vmin.xyz *= -sign(dot(rd, vmin.xyz));
        vmax.xyz *= sign(dot(rd, vmax.xyz));        
        return rayhit(vmin, vmax);                
	} else {
		return miss;
	}		
	
}

/* Return a permutation matrix whose first two columns are u and v basis 
   vectors for a cube face, and whose third column indicates which axis 
   (x,y,z) is maximal. */
mat3 getPT(in vec3 p) {

    vec3 a = abs(p);
    float c = max(max(a.x, a.y), a.z);    
    vec3 s = c == a.x ? vec3(1.,0,0) : c == a.y ? vec3(0,1.,0) : vec3(0,0,1.);
    s *= sign(dot(p, s));
    vec3 q = s.yzx;
    return mat3(cross(q,s), q, s);

}

/* For any point in 3D, obtain the permutation matrix, as well as grid coordinates
   on a cube face. */
void posToGrid(in float N, in vec3 pos, out mat3 PT, out vec2 g) {
    
    // Get permutation matrix and cube face id
    PT = getPT(pos);
    
    // Project to cube face
    vec3 c = pos * PT;     
    vec2 p = c.xy / c.z;      
    
    // Unwarp through arctan function
    vec2 q = atan(p*tan_warp_theta)/warp_theta; 
    
    // Map [-1,1] interval to [0,N] interval
    g = (q*0.5 + 0.5)*N;
    
}

/* For any grid point on a cube face, along with projection matrix, 
   obtain the 3D point it represents. */
vec3 gridToPos(in float N, in mat3 PT, in vec2 g) {
    
    // Map [0,N] to [-1,1]
    vec2 q = g/N * 2.0 - 1.0;
    
    // Warp through tangent function
    vec2 p = tan(warp_theta*q)/tan_warp_theta;

    // Map back through permutation matrix to place in 3D.
    return PT * vec3(p, 1.0);
    
}

/* Return whether a neighbor can be identified for a particular grid cell.
   We do not allow moves that wrap more than one face. For example, the 
   bottom-left corner (0,0) on the +X face may get stepped by (-1,0) to 
   end up on the -Y face, or, stepped by (0,-1) to end up on the -Z face, 
   but we do not allow the motion (-1,-1) from that spot. If a neighbor is 
   found, the permutation/projection matrix and grid coordinates of the 
   neighbor are computed.
*/
bool gridNeighbor(in float N, in mat3 PT, in vec2 g, in vec2 delta, out mat3 PTn, out vec2 gn) {

    vec2 g_dst = g.xy + delta;
    vec2 g_dst_clamp = clamp(g_dst, 0.0, N);

    vec2 extra = abs(g_dst_clamp - g_dst);
    float esum = extra.x + extra.y;
 
    if (max(extra.x, extra.y) == 0.0) {
        PTn = PT;
        gn = g_dst;
        return true;
    } else if (min(extra.x, extra.y) == 0.0 && esum < N) {
        vec3 pos = PT * vec3(g_dst_clamp/N*2.0-1.0, 1.0 - 2.0*esum/N);
        PTn = getPT(pos);
        gn = ((pos * PTn).xy*0.5 + 0.5) * N;
        return true;	        
    } else {
        return false;
    }
    

}

/* From https://www.shadertoy.com/view/Xd23Dh */
vec3 hash3( vec2 p )
{
    vec3 q = vec3( dot(p,vec2(127.1,311.7)), 
                  dot(p,vec2(269.5,183.3)), 
                  dot(p,vec2(419.2,371.9)) );
    return fract(sin(q)*43758.5453);
}

/* Perturb a cube sample based upon location. */
vec3 getRandomPos(in float N, in mat3 PTn, in vec2 gn) {
    
    float face = dot(PTn[2], vec3(1.,2.,3.));   
                
    // Perturb based on grid cell ID
    gn = floor(gn);
    vec3 rn = hash3(gn*seed + face);
    gn += 0.5 + (rn.xy * 2.0 - 1.0)*0.45;

    // Get the 3D position
    return normalize(gridToPos(N, PTn, gn));  
    
}

/* Check one edge of voronoi diagram. */
void checkNeighbor(in vec3 ro, in vec3 rd,
                   in float N, in mat3 PT, in vec2 g, 
                   in vec3 p1, in vec2 uv, 
                   inout rayhit r) {
    
    if (r.ntmin.w < r.ntmax.w) {

        vec2 gn;
        mat3 PTn;

        if (gridNeighbor(N, PT, g, uv, PTn, gn)) {                
            vec3 p2 = getRandomPos(N, PTn, gn);                              
            vec3 n1 = cross(p1.xyz,p2.xyz);
            vec3 n2 = normalize(cross(n1, 0.5*(p1.xyz+p2.xyz)));
            r = rayinter(r, plane(ro, rd, n2));
        }
        
    }
    
}

/* Given hit on sphere, intersect with voronoi */ 
void fragment(in vec3 ro, in vec3 rd, 
              in float N, in mat3 PT, in vec2 g, in vec3 p1,
              inout rayhit r) {
    
    checkNeighbor(ro, rd, N, PT, g, p1, vec2(-1.0, -1.0), r);
    checkNeighbor(ro, rd, N, PT, g, p1, vec2( 0.0, -1.0), r);
    checkNeighbor(ro, rd, N, PT, g, p1, vec2( 1.0, -1.0), r);
    checkNeighbor(ro, rd, N, PT, g, p1, vec2(-1.0,  0.0), r);
    checkNeighbor(ro, rd, N, PT, g, p1, vec2( 1.0,  0.0), r);
    checkNeighbor(ro, rd, N, PT, g, p1, vec2(-1.0,  1.0), r);
    checkNeighbor(ro, rd, N, PT, g, p1, vec2( 0.0,  1.0), r);
    checkNeighbor(ro, rd, N, PT, g, p1, vec2( 1.0,  1.0), r);

}


/* Rotate about x-axis */
mat3 rotX(in float t) {
    float cx = cos(t), sx = sin(t);
    return mat3(1., 0, 0, 
                0, cx, sx,
                0, -sx, cx);
}


/* Rotate about y-axis */
mat3 rotY(in float t) {
    float cy = cos(t), sy = sin(t);
    return mat3(cy, 0, -sy,
                0, 1., 0,
                sy, 0, cy);

}

/* Rotation matrix from angle-axis */
mat3 rotaxisangle(in vec3 a, float t) {
    mat3 K = mat3(0, a.z, -a.y,
                  -a.z, 0, a.x,
                  a.y, -a.x, 0);
    return mat3(1.0) + K*(mat3(sin(t)) + (1.0-cos(t))*K);
}

/* Cast a ray into the scene and get result. */
void castRay( in vec3 ro, in vec3 rd, out vec4 result, out vec3 mat){

    result = vec4(farval);

    const vec3 x = vec3(1.0, 0.0, 0.0);
    const vec3 y = vec3(0.0, 1.0, 0.0);
    const vec3 z = vec3(0.0, 0.0, 1.0); 

    rayhit p = plane(ro+plane_dist*plane_n, rd, plane_n);
    if (p.ntmin.w > eps) {
        result = p.ntmin;
        mat = vec3(1.95, 0.2, 8.0);
    }       
    
    vec2 rando = fract(seed*vec2(139398.4339482,9304.371));    
    vec3 v1_perp = vec3(0.0, rando*vec2(6.0, 1.0)-vec2(3.0, 0)); 
  
    vec3 p1 = -coll_normal*(r1+r2);
    if (rel_time < coll_time) {
        p1 -= (u1*coll_normal + v1_perp)*(coll_time-rel_time);
    } else {
        p1 -= (v1*coll_normal + v1_perp)*(coll_time-rel_time);
    }    
    
    rayhit s = sphere(ro-p1, rd, r1);
    if (s.ntmin.w < s.ntmax.w && s.ntmin.w < result.w) {
        result = s.ntmin;
        mat = vec3(1.1, 0.5, 24.0);
    }
        
    for (int k=0; k<6; ++k) {
        
        mat3 PT;
        
        if (k < 3) {
            if (k == 0) {
                PT = mat3(x, y, z);
            } else if (k == 1) {
                PT = mat3(y, z, x);
            } else {
                PT = mat3(z, x, y);
            }
        } else {
            if (k == 3) {
                PT = mat3(x, -y, -z);
            } else if (k == 4) {
                PT = mat3(y, -z, -x);
            } else {
                PT = mat3(z, -x, -y);
            }
        }        
        
        for (float u=0.0; u<N; ++u) {
            for (float v=0.0; v<N; ++v) {

                vec2 g = vec2(u,v) + 0.5;
                
                vec3 p1 = r2*getRandomPos(N, PT, g);
                
                
                vec3 pos = p1;
                float a = max(appear_time-rel_time, 0.0)/appear_time;
                a = 3.0*a*a - 2.0*a*a*a;
                float da = 2.0*r2+plane_dist+eps;
                pos -= da*plane_n*a;
                
                float time_after_coll = max(rel_time-coll_time, 0.0);
                
                if (time_after_coll > 0.0) {                    
                    vec3 fragment_delta = (p1+coll_normal*v2)*(rel_time-coll_time);
                    pos += fragment_delta;
                    float dp = dot(pos, plane_n) + plane_dist - fragment_radius;                
                    pos -= plane_n * min(0.0, dp);
                }
                
                rayhit f = sphere(ro-pos, rd, fragment_radius);

                if (f.ntmin.w < f.ntmax.w && f.ntmin.w < result.w) {

                    vec3 h1 = hash3(p1.xy);
                    vec3 h2 = hash3(p1.yz);
                    
                    vec3 omega = normalize(2.*h1 - 1.);

                    mat3 R = rotaxisangle(omega, 4.*time_after_coll*h2.x);

                    vec3 fro = R*(ro-pos) + p1;
                    vec3 frd = R*rd;

                    f = sphere(fro, frd, r2);

                    fragment(fro, frd, N, PT, g, p1, f);
                    rayhit s2 = sphere(fro, frd, r2-thickness);
                    f = raydiff(f, s2);
                    
                    if (f.ntmax.w >= f.ntmin.w && f.ntmin.w < result.w) {
                        
                        result = vec4(f.ntmin.xyz*R, f.ntmin.w);
                                                
                        float hue_mod = floor(abs_time/period)*0.6532;
                        float sphere_h = fract(0.03 + hue_mod);
                        
                        mat = vec3(sphere_h, 0.4, 15.0);
                        
                    }
                    
                }
                
            }
        }
    }
    
    
}

vec3 hue(float h) {
	vec3 c = mod(h*6.0 + vec3(2, 0, 4), 6.0);
	return h >= 1.0 ? vec3(h-1.0) : (clamp(min(c, -c+4.0), 0.0, 1.0)*0.75 + 0.2);
}

vec3 shade(in vec3 ro, in vec3 rd) {
    
    vec3 c = vec3(1.0);
    
    vec4 r;
    vec3 m;
    
    castRay(ro, rd, r, m);

    if (r.w < farval) {
        
        vec3 n = r.xyz;
        vec3 color = hue(m.x);
        
        ro = ro + r.w*rd + 2.0*eps * r.xyz;

        vec3 amb = 0.2 * color;
        vec3 diff = 0.8*clamp(dot(n,L), 0.0, 1.0) * color;
                
        vec3 R = 2.0*n*dot(n,L)-L;
                
        float spec = pow(clamp(-dot(R, rd), 0.0, 1.0), m.z)*m.y;
        
		castRay(ro, L, r, m);

        if (r.w < farval) {
			diff = vec3(0);
            spec = 0.0;
        }
                  
        c = diff + amb + spec;

    }
    
    return c;   

}

void mainImage( out vec4 fragColor, in vec2 fragCoord ) {

    abs_time = iTime + appear_time;
    rel_time = mod(abs_time, period);
    seed = 0.2134 + floor(abs_time/period)*0.05395029;        
        
    vec2 uv = (fragCoord.xy - .5*iResolution.xy) * fovy / (iResolution.y);

    vec3 rz = normalize(tgt - cpos),
        rx = normalize(cross(rz,vec3(0,1.,0))),
        ry = cross(rx,rz);

    float t = iTime;
         
    float thetay = 0.0;
    float thetax = 0.0;

    if (max(iMouse.x, iMouse.y) > 20.0) { 
        thetax = (iMouse.y - .5*iResolution.y) * 0.5/iResolution.y; 
        thetay = (iMouse.x - .5*iResolution.x) * -1.0/iResolution.x; 
    }

    Rview = mat3(rx,ry,rz)*rotX(thetax)*rotY(thetay);        
  
    vec3 rd = Rview*normalize(vec3(uv, 1.)),
        ro = tgt + Rview*vec3(0,0,-length(cpos-tgt));
    
    vec2 q = fragCoord.xy / iResolution.xy;
        
    vec3 color = shade(ro, rd);
    
    color = pow(color, vec3(0.7));
    
    // stole iq's vingette code
    color *= pow( 16.0*q.x*q.y*(1.0-q.x)*(1.0-q.y), 0.1 );       

    fragColor = vec4(color, 1.0);


}
