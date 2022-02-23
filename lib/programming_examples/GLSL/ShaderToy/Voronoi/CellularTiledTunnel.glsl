//stage GL_FRAGMENT_SHADER
//import <GLUP/ShaderToy.h>

/*

    Cellular Tiled Tunnel
    ---------------------
    
    I've always liked the look of a 2nd order Voronoi surface. IQ's Leizex demo is a great
	prerendered example, and for anyone who can remember, Tomasz Dobrowolski's Suboceanic
	was cutting edge back in the day.

	Anyway, I've always wanted to get a proper example working in a shader... Yeah, I need
	bigger dreams. :) Unfortunately, I kind of realized that it wasn't going to be possible 
	until GPUs become even faster than they already are, so I figured I'd try the next best 
	thing and come up with a way to emulate the look with something cheap. This is the 
	result. It's not perfect, but it looks surprisingly similar.

	The regular 2nd order Voronoi algorithm involves a "lot" of operations. In general,
	27 cell checks - all involving a bunch of vector arithmetic, fract, sin, floor, 
	comparisons, etc... It's possible to cut down on cell checks, perform a bunch of
	optimizations, etc, but it's still too much work for a raymarcher.

	The surface here is produced via a repeat 3D tile approach. The look is achieved by 
	performing 2nd order distance checks on the tiles. I used a highly scientific approach
	which involved crossing my fingers, doing the distance checks and hoping for the best. :)
	Amazingly, it produced the result I was looking for.

	I covered the tile construction in other "cell tile" examples, so I'll spare you the 
	details, but it's pretty simple. The only additions here are the second order distance
	checks.

	In order to show the surface itself, I've made the example geometric looking - I hope
	you like brown, or whatever color that is. :) Note that individual cell regions are 
	colored	differently. I did that to show that it could be done, but I'm not convinced 
	that it adds to the aesthetics in any meaningful way.

	Anyway, I have a few more interesting examples that I'll put up pretty soon.
	
    Related examples: 

    Cellular Tiling - Shane
    https://www.shadertoy.com/view/4scXz2

	// For comparison, this example uses the standard 2nd order Voronoi algorithm. For fun,
	// I dropped the cell tile routine into it and it ran a lot faster.
	Voronoi - rocks - iq
	https://www.shadertoy.com/view/MsXGzM

	rgba leizex - Inigo Quilez
	http://www.pouet.net/prod.php?which=51829
	https://www.youtube.com/watch?v=eJBGj8ggCXU
	http://www.iquilezles.org/prods/index.htm

	Tomasz Dobrowolski - Suboceanic
	http://www.pouet.net/prod.php?which=18343

*/

#define PI 3.14159265358979
#define FAR 50. // Maximum allowable ray distance.

// Grey scale.
float getGrey(vec3 p){ return p.x*0.299 + p.y*0.587 + p.z*0.114; }

// Non-standard vec3-to-vec3 hash function.
vec3 hash33(vec3 p){ 
    
    float n = sin(dot(p, vec3(7, 157, 113)));    
    return fract(vec3(2097152, 262144, 32768)*n); 
}

// 2x2 matrix rotation.
mat2 rot2(float a){
    
    float c = cos(a); float s = sin(a);
	return mat2(c, s, -s, c);
}

// Tri-Planar blending function. Based on an old Nvidia tutorial.
vec3 tex3D( sampler2D tex, in vec3 p, in vec3 n ){
  
    n = max((abs(n) - 0.2)*7., 0.001); // max(abs(n), 0.001), etc.
    n /= (n.x + n.y + n.z );  
    
	vec3 tx = (texture(tex, p.yz)*n.x + texture(tex, p.zx)*n.y + texture(tex, p.xy)*n.z).xyz;
    
    return tx*tx;
}


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
float drawSphere(in vec3 p){
    
    // Anything that wraps the domain will suffice, so any of the following will work.
    //p = cos(p*3.14159)*0.5; 
    //p = abs(cos(p*3.14159)*0.5);    
    p = fract(p)-.5;    
    
    return dot(p, p);
    
    // Other metrics to try.
    
    //p = abs(fract(p)-.5);
    //return dot(p, vec3(.5));
    
    //p = abs(fract(p)-.5);
    //return max(max(p.x, p.y), p.z);
    
    //p = cos(p*3.14159)*0.5; 
    //p = abs(cos(p*3.14159)*0.5);
    //p = abs(fract(p)-.5);
    //return max(max(p.x - p.y, p.y - p.z), p.z - p.x);
    //return min(min(p.x - p.y, p.y - p.z), p.z - p.x);
    
}

