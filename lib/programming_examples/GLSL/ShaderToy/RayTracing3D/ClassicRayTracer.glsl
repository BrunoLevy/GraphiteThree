//stage GL_FRAGMENT_SHADER
//import <GLUP/ShaderToy.h>

// -----------------------------------------------------------------------------------
//
// Carlos Ureña, Apr,2018
// License Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License.
//
// -----------------------------------------------------------------------------------

// -----------------------------------------------------------------------------------
//
// Carlos Ureña, Apr,2018
// License Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License.
//
// -----------------------------------------------------------------------------------

// ----------------------------------------------------------------
// ISLAMIC STAR PATTERN related functions


// parameters and pre-calculated constants
const float 
    sqr2       = 1.41421356237, // square root of 2
    sqr3       = 1.73205080756, // square root of 3.0
    sqr2_inv   = 1.0/sqr2 ,
    sqr3_inv   = 1.0/sqr3 ,
    cos30      = 0.86602540378, // cos(30 degrees)
    sin30      = 0.50000000000, // sin(30 degrees)
    l          = 5.5,          // length of triangle in NDC (mind --> 1.0)
    l_inv      = 1.0/l ,       // length inverse
    line_w     = 0.03,         // line width for basic symmetry lines render
    sw         = 0.020 ;       // stripes half width for islamic star pattern

const vec2  
    u        = 1.0*vec2( 1.0, 0.0  ) ,          // grid basis: U vector
    v        = 0.5*vec2( 1.0, sqr3 ) ,          // grid basis: V vector
    u_dual   = 1.0*vec2( 1.0, -sqr3_inv ) ,     // dual grid basis: U vector
    v_dual   = 2.0*vec2( 0.0,  sqr3_inv ) ,     // dual grid basis: V vector
    tri_cen  = vec2( 0.5, 0.5*sqr3_inv ) ;      // triangle center

    
// -----------------------------------------------------------------------------------
// point orbit transformation parameters
int 
    nMirrorOdd = 0 , 
    nMirror    = 0 ,
	nGridX     = 0 , 
    nGridY     = 0 ;


// -------------------------------------------------------------------------------
// mirror reflection of 'p' around and axis through 'v1' and 'v2'
// (only for points to right of the line from v1 to v2)
//
vec2 Mirror( vec2 p, vec2 v1, vec2 v2 )
{
 	vec2   s = v2-v1 ,
           n = normalize(vec2( s.y, -s.x )) ;
    float  d = dot(p-v1,n) ;
    
    if ( 0.0 <= d )
    {
       nMirrorOdd = 1-nMirrorOdd ;
       nMirror = nMirror+1 ;
       return p-2.0*d*n ;
    }
    else
       return p ;
}
// -------------------------------------------------------------------
// Signed perpendicular distance from 'p' to line through 'v1' and 'v2'

float SignedDistance( vec2 p, vec2 v1, vec2 v2 )
{
 	vec2   s = v2-v1 ,
           n = normalize(vec2( s.y, -s.x )) ;
    return dot(p-v1,n) ;
}
// -------------------------------------------------------------------
// un-normalized signed distance to line

float UnSignedDistance( vec2 p, vec2 v1, vec2 v2 )
{
 	vec2   s = v2-v1 ,
           un = vec2( s.y, -s.x ) ;
    return dot(p-v1,un) ;
}
// -------------------------------------------------------------------
// Signed perpendicular distance from 'p' to polyline from 'v1' 
// to 'v2' then to 'v3'

float DoubleSignedDistance( vec2 p, vec2 v1, vec2 v2, vec2 v3 )
{
 	
    vec2  dir1 = v2 + normalize(v1-v2),
          dir3 = v2 + normalize(v3-v2);
        
    vec2  vm = 0.5*(dir1+dir3) ;
    
    float dm = UnSignedDistance( p, v2, vm ) ;
    
    if ( dm >= 0.0 )
   		return SignedDistance( p, v1, v2 ) ;
   	else
        return SignedDistance( p, v2, v3 ) ; 
}
// -------------------------------------------------------------------------------
// Takes 'p0' to the group's fundamental region, returns its coordinates in that region

