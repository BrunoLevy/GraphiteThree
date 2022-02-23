//stage GL_FRAGMENT_SHADER
//import <GLUP/ShaderToy.h>

/*
    Lava Tubes
    ----------

	I put this together for fun, as a means to take a break from other things I'm trying 
	to code at the moment. It's nothing exciting, just a glorified bump mapped tunnel.	

	The example was aimed toward the 800 by 450 window and built to accommodate mid-range 
	systems. I also procedurally textured the scene to keep Dr2 happy. :)
	
	I started with a perturbed cylindrical tunnel, combined it with a couple of gyroids, 
	then bump mapped a cellular surface onto it. It wasn't my intention to go for the 
	low-budget 80s sci-fi -- or late 90s demoscene -- look, but that's how it turned out. :) 
	I'll put together a more sophisticated version at some point.	
    
    Creating volumetric dust, vapor, etc, is simple enough in theory -- just accumulate
    multiple layers of dust-like noise, then layer it onto to the scene in some way. In 
	practice, however, you're trying to do the best you can with just a few iteratios of 
	cheap FBM noise -- usually consisting of just two value noise octaves. In this case, 
	I've used 16 iterations of two-octave value-noise FBM, which means there are 32 3D
	value-noise calls at most. It's not ideal, but I think it gets the job done
	well enough.


    Related examples: 

	// A much simpler example, but the one I vaguely had in mind when coding this.
	Nautilus - Weyland
	https://www.shadertoy.com/view/MdXGz4

	// Better usage of the cellular algorithm and XT95's translucency formula.
    3D Cellular Tiling - Shane
    https://www.shadertoy.com/view/ld3Szs

	// One of my favorite simple coloring jobs.
    Skin Peeler - Dave Hoskins
    https://www.shadertoy.com/view/XtfSWX
    Based on one of my all time favorites:
	This was one of the first volumetric dust examples I saw on Shadertoy.
    Xyptonjtroz - Nimitz
	https://www.shadertoy.com/view/4ts3z2


*/

// A more subtle, slightly less heated, scene.
//#define COOLER

#define FAR 50.


// Standard 1x1 hash functions. Using "cos" for non-zero origin result.
float hash( float n ){ return fract(cos(n)*45758.5453); }

// Non-standard vec3-to-vec3 hash function.
vec3 hash33(vec3 p){ 
    
    float n = sin(dot(p, vec3(7, 157, 113)));    
    return fract(vec3(2097152, 262144, 32768)*n); 
}

// 2x2 matrix rotation. Note the absence of "cos." It's there, but in disguise, and comes courtesy
// of Fabrice Neyret's "ouside the box" thinking. :)
mat2 rot2( float a ){ vec2 v = sin(vec2(1.570796, 0) + a);	return mat2(v, -v.y, v.x); }

/*
// Tri-Planar blending function. Based on an old Nvidia writeup:
// GPU Gems 3 - Ryan Geiss: https://developer.nvidia.com/gpugems/GPUGems3/gpugems3_ch01.html
vec3 tex3D(sampler2D t, in vec3 p, in vec3 n ){
    
    n = max(abs(n), 0.001);
    n /= dot(n, vec3(1));
	vec3 tx = texture(t, p.yz).xyz;
    vec3 ty = texture(t, p.zx).xyz;
    vec3 tz = texture(t, p.xy).xyz;
    
    // Textures are stored in sRGB (I think), so you have to convert them to linear space 
    // (squaring is a rough approximation) prior to working with them... or something like that. :)
    // Once the final color value is gamma corrected, you should see correct looking colors.
    return (tx*tx*n.x + ty*ty*n.y + tz*tz*n.z);
    
}
*/


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