// Faster (I'm assuming), more streamlined version. See the comments below for an expanded explanation.
// The function below is pretty quick also, and can be expanded to include more spheres. This one
// takes advantage of the fact that only four object need sorting. With three spheres, it'd be even
// better.
float cellTile(in vec3 p){
    
    // Draw four overlapping objects (spheres, in this case) at various positions throughout the tile.
    vec4 v, d; 
    d.x = drawSphere(p - vec3(.81, .62, .53));
    p.xy = vec2(p.y-p.x, p.y + p.x)*.7071;
    d.y = drawSphere(p - vec3(.39, .2, .11));
    p.yz = vec2(p.z-p.y, p.z + p.y)*.7071;
    d.z = drawSphere(p - vec3(.62, .24, .06));
    p.xz = vec2(p.z-p.x, p.z + p.x)*.7071;
    d.w = drawSphere(p - vec3(.2, .82, .64));

    v.xy = min(d.xz, d.yw), v.z = min(max(d.x, d.y), max(d.z, d.w)), v.w = max(v.x, v.y); 
   
    d.x =  min(v.z, v.w) - min(v.x, v.y); // Maximum minus second order, for that beveled Voronoi look. Range [0, 1].
    //d.x =  min(v.x, v.y);
        
    return (d.x*2.66); // Normalize... roughly.
    
}

/*
// Draw some spheres throughout a repeatable cubic tile. The offsets were partly based on 
// science, but for the most part, you could choose any combinations you want. Note the 
// normalized planar positional roation between sphere rendering to really mix things up. This 
// particular function is used by the raymarcher, so involves fewer spheres.
//
float cellTile(in vec3 p){

    // Storage for the closest distance metric, second closest and the current
    // distance for comparisson testing.
    //
    // Set the maximum possible value - dot(vec3(.5), vec3(.5)). I think my reasoning is
    // correct, but I have lousy deductive reasoning, so you may want to double check. :)
    vec3 d = (vec3(.75)); 
   
    
    // Draw some overlapping objects (spheres, in this case) at various positions on the tile.
    // Then do the fist and second order distance checks. Very simple.
    d.z = drawSphere(p - vec3(.81, .62, .53));
    d.x = min(d.x, d.z); //d.y = max(d.x, min(d.y, d.z)); // Not needed on the first iteration.
    
    p.xy = vec2(p.y-p.x, p.y + p.x)*.7071;
    d.z = drawSphere(p - vec3(.39, .2, .11));
    d.y = max(d.x, min(d.y, d.z)); d.x = min(d.x, d.z);
    
    p.yz = vec2(p.z-p.y, p.z + p.y)*.7071;
    d.z = drawSphere(p - vec3(.62, .24, .06));
    d.y = max(d.x, min(d.y, d.z)); d.x = min(d.x, d.z);
    
    p.xz = vec2(p.z-p.x, p.z + p.x)*.7071; 
    d.z = drawSphere(p - vec3(.2, .82, .64));
    d.y = max(d.x, min(d.y, d.z)); d.x = min(d.x, d.z);

     
	// More spheres means better patterns, but slows things down.
    //p.xy = vec2(p.y-p.x, p.y + p.x)*.7071;
    //d.z = drawSphere(p - vec3(.48, .29, .2));
    //d.y = max(d.x, min(d.y, d.z)); d.x = min(d.x, d.z);
    
    //p.yz = vec2(p.z-p.y, p.z + p.y)*.7071;
    //d.z = drawSphere(p - vec3(.06, .87, .78));
    //d.y = max(d.x, min(d.y, d.z)); d.x = min(d.x, d.z); 
	

    
    // Returning what I'm hoping is a normalized result. Not that it
    // matters too much, but I'd like it normalized.
    // 2.66 seems to work, but I'll double check at some stage.
    // d.x: Minimum distance. Regular round Voronoi looking.
    // d.y - d.x - Maximum minus minimum, for that beveled Voronoi look.
    //
    return (d.y - d.x)*2.66; 
    //return 1. - d.x*2.66;
    //return 1. - sqrt(d.x)*1.63299; // etc.

    
}
*/