vec2 p6mm_ToFundamental( vec2 p0 ) 
{
    nMirrorOdd = 0 ;
    nMirror    = 0 ;
    
    // p1 = fragment coords. in the grid reference frame
    
    vec2 p1 = vec2( dot(p0,u_dual), dot(p0,v_dual) );
    
    // p2 = fragment coords in the translated grid reference frame 
    
    vec2 p2 = vec2( fract(p1.x), fract(p1.y) ) ;
    
    nGridX = int(p1.x-p2.x) ; // largest integer g.e. to p1.x
    nGridY = int(p1.y-p2.y) ; // largest integer g.e. to p2.x
    
    // p3 = barycentric coords in the translated triangle
    // (mirror, using line x+y-1=0 as axis, when point is right and above axis)
    
    vec2 p3 = Mirror( p2, vec2(1.0,0.0), vec2(0.0,1.0) );
    
    // p4 = p3, but expressed back in cartesian coordinates
    
    vec2 p4 = p3.x*u + p3.y*v ;
    
    // p7 = mirror around the three lines through the barycenter, perp. to edges.
    
    vec2 p5 = Mirror( p4, vec2(0.5,0.0), tri_cen );
    vec2 p6 = Mirror( p5, vec2(1.0,0.0), tri_cen );
    vec2 p7 = Mirror( p6, tri_cen, vec2(0.0,0.0) );
  
    return p7 ;
}

// --------------------------------------------------------------------
// A possible distance function

float DistanceFunc( float d )
{
   return 1.0-smoothstep( line_w*0.5, line_w*1.5, d );   
}

// -------------------------------------------------------------------------------
// Point color for basic symmetry lines in (r,g,b)

vec4 p6mm_SimmetryLines( vec2 p_ndc )
{

    vec2 pf = p6mm_ToFundamental( p_ndc );
    
    float d1 = abs(pf.y),
          d2 = abs(pf.x-0.5),
          d3 = abs( SignedDistance( pf, tri_cen, vec2(0.0,0.0) ) );
     
    vec4 res = vec4( 0.0, 0.0, 0.0, 1.0 ) ;
        
    res.r = DistanceFunc(d2);
    res.g = DistanceFunc(d1);
    res.b = DistanceFunc(d3);
    
    return res ;    
}

// ---------------------------------------------------------------------
// Stripe half width for star pattern

vec4 Stripe( float d )
{
   if ( d > sw*0.85 )
     return vec4( 0.0,0.0,0.0,1.0 );
   else
     return vec4(1.0,1.0,1.0,1.0)  ;
}

// ---------------------------------------------------------------------
// Color for islamic star pattern

vec4 p6mm_pattern( vec2 p )
{
    vec2 pf = p6mm_ToFundamental( p );
    
    //return p6mm_SimmetryLines( p ) ;
    vec2 c  = tri_cen ;
    
    // constants defining the stripes 
    float 
        f   = 0.30 ,
        fs1 = 0.14 ,
        s1  = fs1*c.x,
        s2  = 0.5*s1 ;
        
    // stripes vertexes
    vec2 
        // upper strip
        u1 = vec2( f*c.x, 0.0 ) ,
        u2 = vec2( c.x, (1.0-f)*c.y ),
        
        // lower strip
        l1 = vec2( c.x, s1+s2 ),
        l2 = vec2( c.x-s2, s1 ),
        l3 = vec2( sqr3*s1, s1 ),
        
        // right strip
        r1 = vec2( c.x-s1, (1.0-fs1)*c.y ),
        r2 = vec2( c.x-s1, s2 ) ,
        r3 = vec2( c.x-s1-s2, 0.0 ),
        
    	// origin star strip
        mm = vec2( s1*(sqr3-1.0/3.0), s1*(1.0-sqr3_inv) );
                        
    // signed and unsigned distances to stripes:
    
    float
        d1s = SignedDistance( pf, u1, u2 ) ,
        d2s = DoubleSignedDistance( pf, l1, l2, l3 ) ,
        d3s = DoubleSignedDistance( pf, r1, r2, r3 ) ,
        d4s = DoubleSignedDistance( pf, u1, mm, l3 ) ,
        d1  = abs( d1s ),
        d2  = abs( d2s ),
        d3  = abs( d3s ),
        d4  = abs( d4s );
    
   
    // stripes inclusion
    bool in1, in2, in3, in4 ;
    
    if ( nMirrorOdd == 0 )
    {
        in1 = (d1 < sw) && ! (d2 < sw) && ! (d4 < sw);
        in2 = (d2 < sw) && ! (d3 < sw);
        in3 = (d3 < sw) && ! (d1 < sw);
        
        in4 = (d4 < sw) && ! (d2 < sw);
    }
    else
    {
        in1 = (d1 < sw) && ! (d3 < sw) ;
        in2 = (d2 < sw) && ! (d1 < sw) && ! (d4 < sw);;
        in3 = (d3 < sw) && ! (d2 < sw);
        
        in4 = (d4 < sw) && ! (d1 < sw);
    } 
    
    vec4 col ;
    
    // compute final color
    
    if ( in1 )      
        col = Stripe( d1 ) ;
    else if ( in2 ) 
        col = Stripe( d2 ) ;
    else if ( in3 ) 
        col = Stripe( d3 ) ; 
    else if ( in4 )
        col = Stripe( d4 ) ;   
    else if ( d2s < 0.0 && d3s < 0.0 )
        col = vec4( 0.0, 0.4, 0.0, 1.0 ) ;
    else if ( d1s < 0.0 && d2s < 0.0 || d1s <0.0 && d3s < 0.0 )
        col = vec4( 0.1, 0.1, 0.1, 1.0 );
    else if ( d1s < 0.0 || d2s < 0.0 )
        col = vec4( 0.0, 0.4, 0.9, 1.0 );   
    else    
        col = vec4( 0.6, 0.0, 0.0, 1.0 ) ; 
       
    return col ;
}