////////
// The cellular tile routine. Draw a few objects (four spheres, in this case) using a minumum
// blend at various 3D locations on a cubic tile. Make the tile wrappable by ensuring the 
// objects wrap around the edges. That's it.
//
// Believe it or not, you can get away with as few as three spheres. If you sum the total 
// instruction count here, you'll see that it's way, way lower than 2nd order 3D Voronoi.
// Not requiring a hash function provides the biggest benefit, but there is also less setup.
// 
// The result isn't perfect, but 3D cellular tiles can enable you to put a Voronoi looking 
// surface layer on a lot of 3D objects for little cost.
//
float drawObject(in vec3 p){
    
    // Wrap conditions:
    // Anything that wraps the domain will work.
    //p = cos(p*6.2831853)*.25 + .25; 
    //p = abs(cos(p*3.14159)*.5);
    //p = fract(p) - .5; 
    //p = abs(fract(p) - .5); 
  
    // Distance metrics:
    // Here are just a few variations. There are way too many to list them all,
    // but you can try combinations with "min," and so forth, to create some
    // interesting combinations.
    
    // Spherical. (Square root needs to be factored to "d" in the cellTile function.)
    //p = fract(p) - .5;    
    //return dot(p, p)/1.5;
    
    // Octahedral... kind of.
    //p = abs(fract(p)-.5);
    //return dot(p, vec3(.333));
    
    // Triangular tube - Doesn't wrap, but it's here for completeness.
    //p = fract(p) - .5;
    //p = max(abs(p)*.866025 + p.yzx*.5, -p.yzx);
    //return max(max(p.x, p.y), p.z);    
    
    // Cubic.
    //p = abs(fract(p) - .5); 
    //return max(max(p.x, p.y), p.z);
    
    // Cylindrical. (Square root needs to be factored to "d" in the cellTile function.)
    //p = fract(p) - .5; 
    //return max(max(dot(p.xy, p.xy), dot(p.yz, p.yz)), dot(p.xz, p.xz));
    
    // Octahedral.
    //p = abs(fract(p) - .5); 
    //p += p.yzx;
    //return max(max(p.x, p.y), p.z)*.5;

    // Hexagonal tube.
    p = abs(fract(p) - .5); 
    p = max(p*.866025 + p.yzx*.5, p.yzx);
    return max(max(p.x, p.y), p.z);
    
    
}



// Second order cellular tiled routine - I've explained how it works in detail in other examples.
float cellTile(in vec3 p){
    
     
    // Draw four overlapping objects (spheres, in this case) at various positions throughout the tile.
    vec4 v, d; 
    d.x = drawObject(p - vec3(.81, .62, .53));
    p.xy = vec2(p.y - p.x, p.y + p.x)*.7071;
    d.y = drawObject(p - vec3(.39, .2, .11));
    p.yz = vec2(p.z - p.y, p.z + p.y)*.7071;
    d.z = drawObject(p - vec3(.62, .24, .06));
    p.xz = vec2(p.z - p.x, p.z + p.x)*.7071;
    d.w = drawObject(p - vec3(.2, .82, .64));

    v.xy = min(d.xz, d.yw), v.z = min(max(d.x, d.y), max(d.z, d.w)), v.w = max(v.x, v.y); 
   
    d.x =  min(v.z, v.w) - min(v.x, v.y); // First minus second order, for that beveled Voronoi look. Range [0, 1].
    //d.x =  min(v.x, v.y); // Minimum, for the cellular look.
        
    const float scale = 2.;
    return min(d.x*2.*scale, 1.); // Normalize.
    
}

// The path is a 2D sinusoid that varies over time, depending upon the frequencies, and amplitudes.
vec2 path(in float z){ 
    
    //return vec2(0); // Straight line.
    
    // Curved path.
    float a = sin(z * 0.11/1.5);
    float b = cos(z * 0.14/1.5);
    return vec2(a*4. -b*1.5, b*1.7 + a*1.5)*1.5; 
}


// Commutative smooth maximum function. Provided by Tomkh, and taken 
// from Alex Evans's (aka Statix) talk: 
// http://media.lolrus.mediamolecule.com/AlexEvans_SIGGRAPH-2015.pdf
// Credited to Dave Smith @media molecule.
float smax(float a, float b, float k){
    
   float f = max(0., 1. - abs(b - a)/k);
   return max(a, b) + k*.25*f*f;
}


// Commutative smooth minimum function. Provided by Tomkh, and taken 
// from Alex Evans's (aka Statix) talk: 
// http://media.lolrus.mediamolecule.com/AlexEvans_SIGGRAPH-2015.pdf
// Credited to Dave Smith @media molecule.
float smin(float a, float b, float k){

   float f = max(0., 1. - abs(b - a)/k);
   return min(a, b) - k*.25*f*f;
}


// Based on the triangle function that Shadertoy user Nimitz has used in various triangle noise 
// demonstrations. See Xyptonjtroz - Very cool.
// https://www.shadertoy.com/view/4ts3z2
// Anyway, these have been modified slightly to emulate the sin and cos waves.
vec3 triS(in vec3 x){ return 1. - abs(fract(x + .25) - .5)*4.; } // Triangle function.
vec3 triC(in vec3 x){ return 1. - abs(fract(x + .5) - .5)*4.; } // Triangle function.


