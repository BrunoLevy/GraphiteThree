//stage GL_FRAGMENT_SHADER
//import <GLUP/ShaderToy.h>
/*
	Desert Sand
	-----------

	Sand, and more sand -- Monotony, at it's finest. :) I've visited and flown over many sandy 
	regions in my time, and I can say unequivocably that this particular scene doesn't remotely  
	resemble the real thing. :D 

	Having said that, there's something that I really like about minimal artificial dune scenes.  
	They're somewhat of a visual programmer cliche -- I think that's partly due to the fact that 
	they have a decent "aesthetic appeal to algorithmic complexity" ratio.
	
	For the sand dunes, rather than deal with dune physics, I used a pretty standard smoothstep 
	noise layer formula. I wouldn't say that dune physics is particularly difficult, but I'd 
	rather leave that to people like Dr2. :) Besides, with this example, I wanted to save some
	cycles and focus on the sand texture generation.

	There are so many different ways to create wavy sand patterns. Some are expensive -- using
	things like gradient erosion -- and some are cheap. Not suprisingly, the expensive methods 
	tend to look better. I bump mapped the sand layer to give myself a few extra cycles to play 
	with, but I still had to keep things relatively simple.

	The pattern you see is a mixture of a simple trick I've seen around and some of my own
	adjustments. Without going into detail, the idea is to create a layer of repeat rounded 
	gradient lines, and another rotated at a slight angle, then perturb them slightly and mix 
	together using an underlying noise layer. It's simple, but reasonably effective. Anyway, 
	I've explained in more detail in the "sand" function below.

	By the way, with some tweaking, grouping, rearranging, etc, I'm pretty sure I could make this 
	run faster. However, it'd be at the expense of the readability of the sand texture function,
	so for now, I'm going to leave it alone. Anyway, I'm going to produce a few surface realated
	examples later.
	

	Related examples:

	// It won Breakpoint way back in 2009. Ten people on Pouet gave it the thumbs down -- I hope
	// they put their work up on Shadertoy, because I'd imagine it'd be insanely good. :D
	Elevated - IQ
	https://www.shadertoy.com/view/MdX3Rr


	// One of my favorite simple coloring jobs.
    Skin Peeler - Dave Hoskins
    https://www.shadertoy.com/view/XtfSWX
    Based on one of my all time favorites:
    Xyptonjtroz - Nimitz
	https://www.shadertoy.com/view/4ts3z2

*/

// The far plane. I'd like this to be larger, but the extra iterations required to render the 
// additional scenery starts to slow things down on my slower machine.
#define FAR 80.


// Fabrice's concise, 2D rotation formula.
//mat2 rot2(float th){ vec2 a = sin(vec2(1.5707963, 0) + th); return mat2(a, -a.y, a.x); }
// Standard 2D rotation formula - Nimitz says it's faster, so that's good enough for me. :)
mat2 rot2(in float a){ float c = cos(a), s = sin(a); return mat2(c, s, -s, c); }


// 3x1 hash function.
float hash( vec3 p ){ return fract(sin(dot(p, vec3(21.71, 157.97, 113.43)))*45758.5453); }


// IQ's smooth minium function. 
float smin(float a, float b , float s){
    
    float h = clamp( 0.5 + 0.5*(b-a)/s, 0. , 1.);
    return mix(b, a, h) - h*(1.0-h)*s;
}

// Smooth maximum, based on IQ's smooth minimum.
float smax(float a, float b, float s){
    
    float h = clamp( 0.5 + 0.5*(a-b)/s, 0., 1.);
    return mix(b, a, h) + h*(1.0-h)*s;
}


/*
// Dave's hash function. More reliable with large values, but will still eventually break down.
//
// Hash without Sine
// Creative Commons Attribution-ShareAlike 4.0 International Public License
// Created by David Hoskins.
// vec2 to vec2.
vec2 hash22(vec2 p){

	vec3 p3 = fract(vec3(p.xyx) * vec3(.1031, .1030, .0973));
    p3 += dot(p3, p3.yzx + 19.19);
    p = fract((p3.xx + p3.yz)*p3.zy)*2. - 1.;
    return p;
    
    
    // Note the "mod" call. Slower, but ensures accuracy with large time values.
    //mat2  m = r2(mod(iTime, 6.2831853)); 
	//p.xy = m * p.xy;//rotate gradient vector
  	//return p;
    

}
*/


#define RIGID
// Standard 2x2 hash algorithm.
vec2 hash22(vec2 p) {
    
    // Faster, but probaly doesn't disperse things as nicely as other methods.
    float n = sin(dot(p, vec2(113, 1)));
    p = fract(vec2(2097152, 262144)*n)*2. - 1.;
    #ifdef RIGID
    return p;
    #else
    return cos(p*6.283 + iGlobalTime);
    //return abs(fract(p+ iGlobalTime*.25)-.5)*2. - .5; // Snooker.
    //return abs(cos(p*6.283 + iGlobalTime))*.5; // Bounce.
    #endif

}



