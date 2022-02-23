//stage GL_FRAGMENT_SHADER
//import <GLUP/ShaderToy.h>

/*
	Weaved 3D Truchet
	-----------------
	
	Mapping a square Truchet pattern onto an overlapping, interwoven hexagonal Truchet object... 
	or if you prefer a more lively description, it's an abstract representation of pythons. :) 
	I can thank BigWIngs's "Hexagonal Truchet Weaving" example for the idea to do this.

	I produced a relatively simple scene, just to give people an idea, but it's possible to
	create some really cool organic structures this way.

	Coding the main object wasn't particularly difficult, but bump mapping the square Truchet
	pattern onto it was a little tiresome. I originally applied the pattern directly to the 
	object via the distance field equation, but I don't think slower machines would have
	enjoyed running it. Therefore, I took the surface pattern outside the raymarching loop and 
	bump mapped it.	That, of course, added to the complexity, but sped things up considerably. 
	My fast machine	can run it in fullscreen fine, but the example was targetted toward the 
	800 by 450 canvas -	which I'm hoping average systems will be able to run it in.
 
	Procedurally speaking, this is just a 3D application of a standard hexagonal weave, which 
	I explained in my "Arbitrary Weave" example. For anyone interested in producing one, I'd 
	suggest starting with a 2D pattern, then taking it from there. Feel free to use this as a 
	guide, but I doubt it'll be needed.

	The comments, code and logic were a little rushed, so I'll get in and tidy it up in due 
	course.


	2D Weaved Truchet examples:

	// The original: Much less code, so if you're trying to get a handle on how to make
	// a random hexagonal weave pattern, this is the one you should be looking at.
	BigWIngs - Hexagonal Truchet Weaving 
	https://www.shadertoy.com/view/llByzz

	// My version of BigWIngs's example above. The code in this particular example was
	// based on it.
	Arbitrary Weave - Shane
	https://www.shadertoy.com/view/MtlBDs


*/

#define FAR 15.

// Just a regular hexagonal Truchet tile consisting of three small arcs.
//#define NO_WEAVE

// A geometric hack to show the hexagonal grid.
//#define SHOW_GRID

float objID = 0.; // Object ID - Ground: 0; Truchet: 1.

// Standard 2D rotation formula.
mat2 r2(in float a){ float c = cos(a), s = sin(a); return mat2(c, -s, s, c); }


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


// Compact, self-contained version of IQ's 3D value noise function. I have a transparent noise
// example that explains it, if you require it.
float n3D(vec3 p){
    
	const vec3 s = vec3(7, 157, 113);
	vec3 ip = floor(p); p -= ip; 
    vec4 h = vec4(0., s.yz, s.y + s.z) + dot(ip, s);
    p = p*p*(3. - 2.*p); //p *= p*p*(p*(p * 6. - 15.) + 10.);
    h = mix(fract(sin(h)*43758.5453), fract(sin(h + s.x)*43758.5453), p.x);
    h.xy = mix(h.xz, h.yw, p.y);
    return mix(h.x, h.y, p.z); // Range: [0, 1].
}


// The path is a 2D sinusoid that varies over time, depending upon the frequencies, and amplitudes.
vec2 path(in float z){ 

    //return vec2(0);
    
    //return vec2(sin(z * 0.15)*1.2, cos(z * 0.25)*.85); 
    
    return vec2(sin(z * 0.15)*2.4, 0);
}

// Standard float to float hash - Based on IQ's original.
float hash(float n){ return fract(sin(n)*43758.5453); }


// Standard vec2 to float hash - Based on IQ's original.
float hash21(vec2 p){ return fract(sin(dot(p, vec2(141.187, 289.973)))*43758.5453); }


// vec2 to vec2 hash.
vec2 hash22(vec2 p) { 

    // Faster, but doesn't disperse things quite as nicely. However, when framerate
    // is an issue, and it often is, this is a good one to use. Basically, it's a tweaked 
    // amalgamation I put together, based on a couple of other random algorithms I've 
    // seen around... so use it with caution, because I make a tonne of mistakes. :)
    float n = sin(dot(p, vec2(41, 289)));
    return fract(vec2(262144, 32768)*n); 
    
    /*
    // Animated.
    p = fract(vec2(262144, 32768)*n); 
    // Note the ".45," insted of ".5" that you'd expect to see. When edging, it can open 
    // up the cells ever so slightly for a more even spread. In fact, lower numbers work 
    // even better, but then the random movement would become too restricted. Zero would 
    // give you square cells.
    return sin( p*6.2831853 + iTime )*.5 + .5; 
	*/
    
}