// Tube functions - for the tunnel.
float dist2D(in vec2 p){

    // Other tube shapes to try.
    /*
    p = abs(p);
    return max(max(p.x, p.y), (p.x + p.y)*.7071); // Octagon.
    return max(p.x, p.y); // Square.
    return (p.x + p.y)*.7071; // Diamond.
    return max(p.x*.8660254 + p.y*.5, p.y); // Hexagon.
    */
    
    return length(p); // Round cylinder.
    
}

// Perturbed gyroid tunnel function: In essence, it's a couple of smoothly
// combined gyroid functions, with a cylindrical hole (wrapped around the
// camera path) smoothly carved out from them.
//
float map(vec3 p){
    
   
    //float sf = cellTile(p*.25); // Cellular layer.
    //sf = smoothstep(-.1, .5, sf);
    

    // Trancendental gyroid functions and a function to perturb
    // the tunnel. For comparisson, I included a rough triangle
    // function equivalent option.
    #if 1
    vec3 q = p*3.1415926;
    float cav = dot(cos(q/2.), sin(q.yzx/2.5)); // Gyroid one.
    float cav2 = dot(cos(q/6.5), sin(q.yzx/4.5)); // Gyroid two.
    cav = smin(cav, cav2/2., 2.); // Smoothly combine the gyroids.
    
    // Transendental function to perturb the walls.
    float n = dot(sin(q/3. + cos(q.yzx/6.)), vec3(.166));
    //float n = (-cellTile(p*.125) + .5)*.5;
    #else
    vec3 q = p/2.;
    float cav = dot(triC(q/2.), triS(q.yzx/2.5)); // Triangular gyroid one.
	float cav2 = dot(triC(q/6.5), -triS(q.yzx/4.5)); // Triangular gyroid two.
    cav = smin(cav, cav2/2., 2.); // Smoothly combine the gyroids.
    
    // Triangular function to perturb the walls.
    float n = dot(triS(q/3. + triC(q.yzx/6.)), vec3(.166));
    //float n = (-cellTile(p*.125) + .5)*.5;
	#endif

    // Wrap the tunnel around the camera path.
    p.xy -= path(p.z);
    

    // Smoothly combining the wrapped cylinder with the gyroids, then 
    // adding a bit of perturbation on the walls.
    n = smax((2.25 - dist2D(p.xy)), (-cav - .75), 1.) +  n;// - sf*.375;
    
    // Return the distance value for the scene. Sinusoids aren't that great
    // to hone in on, so some ray shortening is a necessary evil.
    return n*.75;
 
}


// Surface bump function. Cheap, but with decent visual impact.
float bumpSurf3D( in vec3 p){
    
    // Cellular tiling.
    float sf = cellTile(p*.5);
    float sf2 = cellTile(p*1.5);
    float n = sf*.66 + sf2*.34; // FBM.
    
    // Noise.
    float ns = noise3D(p*40.);
    
    // Combining the above. Trial and error to achieve a slightly
    // cracked looking surface crust... or something. :)
    return n*.45 + smoothstep(-.1, .6, sf*.75 + sf2*.25)*.5 + ns*.05;
    
    // Another variation that enhances the cracks.
    //return smoothstep(-.1, .5, sf)*.8 + n*.2; 

}

// Standard function-based bump mapping function. Six taps is usually better, but I'm trying
// trying to save some cycles.
vec3 doBumpMap(in vec3 p, in vec3 nor, float bumpfactor){
    
    const vec2 e = vec2(.001, 0);
    float ref = bumpSurf3D(p);                 
    vec3 grad = (vec3(bumpSurf3D(p - e.xyy),
                      bumpSurf3D(p - e.yxy),
                      bumpSurf3D(p - e.yyx) )-ref)/e.x;                     
          
    grad -= nor*dot(nor, grad);          
                      
    return normalize(nor + grad*bumpfactor);
	
}

// Standard raymarching function.
float trace(in vec3 ro, in vec3 rd){

    float t = 0.0, h;
    for(int i = 0; i<80; i++){
    
        h = map(ro+rd*t);
        // Note the "t*b + a" addition. Basically, we're putting less emphasis on accuracy, as
        // "t" increases. It's a cheap trick that works in most situations... Not all, though.
        if(abs(h)<0.001*(t*.125 + 1.) || t>FAR) break; // Alternative: 0.001*max(t*.25, 1.)
        t += h;
        
    }

    return min(t, FAR);
}