// Gradient noise. Ken Perlin came up with it, or a version of it. Either way, this is
// based on IQ's implementation. It's a pretty simple process: Break space into squares, 
// attach random 2D vectors to each of the square's four vertices, then smoothly 
// interpolate the space between them.
float gradN2D(in vec2 f){
    
    // Used as shorthand to write things like vec3(1, 0, 1) in the short form, e.yxy. 
   const vec2 e = vec2(0, 1);
   
    // Set up the cubic grid.
    // Integer value - unique to each cube, and used as an ID to generate random vectors for the
    // cube vertiies. Note that vertices shared among the cubes have the save random vectors attributed
    // to them.
    vec2 p = floor(f);
    f -= p; // Fractional position within the cube.
    

    // Smoothing - for smooth interpolation. Use the last line see the difference.
    //vec2 w = f*f*f*(f*(f*6.-15.)+10.); // Quintic smoothing. Slower and more squarish, but derivatives are smooth too.
    vec2 w = f*f*(3. - 2.*f); // Cubic smoothing. 
    //vec2 w = f*f*f; w = ( 7. + (w - 7. ) * f ) * w; // Super smooth, but less practical.
    //vec2 w = .5 - .5*cos(f*3.14159); // Cosinusoidal smoothing.
    //vec2 w = f; // No smoothing. Gives a blocky appearance.
    
    // Smoothly interpolating between the four verticies of the square. Due to the shared vertices between
    // grid squares, the result is blending of random values throughout the 2D space. By the way, the "dot" 
    // operation makes most sense visually, but isn't the only metric possible.
    float c = mix(mix(dot(hash22(p + e.xx), f - e.xx), dot(hash22(p + e.yx), f - e.yx), w.x),
                  mix(dot(hash22(p + e.xy), f - e.xy), dot(hash22(p + e.yy), f - e.yy), w.x), w.y);
    
    // Taking the final result, and converting it to the zero to one range.
    return c*.5 + .5; // Range: [0, 1].
}

// Gradient noise fBm.
float fBm(in vec2 p){
    
    return gradN2D(p)*.57 + gradN2D(p*2.)*.28 + gradN2D(p*4.)*.15;
    
}


// Cheap and nasty 2D smooth noise function with inbuilt hash function - based on IQ's 
// original. Very trimmed down. In fact, I probably went a little overboard. I think it 
// might also degrade with large time values. I'll swap it for something more robust later.
float n2D(vec2 p) {

	vec2 i = floor(p); p -= i; 
    //p *= p*p*(p*(p*6. - 15.) + 10.);
    p *= p*(3. - p*2.);  
    
	return dot(mat2(fract(sin(vec4(0, 1, 113, 114) + dot(i, vec2(1, 113)))*43758.5453))*
                vec2(1. - p.y, p.y), vec2(1. - p.x, p.x) );

}


// Repeat gradient lines. How you produce these depends on the effect you're after. I've used a smoothed
// triangle gradient mixed with a custom smoothed gradient to effect a little sharpness. It was produced
// by trial and error. If you're not sure what it does, just call it individually, and you'll see.
float grad(float x, float offs){
    
    // Repeat triangle wave. The tau factor and ".25" factor aren't necessary, but I wanted its frequency
    // to overlap a sine function.
    x = abs(fract(x/6.283 + offs - .25) - .5)*2.;
    
    float x2 = clamp(x*x*(-1. + 2.*x), 0., 1.); // Customed smoothed, peaky triangle wave.
    //x *= x*x*(x*(x*6. - 15.) + 10.); // Extra smooth.
    x = smoothstep(0., 1., x); // Basic smoothing - Equivalent to: x*x*(3. - 2.*x).
    return mix(x, x2, .15);
    
/*    
    // Repeat sine gradient.
    float s = sin(x + 6.283*offs + 0.);
    return s*.5 + .5;
    // Sine mixed with an absolute sine wave.
    //float sa = sin((x +  6.283*offs)/2.);
    //return mix(s*.5 + .5, 1. - abs(sa), .5);
    
*/
}

