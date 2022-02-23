//stage GL_FRAGMENT_SHADER
//import <GLUP/ShaderToy.h>

#define ANIMATE

// How far cells can go off center during animation (must be <= .5)
#define ANIMATE_D .5

// Points cannot be closer than sqrt(EPSILON)
#define EPSILON .00001

struct vor{
    vec2 p1;
    vec2 p2;
    vec2 cell1;
    vec2 cell2;
    float distToBorder;
    
    mat4 pointDistances;
};
    float rand(float n){return fract(sin(n) * 43758.5453123);}
float weightFunction(float i)
{
    return 1./(pow(i,3.));
}
float matSum(mat4 m)
{
    float res;
    for (int i = 0; i < 4;i++)
    {
        for (int j = 0; j < 4;j++)
        {
            res += weightFunction(m[i][j]);
        }
    }
    return res;
}


vec3 colors[5] =vec3[]( vec3(1,0,0),vec3(0,1,0),vec3(0,0,1),vec3(1,1,0),vec3(1,0,1));
vec2 hash2(vec2 p)
{
       // Dave Hoskin's hash as in https://www.shadertoy.com/view/4djSRW
       vec3 p3 = fract(vec3(p.xyx) * vec3(.1031, .1030, .0973));
       p3 += dot(p3, p3.yzx+19.19);
       vec2 o = fract(vec2((p3.x + p3.y)*p3.z, (p3.x+p3.z)*p3.y));
    #ifdef ANIMATE
       o = 0.5 + ANIMATE_D*sin( iTime + o*6.2831853 );
    #endif
   return o;
   
}



float noise12(vec2 pi)
{
	vec3 p = vec3(pi,0);
	vec3 ip=floor(p);
    p-=ip; 
    vec3 s=vec3(7,157,113);
    vec4 h=vec4(0.,s.yz,s.y+s.z)+dot(ip,s);
    p=p*p*(3.-2.*p); 
    h=mix(fract(sin(h)*43758.5),fract(sin(h+s.x)*43758.5),p.x);
    h.xy=mix(h.xz,h.yw,p.y);
    return mix(h.x,h.y,p.z); 
}


//---------------------------------------------------------------
// 4x4 scan in both passes = most accurate
//---------------------------------------------------------------

vor voronoi( in vec2 x )
{
    
    
	vor res;
    // slower, but better handles big numbers
    vec2 n = floor(x);
    vec2 f = fract(x);
    vec2 h = step(.5,f) - 2.;
    n += h; f -= h;

    //----------------------------------
    // first pass: regular voronoi
    //----------------------------------
	vec2 mr;

    float md = 8.0;
    for( int j=0; j<=3; j++ )
    for( int i=0; i<=3; i++ )
    {
        vec2 g = vec2(float(i),float(j));
        vec2 o = hash2( n + g );
        vec2 r = g + o - f;
        float d = dot(r,r);

        if( d<md )
        {
            md = d;
            mr = r;
            res.p1 = g+o+n;
            res.cell1 = g+n;
        }
        res.pointDistances[i][j]=d;
        
        
        
        
        
    }

    //----------------------------------
    // second pass: distance to borders
    //----------------------------------
    md = 8.0;
    for( int j=0; j<=3; j++ )
    for( int i=0; i<=3; i++ )
    {
        vec2 g = vec2(float(i),float(j));
        vec2 o = hash2( n + g );
        vec2 r = g + o - f;

        if( dot(mr-r,mr-r)>EPSILON )
        {
            float newD = dot( 0.5*(mr+r), normalize(r-mr) );
            if (newD < md)
            {
                md = newD;
                res.p2 = g+o+n;
                res.cell2 = g+n;
            }
        }// skip the same cell
    }
    res.distToBorder = md;
    
    
    
    
    
   
    return res;
}


vec3 plot( vec2 p, float ss )
{
    vec3 col;
    
    vor O = voronoi(p);
    
    vec2 n = floor(p);
    vec2 f = fract(p);
    vec2 h = step(.5,f) - 2.;
    n += h; f -= h;
    
    float S = matSum(O.pointDistances);
    for (int i = 0; i <= 3;i++)
    {
        for (int j = 0; j <= 3;j++)
        {
            float w = weightFunction(O.pointDistances[i][j]);
                if (w/S > 0.01)
                {
                    
                    vec3 thisCol = colors[int(5.0*noise12(n+vec2(i,j)))];
            		col += thisCol*w/S;
                }
        }
        
    }
    
    
   
    
    
    
    
    //col=c1;
    if (O.distToBorder < 0.01)
    {
        col = vec3(1);
    }
    if (length(O.p1-p) < 0.03 )
    {
        col = vec3(1);
    }
    
    
    if (iMouse.z > 0.) // show grid
    {
		vec2 g = abs(fract(p)-.5);
        col = mix(col,vec3(.8),smoothstep(.5-ss*1.5,.5,max(g.x,g.y)));
    }
    return col;
}

void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
    float sc = step(512., iResolution.y)*4. + 4.; // scale differently for fullscreen
	float ss = sc / iResolution.y; // size of 1 pixel
    vec2 uv = (fragCoord.xy - iResolution.xy*.5) * ss;
    fragColor = vec4(plot(uv, ss), 1.);
}