// --------------------------------------------------------------------
// RAY-TRACER
// --------------------------------------------------------------------

// root of the number of samples for antialiasing
const int n_aa = 1 ;


// struct datatypes

struct Camera
{
   vec3  obs;
   vec3  o,x,y, z ;
   float ratio_yx ;
} ;

// --------------------------------------------------------------------

struct Ray
{
   vec3 org, dir ;
   vec2 fragC ;
   int  obj_id ; // if the ray has origin in one object, the object's id,
                 // -1 otherwise (primary rays)
   bool in_glass ; // true if the start of the ray is in glass,
                   // false otherwise (it is false for primary rays)
} ;
 
// --------------------------------------------------------------------

struct Material
{
    float kd,    // diffuse reflection coefficient (in [0,1])
          kps ,  // perfect specular reflection coefficient (in [0,1])
          kph,   // phong component
          kt ;   // transmitted component (refracted)
} ;
    
// --------------------------------------------------------------------

struct Sphere
{
   vec3  center ;
   float radius ;
   vec3  color ;
   int   id ;  // object id
} ;

// --------------------------------------------------------------------

struct ShadingPoint
{
    vec3 pos,      // position 
         nor,      // normal vector
         view ;    // vista
    int  obj_id ;  // identifier of the object whose surface the point is in
    bool in_glass; // true if the normal points towards a glass medium
                   // false when it points towards air

} ;

// --------------------------------------------------------------------
// scene

const int id_base_plane = 0,
          id_sphere1    = 1,
          id_sphere2    = 2,
          id_sphere3    = 3,
          id_sphere4    = 4,
          
          id_sphere5    = 5,
          
          id_sphere6    = 6,
          
          num_objects   = 6 ;
    
struct Scene
{
   vec3   sun_dir ;
   float  sun_ap_sin ;
   Camera camera ;
   Sphere sphere1,
          sphere2,
          sphere3, // outer transparent sphere (white one)
          sphere4, // inner transparent sphere (white one)
          sphere5, // outer transparent sphere (green one)
          sphere6; // inner transparent sphere (green one)
    
   Material materials[num_objects] ;
} ;
    
// scene
Scene scene ;

// --------------------------------------------------------------------
// rays stack

struct RayStackEntry
{
   Ray   ray ;       // ray to process
   bool  processed ; // true if the color has been already computed,
                     // and the child nodes have been pushed
   int   iparent ;   // parent stack entry index, -1 for root node
   vec3  color ;     // resulting color, only if 'processed == true'
   float weight ;    // weight of this color in parent ray, if any
} ;
    
const int max_n_stack = 20 ;  // max number of items in the stack

RayStackEntry  stack[max_n_stack] ;
    
// --------------------------------------------------------------------

struct InterStat  // ray-scene intersection status data
{
    Ray   ray ;    // ray being intersected
    float t_max,   // max. value for 't' (-1.0 if it is +infinity)
          t_hit ;  // actual smaller positive value found for 't' (-1.0 if none)
   	int   id_hit ; // 'id' of current object (-1 if none)
} ;


// --------------------------------------------------------------------
const float t_threshold = 0.001 ;

bool update_is( inout InterStat is, float t, int id )
{
   if ( t < t_threshold )
     return false;
   if ( 0.0 < is.t_hit && is.t_hit < t )
     return false;
   if ( 0.0 < is.t_max && is.t_max < t )
     return false;

   is.t_hit  = t ;
   is.id_hit = id ;
   return true ;
}