// Smooth maximum, based on IQ's smooth minimum.
float smax(float a, float b, float s){
    
    float h = clamp(.5 + .5*(a - b)/s, 0., 1.);
    return mix(b, a, h) + h*(1. - h)*s;
}

// Helper vector. If you're doing anything that involves regular triangles or hexagons, the
// 30-60-90 triangle will be involved in some way, which has sides of 1, sqrt(3) and 2.
const vec2 s = vec2(1, 1.7320508);


// This function returns the hexagonal grid coordinate for the grid cell, and the corresponding 
// hexagon cell ID - in the form of the central hexagonal point. That's basically all you need to 
// produce a hexagonal grid.
//
// When working with 2D, I guess it's not that important to streamline this particular function.
// However, if you need to raymarch a hexagonal grid, the number of operations tend to matter.
// This one has minimal setup, one "floor" call, a couple of "dot" calls, a ternary operator, etc.
// To use it to raymarch, you'd have to double up on everything - in order to deal with 
// overlapping fields from neighboring cells, so the fewer operations the better.
vec4 getHex(vec2 p){
    
    // The hexagon centers: Two sets of repeat hexagons are required to fill in the space, and
    // the two sets are stored in a "vec4" in order to group some calculations together. The hexagon
    // center we'll eventually use will depend upon which is closest to the current point. Since 
    // the central hexagon point is unique, it doubles as the unique hexagon ID.
    vec4 hC = floor(vec4(p, p - vec2(.5, 1))/s.xyxy) + vec4(.5, .5, 1, 1.5);
    
    // Centering the coordinates with the hexagon centers above.
    vec4 h = vec4(p - hC.xy*s, p - (hC.zw + vec2(0, -.5))*s );
    
    // Nearest hexagon center (with respect to p) to the current point. In other words, when
    // "h.xy" is zero, we're at the center. We're also returning the corresponding hexagon ID -
    // in the form of the hexagonal central point. Note that a random constant has been added to 
    // "hC.zw" to further distinguish it from "hC.xy."
    //
    // On a side note, I sometimes compare hex distances, but I noticed that Iomateron compared
    // the squared Euclidian version, which seems neater, so I've adopted that.
    return dot(h.xy, h.xy)<dot(h.zw, h.zw) ? vec4(h.xy, hC.xy) : vec4(h.zw, hC.zw);
    
}


// Very basic square Truchet routine.
float sTruchet(vec2 p){
    
    // Unique coordinate.
    vec2 ip = floor(p);
    
    float rnd = hash21(ip); // Random ID.

    // Square grid. Equivalent to: p = fract(p) - .5;
    p -= ip + .5;
    
    // Random tile flipping.
    p.y *= (rnd >.5)? -1. : 1.;
    
    // Repeat space trick - to save rendering two arcs.
    // I explained it in detail in my "Square Truchet Flow" example.
    p = p.x>-p.y ? p : -p; 
    

    // Arc(s) of thickness ".15."
    float d = abs(length(p - .5) - .5) - .15;
    
    // Shaping it to suit individual needs.
    return mix(max(-d, 0.)*.35, smoothstep(0., .125, d), .5);
    
}


// Poloidal distance function - The crosss section of the torus. It can be cylindrical,
// square, hexagonal, octagonal, etc.
float polDist(vec2 p){

    return length(p);
    
    // Square cross section.
    //p = abs(p);
    //return max(p.x, p.y);
    
    // Hexagonal cross section.
    //p = abs(p);
    //return max(p.x*.8660254 + p.y*.5, p.y);

    // Octagonal cross section.
    //p = abs(p);
    //return max(max(p.x, p.y), (p.x + p.y)*.7071);

}

// Toroidal distance function - The sweeping section of the torus. It can be cylindrical,
// square, hexagonal, octagonal, etc.
float torDist(vec2 p){
    
    return length(p);
    
    /*
    // Hexagon arcs.
    p = abs(p);
    return max(p.x*.8660254 + p.y*.5, p.y);
    */
      
    
}