// Standard normal function. It's not as fast as the tetrahedral calculation, but more symmetrical.
vec3 getNormal(in vec3 p) {
	const vec2 e = vec2(0.001, 0);
	return normalize(vec3(map(p + e.xyy) - map(p - e.xyy), map(p + e.yxy) - map(p - e.yxy),	map(p + e.yyx) - map(p - e.yyx)));
}



// XT95's really clever, cheap, SSS function. The way I've used it doesn't do it justice,
// so if you'd like to really see it in action, have a look at the following:
//
// Alien Cocoons - XT95: https://www.shadertoy.com/view/MsdGz2
//
float thickness( in vec3 p, in vec3 n, float maxDist, float falloff )
{
	const float nbIte = 6.0;
	float ao = 0.0;
    
    for( float i=1.; i< nbIte+.5; i++ ){
        
        float l = (i*.75 + fract(cos(i)*45758.5453)*.25)/nbIte*maxDist;
        
        ao += (l + map( p -n*l )) / pow(1. + l, falloff);
    }
	
    return clamp( 1.-ao/nbIte, 0., 1.);
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
        //shade = min(shade, k*h/dist);
        shade = min(shade, smoothstep(0.0, 1.0, k*h/dist)); // Subtle difference. Thanks to IQ for this tidbit.
        // So many options here, and none are perfect: dist += min(h, .2), dist += clamp(h, .01, stepDist), etc.
        dist += clamp(h, .01, .5); 
        
        // Early exits from accumulative distance function calls tend to be a good thing.
        if (h<0. || dist > end) break; 
    }

    // I've added a constant to the final shade value, which lightens the shadow a bit. It's a preference thing. 
    // Really dark shadows look too brutal to me. Sometimes, I'll add AO also just for kicks. :)
    return min(max(shade, 0.) + .05, 1.); 
}

/*
// Ambient occlusion, for that self shadowed look. Based on the original by XT95. I love this 
// function, and in many cases, it gives really, really nice results. For a better version, and 
// usage, refer to XT95's examples below:
//
// Hemispherical SDF AO - https://www.shadertoy.com/view/4sdGWN
// Alien Cocoons - https://www.shadertoy.com/view/MsdGz2
float calcAO( in vec3 p, in vec3 n )
{
	float ao = 0.0, l;
    const float maxDist = 4.;
	const float nbIte = 6.0;
	//const float falloff = 0.9;
    for( float i=1.; i< nbIte+.5; i++ ){
    
        l = (i + hash(i))*.5/nbIte*maxDist;
        
        ao += (l - map( p + n*l ))/(1.+ l);// / pow(1.+l, falloff);
    }
	
    return clamp(1.- ao/nbIte, 0., 1.);
}
*/

// I keep a collection of occlusion routines... OK, that sounded really nerdy. :)
// Anyway, I like this one. I'm assuming it's based on IQ's original.
float calcAO(in vec3 p, in vec3 n)
{
	float ao = 0.0, l;
    const float maxDist = 3.;
	const float nbIte = 5.;
	//const float falloff = .9;
    for( float i=1.; i< nbIte+.5; i++ ){
    
        l = (i + .0)*.5/nbIte*maxDist;        
        ao += (l - map( p + n*l )); // / pow(1.+l, falloff);
    }
	
    return clamp(1.- ao/nbIte, 0., 1.);
}



/////
// Code block to produce some layers of smokey haze. Not sophisticated at all.
// If you'd like to see a much more sophisticated version, refer to Nitmitz's
// Xyptonjtroz example. Incidently, I wrote this off the top of my head, but
// I did have that example in mind when writing this.

// Hash to return a scalar value from a 3D vector.
float hash31(vec3 p){ return fract(sin(dot(p, vec3(127.1, 311.7, 74.7)))*43758.5453); }