// One sand function layer... which is comprised of two mixed, rotated layers of repeat gradients lines.
float sandL(vec2 p){
    
    // Layer one. 
    vec2 q = rot2(3.14159/18.)*p; // Rotate the layer, but not too much.
    q.y += (gradN2D(q*18.) - .5)*.05; // Perturb the lines to make them look wavy.
    float grad1 = grad(q.y*80., 0.); // Repeat gradient lines.
   
    q = rot2(-3.14159/20.)*p; // Rotate the layer back the other way, but not too much.
    q.y += (gradN2D(q*12.) - .5)*.05; // Perturb the lines to make them look wavy.
    float grad2 = grad(q.y*80., .5); // Repeat gradient lines.
      
    
    // Mix the two layers above with an underlying 2D function. The function you choose is up to you,
    // but it's customary to use noise functions. However, in this case, I used a transcendental 
    // combination, because I like the way it looked better.
    // 
    // I feel that rotating the underlying mixing layers adds a little variety. Although, it's not
    // completely necessary.
    q = rot2(3.14159/4.)*p;
    //float c = mix(grad1, grad2, smoothstep(.1, .9, n2D(q*vec2(8))));//smoothstep(.2, .8, n2D(q*8.))
    //float c = mix(grad1, grad2, n2D(q*vec2(6)));//smoothstep(.2, .8, n2D(q*8.))
    //float c = mix(grad1, grad2, dot(sin(q*12. - cos(q.yx*12.)), vec2(.25)) + .5);//smoothstep(.2, .8, n2D(q*8.))
    
    // The mixes above will work, but I wanted to use a subtle screen blend of grad1 and grad2.
    float a2 = dot(sin(q*12. - cos(q.yx*12.)), vec2(.25)) + .5;
    float a1 = 1. - a2;
    
    // Screen blend.
    float c = 1. - (1. - grad1*a1)*(1. - grad2*a2);
    
    // Smooth max\min
    //float c = smax(grad1*a1, grad2*a2, .5);
   
    return c;
    
    
}

// A global value to record the distance from the camera to the hit point. It's used to tone
// down the sand height values that are further away. If you don't do this, really bad
// Moire artifacts will arise. By the way, you should always avoid globals, if you can, but
// I didn't want to pass an extra variable through a bunch of different functions.
float gT;

float sand(vec2 p){
    
    // Rotating by 45 degrees. I thought it looked a little better this way. Not sure why.
    // I've also zoomed in by a factor of 4.
    p = vec2(p.y - p.x, p.x + p.y)*.7071/4.;
    
    // Sand layer 1.
    float c1 = sandL(p);
    
    // Second layer.
    // Rotate, then increase the frequency -- The latter is optional.
    vec2 q = rot2(3.14159/12.)*p;
    float c2 = sandL(q*1.25);
    
    // Mix the two layers with some underlying gradient noise.
    c1 = mix(c1, c2, smoothstep(.1, .9, gradN2D(p*vec2(4))));
    
/*   
	// Optional screen blending of the layers. I preferred the mix method above.
    float a2 = gradN2D(p*vec2(4));
    float a1 = 1. - a2;
    
    // Screen blend.
    c1 = 1. - (1. - c1*a1)*(1. - c2*a2);
*/    
    
    // Extra grit. Not really necessary.
    //c1 = .7 + fBm(p*128.)*.3;
    
    // A surprizingly simple and efficient hack to get rid of the super annoying Moire pattern 
    // formed in the distance. Simply lessen the value when it's further away. Most people would
    // figure this out pretty quickly, but it took me far too long before it hit me. :)
    return c1/(1. + gT*gT*.015);
}

/////////


// The path is a 2D sinusoid that varies over time, which depends upon the frequencies and amplitudes.
vec2 path(in float z){ 
    
    return vec2(4.*sin(z * .1), 0);
}

// The standard way to produce "cheap" dunes is to apply a triangle function to individual
// noise layers varying in amplitude and frequency. However, I needed something more subtle
// and rounder, so I've only applied a triangle function to the middle layer.
// 
// Here's an example using a more standard routine that's worth taking a look at:
//
// desert - wachel
// https://www.shadertoy.com/view/ltcGDl
float surfFunc( in vec3 p){
    
    p /= 2.5;
    
    // Large base ampltude with lower frequency.
    float layer1 = n2D(p.xz*.2)*2. - .5; // Linear-like discontinuity - Gives an edge look.
    layer1 = smoothstep(0., 1.05, layer1); // Smoothing the sharp edge.

    // Medium amplitude with medium frequency. 
    float layer2 = n2D(p.xz*.275);
    layer2 = 1. - abs(layer2 - .5)*2.; // Triangle function, to give the dune edge look.
    layer2 = smoothstep(.2, 1., layer2*layer2); // Smoothing the sharp edge.
    
    // Smaller, higher frequency layer.
	float layer3 = n2D(p.xz*.5*3.);

     // Combining layers fBm style. Ie; Amplitudes inversely proportional to frequency.
    float res = layer1*.7 + layer2*.25 + layer3*.05;
    //float res = 1. - (1. - layer1*.7)*(1. - layer2*.25)*(1. - layer3*.05); // Screen 
    //float res = layer1*.75 + layer2*.25;

    return res;
    
}


