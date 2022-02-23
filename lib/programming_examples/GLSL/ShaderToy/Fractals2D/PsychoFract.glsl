//stage GL_FRAGMENT_SHADER
//import <GLUP/ShaderToy.h>

// "Psychofract" by Carlos UreÃ±a - 2015
// License Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License.

mat3 tran1, 
     tran2 ; //transform matrix for each branch

const float pi = 3.1415926535 ;

const float rsy = 0.30 ; // length of each tree root trunk in NDC

// -----------------------------------------------------------------------------

mat3 RotateMat( float rads )
{
   float c = cos(rads),
         s = sin( rads ) ;
   
   return mat3(  c,  s, 0.0,
                -s,   c, 0.0,
               0.0, 0.0, 1.0  );
}
// -----------------------------------------------------------------------------

mat3 TranslateMat( vec2 d )
{
   return mat3(  1.0, 0.0, 0.0,
                 0.0, 1.0, 0.0,
                 d.x, d.y, 1.0 );
}
// -----------------------------------------------------------------------------

mat3 ScaleMat( vec2 s )
{
   return mat3( s.x, 0.0, 0.0,
                0.0, s.y, 0.0,
                0.0, 0.0, 1.0 );
}
// -----------------------------------------------------------------------------

mat3 ChangeFrameToMat( vec2 org, float angg, float scale )
{
   float angr = (angg*pi)/180.0 ;
   return
        ScaleMat( vec2( 1.0/scale, 1.0/scale ) )
      * RotateMat( -angr )
      * TranslateMat( -org ) ;
}
// -----------------------------------------------------------------------------

float RectangleDistSq( vec3 p )
{ 
   if ( 0.0 <= p.y && p.y <= rsy )
       return p.x * p.x;
    
   if (p.y > rsy)
       return p.x*p.x + (p.y-rsy)*(p.y-rsy) ;
    
   return p.x*p.x + p.y*p.y ;
}
// -----------------------------------------------------------------------------

float BlendDistSq( float d1, float d2, float d3 )
{
   float dmin = min( d1, min(d2,d3)) ;
      
   return 0.5*dmin ; 
}
// -----------------------------------------------------------------------------

vec4 ColorF( float distSq, float angDeg )
{
   float b = min(1.0, 0.1/(sqrt(distSq)+0.1)),
         v = 0.5*(1.0+cos( 200.0*angDeg/360.0 + b*15.0*pi ));
     
   return vec4( b*b*b,b*b,0.0,distSq) ; // returns squared distance in alpha component
}
// -----------------------------------------------------------------------------

float Trunk4DistSq( vec3 p )
{
   float d1 = RectangleDistSq( p ); 
    
   return d1 ;   
}

// -----------------------------------------------------------------------------

float Trunk3DistSq( vec3 p )
{
   float d1 = RectangleDistSq( p ),
         d2 = Trunk4DistSq( tran1*p ),
         d3 = Trunk4DistSq( tran2*p );
      
   return BlendDistSq( d1, d2, d3 ) ;  
}

// -----------------------------------------------------------------------------

float Trunk2DistSq( vec3 p )
{
   float d1 = RectangleDistSq( p ), 
         d2 = Trunk3DistSq( tran1*p ),
         d3 = Trunk3DistSq( tran2*p );
    
   return BlendDistSq( d1, d2, d3 ) ;  
}

// -----------------------------------------------------------------------------

float Trunk1DistSq( vec3 p )
{
   float d1 = RectangleDistSq( p ) ,
         d2 = Trunk2DistSq( tran1*p ),
         d3 = Trunk2DistSq( tran2*p );
    
   return BlendDistSq( d1, d2, d3 ) ;  
}

// -----------------------------------------------------------------------------

float Trunk0DistSq( vec3 p )
{
   float d1 = RectangleDistSq( p ) ,
         d2 = Trunk1DistSq( tran1*p ),
         d3 = Trunk1DistSq( tran2*p );
    
   return BlendDistSq( d1, d2, d3 ) ;  
}
// -----------------------------------------------------------------------------
// compute the color and distance to tree, for a point in NDC coords

