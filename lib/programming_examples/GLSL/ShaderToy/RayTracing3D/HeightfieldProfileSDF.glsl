//stage GL_FRAGMENT_SHADER
//import <GLUP/ShaderToy.h>

// A small shader to demonstrate SDF modelling with two 2D fields.
// Check out https://www.shadertoy.com/view/MsdBDj for a similar way of modelling with 2D maps.
//
// Click mouse button to see the result full-screen.
//

#define SHAPE	2	// Can be 0, 1, or 2
#define AA		1	// Anti-aliasing box size

float box(vec2 p,vec2 s)
{
    p=abs(p)-s;
    return max(p.x,p.y);
}

// The next sets of functions define two 2D distance field functions. Each set has a heightfield function
// and a profile function. The parameters to the heightfield function are the X and Z coordinate of the point.
// The parameters to the profile function are the Y coordinate, and the value returned from the heightfield function.
// The heightfield is not really the height of a surface, but just the 'height' of the coordinate that the profile
// shape is sampled by.
#if SHAPE==0

float heightfield(vec2 p)
{
    return box(p,vec2(0));
}

float profile(vec2 p)
{
    return box(p,vec2(0))-.1;
    return length(p)-.1;
}

#elif SHAPE==1

float heightfield(vec2 p)
{
    p=abs(p)-.15;
    return box(p,vec2(0));
}

float profile(vec2 p)
{
   return max(box(p-vec2(0,.03),vec2(0,-.02))-.1+p.y/2.,-box(p-vec2(.15,0.0),vec2(.07,.02)));
}

#else

float heightfield(vec2 p)
{
    p=abs(p)-.15;
    return min(box(p,vec2(0)),box(p+.1,vec2(0,.2)));
}

float profile(vec2 p)
{
    p*=2.;
    float s=1.5,ds=1.;
    for(int i=0;i<3;++i)
    {
        p=abs(p)-.05;
        float a=3.1415926535/4.;
        p*=mat2(cos(a),sin(a),-sin(a),cos(a))*s;
        ds*=s;
    }
    return box(p,vec2(.4,.03))/ds/2.;
}

#endif

// The composed 3D signed distance field.
float f(vec3 p)
{
    return min(p.y+.1,profile(vec2(heightfield(p.xz),p.y)));
}

// Orthonormal basis, based on code from PBRT.
mat3 coordinateSystem(vec3 w)
{
    vec3 u = cross(w, vec3(1,0,0));
    if (dot(u, u) < 1e-6)
        u = cross(w, vec3(0,1,0));
    return mat3(normalize(u), normalize(cross(u, w)), normalize(w));
}

// Signed distance field normal direction.
vec3 getNormal(vec3 p)
{
    float d=f(p);
    const vec2 e=vec2(1e-3,0);
    vec3 n=vec3(f(p+e.xyy)-d,f(p+e.yxy)-d,f(p+e.yyx)-d);
    return n;
}

// An approximate 'curvature' value (basically the amount of local change in the field normal).
// It's not perfect, but I'm still working on it...
float curvature(vec3 p)
{
    vec3 n=getNormal(p);

    mat3 m=coordinateSystem(n);

    const float e=1e-4;
    vec3 n1=getNormal(p+m[0]*e);
    vec3 n2=getNormal(p+m[1]*e);
    vec3 n3=getNormal(p-m[0]*e);
    vec3 n4=getNormal(p-m[1]*e);

    return (length((n1-n3)/e)+length((n2-n4)/e))*5.;
}

// From IQ.
float ambientOcclusion(vec3 p, vec3 n)
{
    const int steps = 9;
    const float delta = .18;

    float a = 0.0;
    float weight = .5;
    for(int i=1; i<=steps; i++) {
        float d = (float(i) / float(steps)) * delta; 
        a += weight*(d - f(p + n*d));
        weight *= 0.99;
    }
    return clamp(1.0 - a, 0.0, 1.0);
}

vec3 distanceColourMap(float d)
{
    float lines=mix(.5,1.,smoothstep(-.5,.5,(min(fract(d*64.),1.-fract(d*64.))-.5)*3.)*2.);
    return clamp(mix(vec3(lines)*vec3(.1),vec3(.5,.5,1.),step(d,0.))+d/3.+(1.-smoothstep(.00,.002,abs(d)-.002)),0.,1.);
}

// Analytically triangle-filtered checkerboard, from IQ
// http://iquilezles.org/www/articles/morecheckerfiltering/morecheckerfiltering.htm
vec3 pri( in vec3 x )
{
    // see https://www.shadertoy.com/view/MtffWs
    vec3 h = fract(x/2.0)-0.5;
    return x*0.5 + h*(1.0-2.0*abs(h));
}