// A similar -- trimmed down and smoothed out -- version of function above, for camera path usage.
float camSurfFunc( in vec3 p){
    
    p /= 2.5;
    
    // Large base ampltude with lower frequency.
    float layer1 = n2D(p.xz*.2)*2. - .5; // Linear-like discontinuity - Gives an edge look.
    layer1 = smoothstep(0., 1.05, layer1); // Smoothing the sharp edge.

    // Medium amplitude with medium frequency. 
    float layer2 = n2D(p.xz*.275);
    layer2 = 1. - abs(layer2 - .5)*2.; // Triangle function, to give the dune edge look.
    layer2 = smoothstep(.2, 1., layer2*layer2); // Smoothing the sharp edge.

     // Combining layers fBm style. Ie; Amplitudes inversely proportional to frequency.
    float res = (layer1*.7 + layer2*.25)/.95;
    //float res = 1. - (1. - layer1*.75)*(1. - layer2*.25); // Screen 

    return res;
    
}



// The desert scene. Adding a heightmap to an XZ plane. Not a complicated distance function. :)
float map(vec3 p){
    
	// Height map to perturb the flat plane. On a side note, I'll usually keep the
    // surface function within a zero to one range, which means I can use it later
    // for a bit of shading, etc. Of course, I could cut things down a bit, but at
    // the expense of confusion elsewhere... if that makes any sense. :)
    float sf = surfFunc(p);

    // Add the height map to the plane.
    return p.y + (.5-sf)*2.; 
 
}



// Basic raymarcher.
float trace(in vec3 ro, in vec3 rd){

    float t = 0., h;
    
    for(int i=0; i<96; i++){
    
        h = map(ro + rd*t);
        // Note the "t*b + a" addition. Basically, we're putting less emphasis on accuracy, as
        // "t" increases. It's a cheap trick that works in most situations... Not all, though.
        if(abs(h)<0.001*(t*.125 + 1.) || t>FAR) break; // Alternative: 0.001*max(t*.25, 1.), etc.
        
        t += h; 
    }

    return min(t, FAR);
}

/*
// Tetrahedral normal - courtesy of IQ. I'm in saving mode, so the two "map" calls saved make
// a difference. Also because of the random nature of the scene, the tetrahedral normal has the 
// same aesthetic effect as the regular - but more expensive - one, so it's an easy decision.
vec3 normal(in vec3 p)
{  
    vec2 e = vec2(-1., 1.)*0.001;   
	return normalize(e.yxx*map(p + e.yxx) + e.xxy*map(p + e.xxy) + 
					 e.xyx*map(p + e.xyx) + e.yyy*map(p + e.yyy) );   
}
*/

 
// Standard normal function. It's not as fast as the tetrahedral calculation, but more symmetrical.
vec3 normal(in vec3 p, float ef) {
	vec2 e = vec2(0.001*ef, 0);
	return normalize(vec3(map(p + e.xyy) - map(p - e.xyy), map(p + e.yxy) - map(p - e.yxy),	map(p + e.yyx) - map(p - e.yyx)));
}

/*
// Tri-Planar blending function. Based on an old Nvidia writeup:
// GPU Gems 3 - Ryan Geiss: https://developer.nvidia.com/gpugems/GPUGems3/gpugems3_ch01.html
vec3 tex3D(sampler2D t, in vec3 p, in vec3 n ){
    
    n = max(abs(n) - .2, 0.001);
    n /= dot(n, vec3(1));
	vec3 tx = texture(t, p.yz).xyz;
    vec3 ty = texture(t, fract(p.zx)).xyz;
    vec3 tz = texture(t, p.xy).xyz;

    // Textures are stored in sRGB (I think), so you have to convert them to linear space 
    // (squaring is a rough approximation) prior to working with them... or something like that. :)
    // Once the final color value is gamma corrected, you should see correct looking colors.
    return (tx*tx*n.x + ty*ty*n.y + tz*tz*n.z);
    
}


// Texture bump mapping. Four tri-planar lookups, or 12 texture lookups in total.
vec3 doBumpMap( sampler2D tx, in vec3 p, in vec3 n, float bf){
   
    const vec2 e = vec2(0.001, 0);
    
    // Three gradient vectors rolled into a matrix, constructed with offset greyscale texture values.    
    mat3 m = mat3( tex3D(tx, p - e.xyy, n), tex3D(tx, p - e.yxy, n), tex3D(tx, p - e.yyx, n));
    
    vec3 g = vec3(0.299, 0.587, 0.114)*m; // Converting to greyscale.
    g = (g - dot(tex3D(tx,  p , n), vec3(0.299, 0.587, 0.114)) )/e.x; g -= n*dot(n, g);
                      
    return normalize( n + g*bf ); // Bumped normal. "bf" - bump factor.
	
}
*/

// Compact, self-contained version of IQ's 3D value noise function. I have a transparent noise
// example that explains it, if you require it.
float n3D(in vec3 p){
    
	const vec3 s = vec3(113, 157, 1);
	vec3 ip = floor(p); p -= ip; 
    vec4 h = vec4(0., s.yz, s.y + s.z) + dot(ip, s);
    p = p*p*(3. - 2.*p); //p *= p*p*(p*(p * 6. - 15.) + 10.);
    h = mix(fract(sin(h)*43758.5453), fract(sin(h + s.x)*43758.5453), p.x);
    h.xy = mix(h.xz, h.yw, p.y);
    return mix(h.x, h.y, p.z); // Range: [0, 1].
}