// Shade, pattern and random ID globals. Hacked in along the way. I might tidy these up later.
vec4 gHgt, gA, gA2, gD;
//vec4 gRnd;

vec4 Truchet(vec2 p){
    
    // Initialize the height vector to zero.
    gHgt = vec4(0);
    
    // Scaling, translating, then converting the input to a hexagonal grid cell coordinate and
    // a unique coordinate ID. The resultant vector contains everything you need to produce a
    // pretty pattern, so what you do from here is up to you.
    vec4 h = getHex(p);
    
    
    #ifdef SHOW_GRID
    // Offset hexagonal variable - used to render the joins. You could render them using the
    // value above, but sometimes, pixelated seem lines can appear at the hexagonal boundaries
    // when trying to perform offset diffuse calculations... It's a long story, but creating
    // an offset grid ensures no artefacts.
    vec4 h2 = getHex(p - vec2(0, .8660254*2./3.));
    #endif
    
    // Using the idetifying coordinate - stored in "h.zw," to produce some unique random numbers
    // for the hexagonal grid cell.
    vec2 rnd = hash22(h.zw + .673);
     
    // Store the hexagonal coordinates in "p" to save some writing. "p" tends to be the universal
    // shader variable for "point," so it reads a little better too.
    p = h.xy;
    

    
    // Constants used for the Truchet pattern. The arc thickness, the small are radius and the large
    // arc radius.
    //const float th = 0.;//.1; // Applying tube thickness outside the function.
    const float rSm = .8660254/3.;
    const float rLg = .8660254;
    
    float a; // Storage for the arc angle, which is used for height, etc.
    
    
    // Randomly rotate the tile.
    float rFactor = floor(rnd.x*6.)*3.14159265/3.;
    p = r2(rFactor)*p;
    
    float hDir = rnd.y>.5? -1.: 1.;
    
 
    // Distance field variable, and a hacky flag to turn off pattern rendering - Basically, to stop the 
    // portals being rendered under the joins.
    vec4 d;
    
    // Utils for point storage and the UV coordinates for each of the shapes.
    vec2 p1, p2, p3;
    
    
    // DISTANCE FIELD, ID, AND TEXTURE CALCULATIONS
    
    // If one of the random variables is above a certain threshold, render one of the
    // tiles. The thresholds are distributed according to how much weighting you'd like
    // to give a certain kind of tile. For instance, I wanted more curves, so gave more
    // weighting to the arc tiles than the straight line ones.
    
    
    #ifdef NO_WEAVE
     
    // Small arc one.        
    p1 = p - vec2(.5, .8660254/3.);
    d.x = torDist(p1) - rSm;        

    d.x = abs(d.x);
    a = atan(p1.y, p1.x);
    gA.x = a;
    // One dot per small arc segment. Three make up a circle.
    //gRnd.x = hash21(h.zw + floor(a/6.283*3.));
    //a = mod(a/3.14159265, 2.) - 1.;

    // Small arc two.
    p2 = p - vec2(-.5, .8660254/3.);
    d.y = torDist(p2) - rSm;

    d.y = abs(d.y);
    a = atan(p2.y, p2.x);
    gA.y = a;
    //gRnd.y = hash21(h.zw + floor(a/6.283*3.) + 7.45);
    //a = mod(a/3.14159265, 2.) - 1.;

    // Small arc three.
    p3 = p - vec2(0, -.8660254*2./3.);
    d.z = torDist(p3) - rSm;

    d.z = abs(d.z);
    a = atan(p3.y, p3.x);
    gA.z = a;
    //gRnd.z = hash21(h.zw + floor(a/6.283*3.) + 9.23);
    //a = mod(a/3.14159265, 2.) - 1.;
    
    //d.xyz -= th;
         
         
	#else
 
    // Large arc one.
    p1 = p - vec2(1, 0); 
    a = atan(p1.y, p1.x); // Longer arc.
    gA.x = a*3.;
    d.x = torDist(p1) - rLg;
    d.x = abs(d.x);
    //gRnd.x = hash21(h.zw + floor(a/6.283*6.) + 8.71);
    gHgt.x = (cos(a*6.) + 1.)*.05*hDir;
    //a = mod(a/3.14159265, 2.) - 1.;
    //hgt.x = (1. - abs(a*6.));
    //hgt.x = smoothstep(.0, .95, hgt.x)*hDir*.1;



    // Large arc two.
    p2 = p - r2(3.14159265/3.)*vec2(1, 0);
    a = atan(p2.y, p2.x); // Longer arc.
    gA.y = a*3.;
    d.y = torDist(p2) - rLg;
    d.y = abs(d.y);
    //gRnd.y = hash21(h.zw + floor(a/6.283*6.) + 3.87);
    gHgt.y = -(cos(a*6.) + 1.)*.05*hDir;
    //a = mod(a/3.14159265, 2.) - 1.;
    //hgt.y = (1. - abs(a*6. + 2.));
    //hgt.y = -smoothstep(.0, .95, hgt.y)*hDir*.1;


    // Small arc.
    p3 = p - r2(-3.14159265/3.)*vec2(0, .57735);
    a = atan(p3.y, p3.x); // Smaller arc.
    gA.z = a;
    d.z = torDist(p3) - rSm;
    d.z = abs(d.z);
    //gRnd.z = hash21(h.zw + floor(a/6.283*3.) + 5.54);
    //a = mod(a/3.14159265, 2.) - 1.;
    
    //d.xyz -= th;

    
    #endif
    
    
/////
    
    #ifdef SHOW_GRID
    
    // JOINER BLOCKS
    //
    // The joins need to be rendered seperately using an offset hexagonal grid, due to pixelated 
    // mismatching at the hexagonal boundaries when highlighting... It's a long story... :)
    //
    // Three blocks, arranged in a tri blade propellor scheme. 
    //     
    vec2 q = h2.xy;
    float blc = abs(length(q) - .8660254/3.);// - th;
    
    q = r2(3.14159/6.)*q;
    a = atan(q.y, q.x);
    float ia = floor(a/6.283*3.) + .5;
    q = r2(ia*6.283/3.)*q;
    q.x -= .8660254/3.;
        
    q = abs(q);// - vec2(th, .04);
    // q.x = abs(q.x - .08);// - .2;
    
    // Holding the joiner block distance field value in a global variable to be 
    // used elsewhere.
    // The weird mix function gives the joins a slight pinch at the ends to feed the illusion a little more.
    //d.w = mix(length(q) - .04, max(q.x - .15, q.y - .04), .9);
    d.w = max(q.x - .2, q.y - .02);//
    //d.w = length(q.xy) - .2;//.06333;//
    //float blocks = mix(length(q) - .04, min(max(q.x - .15, q.y - .04), 0.) + length(max(q - vec2(.15, .04), 0.)), .9);
    // The shade value (used for highlighting) has been tweaked in an unnatural way to give the reflected
    // look I was after, but I wouldn't put too much stock in it.
    
    #endif
    
    return d;
    
    
}




