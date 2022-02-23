//stage GL_FRAGMENT_SHADER
//import <GLUP/ShaderToy.h>

//using some nice voronoi maths from here by csbdev https://www.shadertoy.com/view/MdtBz2

#define speed .2

float chebychev(vec3 a, vec3 b)
{
    return max(max(abs(a.x - b.x), abs(a.y - b.y)), abs(a.z - b.z));
}


float manhattan(vec3 a, vec3 b)
{
    vec3 d = abs(a - b);
    return d.x + d.y + d.z;
}

// iq
vec3 random3f( vec3 p )
{
    return fract(sin(vec3( dot(p,vec3(1.0,57.0,113.0)), 
                           dot(p,vec3(57.0,113.0,1.0)),
                           dot(p,vec3(113.0,1.0,57.0))))*43758.5453);
}

float voronoi3(vec3 p)
{
    vec3 fp = floor(p);
    
    float d1 = 1./0.;
    float d2 = 1./0.;
    
    for(int i = -1; i < 2; i++)
    {
        for(int j = -1; j < 2; j++)
        {
            for(int k = -1; k < 2; k++)
            {
                vec3 cur_p = fp + vec3(i, j, k);
                
                vec3 r = random3f(cur_p);
                
                float cd = 0.0;                    
                cd = chebychev(p, cur_p + r);
                d2 = min(d2, max(cd, d1));
                d1 = min(d1, cd);
            }
        }
    }
    return clamp(max(0.0, 16.0 * (d2-d1)), 0.0, 1.0);
}


void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
    float piT = iTime*3.14159*speed;
    float linearT = iTime*speed;
    fragCoord -= iResolution.xy*.5;
    
    vec2 uv = fragCoord/iResolution.y;
    
    uv*=20.+pow(smoothstep(.3, -.3, sin(piT+.8-length(uv)*.1)), 1.)*100.;
    
    
	vec3 c = vec3(0.);
    uv = abs(uv);
    uv.x-=linearT*.5;
    float d = uv.x + uv.y;
    linearT-= d*.1;
    uv-=33.333;
    vec2 off = vec2(sin(cos(piT)-d), cos(sin(piT)+d))*.05;
    float amp = 1.;
    for(float i = 2.; i<8.; i++){
        vec3 m = vec3(
    	voronoi3(vec3(uv,i*5.+ abs(mod(linearT+.15, 2.5)-1.25))),
    	voronoi3(vec3(uv,i*5.+ abs(mod(linearT+.1, 2.5)-1.25))),
    	voronoi3(vec3(uv,i* 5.+ abs(mod(linearT+.05, 2.5)-1.25)))  //*/      
        );
        m = pow(m, vec3(2.));
    	uv*= .666;
        uv+=i;
        if(floor(mod(i, 2.))==1.){
            c*=m;
            uv.x+=sin(piT)*amp;
            //uv.y+=cos(t)*amp;
        }else{
            c+=m;
            uv.x-=sin(piT)*amp;
            //uv.y-=cos(t)*amp;
        }
    
    }
    c = c*.333;
    // Output to screen
    fragColor = vec4(c,1.0);
}