// Just like the function above, but used to return the regional cell ID...
// kind of. Either way, it's used to color individual raised sections in
// the same way that a regular Voronoi function can. It's only called once,
// so doesn't have to be particularly fast. It's kept separate to the
// raymarched version, because you don't want to be performing ID checks
// several times a frame when you don't have to. By the way, that applies
// to identifying any object in any scene.
//
// By the way, it's customary to bundle the respective distance and cell
// ID into a vector (vec3(d.x, d.y, cellID)) and return that, but I'm 
// keeping it simple here.
//
int cellTileID(in vec3 p){
    
    int cellID = 0;
    
    // Storage for the closest distance metric, second closest and the current
    // distance for comparisson testing.
    vec3 d = (vec3(.75)); // Set the maximum.
    
    // Draw some overlapping objects (spheres, in this case) at various positions on the tile.
    // Then do the fist and second order distance checks. Very simple.
    d.z = drawSphere(p - vec3(.81, .62, .53)); if(d.z<d.x) cellID = 1;
    d.y = max(d.x, min(d.y, d.z)); d.x = min(d.x, d.z);
    
    p.xy = vec2(p.y-p.x, p.y + p.x)*.7071;
    d.z = drawSphere(p - vec3(.39, .2, .11)); if(d.z<d.x) cellID = 2;
    d.y = max(d.x, min(d.y, d.z)); d.x = min(d.x, d.z);
    
    
    p.yz = vec2(p.z-p.y, p.z + p.y)*.7071;
    d.z = drawSphere(p - vec3(.62, .24, .06)); if(d.z<d.x) cellID = 3;
    d.y = max(d.x, min(d.y, d.z)); d.x = min(d.x, d.z);
   
    p.xz = vec2(p.z-p.x, p.z + p.x)*.7071; 
    d.z = drawSphere(p - vec3(.2, .82, .64)); if(d.z<d.x) cellID = 4;
    d.y = max(d.x, min(d.y, d.z)); d.x = min(d.x, d.z);

/* 
    p.xy = vec2(p.y-p.x, p.y + p.x)*.7071;
    d.z = drawSphere2(p - vec3(.48, .29, .2)); if(d.z<d.x) cellID = 5;
    d.y = max(d.x, min(d.y, d.z)); d.x = min(d.x, d.z);
    
    p.yz = vec2(p.z-p.y, p.z + p.y)*.7071;
    d.z = drawSphere2(p - vec3(.06, .87, .78)); if(d.z<d.x) cellID = 6;
    d.y = max(d.x, min(d.y, d.z)); d.x = min(d.x, d.z); 
*/ 
    
    return cellID;
    
}


// The path is a 2D sinusoid that varies over time, depending upon the frequencies, and amplitudes.
vec2 path(in float z){ float s = sin(z/24.)*cos(z/16.); return vec2(s*9., 0); }

// Standard tunnel distance function with some perturbation thrown into the mix. A tunnel is just a tube 
// with a smoothly shifting center as you traverse lengthwise. The walls of the tube are perturbed by the
// cheap 3D surface function I described above.
float map(vec3 p){

    
    float sf = cellTile(p/2.5);
    
    // Tunnel bend correction, of sorts. Looks nice, but slays framerate, which is disappointing. I'm
    // assuming that "tan" function is the bottleneck, but I can't be sure.
    //vec2 g = (path(p.z + 0.1) - path(p.z - 0.1))/0.2;
    //g = cos(atan(g));
    p.xy -= path(p.z);
    //p.xy *= g;
  
    // Round tunnel.
    // For a round tunnel, use the Euclidean distance: length(p.xy).
    return 1.- length(p.xy*vec2(0.5, 0.7071)) + (0.5-sf)*.35;

    
/*
    // Rounded square tunnel using Minkowski distance: pow(pow(abs(tun.x), n), pow(abs(tun.y), n), 1/n)
    vec2 tun = abs(p.xy)*vec2(0.5, 0.7071);
    tun = pow(tun, vec2(8.));
    float n =1.-pow(tun.x + tun.y, 1.0/8.) + (0.5-sf)*.35;
    return n;//min(n, p.y + FH);
*/
    
/*
    // Square tunnel.
    // For a square tunnel, use the Chebyshev(?) distance: max(abs(tun.x), abs(tun.y))
    vec2 tun = abs(p.xy - path(p.z))*vec2(0.5, 0.7071);
    float n = 1.- max(tun.x, tun.y) + (0.5-sf)*.5;
    return n;
*/
 
}