// 3D noise fBm.
float fBm(in vec3 p){
    
    return n3D(p)*.57 + n3D(p*2.)*.28 + n3D(p*4.)*.15;
    
}


// Surface bump function..
float bumpSurf3D( in vec3 p){
    
    // Obtaining some terrain samples in order to produce a gradient
    // with which to distort the sand. Basically, it'll make it look
    // like the underlying terrain it effecting the sand. The downside
    // is the three extra taps per bump tap... Ouch. :) Actually, it's
    // not that bad, but I might attempt to come up with a better way.
    float n = surfFunc(p);
    vec3 px = p + vec3(.001, 0, 0);
    float nx = surfFunc(px);
    vec3 pz = p + vec3(0, 0, .001);
    float nz = surfFunc(pz);
    
    // The wavy sand, that have been perturbed by the underlying terrain.
    return sand(p.xz + vec2(n - nx, n - nz)/.001*1.);

}

// Standard function-based bump mapping routine: This is the cheaper four tap version. There's
// a six tap version (samples taken from either side of each axis), but this works well enough.
vec3 doBumpMap(in vec3 p, in vec3 nor, float bumpfactor){
    
    // Larger sample distances give a less defined bump, but can sometimes lessen the aliasing.
    const vec2 e = vec2(0.001, 0); 
    
    // Gradient vector: vec3(df/dx, df/dy, df/dz);
    float ref = bumpSurf3D(p);
    vec3 grad = (vec3(bumpSurf3D(p - e.xyy),
                      bumpSurf3D(p - e.yxy),
                      bumpSurf3D(p - e.yyx)) - ref)/e.x; 
    
    /*
    // Six tap version, for comparisson. No discernible visual difference, in a lot of cases.
    vec3 grad = vec3(bumpSurf3D(p - e.xyy) - bumpSurf3D(p + e.xyy),
                     bumpSurf3D(p - e.yxy) - bumpSurf3D(p + e.yxy),
                     bumpSurf3D(p - e.yyx) - bumpSurf3D(p + e.yyx))/e.x*.5;
    */
       
    // Adjusting the tangent vector so that it's perpendicular to the normal. It's some kind 
    // of orthogonal space fix using the Gram-Schmidt process, or something to that effect.
    grad -= nor*dot(nor, grad);          
         
    // Applying the gradient vector to the normal. Larger bump factors make things more bumpy.
    return normalize(nor + grad*bumpfactor);
	
}

// Cheap shadows are the bain of my raymarching existence, since trying to alleviate artifacts is an excercise in
// futility. In fact, I'd almost say, shadowing - in a setting like this - with limited  iterations is impossible... 
// However, I'd be very grateful if someone could prove me wrong. :)
float softShadow(vec3 ro, vec3 lp, float k, float t){

    // More would be nicer. More is always nicer, but not really affordable.
    const int maxIterationsShad = 24; 
    
    vec3 rd = lp - ro; // Unnormalized direction ray.

    float shade = 1.;
    float dist = 0.001*(t*.125 + 1.);  // Coincides with the hit condition in the "trace" function.  
    float end = max(length(rd), 0.0001);
    //float stepDist = end/float(maxIterationsShad);
    rd /= end;

    // Max shadow iterations - More iterations make nicer shadows, but slow things down. Obviously, the lowest 
    // number to give a decent shadow is the best one to choose. 
    for (int i=0; i<maxIterationsShad; i++){

         
        float h = map(ro + rd*dist);
        shade = min(shade, k*h/dist);
        //shade = min(shade, smoothstep(0.0, 1.0, k*h/dist)); // Subtle difference. Thanks to IQ for this tidbit.
        // So many options here, and none are perfect: dist += min(h, .2), dist += clamp(h, .01, stepDist), etc.
        h = clamp(h, .1, .5); // max(h, .02);//
        dist += h;

        
        // Early exits from accumulative distance function calls tend to be a good thing.
        if (shade<0.001 || dist > end) break; 
    }

    // I've added a constant to the final shade value, which lightens the shadow a bit. It's a preference thing. 
    // Really dark shadows look too brutal to me. Sometimes, I'll add AO also, just for kicks. :)
    return min(max(shade, 0.) + .05, 1.); 
}



// I keep a collection of occlusion routines... OK, that sounded really nerdy. :)
// Anyway, I like this one. I'm assuming it's based on IQ's original.
float calcAO(in vec3 p, in vec3 n)
{
	float ao = 0.0, l;
    const float maxDist = 4.;
	const float nbIte = 5.;
	//const float falloff = .9;
    for( float i=1.; i< nbIte+.5; i++ ){
    
        l = (i + .0)*.5/nbIte*maxDist;        
        ao += (l - map( p + n*l )); // / pow(1.+l, falloff);
    }
	
    return clamp(1.- ao/nbIte, 0., 1.);
}