// Four layers of cheap cell tile noise to produce some subtle mist.
// Start at the ray origin, then take four samples of noise between it
// and the surface point. Apply some very simplistic lighting along the 
// way. It's not particularly well thought out, but it doesn't have to be.
float getMist(in vec3 ro, in vec3 rd, in vec3 lp, in float t){

    float mist = 0.;
    
    //ro -= vec3(0, 0, iTime*3.);
    
    float t0 = 0.;
    
    for (int i = 0; i<16; i++){
        
        // If we reach the surface, don't accumulate any more values.
        if (t0>t) break; 
        
        // Lighting. Technically, a lot of these points would be
        // shadowed, but we're ignoring that.
        float sDi = length(lp-ro)/FAR; 
	    float sAtt = 1./(1. + sDi*.25);
	    
        // Noise layer.
        vec3 ro2 = (ro + rd*t0)*3.;
        float c = noise3D(ro2)*.66 + noise3D(ro2*3.)*.34; //cellTile
        float n = (c - .25);
        mist += n*sAtt;
        
        // Advance the starting point towards the hit point. You can 
        // do this with constant jumps (FAR/8., etc), but I'm using
        // a variable jump here, because it gave me the aesthetic 
        // results I was after.
        t0 += max(c*.5, .05);
        
    }
    
    // Add a little noise, then clamp, and we're done.
    return max(mist/32., 0.);
    
    // A different variation (float n = (c. + 0.);)
    //return smoothstep(.05, 1., mist/32.);

}

//////