// --------------------------------------------------------------------
// ray-sphere intersection
//
// 'is.ray.dir' is assumed normalized
// returns 'true' if there is an intersection, and, if there is a previous
// intersection, this is nearest than previous

bool sphere_intersect( in Sphere sphere, inout InterStat is  )
{
	vec3
      oc = is.ray.org - sphere.center;
	float
      c  = dot(oc, oc) - (sphere.radius*sphere.radius),
	  b  = dot(is.ray.dir, oc) ,
      di = b*b - c ;   // discriminant

    if ( di < 0.0 ) // no sphere-ray intersection, 'is' is not written
        return false ;

    float
      sqrt_di = sqrt(di),
      t ;

    t = -b - sqrt_di ;
	if ( t < t_threshold )
      t = -b + sqrt_di ;

    return update_is( is, t, sphere.id );
}

// --------------------------------------------------------------------
// ray-plane intersection (infinite plane at y==0)
// returns 'true' if there is an intersection and it is nearest than a 
// the previous one stored in 'is', if any.

bool horizon_plane_intersection( in int plane_id, inout InterStat is )
{
    if ( abs(is.ray.dir.y) < 1e-5 )
       return  false ;

    float
      t = -is.ray.org.y / is.ray.dir.y ;

    return update_is( is, t, plane_id );
}

// --------------------------------------------------------------------
// it just test if the ray is blocked by any object in the scene

bool ray_blocked( in Ray ray )
{
   InterStat is ;

   is.t_max  = -1.0 ;
   is.t_hit  = -1.0 ;
   is.id_hit = -1 ;
   is.ray    = ray ;

   if ( ray.obj_id != id_base_plane )
   {
      horizon_plane_intersection( id_base_plane, is );
      if ( is.id_hit != -1 )
        return true ;
   }

   if ( ray.obj_id != id_sphere1 )
   {
      sphere_intersect( scene.sphere1, is );
      if ( is.id_hit != -1 )
         return true ;
   }

   if ( ray.obj_id != id_sphere2 )
   {
   	  sphere_intersect( scene.sphere2, is );
      if ( is.id_hit != -1 )
        return true ;
   }
    
   if ( ray.obj_id != id_sphere3 )
   {
   	  sphere_intersect( scene.sphere3, is );
      if ( is.id_hit != -1 )
        return true ;
   }
    
   if ( ray.obj_id != id_sphere4 )
   {
   	  sphere_intersect( scene.sphere4, is );
      if ( is.id_hit != -1 )
        return true ;
   }
    
  

   return false ;
}

// --------------------------------------------------------------------
// returns true if the shading point is visible from 'scene.sun_dir', false otherwise

bool sun_dir_visible( in ShadingPoint sp )
{
   Ray ray ;
    
   if ( dot( sp.nor, scene.sun_dir ) < 0.0 )
      return false ;

   ray.org    = sp.pos ;
   ray.dir    = scene.sun_dir ;
   ray.obj_id = sp.obj_id ;

   return ! ray_blocked( ray );
}
// --------------------------------------------------------------------

vec3 phong_component( in vec3 nor, in vec3 view, in vec3 light )
{
    float vh = dot( nor, normalize( view+light ) ),
          b  = pow( vh, 8.0 );
    
    return vec3( b, b, b );
}

// --------------------------------------------------------------------

vec3 sphere_shader( in Sphere sphere, in ShadingPoint sp )
{
    vec3 res = vec3(0.0,0.0,0.0);
    
    if ( sun_dir_visible( sp ) )
    {
      float ldn = dot( sp.nor, scene.sun_dir ),
            kd  = scene.materials[sp.obj_id].kd ,
            kph = scene.materials[sp.obj_id].kph ;
        
      res += ldn*kd*sphere.color ;
        
      if ( 0.0 < kph )
         res += kph*phong_component( sp.nor, sp.view, scene.sun_dir ); 
    }

    return max(res, 0.1*sphere.color ) ;
}

// --------------------------------------------------------------------

vec3 sphere_color( in Sphere sphere, in InterStat is,  out ShadingPoint sp )
{
    sp.pos    = is.ray.org + is.t_hit*is.ray.dir ;
    sp.view   = -is.ray.dir ;
    
    sp.nor    = normalize( sp.pos - sphere.center );
    sp.obj_id = sphere.id ;
    sp.in_glass = is.ray.in_glass ;

    return sphere_shader( sphere, sp );
}

// --------------------------------------------------------------------