// Distance field function.
float m(vec3 p){
    
    
    //p.xy -= path(p.z);
    
    //float sf = .5;//dot(sin(p*3. - cos(p.yzx*3.)), vec3(.333));
    
    // Moving the scene down a bit... I should probably moved the camera up. :)
    p.y += 1.5;
    
    
    // The floor plane. Set roughly at the bottom of the Truchet object.
    float fl = .25 + p.y;// + (.5 - sf)*.5;
    
    
    // The hexagonal Truchet object - weaved or otherwise.
    const float sc = 1.;
    vec4 d = Truchet(p.xz*sc);
 
	// The Truchet function above retuns the toroidal arc distances, which have the poloidal
    // function applied to produce the torus like arcs. There are three arcs in all.
    d.x = polDist(vec2(d.x/sc, p.y + gHgt.x));
    d.y = polDist(vec2(d.y/sc, p.y + gHgt.y));    
    d.z = polDist(vec2(d.z/sc, p.y + gHgt.z));  
    
    d.xyz -= .16/sc; // Give the torus-like arcs some thickness.
    
    
    gD = d; // Storing all three distance functions above into a global to be used elsewhere.
    
    
    // The Truchet object distance, which is the minimum of the three arcs.
    float ob = min(min(d.x, d.y), d.z);
 
    // Smoothly imprinting the Truchet object into the floor a bit. Not sure why, but it seemed
    // like a good idea at the time. :)
    fl = smax(fl, -ob*2., .5) - .2;
    
    // Hacky hexagonal grid display.
    #ifdef SHOW_GRID
    d.w = max(d.w/sc, p.y - .25);
    fl = min(fl, d.w);
    #endif
 
    // Object ID: Either the floor or the Truchet object.
    objID = fl<ob? 0. : 1.;
    
    // Return the minimum distance.
    return min(fl, ob);
    
}