void mainImage( out vec4 fragColor, in vec2 fragCoord ){
	
	// Screen coordinates.
	vec2 uv = (fragCoord - iResolution.xy*0.5)/iResolution.y;
	
	// Camera Setup.
	vec3 lookAt = vec3(0., 0.0, iTime*4. + 0.1);  // "Look At" position.
	vec3 camPos = lookAt + vec3(0.0, 0.0, -0.1); // Camera position, doubling as the ray origin.

 
    // Light positioning. 
 	vec3 lightPos = camPos + vec3(0, .25, 5); // Placed in front of the camera.

	// Using the Z-value to perturb the XY-plane.
	// Sending the camera, "look at," and two light vectors down the tunnel. The "path" function is 
	// synchronized with the distance function. Change to "path2" to traverse the other tunnel.
	lookAt.xy += path(lookAt.z);
	camPos.xy += path(camPos.z);
	lightPos.xy += path(lightPos.z);

    // Using the above to produce the unit ray-direction vector.
    float FOV = 3.14159265/2.; // FOV - Field of view.
    vec3 forward = normalize(lookAt - camPos);
    vec3 right = normalize(vec3(forward.z, 0., -forward.x )); 
    vec3 up = cross(forward, right);

    // rd - Ray direction.
    vec3 rd = normalize(forward + (uv.x*right + uv.y*up)*FOV);
    
    // A bit of lens mutation to increase the scene peripheral, if that's your thing.
    //vec3 rd = normalize(forward + FOV*uv.x*right + FOV*uv.y*up);
    //rd = normalize(vec3(rd.xy, rd.z - dot(rd.xy, rd.xy)*.25));    
    
    // Swiveling the camera about the XY-plane (from left to right) when turning corners.
    // Naturally, it's synchronized with the path in some kind of way.
	rd.xy = rot2(path(lookAt.z).x/16.)*rd.xy;
		
    // Standard ray marching routine. I find that some system setups don't like anything other than
    // a "break" statement (by itself) to exit. 
	float t = trace(camPos, rd);
	
    // Initialize the scene color.
    vec3 sceneCol = vec3(0);
	
	// The ray has effectively hit the surface, so light it up.
	if(t<FAR){
	
   	
    	// Surface position and surface normal.
	    vec3 sp = t * rd+camPos;
	    vec3 sn = getNormal(sp);
        
        vec3 oSn = sn; // A copy of the unperturbed (unbumped) normal.        
        
        // Function based bump mapping. Comment it out to see the under layer. It's pretty
        // comparable to regular beveled Voronoi... Close enough, anyway.
        sn = doBumpMap(sp, sn, .5);
        
        // For aesthetic reasons, sometimes you might not want the reflected vector to be
        // perturbed too much by the bumped surface. The following tone it down a little. 
        //oSn = mix(oSn, sn, .75);
	    
	    // Ambient occlusion and shadows.
        float ao = calcAO(sp, sn);
        float sh = softShadow(sp + sn*.002, lightPos, 4., t); // Set to "1.," if you can do without them.
        sh = min(sh + ao*.3, 1.);
    	
    	// Light direction vectors.
	    vec3 ld = lightPos - sp;

        // Distance from respective lights to the surface point.
	    float distlpsp = max(length(ld), 0.001);
    	
    	// Normalize the light direction vectors.
	    ld /= distlpsp;
	    
	    // Light attenuation, based on the distances above.
	    float atten = 1./(1. + distlpsp*0.25); // + distlpsp*distlpsp*0.025
    	
    	// Ambient light.
	    float ambience = ao*.5;
    	
    	// Diffuse lighting.
	    float diff = max( dot(sn, ld), 0.0);
        //diff *= diff;
   	
    	// Specular lighting.
	    float spec = pow(max( dot( reflect(-ld, sn), -rd), 0.), 32.);

	    
	    // Fresnel term. Good for giving a surface a bit of a reflective glow.
        float fre = pow( clamp(dot(sn, rd) + 1., .0, 1.), 1.);
        
        // Refected vector at the surface hit point. 
        vec3 ref = reflect(rd, oSn);

        // Object texturing and coloring: I made this up as I went along. It's just a combination 
        // of value noise and cellular noise. Sometimes, I like to darken crevices, etc, a little 
        // more by applying combinations that match those in the distance field calculations, or 
        // bump map calculations.
        vec3 texCol = mix(vec3(.5, .4, .3), vec3(0, .1, .2), (noise3D(sp*32.)*.66 + noise3D(sp*64.)*.34));
        texCol *= mix(vec3(0, .1, .2).zyx, vec3(1, .9, .8), (1. - cellTile(sp*4.5)*.75));
 
        // Extra shading in the cracked crevices for a slightly more cartoonish look.
        texCol *= mix(vec3(.9, .95, 1), vec3(.1, 0, 0), .75 - smoothstep(-.1, .5, cellTile(sp*.5))*.75);
        
        
        // Last minute decision to give the rocky surface a blueish charcoal tinge. Still not sure about it. :)
        texCol *= vec3(.8, 1, 1.2); 
        
        
    	/////////   
        // Translucency, courtesy of XT95. See the "thickness" function.
        vec3 hf =  normalize(ld + sn);
        float th = thickness( sp, sn, 1., 1.);
        float tdiff =  pow( clamp( dot(rd, -hf), 0., 1.), 1.);
        float trans = (tdiff + .0)*th;  
        trans = pow(trans, 4.);        
    	//////// 
    	
    	
    	// Combining the above terms to produce the final color. It's based more on acheiving a
        // certain aesthetic than science.
        sceneCol = texCol*(diff + ambience) + vec3(.7, .9, 1.)*spec;
        sceneCol += texCol*vec3(.8, .95, 1)*pow(fre, 3.)*3.;
        sceneCol += texCol*vec3(1, .1, .05)*trans*12.;
        
        // Fake reflection. Other that using the refected vector, there's very little science 
        // involved, but since the effect is subtle, you can get away with it.
        vec3 refCol = vec3(1, .05, .075)*smoothstep(.25, 1., noise3D(ref*2.)*.66 + noise3D(ref*4.)*.34 );
        sceneCol += texCol*refCol*2.;

	    // Shading.
        sceneCol *= atten*ao*sh;
	   
	
	}
    
   
       
    // Blend the scene and the background with some very basic, 16-layered smokey haze.
    float mist = getMist(camPos, rd, lightPos, t);
    vec3 sky = vec3(2, 1, .5);
    sky = mix(sky, sky.xzy, .25 - rd.y*.25);
    sceneCol = mix(sky, sceneCol, 1./(t*t/FAR/FAR*12. + 1.));
    

    // Brown mist was too brown, and blue mist was too blue, so I combined the two... Not a lot of thought
    // went into it. :)
    //vec3 mistCol = vec3(1.2, 1, .8); // Probably, more realistic, but less interesting.
    vec3 mistCol = mix(vec3(1.4, 1, .85), vec3(1.4, 1, .85).zyx, dot(sin(rd*9. + cos(rd.yzx*5.)), vec3(.166)) + .5);
    // Applying the mist to the scene. This particular compositing formula was based on how I wanted
    // the mist to look when blended in with the background... More Star Trek science. :D
    sceneCol += (mix(sceneCol, mistCol, .66)*.66 + sceneCol*mistCol*1.5)*mist;
    
    #ifdef COOLER
    // Cooler, more subtle version.
    sceneCol *= vec3(.85, .95, 1.25);
    #endif
    
    // Vignette.
    uv = fragCoord/iResolution.xy;
    sceneCol = mix(sceneCol, vec3(0), (1. - pow(16.*uv.x*uv.y*(1.-uv.x)*(1.-uv.y), 0.25))*.5);


    // Clamp and present the pixel to the screen.
	fragColor = vec4(sqrt(max(sceneCol, 0.)), 1.0);
	
}