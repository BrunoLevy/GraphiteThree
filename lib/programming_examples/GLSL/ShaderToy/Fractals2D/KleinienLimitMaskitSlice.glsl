//stage GL_FRAGMENT_SHADER
//import <GLUP/ShaderToy.h>

//by Jos Leys
vec3  background1Color=vec3(1.0,1.0,1.0);
vec3  color3=vec3(0.2,0.0,0.6);

float box_size_x=1.;

float wrap(float x, float a, float s){
	x -= s; 
	return (x-a*floor(x/a)) + s;
}

void TransA(inout vec2 z, float a, float b){
	float iR = 1. / dot(z,z);
	z *= -iR;
	z.x = -b - z.x; z.y = a + z.y; 
	
}

float  JosKleinian(vec2 z)
{
	vec2 lz=z+vec2(1.), llz=z+vec2(-1.);
    float flag=0.;
 float KleinR = 1.8462756+(1.958591-1.8462756)*0.5+0.5*(1.958591-1.8462756)*sin(-iTime*0.2);  
 float KleinI = 0.09627581+(0.0112786-0.09627581)*0.5+0.5*(0.0112786-0.09627581)*sin(-iTime*0.2);
      
	float a = KleinR;
    float b = KleinI;
	float f = sign(b)*1. ;     
	for (int i = 0; i < 150 ; i++) 
	{
                z.x=z.x+f*b/a*z.y;
		z.x = wrap(z.x, 2. * box_size_x, - box_size_x);
		z.x=z.x-f*b/a*z.y;
                       
		//If above the separation line, rotate by 180° about (-b/2, a/2)
        if  (z.y >= a * 0.5 + f *(2.*a-1.95)/4. * sign(z.x + b * 0.5)* (1. - exp(-(7.2-(1.95-a)*15.)* abs(z.x + b * 0.5))))	
        {z = vec2(-b, a) - z;}
        
		//Apply transformation a
		TransA(z, a, b);
		
        //
		//If the iterated points enters a 2-cycle , bail out.
        if(dot(z-llz,z-llz) < 1e-6) {break;}
        //if the iterated point gets outside z.y=0 and z.y=a
        if(z.y<0. || z.y>a){flag=1.; break;}
        //Store prévious iterates
		llz=lz; lz=z;
	}

	return flag;
}


void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
    
   	vec2 uv = fragCoord.xy / iResolution.xy;
     uv = (1.99)*uv-vec2(0.42,.0);
   
    uv.x *= iResolution.x/iResolution.y;
    float hit=JosKleinian(uv);
      vec3 c =(1.-hit)*background1Color+hit*color3;
   
	fragColor = vec4(c, 1.0);
    
}