// This is an exact duplicate of the distance function with some "atan" calculations thrown in. It's a bit
// of a waste of code, but worth it to take the three "atan" calculations out of the distance function
// loop... but such a character waste. I'll try to amalgamate some things later.
float m2(vec3 p){
    
    
    //p.xy -= path(p.z);
    
    //float sf = .5;//dot(sin(p*3. - cos(p.yzx*3.)), vec3(.333));
    
    // Moving the scene down a bit... I should probably moved the camera up. :)
    p.y += 1.5;
    
    
    // The floor plane. Set roughly at the bottom of the Truchet object.
    float fl = .25 + p.y;// + (.5 - sf)*.5;
    
    
    // The hexagonal Truchet object - weaved or otherwise.
    const float sc = 1.;
    vec4 d = Truchet(p.xz*sc);
    
    
    gA2.x = atan(p.y + gHgt.x, d.x/sc);
    gA2.y = atan(p.y + gHgt.y, d.y/sc);
    gA2.z = atan(p.y + gHgt.z, d.z/sc);

	
	// The Truchet function above retuns the toroidal arc distances, which have the poloidal
    // function applied to produce the torus like arcs. There are three arcs in all.
    d.x = polDist(vec2(d.x/sc, p.y + gHgt.x));
    d.y = polDist(vec2(d.y/sc, p.y + gHgt.y));    
    d.z = polDist(vec2(d.z/sc, p.y + gHgt.z));  
    
    d.xyz -= .16/sc; // Give the torus-like arcs some thickness.
    
    
    gD = d; // Storing all three distance functions above into a global to be used elsewhere.
    
    
    // The Truchet object distance, which is the minimum of the three arcs.
    float ob = min(min(d.x, d.y), d.z);
 
    // Smoothly imprinting the Truchet object into the floor a bit. Not sure why, but it seemed
    // like a good idea at the time. :)
    fl = smax(fl, -ob*2., .5) - .2;
    
    // Hacky hexagonal grid display.
    #ifdef SHOW_GRID
    d.w = max(d.w/sc, p.y - .25);
    fl = min(fl, d.w);
    #endif
 
    // Object ID: Either the floor or the Truchet object.
    objID = fl<ob? 0. : 1.;
    
    // Return the minimum distance.
    return min(fl, ob);
    
}


/////////
// The bump function.
float bumpFunc(vec3 p, vec3 n){
    
    // Place a square Truchet pattern on the ground, or the wrap it around
    // the snake-like Truchet object.

 
    
    float d = m2(p);
    
    float c = 0.;
    
    if(objID<.5) {     
        
        c =  sTruchet(p.xz*6.);//*.95 +  sTruchet(p.xz*12.)*.05;
         
    }
    else {
    
        // Obtain the nearest toroidal and poloidal angles, convert them to cylindrical
        // coordinates, then pass them to a wrappable function. In this case, it's a simple
        // square Truchet function.
        float a;
        float a2;

        if(gD.x<gD.y && gD.x<gD.z){
            a = gA.x;
            a2 = gA2.x;
        }
        else if(gD.y<gD.z){
            a = gA.y;
            a2 = gA2.y;
        }
        else {
            a = gA.z;
            a2 = gA2.z;

        }

        // "a2" represents the toroidal axis, and "a" the poloidal axis. Due to the nature of the hexagonal
        // Truchet geometry, "a" needs to be a multiple of 12, in most cases. "a2" is more flexible, but needs
        // to be a multiple of 4, in this particular case... 6 might work with an offset.

        c = sTruchet(vec2(a2*8., a*12.)/6.283);
        //c = sTruchet(vec2(a2*12., a*24.)/6.283); // More detailed, but a little too busy.
        

        // More interesting, but a little too busy for this example.
        //c = sTruchet(vec2(a2*8., a*12.)/6.283)*.975 + sTruchet(vec2(a2*16., a*24.)/6.283)*.025;

        // For testing purposes.
        //c = (cos(a*12.) + cos(a2*8.))*.05 + .1;
        
        
    }
   
   
    
    //float c = min(min(svGd.x, svGd.y), svGd.z);
    // Note that I could perform two returns and dispense with the float declaration,
    // but some graphics cards used to complain. I think all of them should be
    // fine now, but just in case.
    return c;//*.998 + n3D(p*192.)*.002; 

}