// Basic raymarcher.
float trace(in vec3 ro, in vec3 rd){

    float t = 0.0, h;
    for(int i = 0; i < 96; i++){
    
        h = map(ro+rd*t);
        // Note the "t*b + a" addition. Basically, we're putting less emphasis on accuracy, as
        // "t" increases. It's a cheap trick that works in most situations... Not all, though.
        if(abs(h)<0.002*(t*.125 + 1.) || t>FAR) break; // Alternative: 0.001*max(t*.25, 1.)
        t += h*.8;
        
    }

    return min(t, FAR);
    
}

// Texture bump mapping. Four tri-planar lookups, or 12 texture lookups in total. I tried to 
// make it as concise as possible. Whether that translates to speed, or not, I couldn't say.
vec3 doBumpMap( sampler2D tx, in vec3 p, in vec3 n, float bf){
   
    const vec2 e = vec2(0.001, 0);
    
    // Three gradient vectors rolled into a matrix, constructed with offset greyscale texture values.    
    mat3 m = mat3( tex3D(tx, p - e.xyy, n), tex3D(tx, p - e.yxy, n), tex3D(tx, p - e.yyx, n));
    
    vec3 g = vec3(0.299, 0.587, 0.114)*m; // Converting to greyscale.
    g = (g - dot(tex3D(tx,  p , n), vec3(0.299, 0.587, 0.114)) )/e.x; g -= n*dot(n, g);
                      
    return normalize( n + g*bf ); // Bumped normal. "bf" - bump factor.
    
}


// Tetrahedral normal, to save a couple of "map" calls. Courtesy of IQ. By the way, there is an 
// aesthetic difference between this and the regular six tap version. Sometimes, it's noticeable,
// and other times, like this example, it's not.
vec3 calcNormal(in vec3 p){

    // Note the slightly increased sampling distance, to alleviate artifacts due to hit point inaccuracies.
    vec2 e = vec2(0.0025, -0.0025); 
    return normalize(e.xyy * map(p + e.xyy) + e.yyx * map(p + e.yyx) + e.yxy * map(p + e.yxy) + e.xxx * map(p + e.xxx));
}

/*
// Standard normal function. 6 taps.
vec3 calcNormal(in vec3 p) {
	const vec2 e = vec2(0.005, 0);
	return normalize(vec3(map(p + e.xyy) - map(p - e.xyy), map(p + e.yxy) - map(p - e.yxy),	map(p + e.yyx) - map(p - e.yyx)));
}
*/

// Ambient occlusion, for that self shadowed look. Based on the original by XT95. I love this 
// function, and in many cases, it gives really, really nice results. For a better version, and 
// usage, refer to XT95's examples below:
//
// Hemispherical SDF AO - https://www.shadertoy.com/view/4sdGWN
// Alien Cocoons - https://www.shadertoy.com/view/MsdGz2
float calculateAO( in vec3 p, in vec3 n )
{
	float ao = 0.0, l;
    const float maxDist = 2.;
	const float nbIte = 6.0;
	//const float falloff = 0.9;
    for( float i=1.; i< nbIte+.5; i++ ){
    
        l = (i*.75 + fract(cos(i)*45758.5453)*.25)/nbIte*maxDist;
        
        ao += (l - map( p + n*l ))/(1.+ l);// / pow(1.+l, falloff);
    }
	
    return clamp(1.- ao/nbIte, 0., 1.);
}