vec3 horizon_plane_shader( in ShadingPoint sp )
{
    
    float vis = sun_dir_visible( sp ) ?  1.0 : 0.5 ,
          kd  = scene.materials[sp.obj_id].kd,
          kph = scene.materials[sp.obj_id].kph ;
    
// #define BASE_PLANE_RELIEF
    
#ifdef BASE_PLANE_RELIEF 
    
    float delta = 0.001, k = 2.0 ;
    vec2 p00 = sp.pos.xz ;
    vec2 p10 = p00 + vec2( delta, 0.0 ),
         p01 = p10 + vec2( 0.0, delta );
    
    vec4 col = p6mm_pattern( p00 ),
         col01 = p6mm_pattern( p01 ),
         col10 = p6mm_pattern( p10 );
    
    float v00 = col.r + col.g + col.b ,
          v01 = col01.r + col01.g + col01.b ,
          v10 = col10.r + col10.g + col10.b ;
    
    vec3 t1 = normalize( vec3( delta, k*(v10-v00), 0.0 ) ),
         t2 = normalize( vec3( 0.0,   k*(v01-v00), delta ) );
    
    vec3 nor = normalize( cross( t1, t2 ) );
    if ( nor.y < 0.0 )
        nor = -1.0*nor ;
#else
    vec4 col = p6mm_pattern( sp.pos.xz );
    vec3 nor = vec3( 0.0, 1.0, 0.0 );
#endif
    vec3 res_color = vis*kd*col.rgb ;
    
    if ( vis == 1.0 && 0.0 < kph)
       res_color += kph*phong_component(nor,sp.view,scene.sun_dir) ; 
    
    return res_color ;
}

// --------------------------------------------------------------------

vec3 horizon_plane_color( in InterStat is, out ShadingPoint sp )
{

    sp.pos    = is.ray.org + is.t_hit*is.ray.dir ;
    sp.view   = -is.ray.dir ;
    sp.obj_id = is.id_hit ;
    sp.nor    = vec3( 0.0, 1.0, 0.0 );
    sp.in_glass = false ;

    return horizon_plane_shader( sp );
}

// --------------------------------------------------------------------
// compute primary ray origin and direction, from the fragment coord and
// camera parameters

Camera compute_camera( in vec3 cam_look_at )
{

    Camera cam ;
    vec3 cam_vup = vec3( 0.0, 1.0, 0.0 );
    float dist = 2.0 ;
    vec2 dxy = iMouse.xy - abs(iMouse.zw) ; // sign of .zw tells if a click happened or not...

    float fx   = 0.011,
          fy   = 0.01 ,
          lat0 = 0.5,
          lon0 = 0.5,
          lon  = lon0 + fx*dxy.x,
          lat  = max( 0.0, min( 1.5, lat0 - fy*dxy.y )),
          cos_lat = cos(lat);

    cam.z = vec3( cos(lon)*cos_lat , sin(lat), sin(lon)*cos_lat );

    cam.obs = cam_look_at + dist*cam.z ;
    cam.o   = cam.obs - 1.5*cam.z ;
    cam.x   = normalize( cross( cam_vup, cam.z ) );
    cam.y   = normalize( cross( cam.x, cam.z ) ); // do we really need to normalize here ?

    cam.ratio_yx = iResolution.y/iResolution.x ;

    return cam ;
}


// --------------------------------------------------------------------
// compute primary ray origin and direction, from the fragment coord and
// camera parameters

Ray primary_ray( in vec2 sample_coords, in Camera cam )
{
    Ray pray ;
    vec2  uv  = sample_coords/iResolution.xy;     // uv in [0,1]^2
    float x   = 2.0*uv[0] - 1.0 ,                 // x in [-1,+1]
          y   = (1.0 - 2.0*uv[1] )*cam.ratio_yx ; // y in [-ratio_yx,+ratio_yx]
    vec3  p   = cam.o + x*cam.x + y*cam.y ; // p is the point in the view-plane

    pray.dir    = normalize( p - cam.obs ),
    pray.org    = cam.obs ;
    pray.fragC  = sample_coords ;
    pray.obj_id = -1 ;
    pray.in_glass = false ; // we assume observer is not 'in glass'

    return pray ;
}

// --------------------------------------------------------------------