float checkersTextureGradTri( in vec3 p, in vec3 ddx, in vec3 ddy )
{
    vec3 w = max(abs(ddx), abs(ddy)) + 0.01;       // filter kernel
    vec3 i = (pri(p+w)-2.0*pri(p)+pri(p-w))/(w*w); // analytical integral (box filter)
    return 0.5 - 0.5*i.x*i.y*i.z;                  // xor pattern
}

void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
    float an=.7;
    vec2 t=fragCoord/iResolution.xy*2.-1.;

    t.x*=iResolution.x/iResolution.y;

    if(abs(t.x)>1.)
    {
        fragColor=vec4(0,0,0,1);
        return;
    }

    float full = step(0.5,iMouse.z);

    if(full>.5)
    {
        t.y=t.y+.5;
    }

    // Check which quadrant the pixel is in.
    if(t.x<0.&&t.y<0.&&full<.5)
    {
        fragColor.rgb=distanceColourMap(heightfield((t.xy*.5+.25)*1.5));
    }
    else if(t.x>0.&&t.y<0.&&full<.5)
    {
        fragColor.rgb=distanceColourMap(profile((t.xy*.5+.25*vec2(-1.,1.))*1.5));
    }
    else
    {
        // Anti-aliasing loop.
        vec3 c=vec3(0);
        for(int y=-AA;y<=AA;++y)
            for(int x=-AA;x<=AA;++x)
            {
                float u=float(x)*.25,v=float(y)*.25;

                // Set up the primary ray.
                vec3 ro=vec3(0.,0.,1.25);
                vec3 rd=normalize(vec3((t.xy+dFdx(t.xy)*u+dFdy(t.xy)*v)*vec2(.5,.25)-.125*vec2(0,1),-1.));

                rd.xz=mat2(cos(an),sin(an),sin(an),-cos(an))*rd.xz;
                ro.xz=mat2(cos(an),sin(an),sin(an),-cos(an))*ro.xz;

                // Get the ray directions of neighbouring pixels for this sample.
                vec3 ddx_rd = rd + dFdx(rd);
                vec3 ddy_rd = rd + dFdy(rd);

                float s=20.;

                // Trace primary ray.
                float t=0.,d=0.;
                for(int i=0;i<100;++i)
                {
                    d=f(ro+rd*t);
                    if(d<1e-3||t>10.)break;
                    t+=d;
                }


                vec3 rp=ro+rd*t;
                vec3 n = normalize(getNormal(rp));

                // Compute ray differentials (based on code from IQ)
                vec3 ddx_pos = ro - ddx_rd*dot(ro-rp,n)/dot(ddx_rd,n);
                vec3 ddy_pos = ro - ddy_rd*dot(ro-rp,n)/dot(ddy_rd,n);

                // Calc texture sampling footprint (based on code from IQ)
                vec3     uvw = rp*32.;
                vec3 ddx_uvw = ddx_pos*32. - uvw;
                vec3 ddy_uvw = ddy_pos*32. - uvw;

                // Basic light value.
                float l=.7+.3*dot(n,normalize(vec3(1)));

                // Ambient occlusion.
                l*=ambientOcclusion(rp,n);

                // Depth cueing.
                l*=exp(-t/1.5)*2.;

                // Darken the geometric edges.
                l*=clamp(1.-curvature(rp),0.,1.);

                // Get a directional shadow.
                t=0.;
                d=0.;
                ro=rp+n*2e-3;
                rd=normalize(vec3(1,.3,1));
                float mind=1e4;
                for(int i=0;i<40;++i)
                {
                    d=f(ro+rd*t);
                    mind=min(mind,d);
                    if(d<1e-3||t>10.)break;
                    t+=d;
                }

                l*=mix(.3,1.,smoothstep(0.001,0.0015,mind));

                // Apply the filtered checkerboard texture.
                c+=l*(.5+.5*checkersTextureGradTri( uvw, ddx_uvw, ddy_uvw ));
            }
        fragColor.rgb=c/((float(AA)*2.+1.)*(float(AA)*2.+1.));
    }

    if(full<.5)
    {
        // Add separation lines.
        fragColor.rgb=mix(fragColor.rgb,vec3(.1),step(t.y,0.)*step(min(abs(t.x),abs(t.y)),.004));
    }

    fragColor.rgb=sqrt(max(fragColor.rgb,0.));
}