// Cool curve function, by Shadertoy user, Nimitz.
//
// From an intuitive sense, the function returns a weighted difference between a surface 
// value and some surrounding values. Almost common sense... almost. :)
//
// Original usage (I think?) - Cheap curvature: https://www.shadertoy.com/view/Xts3WM
// Other usage: Xyptonjtroz: https://www.shadertoy.com/view/4ts3z2
float curve(in vec3 p, in float w){

    vec2 e = vec2(-1., 1.)*w;
    
    float t1 = map(p + e.yxx), t2 = map(p + e.xxy);
    float t3 = map(p + e.xyx), t4 = map(p + e.yyy);
    
    return 0.125/(w*w) *(t1 + t2 + t3 + t4 - 4.*map(p));
}

/*
// Oldschool hatching effect. Interesting under the right circumstances.
vec3 ch(in vec3 col, in vec2 fragCoord){
    
    vec3 fColor = col;
    
    float lum = dot(col, vec3(.299, .587, .114));// length(col);
	float mx = 1./7.; // 1.732/7.;
    
    float rgt = fragCoord.x + fragCoord.y;
    float lft = fragCoord.x - fragCoord.y;
    
    fColor = col*4.; col *= .6;
    
    if (lum < mx*6. && mod(rgt, 8.) == 0.) fColor = col;
    if (lum < mx*5. && mod(lft, 8.) == 0.) fColor = col;
    if (lum < mx*4. && mod(rgt, 4.) == 0.) fColor = col;
    if (lum < mx*3. && mod(lft, 4.) == 0.) fColor = col;
    if (lum < mx*2. && mod(rgt, 2.) == 0.) fColor = col;
    if (lum < mx*1. && mod(lft, 2.) == 0.) fColor = col;
    
    return min(fColor, 1.);
}
*/