vec3 background_color( in Ray ray )
{
    float b = max( 0.0, dot( ray.dir, scene.sun_dir ) );
    if ( 1.0-scene.sun_ap_sin < b )
        return vec3( 1.0,1.0,1.0 );
    //else
    //    return max(0.5,pow(b,5.0))*vec3( 0.0, 0.1, 0.2 );
    
    vec3 d = vec3( ray.dir.x, ray.dir.y/2.0, ray.dir.z ),
         da = abs( d );
    
    vec2 tcoords ;
    if ( da.x <= da.y && da.z <= da.y ) // max is Y (use x,z)
        tcoords = vec2( 0.5, 0.5) + 0.5*vec2( d.x, d.z )/da.y;
    else if ( da.z <= da.x && da.y <= da.x ) // max is X (use z,y)
        tcoords = vec2( 0.5, 0.0 ) + vec2( 0.5*d.z, d.y )/da.x;
    else // max is Z (use x,y)
        tcoords = vec2( 0.5, 0.0 ) + vec2( 0.5*d.x, d.y )/da.z;
    
    const float margin = 0.001 ;
    
    if ( tcoords.x < margin || 1.0-margin < tcoords.x   )
        return vec3( 0.0, 0.0, 0.0 );
        
    if ( tcoords.y < margin || 1.0-margin < tcoords.y )
        return vec3( 0.0, 0.0, 0.0 );
    
   	vec2 tc2 = (tcoords-vec2(margin,margin))/(1.0-2.0*margin) ;
    const float nrep = 2.0 ;
        
    vec4 col = texture( iChannel0, fract( nrep*tc2 ));
    return 1.0*pow( col.rgb, 2.0*vec3(1.0, 1.0, 1.0) ) ;
    
}
// --------------------------------------------------------------------

InterStat scene_intersect( in Ray ray )
{
   InterStat is ;

   is.t_max  = -1.0 ;
   is.t_hit  = -1.0 ;
   is.id_hit = -1 ;
   is.ray    = ray ;

   if ( ray.obj_id != id_sphere1 )
   	 sphere_intersect( scene.sphere1, is );
   
   if ( ray.obj_id != id_sphere2 )
   	 sphere_intersect( scene.sphere2, is );
    
   //if ( ray.obj_id != id_sphere3 )
   	 sphere_intersect( scene.sphere3, is );
    
   //if ( ray.obj_id != id_sphere4 )
   	 sphere_intersect( scene.sphere4, is );
      
   if ( ray.obj_id != id_base_plane )
     horizon_plane_intersection( id_base_plane, is ); 
   
   return is ;
}

// --------------------------------------------------------------------

vec3 scene_color( in InterStat is, out ShadingPoint sp )
{
    if ( is.id_hit == id_base_plane )
       return horizon_plane_color( is, sp );
    
    if ( is.id_hit == id_sphere1 )
      return sphere_color( scene.sphere1, is, sp  );
     
    if ( is.id_hit == id_sphere2 )
       return sphere_color( scene.sphere2, is, sp  );
    
    if ( is.id_hit == id_sphere3 )
       return sphere_color( scene.sphere3, is, sp  );
    
    if ( is.id_hit == id_sphere4 )
       return sphere_color( scene.sphere4, is, sp  );
      
    return background_color( is.ray ) ;
}
// --------------------------------------------------------------------
// computes the reflected ray, the classical formula for the direction 
// is here:
// https://en.wikipedia.org/wiki/Specular_reflection#Vector_formulation

Ray reflected_ray( in ShadingPoint sp )
{
   Ray rr ; 
    
   rr.org      = sp.pos ;
   rr.dir      = 2.0*dot(sp.view,sp.nor)*sp.nor - sp.view ;
   rr.obj_id   = sp.obj_id ;
   rr.in_glass = sp.in_glass ;
    
   return rr ;
}
// --------------------------------------------------------------------
// computed the refracted ray
// the formula for the refracted ray direction can be seen here:
// https://en.wikipedia.org/wiki/Snell%27s_law#Vector_form

Ray get_refracted_ray( in ShadingPoint sp )
{
   const float glass_ri = 1.3 , // refractive index of glass 
               glass_ri_inv = 1.0/glass_ri ;
    
   vec3 l = -sp.view , // wikipedia formulation uses l "from light to shading point"
        no = sp.nor ;
    
   // r == ratio of refraction indexes
   float r = sp.in_glass ? glass_ri : glass_ri_inv ;
    
    // c == cosine of incidence angle
   float c = -dot( no, l );
   
   // if we hit the point in the 'back' side (w.r.t the normal), flip normal.
   if ( c < 0.0 ) 
   {  
      c = -c ;
      no = -no ;
   }
   
   Ray rr ; // resulting ray 
   float radicand = 1.0-r*r*(1.0-c*c);
    
   if ( radicand < 0.0 ) 
   {
      // total internal reflection 
      rr.org      = sp.pos ;
      rr.dir      = 2.0*c*no + l ; // reflected ray formula, with l = -sp.view
      rr.obj_id   = sp.obj_id ;
      rr.in_glass = sp.in_glass ; // no medium switch 
   }
   else
   {
      // normal refraction: build rr 
      rr.org      = sp.pos ;
      rr.dir      = r*l + (r*c-sqrt(radicand))*no ;
      rr.obj_id   = sp.obj_id ;
      rr.in_glass = ! sp.in_glass ; // medium switch
   } 
   return rr ;
}