// Standard sky routine: Gradient with sun and overhead cloud plane. I debated over whether to put more 
// effort in, but the dust is there and I'm saving cycles. I originally included sun flares, but wasn't 
// feeling it, so took them out. I might tweak them later, and see if I can make them work with the scene.
vec3 getSky(vec3 ro, vec3 rd, vec3 ld){ 
    
    // Sky color gradients.
    vec3 col = vec3(.8, .7, .5), col2 = vec3(.4, .6, .9);
    
    //return mix(col, col2, pow(max(rd.y*.5 + .9, 0.), 5.));  // Probably a little too simplistic. :)
     
    // Mix the gradients using the Y value of the unit direction ray. 
    vec3 sky = mix(col, col2, pow(max(rd.y + .15, 0.), .5));
    sky *= vec3(.84, 1, 1.17); // Adding some extra vibrancy.
     
    float sun = clamp(dot(ld, rd), 0.0, 1.0);
    sky += vec3(1, .7, .4)*vec3(pow(sun, 16.))*.2; // Sun flare, of sorts.
    sun = pow(sun, 32.); // Not sure how well GPUs handle really high powers, so I'm doing it in two steps.
    sky += vec3(1, .9, .6)*vec3(pow(sun, 32.))*.35; // Sun.
    
     // Subtle, fake sky curvature.
    rd.z *= 1. + length(rd.xy)*.15;
    rd = normalize(rd);
   
    // A simple way to place some clouds on a distant plane above the terrain -- Based on something IQ uses.
    const float SC = 1e5;
    float t = (SC - ro.y - .15)/(rd.y + .15); // Trace out to a distant XZ plane.
    vec2 uv = (ro + t*rd).xz; // UV coordinates.
    
    // Mix the sky with the clouds, whilst fading out a little toward the horizon (The rd.y bit).
	if(t>0.) sky =  mix(sky, vec3(2), smoothstep(.45, 1., fBm(1.5*uv/SC))*
                        smoothstep(.45, .55, rd.y*.5 + .5)*.4);
    
    // Return the sky color.
    return sky;
}



// More concise, self contained version of IQ's original 3D noise function.
float noise3D(in vec3 p){
    
    // Just some random figures, analogous to stride. You can change this, if you want.
	const vec3 s = vec3(113, 157, 1);
	
	vec3 ip = floor(p); // Unique unit cell ID.
    
    // Setting up the stride vector for randomization and interpolation, kind of. 
    // All kinds of shortcuts are taken here. Refer to IQ's original formula.
    vec4 h = vec4(0., s.yz, s.y + s.z) + dot(ip, s);
    
	p -= ip; // Cell's fractional component.
	
    // A bit of cubic smoothing, to give the noise that rounded look.
    p = p*p*(3. - 2.*p);
    
    // Standard 3D noise stuff. Retrieving 8 random scalar values for each cube corner,
    // then interpolating along X. There are countless ways to randomize, but this is
    // the way most are familar with: fract(sin(x)*largeNumber).
    h = mix(fract(sin(h)*43758.5453), fract(sin(h + s.x)*43758.5453), p.x);
	
    // Interpolating along Y.
    h.xy = mix(h.xz, h.yw, p.y);
    
    // Interpolating along Z, and returning the 3D noise value.
    return mix(h.x, h.y, p.z); // Range: [0, 1].
	
}

/////
// Code block to produce some layers of smokey haze. Not sophisticated at all.
// If you'd like to see a much more sophisticated version, refer to Nitmitz's
// Xyptonjtroz example. Incidently, I wrote this off the top of my head, but
// I did have that example in mind when writing this.

// Hash to return a scalar value from a 3D vector.
float hash31(vec3 p){ return fract(sin(dot(p, vec3(127.1, 311.7, 74.7)))*43758.5453); }

// Several layers of cheap noise to produce some subtle smokey haze.
// Start at the ray origin, then take some samples of noise between it
// and the surface point. Apply some very simplistic lighting along the 
// way. It's not particularly well thought out, but it doesn't have to be.
float getMist(in vec3 ro, in vec3 rd, in vec3 lp, in float t){

    float mist = 0.;
    
    //ro -= vec3(0, 0, iTime*3.);
    
    float t0 = 0.;
    
    for (int i = 0; i<24; i++){
        
        // If we reach the surface, don't accumulate any more values.
        if (t0>t) break; 
        
        // Lighting. Technically, a lot of these points would be
        // shadowed, but we're ignoring that.
        float sDi = length(lp-ro)/FAR; 
	    float sAtt = 1./(1. + sDi*.25);
	    
        // Noise layer.
        vec3 ro2 = (ro + rd*t0)*2.5;
        float c = noise3D(ro2)*.65 + noise3D(ro2*3.)*.25 + noise3D(ro2*9.)*.1;
        //float c = noise3D(ro2)*.65 + noise3D(ro2*4.)*.35; 

        float n = c;//max(.65-abs(c - .5)*2., 0.);//smoothstep(0., 1., abs(c - .5)*2.);
        mist += n*sAtt;
        
        // Advance the starting point towards the hit point. You can 
        // do this with constant jumps (FAR/8., etc), but I'm using
        // a variable jump here, because it gave me the aesthetic 
        // results I was after.
        t0 += clamp(c*.25, .1, 1.);
        
    }
    
    // Add a little noise, then clamp, and we're done.
    return max(mist/48., 0.);
    
    // A different variation (float n = (c. + 0.);)
    //return smoothstep(.05, 1., mist/32.);

}