// Standard function-based bump mapping function with some edging thrown into the mix.
vec3 doBumpMap(in vec3 p, in vec3 n, float bumpfactor, inout float edge, inout float crv){
    
    // Resolution independent sample distance... Basically, I want the lines to be about
    // the same pixel with, regardless of resolution... Coding is annoying sometimes. :)
    vec2 e = vec2(1./iResolution.y, 0); 
    
    float f = bumpFunc(p, n); // Hit point function sample.
    
    float fx = bumpFunc(p - e.xyy, n); // Nearby sample in the X-direction.
    float fy = bumpFunc(p - e.yxy, n); // Nearby sample in the Y-direction.
    float fz = bumpFunc(p - e.yyx, n); // Nearby sample in the Y-direction.
    
    float fx2 = bumpFunc(p + e.xyy, n); // Sample in the opposite X-direction.
    float fy2 = bumpFunc(p + e.yxy, n); // Sample in the opposite Y-direction.
    float fz2 = bumpFunc(p + e.yyx, n);  // Sample in the opposite Z-direction.
    
     
    // The gradient vector. Making use of the extra samples to obtain a more locally
    // accurate value. It has a bit of a smoothing effect, which is a bonus.
    vec3 grad = vec3(fx - fx2, fy - fy2, fz - fz2)/(e.x*2.);  
    //vec3 grad = (vec3(fx, fy, fz ) - f)/e.x;  // Without the extra samples.


    // Using the above samples to obtain an edge value. In essence, you're taking some
    // surrounding samples and determining how much they differ from the hit point
    // sample. It's really no different in concept to 2D edging.
    edge = abs(fx + fy + fz + fx2 + fy2 + fz2 - 6.*f);
    edge = smoothstep(0., 1., edge/e.x*2.);
    
    
    // We may as well use the six measurements to obtain a rough curvature value while we're at it.
    //crv = clamp((fx + fy + fz + fx2 + fy2 + fz2 - 6.*f)*32. + .5, 0., 2.);
    
    // Some kind of gradient correction. I'm getting so old that I've forgotten why you
    // do this. It's a simple reason, and a necessary one. I remember that much. :D
    grad -= n*dot(n, grad);          
                      
    return normalize(n + grad*bumpfactor); // Bump the normal with the gradient vector.
	
}



////////

/*
// Tetrahedral normal, to save a couple of "map" calls. Courtesy of IQ.
vec3 nr(in vec3 p){

    // Note the slightly increased sampling distance, to alleviate artifacts due to hit point inaccuracies.
    vec2 e = vec2(0.005, -0.005); 
    return normalize(e.xyy * m(p + e.xyy) + e.yyx * m(p + e.yyx) + e.yxy * m(p + e.yxy) + e.xxx * m(p + e.xxx));
}
*/


// Standard normal function - for comparison with the one below.
vec3 nr(in vec3 p) {
	const vec2 e = vec2(0.002, 0);
	return normalize(vec3(m(p + e.xyy) - m(p - e.xyy), m(p + e.yxy) - m(p - e.yxy),	m(p + e.yyx) - m(p - e.yyx)));
}




// Cheap shadows are the bain of my raymarching existence, since trying to alleviate artifacts is an excercise in
// futility. In fact, I'd almost say, shadowing - in a setting like this - with limited  iterations is impossible... 
// However, I'd be very grateful if someone could prove me wrong. :)
float shad(vec3 ro, vec3 lp, float k, float t){

    // More would be nicer. More is always nicer, but not really affordable... Not on my slow test machine, anyway.
    const int maxIterationsShad = 24; 
    
    vec3 rd = lp - ro; // Unnormalized direction ray.

    float shade = 1.;
    float dist = .001*(t*.125 + 1.);  // Coincides with the hit condition in the "trace" function.  
    float end = max(length(rd), 0.0001);
    //float stepDist = end/float(maxIterationsShad);
    rd /= end;

    // Max shadow iterations - More iterations make nicer shadows, but slow things down. Obviously, the lowest 
    // number to give a decent shadow is the best one to choose. 
    for (int i=0; i<maxIterationsShad; i++){

        float h = m(ro + rd*dist);
        //shade = min(shade, k*h/dist);
        shade = min(shade, smoothstep(0.0, 1.0, k*h/dist)); // Subtle difference. Thanks to IQ for this tidbit.
        // So many options here, and none are perfect: dist += min(h, .2), dist += clamp(h, .01, stepDist), etc.
        dist += clamp(h, .01, .2); 
        
        // Early exits from accumulative distance function calls tend to be a good thing.
        if (h<0.0 || dist > end) break; 
    }

    // I've added a constant to the final shade value, which lightens the shadow a bit. It's a preference thing. 
    // Really dark shadows look too brutal to me. Sometimes, I'll add AO also, just for kicks. :)
    return min(max(shade, 0.), 1.); 
}