void mainImage( out vec4 fragColor, in vec2 fragCoord ){
	

	// Screen coordinates.
	vec2 uv = (fragCoord - iResolution.xy*0.5)/iResolution.y;
	
	// Camera Setup.
	vec3 lookAt = vec3(0.0, 0.0, iTime*6.);  // "Look At" position.
	vec3 camPos = lookAt + vec3(0.0, 0.1, -0.5); // Camera position, doubling as the ray origin.
 
    // Light positioning. One is a little behind the camera, and the other is further down the tunnel.
 	vec3 light_pos = camPos + vec3(0.0, 0.125, 4.125);// Put it a bit in front of the camera.
	vec3 light_pos2 = camPos + vec3(0.0, 0.0, 8.0);// Put it a bit in front of the camera.

	// Using the Z-value to perturb the XY-plane.
	// Sending the camera, "look at," and two light vectors down the tunnel. The "path" function is 
	// synchronized with the distance function. Change to "path2" to traverse the other tunnel.
	lookAt.xy += path(lookAt.z);
	camPos.xy += path(camPos.z);
	light_pos.xy += path(light_pos.z);
	light_pos2.xy += path(light_pos2.z);

    // Using the above to produce the unit ray-direction vector.
    float FOV = PI/3.; // FOV - Field of view.
    vec3 forward = normalize(lookAt-camPos);
    vec3 right = normalize(vec3(forward.z, 0., -forward.x )); 
    vec3 up = cross(forward, right);

    // rd - Ray direction.
    vec3 rd = normalize(forward + FOV*uv.x*right + FOV*uv.y*up);
    
    // Swiveling the camera from left to right when turning corners.
    rd.xy = rot2( path(lookAt.z).x/32. )*rd.xy;
		
    // Standard ray marching routine.
    float t = trace(camPos, rd);
	
    // The final scene color. Initated to black.
	vec3 sceneCol = vec3(0.);
	
	// The ray has effectively hit the surface, so light it up.
	if(t < FAR){
	
    	
    	// Surface position and surface normal.
	    vec3 sp = t * rd+camPos;
	    vec3 sn = calcNormal(sp);
        
        // Texture scale factor.
        const float tSize0 = 1./1.; 
        const float tSize1 = 1./1.;
    	
    	// Texture-based bump mapping.
	    //if (sp.y<-(FH-0.005)) sn = doBumpMap(iChannel1, sp*tSize1, sn, 0.025); // Floor.
	    //else sn = doBumpMap(iChannel0, sp*tSize0, sn, 0.025); // Walls.
        
        sn = doBumpMap(iChannel0, sp*tSize0, sn, 0.02);
        //sn = doBumpMap(sp, sn, 0.01);
	    
	    // Ambient occlusion.
	    float ao = calculateAO(sp, sn);
    	
    	// Light direction vectors.
	    vec3 ld = light_pos-sp;
	    vec3 ld2 = light_pos2-sp;

        // Distance from respective lights to the surface point.
	    float lDdist = max(length(ld), 0.001);
	    float lDdist2 = max(length(ld2), 0.001);
    	
    	// Normalize the light direction vectors.
	    ld /= lDdist;
	    ld2 /= lDdist2;
	    
	    // Light attenuation, based on the distances above. In case it isn't obvious, this
        // is a cheap fudge to save a few extra lines. Normally, the individual light
        // attenuations would be handled separately... No one will notice, or care. :)
	    float atten = 1./(1. + lDdist*.125 + lDdist*lDdist*.05);
        float atten2 =  1./(1. + lDdist2*.125 + lDdist2*lDdist2*.05);
    	
    	// Ambient light.
	    float ambience = 0.75;
    	
    	// Diffuse lighting.
	    float diff = max( dot(sn, ld), 0.0);
	    float diff2 = max( dot(sn, ld2), 0.0);
    	
    	// Specular lighting.
	    float spec = pow(max( dot( reflect(-ld, sn), -rd ), 0.0 ), 32.);
	    float spec2 = pow(max( dot( reflect(-ld2, sn), -rd ), 0.0 ), 32.);
    	
    	// Curvature.
	    float crv = clamp(curve(sp, 0.125)*0.5+0.5, .0, 1.);
	    
	    // Fresnel term. Good for giving a surface a bit of a reflective glow.
        float fre = pow( clamp(dot(sn, rd) + 1., .0, 1.), 1.);
 
        
        vec3 texCol = vec3(0.2, 0.2, 0.2); // tex3D(iChannel0, sp*tSize0, sn);
        texCol = min(texCol*1.5, 1.);
        //texCol = vec3(1)*dot(texCol, vec3(0.299, 0.587, 0.114));
        //texCol = smoothstep(-.0, .6, texCol); // etc.
        
        //texCol = texCol*vec3(1., .5, .2); 
        int id = cellTileID(sp/2.5);
        if(id == 4) texCol = texCol*vec3(1., .5, .3); 
        if(id == 3) texCol = texCol*.5 + texCol*vec3(.5, .25, .15); 
        
    	
    	// Darkening the crevices. Otherwise known as cheap, scientifically-incorrect shadowing.	
	    float shading =  crv*0.75+0.25; 
    	
    	// Combining the above terms to produce the final color. It was based more on acheiving a
        // certain aesthetic than science.
        //
        // Shiny.
        sceneCol = (texCol*(diff + ambience + spec) + spec*vec3(.7, .9, 1))*atten;
        sceneCol += (texCol*(diff2 + ambience + spec2) + spec2*vec3(.7, .9, 1))*atten2;
        //
        // Other combinations:
        //
        // Glow.
        //float gr = dot(texCol, vec3(0.299, 0.587, 0.114));
        //sceneCol = (gr*(diff + ambience*0.25) + spec*texCol*2. + fre*crv*texCol.zyx*2.)*atten;
        //sceneCol += (gr*(diff2 + ambience*0.25) + spec2*texCol*2. + fre*crv*texCol.zyx*2.)*atten2;
        
        // Shading.
        sceneCol *= shading*ao;
        
        // Drawing the lines on the surface.      
        sceneCol *= clamp(abs(curve(sp, 0.035)), .0, 1.)*.5 + 1.;  // Glow lines.
        sceneCol *= 1. - smoothstep(0., 4., abs(curve(sp, 0.0125)))*vec3(.82, .85, .88); // Darker.
	   
	
	}
    
    // Some simple post processing effects.
    //float a = dot(sceneCol, vec3(0.299, 0.587, 0.114));
    //sceneCol = min(vec3(a*3., pow(a, 2.5)*2., pow(a, 6.)), 1.); // Fire palette.
    //sceneCol = floor(sceneCol*15.999)/15.; // Oldschool effect.  
    // Oldschool hatching effect. Uncomment the "ch" function to use this one.
    //sceneCol = ch(clamp(sceneCol, 0., 1.), fragCoord); 
	
	fragColor = vec4(sqrt(clamp(sceneCol, 0., 1.)), 1.0);
    
	
}