//////

void mainImage( out vec4 fragColor, in vec2 fragCoord ){	


	
	// Screen coordinates.
	vec2 u = (fragCoord - iResolution.xy*.5)/iResolution.y;
	
	// Camera Setup.     
	vec3 ro = vec3(0, 1.2, iTime*2.); // Camera position, doubling as the ray origin.
    vec3 lookAt = ro + vec3(0, -.15, .5);  // "Look At" position.
    
	
	// Using the Z-value to perturb the XY-plane.
	// Sending the camera and "look at" vectors down the tunnel. The "path" function is 
	// synchronized with the distance function.
	ro.xy += path(ro.z);
	lookAt.xy += path(lookAt.z);
    
    // Raising the camera up and down with the terrain function and tilting it up or down
    // according to the slope. It's subtle, but it adds to the immersiveness of that mind 
    // blowing, endless-sand experience. :D
    float sfH = camSurfFunc(ro); 
    float sfH2 = camSurfFunc(lookAt); 
    float slope = (sfH2 - sfH)/length(lookAt - ro); // Used a few lines below.
    //slope = smoothstep(-.15, 1.15, (slope*.5 + .5)) - .5; // Smoothing the slope... Needs work.
     
    // Raising the camera with the terrain.
    ro.y += sfH2; 
    lookAt.y += sfH2;
    
 
    // Using the above to produce the unit ray-direction vector.
    float FOV = 3.14159265/2.5; // FOV - Field of view.
    vec3 forward = normalize(lookAt - ro);
    vec3 right = normalize(vec3(forward.z, 0, -forward.x )); 
    vec3 up = cross(forward, right);

    // rd - Ray direction.
    vec3 rd = normalize(forward + FOV*u.x*right + FOV*u.y*up);
    
    // Swiveling the camera about the XY-plane (from left to right) when turning corners.
    // Naturally, it's synchronized with the path in some kind of way.
	rd.xy = rot2( path(lookAt.z).x/96.)*rd.xy;
    
    // Subtle up and down tilt, or camera pitch, if you prefer.
    rd.yz = rot2(-slope/3.)*rd.yz;
	
    // Usually, you'd just make this a unit directional light, and be done with it, but I
    // like some of the angular subtleties of point lights, so this is a point light a
    // long distance away. Fake, and probably not advisable, but no one will notice.
    vec3 lp = vec3(FAR*.25, FAR*.25, FAR) + vec3(0, 0, ro.z);
 

	// Raymarching.
    float t = trace(ro, rd);
    
    gT = t;
    
   
    // Sky. Only retrieving a single color this time.
    //vec3 sky = getSky(rd);
    
    // The passage color. Can't remember why I set it to sky. I'm sure I had my reasons.
    vec3 col = vec3(0);
    
    // Surface point. "t" is clamped to the maximum distance, and I'm reusing it to render
    // the mist, so that's why it's declared in an untidy postion outside the block below...
    // It seemed like a good idea at the time. :)
    vec3 sp = ro+t*rd; 
    
    float pathHeight = sp.y;//surfFunc(sp);// - path(sp.z).y; // Path height line, of sorts.
    
    // If we've hit the ground, color it up.
    if (t < FAR){
    
        
        vec3 sn = normal(sp, 1.); // Surface normal. //*(1. + t*.125)
        
        // Light direction vector. From the sun to the surface point. We're not performing
        // light distance attenuation, since it'll probably have minimal effect.
        vec3 ld = lp - sp;
        float lDist = max(length(ld), 0.001);
        ld /= lDist; // Normalize the light direct vector.
        
        lDist /= FAR; // Scaling down the distance to something workable for calculations.
        float atten = 1./(1. + lDist*lDist*.025);

        
        // Texture scale factor.        
        const float tSize = 1./8.;
        
        
        // Function based bump mapping.
        sn = doBumpMap(sp, sn, .07);///(1. + t*t/FAR/FAR*.25)
        
        // Texture bump mapping.
        float bf = .01;//(pathHeight + 5. < 0.)?  .05: .025;
        //sn = doBumpMap(iChannel0, sp*tSize, sn, bf/(1. + t/FAR));
        
        
        // Soft shadows and occlusion.
        float sh = softShadow(sp + sn*.002, lp, 6., t); 
        float ao = calcAO(sp, sn); // Ambient occlusion.
        
        // Add AO to the shadow. No science, but adding AO to things sometimes gives a bounced light look.
        sh = min(sh + ao*.25, 1.); 
        
        float dif = max( dot( ld, sn ), 0.0); // Diffuse term.
        float spe = pow(max( dot( reflect(-ld, sn), -rd ), 0.0 ), 5.); // Specular term.
        float fre = clamp(1.0 + dot(rd, sn), 0.0, 1.0); // Fresnel reflection term.
 
        // Schlick approximation. I use it to tone down the specular term. It's pretty subtle,
        // so could almost be aproximated by a constant, but I prefer it. Here, it's being
        // used to give a sandstone consistency... It "kind of" works.
		float Schlick = pow( 1. - max(dot(rd, normalize(rd + ld)), 0.), 5.0);
		float fre2 = mix(.2, 1., Schlick);  //F0 = .2 - Hard clay... or close enough.
       
        // Overal global ambience. It's made up, but I figured a little occlusion (less ambient light
        // in the corners, etc) and reflectance would be in amongst it... Sounds good, anyway. :)
        float amb = ao*.35;// + fre*fre2*.2;
        

        
        // Give the sand a bit of a sandstone texture.
        col = mix(vec3(1, .95, .7), vec3(.9, .6, .4), fBm(sp.xz*16.));
        col = mix(col*1.4, col*.6, fBm(sp.xz*32. - .5));///(1. + t*t*.001)
        
       
        // Extra shading in the sand crevices.
        float bSurf = bumpSurf3D(sp);
        col *= bSurf*.75 + .5;
       
        
        // Lamest sand sprinkles ever. :)
        col = mix(col*.7 + (hash(floor(sp*96.))*.7 + hash(floor(sp*192.))*.3)*.3, col, min(t*t/FAR, 1.));
        
        col *= vec3(1.2, 1, .9); // Extra color -- Part of last minute adjustments.
        
        
        // Combining all the terms from above. Some diffuse, some specular - both of which are
        // shadowed and occluded - plus some global ambience. Not entirely correct, but it's
        // good enough for the purposes of this demonstation.        
        col = col*(dif + amb + vec3(1, .97, .92)*fre2*spe*2.)*atten;
        
        
        // A bit of sky reflection. Not really accurate, but I've been using fake physics since the 90s. :)
        vec3 refSky = getSky(sp, reflect(rd, sn), ld);
        col += col*refSky*.05 + refSky*fre*fre2*atten*.15; 
        
 
        // Applying the shadows and ambient occlusion.
        col *= sh*ao;

        //col = vec3(ao);
    }
    
  
    // Combine the scene with the sky using some cheap volumetric substance.
	float dust = getMist(ro, rd, lp, t)*(1. - smoothstep(0., 1., pathHeight*.05));//(-rd.y + 1.);
    vec3 gLD = normalize(lp - vec3(0, 0, ro.z));
    vec3 sky = getSky(ro, rd, gLD);//*mix(1., .75, dust);
    //col = mix(col, sky, min(t*t*1.5/FAR/FAR, 1.)); // Quadratic fade off. More subtle.
    col = mix(col, sky, smoothstep(0., .95, t/FAR)); // Linear fade. Much dustier. I kind of like it.
    
    
    // Mild dusty haze... Not really sure how it fits into the physical situation, but I thought it'd
    // add an extra level of depth... or something. At this point I'm reminded of the "dog in a tie 
    // sitting at the computer" meme with the caption, "I have no idea what I'm doing." :D
    vec3 mistCol = vec3(1, .95, .9); // Probably, more realistic, but less interesting.
    //col += (mix(col, mistCol, .66)*.66 + col*mistCol*1.)*dust;
    
    
    // Simulating sun scatter over the sky and terrain: IQ uses it in his Elevated example.
    col += vec3(1., .6, .2)*pow( max(dot(rd, gLD), 0.), 16.)*.45;
    
    
    // Applying the mild dusty haze.
    col = col*.75 + (col + .25*vec3(1.2, 1, .9))*mistCol*dust*1.5;
    //col *= 1.05;
    
    
    // Really artificial. Kind of cool, but probably a little too much.    
    //col *= vec3(1.2, 1, .9);

    
    // Standard way to do a square vignette. Note that the maxium value value occurs at "pow(0.5, 4.) = 1./16," 
    // so you multiply by 16 to give it a zero to one range. This one has been toned down with a power
    // term to give it more subtlety.
    u = fragCoord/iResolution.xy;
    col = min(col, 1.)*pow( 16.*u.x*u.y*(1. - u.x)*(1. - u.y) , .0625);
 
    // Done.
	fragColor = vec4(sqrt(clamp(col, 0., 1.)), 1);
}