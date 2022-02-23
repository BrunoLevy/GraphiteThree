//stage GL_FRAGMENT_SHADER
//import <GLUP/ShaderToy.h>

float rnd(float x) { return fract(1000.*sin(234.56*x)); }
vec3 rnd3(float x) { return vec3(rnd(x),rnd(x+.1),rnd(x+.2)); }
float hash(float x,float y,float z) { return (x+432.432*y-1178.65*z); }
float hash(vec3 v) { return dot(v,vec3(1., 32.432, -1178.65)); }
    
vec4 Worley(vec3 uvw) {
    
   vec3 uvwi = floor(uvw);							// cell coords
   float dmin = 1e9, d2min=1e9, nmin=-1.;
    
    for (int i=-1; i<=1; i++)						// visit neighborhood
      for (int j=-1; j<=1; j++)						// to find the closest point
          for (int k=-1; k<=1; k++) 
          {
              vec3 c = uvwi + vec3(float(i),float(j),float(k)); // neighbor cells
              float n = hash(c);	 							// cell ID
              vec3 p = c + rnd3(n+.1);							// random point in cell
              float d = length(p-uvw);							// dist to point
              if (d<dmin) { d2min=dmin; dmin=d; nmin=n; }		// 2 closest dists
              else if (d<d2min) { d2min=d; }
          }
	return vec4(dmin,d2min,d2min-dmin, nmin);			// 2 closest dists + closest ID
}

void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
	vec3 uvw = vec3(2.*(fragCoord.xy / iResolution.y-vec2(.9,.5)), .02*iTime);
    float a = .2*iTime,c=cos(a),s=sin(a); uvw.xy *= mat2(c,-s,s,c);	// rotate
    uvw *= 20.*(.7+.5*vec3(vec2(cos(.5*iTime)),0.));					// zoom

    vec4 wor = Worley(uvw);          
    vec3 col, ccol = mix(vec3(1.), rnd3(wor.a+.4), wor.z);
    int mode = int(mod(.25*iTime,3.));								// demo mode
    if      (mode==0) col = vec3(wor.x);
    else if (mode==1) col = vec3(wor.y);
    else              col = vec3(wor.z);
    //col = pow(col,vec3(4.));   
    //col = 1.- pow(1.-col,vec3(4.));   
    col *= ccol;
	fragColor = vec4(col,1.0);
}