// --------------------------------------------------------------------
// returns the color (radiance) incident on ray origin, coming 
// from ray direction

vec3 ray_color( in Ray ray )
{
   int  n = 0;     // number of entries already in the stack 
   vec3 res_color; // resulting color

   // push the first ray
   stack[n].ray       = ray ;
   stack[n].processed = false ;
   stack[n].iparent   = -1 ;      // -1 means this is first node in stack (has no parent)
   stack[n].color     = vec3( 0.0, 0.0, 0.0 );
   stack[n].weight    = 1.0 ;
   n++ ;
    
   // loop while the stack is not empty
    
   while( n > 0 )
   {
     int itop = n-1 ;

     // if node on top is already processed, pop it
     if ( stack[itop].processed ) 
     {
         vec3 col     = stack[itop].weight*stack[itop].color ;
         int  iparent = stack[itop].iparent ;
          
         if ( iparent == -1 ) 
             res_color = col ; 
         else  
         	stack[iparent].color += col ;
         	
         n-- ;  // pop this node
         continue ;
     }
     
     // Process an unprocessed node:
     
     // (1) intersect ray and get 'is' object
       
     ShadingPoint sp ;
     InterStat    is     = scene_intersect( stack[itop].ray ); 
     bool         inters = is.id_hit != -1 ;
     
       
     // (2) compute and update node color (initializes 'sp')
     stack[itop].color = scene_color( is, sp );    
      
     // (3) push child rays if neccesary   
     if ( inters && (n < max_n_stack) )
     {  
        float kps = scene.materials[is.id_hit].kps ;
         
        float vn = dot(sp.nor,sp.view);
            
        if ( 0.0 < kps && 0.0 < vn && ! sp.in_glass ) // if we should compute reflected ray...
        {
           // computed reflected ray
           Ray refl_ray = reflected_ray( sp ); 
             
           // push reflected ray entry      
           stack[n].ray       = refl_ray ;
           stack[n].processed = false ;
           stack[n].iparent   = itop ; 
           stack[n].color     = vec3( 0.0, 0.0, 0.0 );
           stack[n].weight    = kps ;
           n++ ;
        }
         
        float kt = scene.materials[is.id_hit].kt ;
         
        if ( (n < max_n_stack) && 0.0 < kt )
        {
           // compute refracted ray
           Ray refrac_ray = get_refracted_ray( sp );
              
           // push refracted ray entry      
           stack[n].ray       = refrac_ray ;
           stack[n].processed = false ;
           stack[n].iparent   = itop ; 
           stack[n].color     = vec3( 0.0, 0.0, 0.0 );
           stack[n].weight    = kt ;
           n++ ;  
        }
     }
     // (4) mark the node as processed
     stack[itop].processed = true ;
       
   } // end while 

   return res_color; 
}

// --------------------------------------------------------------------
// computed anti-aliased pixel color
//
// fcoords.x  goes from 0.5 to iResolution.x-0.5 (same for .y)

vec4 AA_pixel_color( in vec2 pixel_coords )
{
    vec3 sum = vec3( 0.0, 0.0, 0.0 );
    const float n_aa_f = float(n_aa);

    for( int i = 0 ; i < n_aa ; i++ )
    {
       float desplx = (float(i)+0.5)/n_aa_f -0.5 ;

       for( int j = 0 ; j < n_aa ; j++ )
       {
          float desply = (float(j)+0.5)/n_aa_f -0.5;
          float fac = 1.0000 ;
          vec2  despl         = fac*vec2( desplx, desply  ), // why we must multiply by something other than 1 ?
                sample_coords = pixel_coords + despl ;
          Ray   ray           = primary_ray( sample_coords, scene.camera ) ;
          vec3  col           = ray_color( ray );

          sum = sum + col ;
       }
    }

    return vec4( sum/(n_aa_f*n_aa_f) ,1.0 );
}