// I keep a collection of occlusion routines... OK, that sounded really nerdy. :)
// Anyway, I like this one. I'm assuming it's based on IQ's original.
float cAO(in vec3 p, in vec3 n)
{
	float sca = 1., occ = 0.;
    for(float i=0.; i<5.; i++){
    
        float hr = .01 + i*.5/4.;        
        float dd = m(n * hr + p);
        occ += (hr - dd)*sca;
        sca *= 0.7;
    }
    return clamp(1.0 - occ, 0., 1.);    
}



// Standard hue rotation formula... compacted down a bit.
vec3 rotHue(vec3 p, float a){

    vec2 cs = sin(vec2(1.570796, 0) + a);

    mat3 hr = mat3(0.299,  0.587,  0.114,  0.299,  0.587,  0.114,  0.299,  0.587,  0.114) +
        	  mat3(0.701, -0.587, -0.114, -0.299,  0.413, -0.114, -0.300, -0.588,  0.886) * cs.x +
        	  mat3(0.168,  0.330, -0.497, -0.328,  0.035,  0.292,  1.250, -1.050, -0.203) * cs.y;
							 
    return clamp(p*hr, 0., 1.);
}


// Simple environment mapping. Pass the reflected vector in and create some
// colored noise with it. The normal is redundant here, but it can be used
// to pass into a 3D texture mapping function to produce some interesting
// environmental reflections.
//
// More sophisticated environment mapping:
// UI easy to integrate - XT95    
// https://www.shadertoy.com/view/ldKSDm
vec3 eMap(vec3 rd, vec3 sn){
    
    vec3 sRd = rd; // Save rd, just for some mixing at the end.
    
    // Add a time component, scale, then pass into the noise function.
    rd.xy -= iTime*.25;
    rd *= 3.;
    
    //vec3 tx = tex3D(iChannel0, rd/3., sn);
    //float c = dot(tx*tx, vec3(.299, .587, .114));
    
    float c = n3D(rd)*.57 + n3D(rd*2.)*.28 + n3D(rd*4.)*.15; // Noise value.
    c = smoothstep(0.5, 1., c); // Darken and add contast for more of a spotlight look.
    
    //vec3 col = vec3(c, c*c, c*c*c*c).zyx; // Simple, warm coloring.
    //vec3 col = vec3(min(c*1.5, 1.), pow(c, 2.5), pow(c, 12.)).zyx; // More color.
    vec3 col = pow(vec3(1.5, 1, 1)*c, vec3(1, 2.5, 12)).zyx; // More color.
    
    // Mix in some more red to tone it down and return.
    return mix(col, col.yzx, sRd*.25 + .25); 
    
}