vec4 ComputeColorNDC( vec3 p, float angDeg )
{   
   vec2 org = vec2(0.5,0.5) ;
   vec4 col = vec4( 0.0, 0.0, 0.0, 1.0 );
   float dmin ;
    
   for( int i = 0 ; i < 4 ; i++ )
   {
      mat3 m = ChangeFrameToMat( org, angDeg + float(i)*90.0, 0.7 ); 
   	  vec3 p_transf = m*p ;
      float dminc = Trunk0DistSq( p_transf ) ;
            
      if ( i == 0 )
         dmin = dminc ;
      else if ( dminc < dmin )
         dmin = dminc ;
   }  
   return ColorF( dmin, angDeg ); // returns squared dist in alpha component
}
// -----------------------------------------------------------------------------

vec3 ComputeNormal( vec3 p, float dd, float ang, vec4 c00 )
{
   vec4   //c00  = ComputeColorNDC( p, ang )  ,
         c10  = ComputeColorNDC( p + vec3(dd,0.0,0.0), ang )  ,
         c01  = ComputeColorNDC( p + vec3(0.0,dd,0.0) , ang ) ;
   float h00  = sqrt(c00.a),
         h10  = sqrt(c10.a),
         h01  = sqrt(c01.a);
   vec3  tanx = vec3( dd, 0.0, h10-h00 ),
         tany = vec3( 0.0, dd, h01-h00 );
   vec3  n    = normalize( cross( tanx,tany ) );
       
   if ( n.z < 0.0 ) n *= -1.0 ; 
   return n ;
}

// -----------------------------------------------------------------------------

void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
   
   const float width = 0.1 ;

   
   vec2  res  = iResolution.xy ;
   float mind = min(res.x,res.y);
   vec2  pos  = fragCoord.xy ;
   float x0   = (res.x - mind)/2.0 ,
         y0   = (res.y - mind)/2.0 ,
         px   = pos.x - x0 ,
         py   = pos.y - y0 ;
      
  
   // compute 'tran1' and 'tran2':
    
   vec2  org1      = vec2( 0.0, rsy ) ;
   float ang1_deg  = +20.0 + 30.00*cos( 2.0*pi*iTime/4.05 ),
         scale1    = +0.85 +  0.40*cos( 2.0*pi*iTime/2.10 )  ;

   vec2  org2      = vec2( 0.0, rsy ) ;
   float ang2_deg  = -30.0 + 40.00*sin( 2.0*pi*iTime/2.52 ),
         scale2    = +0.75 +  0.32*sin( 2.0*pi*iTime/4.10 )  ;
   
   tran1 = ChangeFrameToMat( org1, ang1_deg, scale1 ) ;
   tran2 = ChangeFrameToMat( org2, ang2_deg, scale2 ) ; 
   
   // compute pixel color (pixCol)
    
   float mainAng = 360.0*iTime/15.0 ,    // main angle, proportional to time
         dd      = 1.0/float(mind) ;           // pixel width or height in ndc
   vec3  pixCen  = vec3( px*dd, py*dd, 1.0 ) ; // pixel center
   vec4  pixCol  = ComputeColorNDC( pixCen, mainAng ), 
         resCol  ;
   
   // compute output color as a function 'use_normal'
   
    const bool use_gradient = true ;
    
   if ( use_gradient )
   {
      vec3 nor     = ComputeNormal( pixCen, dd, mainAng, pixCol );
      vec4 gradCol = vec4( max(nor.x,0.0), max(nor.y,0.0), max(nor.z,0.0), 1.0 ) ;
       
      resCol = 0.8*pixCol+ 0.2*gradCol ;
   }
   else
      resCol = pixCol ;
      
       
   fragColor = vec4( resCol.rgb, 1.0 ) ;
}