// --------------------------------------------------------------------

void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
   

    int n_aa =1; // root of the number of samples per pixel

    // scene parameters
    
    vec3 cam_look_at = vec3( 0.0, 0.15, 0.5 ) 
                          + cos( iTime )*vec3(1.0,0.0,0.0)
                          + sin( iTime )*vec3(0.0,0.1,0.3) ;

    scene.camera  = compute_camera( cam_look_at );
    scene.sun_dir = normalize( vec3( 0.2, 1.2, 1.0 ) );
    scene.sun_ap_sin = 0.003 ;
    
    // base plane 
    
    scene.materials[id_base_plane].kd  = 0.6;
    scene.materials[id_base_plane].kps = 0.0;
    scene.materials[id_base_plane].kph = 0.7;
    scene.materials[id_base_plane].kt  = 0.0;
    
    // sphere 1 

    scene.sphere1.id      = id_sphere1 ;
    scene.sphere1.center  = vec3( 0.0, 0.5, 0.0 );
    scene.sphere1.radius  = 0.5 ;
    scene.sphere1.color   = vec3(0.5,0.5,1.0);
    
    scene.materials[id_sphere1].kd  = 0.6;
    scene.materials[id_sphere1].kph = 0.6;
    scene.materials[id_sphere1].kps = 0.4;
    scene.materials[id_sphere1].kt  = 0.0;
    
    
    // sphere 2 

    scene.sphere2.id      = id_sphere2 ;
    scene.sphere2.center  = vec3( 0.0, 0.4, 0.9 );
    scene.sphere2.radius  = 0.4 ;
    scene.sphere2.color   = vec3( 1.0, 0.2, 0.2 );

    scene.materials[id_sphere2].kd  = 0.3;
    scene.materials[id_sphere2].kph = 0.3;
    scene.materials[id_sphere2].kps = 0.4;
    scene.materials[id_sphere2].kt  = 0.0;
    
    // sphere 3 (outer sphere in the transparent ball)
    
    float tr_sph_rad    = 0.35 ;
    vec3  tr_sph_center = vec3( 1.0, tr_sph_rad, 0.6 ) ;

    scene.sphere3.id      = id_sphere3 ;
    scene.sphere3.center  = tr_sph_center;
    scene.sphere3.radius  = tr_sph_rad ;
    scene.sphere3.color   = vec3( 1.0, 1.0, 1.0 );

    scene.materials[id_sphere3].kd  = 0.0;
    scene.materials[id_sphere3].kph = 0.0;
    scene.materials[id_sphere3].kps = 0.3;
    scene.materials[id_sphere3].kt  = 0.7;
    
    // sphere 4 (inner sphere in the transparent ball)

    scene.sphere4.id      = id_sphere4 ;
    scene.sphere4.center  = tr_sph_center;
    scene.sphere4.radius  = 0.9*tr_sph_rad ;
    scene.sphere4.color   = vec3( 1.0, 1.0, 1.0 );

    scene.materials[id_sphere4].kd  = 0.0;
    scene.materials[id_sphere4].kph = 0.0;
    scene.materials[id_sphere4].kps = 0.0;
    scene.materials[id_sphere4].kt  = 1.0;
    
    /**
    // sphere 5 

    scene.sphere5.id      = id_sphere5 ;
    scene.sphere5.center  = vec3( 0.9, 0.25, 0.0 );
    scene.sphere5.radius  = 0.25 ;
    scene.sphere5.color   = vec3( 1.0, 1.0, 1.0 );

    scene.materials[id_sphere5].kd  = 0.0;
    scene.materials[id_sphere5].kph = 0.0;
    scene.materials[id_sphere5].kps = 0.2;
    scene.materials[id_sphere5].kt  = 0.8;
    
    // sphere 6 

    scene.sphere6.id      = id_sphere6 ;
    scene.sphere6.center  = vec3( 0.9, 0.25, 0.0 );
    scene.sphere6.radius  = 0.22 ;
    scene.sphere6.color   = vec3( 1.0, 1.0, 1.0 );

    scene.materials[id_sphere6].kd  = 0.0;
    scene.materials[id_sphere6].kph = 0.0;
    scene.materials[id_sphere6].kps = 0.0;
    scene.materials[id_sphere6].kt  = 1.0;
    **/
    
    // ----
    
    vec2 pixel_coords = fragCoord.xy ;

    fragColor = AA_pixel_color( pixel_coords ) ;
               
}