void mainImage(out vec4 fCol, vec2 fCoord){

    // Screen coordinates.
	vec2 u = (fCoord - iResolution.xy*.5) / iResolution.y;
    
	
	// Camera Setup.
	vec3 lk = vec3(0, 0, iTime*1.);  // "Look At" position.
	vec3 o = lk + vec3(0, .3, -.25); // Camera position, doubling as the ray origin.

   
    // Light position. Set in the vicinity the ray origin.
    vec3 l = o + vec3(0, .5, 2.);
    
	// Using the Z-value to perturb the XY-plane.
	// Sending the camera, "look at," and two light vectors down the tunnel. The "path" function is 
	// synchronized with the distance function. Change to "path2" to traverse the other tunnel.
	lk.xy += path(lk.z);
	o.xy += path(o.z);
	l.xy += path(l.z);

    // Using the above to produce the unit ray-direction vector.
    float FOV = 3.14159/3.; // FOV - Field of view.
    vec3 forward = normalize(lk-o);
    vec3 right = normalize(vec3(forward.z, 0., -forward.x )); 
    vec3 up = cross(forward, right);

    // r - Ray direction.
    vec3 r = normalize(forward + FOV*u.x*right + FOV*u.y*up);
    /////////
    
    // Standard raymarching routine.
    float d, t = 0.;
    
    for(int i=0; i<96;i++){
        
        d = m(o + r*t);
        // There isn't really a far plane to go beyond, but it's there anyway.
        if(abs(d)<.001*(t*.125 + 1.) || t>FAR) break;
        t += d;

    }
    
    t = min(t, FAR);
    
    // Save the object ID right after the final distance function call.
    float svID = objID;
    
    
    // Set the initial scene color to black.
    vec3 col = vec3(0);
    
     
    if(t<FAR){
    
        // Hit point and normal at the hit point.
        vec3 p = o + r*t, n = nr(p);
        
        // Bump mapping with bumped edging and curvature - The latter isn't used here.
        float edge2 = 0., crv2 = 1., bf = .25; 
        //if(svID<.5) bf = .5;
        n = doBumpMap(p, n, bf, edge2, crv2); ///(1. + t/FAR*.125)
        
        
        // Shadows and ambient occlusion.
        float sh = shad(p + n*.002, l, 16., t);
        float ao = cAO(p, n);

        l -= p; // Light to surface vector. Ie: Light direction vector.
        d = max(length(l), 0.001); // Light to surface distance.
        l /= d; // Normalizing the light direction vector.
 
        
        // Texture value at the surface. Use the heighmap value above to distort the
        // texture a bit.
        float txSc = .5;
        vec3 tx = vec3(0.5, 0.5, 0.5); // tex3D(iChannel0, (p*txSc), n);
        tx = smoothstep(.0, .5, tx);

       
        col = tx; //vec3(1)*fBm; // Initializing to the texture color.
         
        // Addind a sprinking of noise. The scene looks a little too clean without it.
        float fBm = n3D(p*128.)*.66 + n3D(p*256.)*.34;
        col *= mix(vec3(0), vec3(1), fBm*2.*.5 + .5);
        
 
        // Golden coloring for the Truchet and greyish coloring for the ground.
        if(svID>.5){
            //col *= max(1. - bumpFunc(p, n)*1.5, 0.);
        	col *= mix(vec3(2, 1, .3), vec3(.1, 0, 0), bumpFunc(p, n)*1.5);
            //col *= vec3(1, .1, .2)*1.5;
                 
        }
        else {
            //col *= max(1. - bumpFunc(p, n)*1.5, 0.);
            col *= vec3(.8, .6, .4);
        }
        
        
        // Diffuse and specular.        
        float df = max(dot(l, n), 0.); // Diffuse.
        df = pow(df, 4.)*2.;
        float sp = pow(max(dot(reflect(-l, n), -r), 0.), 32.); // Specular.
        
        
		// Applying some diffuse and specular lighting to the surface.
        col = col*(df + .5*ao) + vec3(1, .97, .92)*sp*2.;
        
        // Add the fake environmapping. Not as good as a reflective pass, but it gives that
        // impresssion for just a fraction of the cost.
        vec3 em = eMap(reflect(r, n), n); // Fake environment mapping.
        col += em*1.5;
        
        // Edges.
        col *= 1. - edge2*.65; // Bump mapped edgingy.  
        
        // Attenuation, based on light to surface distance.    
        col *= 1./(1. + d*d*.1);
        
        // Shadows and AO application.
        col *= (sh + ao*.3)*ao;
        

        
    }
    
    
    // APPLYING FOG
    // Blend in a bit of light fog for atmospheric effect.
    vec3 fogCol = vec3(0);//vec3(.7, .8, 1.)*(rd.y*.5 + .5)*2.5;
    col = mix(col, fogCol, smoothstep(0., .95, t/FAR)); // exp(-.002*t*t), etc.
    
    
    // Vignette.
    u = fCoord/iResolution.xy;
    col = mix(col, vec3(0), (1. - pow(16.*u.x*u.y*(1.-u.x)*(1.-u.y), 0.25))*.5);
    
    // Apply some statistically unlikely (but close enough) 2.0 gamma correction. :)
    fCol = vec4(sqrt(clamp(col, 0., 1.)), 1.);
}

