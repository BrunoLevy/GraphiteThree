//stage GL_FRAGMENT_SHADER
//import <GLUP/ShaderToy.h>

/* 
Day 25
*/

//#define DEBUG
#ifdef DEBUG
int debugStep = 0;
float debugValue = 0.0;
#endif

// Rotation
#define R(p,a) p=cos(a)*p+sin(a)*vec2(-p.y,p.x);


#define kINFINITY 10000.0 // An unimaginably large number
#define kSQRT2 1.414213
#define kISQRT2 0.707107
#define kPI 3.141592

// maximum iteration count
#define kMAXITERS 150
#define kEPSILON 0.001
#define kMAXINTERSECTIONS 1

// refractive index
#define kREFRACT 1.0/1.5

// materials
#define kFLOORMATERIAL 0
#define kGLASSMATERIAL 1
#define kMIRRORMATERIAL 2
#define kGLOWMATERIAL 3
#define kMATTEMATERIAL 4

#define kFLOORCOLOUR vec3(0.7, 0.65, 0.6)
#define kGLASSCOLOUR vec3(0.9)
#define kMIRRORCOLOUR vec3(0.9)
#define kMIRRORCOLOUR2 vec3(0.5)
#define kGLOWCOLOUR vec3(0.5)

#define kZENITHCOLOUR vec3(0.3, 0.7, 1.0)
#define kNADIRCOLOUR vec3(0.7, 0.6, 0.4)
#define kHORIZONCOLOUR vec3(0.95)
#define kSUNCOLOUR vec3(100, 90, 70)

// A ray. Has origin + direction.
struct Ray {
    vec3 origin;
    vec3 dir;
};
    
// Distance to nearest surface
struct SDResult {
    float d; // Distance
    int material; // Nearest material
};

// A camera. Has a position and a direction. 
struct Camera {
    vec3 pos;
    Ray ray;
};
    
// A disk. Has position, size, colour.
struct Disk {
    vec3 pos;
    float radius;
    vec3 col;
};
    
struct Sphere {
    vec3 pos;
    float radius;
};
    
struct Box {
	vec3 pos;
	vec3 size;
    float radius;
};
    
float eps = kEPSILON;
float divergence;

vec3 smoothBlend(in vec3 point, in vec3 about, in float radius) {
    point -= about;
    point = mix(-point, point, smoothstep(-radius, radius, point));
    return point + about;
}
    
// Distance to sphere (signed)
float sphereDist(in Ray ray, in Sphere sphere) {
    return length(ray.origin - sphere.pos) - sphere.radius;
}

// Distance to sphere surface
float uSphereDist(in Ray ray, in Sphere sphere) {
    return abs(length(ray.origin - sphere.pos) - sphere.radius);
}

// Distance to box surface (signed)
float boxDist(in Ray ray, in Box box) {
    vec3 dist = abs(ray.origin - box.pos) - (box.size * 0.5);
    vec3 cDist = max(dist, 0.0);
    return min(max(dist.x, max(dist.y, dist.z)), 0.0) + length(cDist) - box.radius;
}

// Distance to box surface
float uBoxDist(in Ray ray, in Box box) {
    return abs(length(max(abs(ray.origin - box.pos) - (box.size * 0.5), 0.0)) - box.radius);
}

// distance to floor
float floorDist(in Ray ray) {
    float dist = ray.origin.y;
    return dist;
}

// traced coords of floor intersection, floor at height y
vec2 floorPos(in Ray ray, in float y) {
    float dist = (ray.origin.y - y) / ray.dir.y;
 	return ray.origin.xz + ray.dir.xz * dist; 
}

/*
---- Random/hash stuff ----
*/

// Normalised random number, borrowed from Hornet's noise distributions: https://www.shadertoy.com/view/4ssXRX
float nrand(in vec2 n) {
	return fract(sin(dot(n.xy, vec2(12.9898, 78.233)))* 43758.5453);
}

// random value based on texture channel 2
vec3 texrand(in vec2 n, in float mipLevel) {
    return texture(iChannel2, n, mipLevel).xyz;
}

// returns a hash value for a fixed size cell
float hashForCell(in vec3 pos, in float cellSize) {
    float hash = nrand(floor(pos.xz / cellSize) + 68.0);
    return hash;
}

// returns a random colour based on the cell hash
vec3 randomColourForCell(in vec3 pos, in float cellSize) {
	float hash = hashForCell(pos, cellSize); 
    return vec3(
        nrand(vec2(hash * 2.0, hash * 4.0)),
        nrand(vec2(hash * 4.0, hash * 8.0)),
        nrand(vec2(hash * 8.0, hash * 16.0))
	);
	vec3 c = vec3(hash, mod(hash + 0.15, 1.0), mod(hash + 0.3, 1.0)) * 0.75;
}

/*
---- INTERSECTION OPS ----
*/

// Union of two signed distances
float unionOp(float d0, float d1) {
    return min(d0, d1);
}

// Intersection of two signed distances
float intersectOp(float d0, float d1) {
    return max(d0, d1);
}

// Difference of two signed distances
float differenceOp(float d0, float d1) {
    return max(d0, -d1);
}
	
// from http://www.iquilezles.org/www/articles/smin/smin.htm
// polynomial smooth min (k = 0.1); 
float smin( float a, float b, float k )
{
    float h = clamp( 0.5+0.5*(b-a)/k, 0.0, 1.0 );
    return mix( b, a, h ) - k*h*(1.0-h);
}


/*
---- Scene rendering ----
*/

// Get the distance to the scene (returns a struct containing distance and nearest material)
SDResult sceneDist(in Ray ray) {
    SDResult result;
    float t = iTime + 10.0;
    R(ray.origin.yz, cos(ray.origin.z - t) / t);
    //ray.origin.y += sin(ray.origin.x * 0.5 + t);
    float floorDist = ray.origin.y-0.3;
    
    vec3 p = ray.origin;
    p.y *= 0.75;
    
	p += vec3(
		sin(p.z * 1.55) + sin(p.z * 1.34),
		0.,
		sin(p.x * 1.34) + sin(p.x * 1.55)
	) * .5;

	vec3 mp = p;
    mp.xz = mod(p.xz, 1.);
    
        
	ray.origin = vec3(mp.x, mp.y + (sin(p.z * kPI) * sin(p.x * kPI)) * 0.25, 0.5);
	float s1 = boxDist(
        ray, 
        Box(
            vec3(0.5), 
            vec3(0.2), 
            0.02
        ));
    
    ray.origin = vec3(0.5, mp.y + (sin(p.x * kPI) * -sin(p.z * kPI)) * 0.25, mp.z);
	float s2 = boxDist(
        ray, 
        Box(
            vec3(0.5),
            vec3(0.0),
            0.15
            ));
	
    result.d = unionOp(s1, s2);
    
    //result.d = unionOp(result.d, floorDist);
    result.d = smin(result.d, floorDist, 0.5);
    result.material = kMATTEMATERIAL;
    
    return result;
}

// Gets the normal
vec3 normal(in Ray ray) {
    vec2 eps = vec2(0.0001, 0);
    float baseDist = sceneDist(ray).d;
 	return normalize(vec3(
        sceneDist(Ray(ray.origin + eps.xyy, ray.dir)).d - 
        sceneDist(Ray(ray.origin - eps.xyy, ray.dir)).d,
        sceneDist(Ray(ray.origin + eps.yxy, ray.dir)).d -
        sceneDist(Ray(ray.origin - eps.yxy, ray.dir)).d,
        sceneDist(Ray(ray.origin + eps.yyx, ray.dir)).d -
        sceneDist(Ray(ray.origin - eps.yyx, ray.dir)).d
        ));
}

// Moves the ray to the surface. Helps avoid artefacts due to ray intersection imprecision.
void clampToSurface(in Ray ray, in float d, inout vec3 n) {
    for (int i=0; i<5; i++) {
 		ray.origin += n * d*0.5;
 		d = sceneDist(ray).d;
 		n = normal(ray);
    }
}

// Calulcate a fresnel term for reflections
float fresnelTerm(in Ray ray, in vec3 n, in float power) {
	float fresnel = min(1., dot(ray.dir, n) + 1.0);
	fresnel = pow(fresnel, power);
    return fresnel;
}

/*
---- LIGHTING ----
*/

// Pretty background colour of some sort...
vec3 backgroundColour (in Ray ray, in float divergence) {
    return vec3(1);
    // Declare a horizon and extremity (either zenith or nadir)
    vec3 base, extremity, texture;
    float x = smoothstep(0.0,1.0,abs(ray.dir.y));
   // base = mix(vec3(0.05, 0.1, 0.2), vec3(0.0), x);
    
    if (ray.dir.y >= 0.0) {
        // sky: fake as hell clouds
    	base = mix(kHORIZONCOLOUR, kZENITHCOLOUR, x);
        texture = vec3(1);
        float cloudFactor = 0.0;
        
        vec2 coord = ray.dir.xz / 8.0;
        float scale = 1.0;
        for (int i=0; i<4; i++) {
            float offset = iTime * 0.03 / scale;
            cloudFactor += texrand(coord + vec2(0, -offset), 3.0).r * scale;
            coord *= 2.0;
            scale *= 0.5;
        }
        cloudFactor /= 1.5;
        vec3 cloud = mix(vec3(0.4), vec3(1), cloudFactor);
        base = mix(base, cloud, cloudFactor * ray.dir.y * ray.dir.y);
        
    } else {
        // ground: fake as hell desert
        texture = kNADIRCOLOUR * 0.5;
        vec2 floorCoord = floorPos(ray, -30.0) + vec2(0, iTime * 100.0);
        texture *= (sin(floorCoord.x * .183 + floorCoord.y * 0.183 + sin(floorCoord.y * 0.3) * 0.5) 
                    + sin(floorCoord.x * .15 + floorCoord.y * 0.22 + sin(floorCoord.x * 0.26) * 0.5))
            * 0.25 + 0.5;
		base = mix(kHORIZONCOLOUR, kNADIRCOLOUR + texture, x);
        
    }
    
    
    
    return base;
}

// Technically broken, but seems to make soft shadows? *shrug*
float occlusion(in Ray ray, in vec3 n) {
    vec3 origin = ray.origin;
    float o = 0.0;
    ray.dir = n;
    float x = eps*2.0;
    origin += ray.dir * x;
    for (float i=1.0; i<7.0; i++) {
    	ray.origin = origin + ray.dir * x;
        float d = sceneDist(ray).d;
        o += max(x - d, 0.0) / x / i;
        
        x *= 4.0;
    }
 	return 1.0 - o;
}

vec3 highlight(in Ray ray, in vec3 n) {
    // sun
	vec3 sunDir = normalize(vec3(1,0.3,1));
	float sunDist = distance(sunDir, ray.dir)-0.00;
	return mix(vec3(10,10,8), vec3(0), smoothstep(0.0, 0.2, sunDist));
}

/*
---- RAY MARCHING ----
*/

// The main marching loop
void marchRay(inout Ray ray, inout vec3 colour) {
    bool inside = false; // are we inside or outside the glass object
    vec3 impact = vec3(1); // This decreases each time the ray passes through glass, darkening colours

    vec3 startpoint = ray.origin;
    
#ifdef DEBUG   
vec3 debugColour = vec3(1, 0, 0);
#endif
    
    SDResult result;
    vec3 n;
    vec3 glassStartPos;
    
    //float glow = 0.0;
    
    for (int i=0; i<kMAXITERS; i++) {
        // Get distance to nearest surface
        result = sceneDist(ray);
        
        //glow += result.material == kGLOWMATERIAL ? 
        //    pow(max(0.0, (80.0 - result.d) * 0.0125), 4.0) * result.d * 0.01
        //    : 0.0;
        
        // Step half that distance along ray (helps reduce artefacts)
        float stepDistance = (inside ? abs(result.d) : result.d) * 0.5;
        ray.origin += ray.dir * stepDistance;
        //if (length(ray.origin) > 40.0) { break; }
        
        if (stepDistance < eps) {
            // colision
            // normal
            // Get the normal, then clamp the intersection to the surface
    		n = normal(ray);
            //clampToSurface(ray, stepDistance, n);
#ifdef DEBUG
//debugColour = n;
//break;
#endif
            
            if ( result.material == kFLOORMATERIAL ) {
                // ray hit floor
                
                // Add some noise to the normal, since this is pretending to be grit...
                vec3 randomNoise = texrand(ray.origin.xz * 0.4, 0.0);
                randomNoise.xz = randomNoise.xz * 2. - 1.;
                n = mix(n, normalize(vec3(randomNoise.x, 1, randomNoise.y)), randomNoise.z * 0.3);
                
                // Colour is just grey with crappy fake lighting...
                float o = occlusion(ray, n);
                colour += vec3(1) * o * impact;
                impact *= 0.;
                break;
            }
            
            if ( result.material == kMATTEMATERIAL ) {
                // ray hit floor
                
                // Add some noise to the normal, since this is pretending to be grit...
                vec3 randomNoise = texrand(n.xz * 0.5 + 0.5, 0.0);
                randomNoise.xz = randomNoise.xz * 2. - 1.;
               // n = mix(n, normalize(vec3(randomNoise.x, 1, randomNoise.y)), randomNoise.z * 0.1);
                
                // Colour is just grey with crappy fake lighting...
                float o = occlusion(ray, n);
                colour += vec3(1) * o * impact;
                impact *= 0.;
                break;
            }
            
            if (result.material == kGLOWMATERIAL) {
             	colour = mix(colour, kGLOWCOLOUR, impact);
                impact *= 0.;
                break;
            }
            
            // check what material it is...
            
            if (result.material == kMIRRORMATERIAL) {
                
                // handle interior glass / other intersecion
                if (inside) {
                     float glassTravelDist =  min(distance(glassStartPos, ray.origin) / 16.0, 1.);
    				glassStartPos = ray.origin;
                    // mix in the colour
                	impact *= mix(kGLASSCOLOUR, kGLASSCOLOUR * 0.1, glassTravelDist);
                    
                }
                
                // it's a mirror, reflect the ray
                ray.dir = reflect(ray.dir, n);
                    
                // Step 2x epsilon into object along normal to ensure we're beyond the surface
                // (prevents multiple intersections with same surface)
                ray.origin += n * eps * 4.0;
                
                // Mix in the mirror colour
                colour += highlight(ray, n);
                impact *= kMIRRORCOLOUR;
                float o = occlusion(ray, n);
                impact *= o;
#ifdef DEBUG
debugColour = vec3(o);
break;
#endif
                
            } else {
                // glass material
            
                if (inside) {
                	// refract glass -> air
                	ray.dir = refract(ray.dir, -n, 1.0/kREFRACT);
                    
                    // Find out how much to tint (how far through the glass did we go?)
                    float glassTravelDist =  min(distance(glassStartPos, ray.origin) / 16.0, 1.);
    
                    // mix in the colour
                	impact *= mix(kGLASSCOLOUR, kGLASSCOLOUR * 0.1, glassTravelDist);
                    
#ifdef DEBUG
debugValue += glassTravelDist / 2.0;
#endif
      
                
              	} else {
               		// refract air -> glass
                	glassStartPos = ray.origin;
                    
              	  	// Mix the reflection in, according to the fresnel term
                	float fresnel = fresnelTerm(ray, n, 1.0);
                    fresnel = fresnel;
    				/*
                    colour = mix(
                    	colour, 
                    	texture(iChannel1, reflect(ray.dir, n)), 
                    	vec4(fresnel) * impact);
*/
                    colour = mix(
                        colour,
                        backgroundColour(ray, 0.0),
                        vec3(fresnel) * impact);
                	colour += n.x * 0.1;//highlight(ray, n);
                    impact *= 1.0 - fresnel;
    			
                	// refract the ray
            		ray.dir = refract(ray.dir, n, kREFRACT);
                    
#ifdef DEBUG
//debugValue += 0.5;
#endif
                }
            
            	// Step 2x epsilon into object along normal to ensure we're beyond the surface
                ray.origin += (inside ? n : -n) * eps * 2.0;
                
                // Flip in/out status
                inside = !inside;
            }
        }
        
        // increase epsilon
        eps += divergence * stepDistance;
    }
    
    // So far we've traced the ray and accumulated reflections, now we need to add the background.
   // colour += texture(iChannel0, ray.dir) * impact;
    ray.origin = startpoint;
    colour.rgb += backgroundColour(ray, 0.0) * impact; // + glow * kGLOWCOLOUR;
    
#ifdef DEBUG
//debugColour.rgb = ray.dir;
//debugColour = vec3(float(debugStep)/2.0);
colour = debugColour;
#endif
}

// Sets up a camera at a position, pointing at a target.
// uv = fragment position (-1..1) and fov is >0 (<1 is telephoto, 1 is standard, 2 is fisheye-like)
Camera setupCam(in vec3 pos, in vec3 target, in float fov, in vec2 uv) {
		// cam setup
    // Create camera at pos
	Camera cam;
    cam.pos = pos;
    
    // A ray too
    Ray ray;
    ray.origin = pos;
    
    // FOV is a simple affair...
    uv *= fov;
    
    // Now we determine hte ray direction
	vec3 cw = normalize (target - pos );
	vec3 cp = vec3 (0.0, 1.0, 0.0);
	vec3 cu = normalize ( cross(cw,cp) );
	vec3 cv = normalize ( cross (cu,cw) );
    
	ray.dir = normalize ( uv.x*cu + uv.y*cv + 0.5 *cw);
    
    // Add the ray to the camera and our work here is done.
	cam.ray = ray;
    
    // Ray divergence
    divergence = fov / iResolution.x;
    
	return cam;
}

vec3 camPath(in float time) {
    
    vec2 mouse = iMouse.xy / iResolution.xy;
    mouse = mouse * kPI * 2.0 + kPI;
    
    vec3 camPos = vec3(sin(time * 0.3) * 2.0, sin(time * 0.13) * 2.0 + 4.5, time);
	
    //R(camPos.xz, time + mouse.x);
    //R(camPos.zy, sin(time) * kPI * 0.1 + 0.2);
    
    return camPos;
}

void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
    // We'll need a camera. And some perspective.
    
	// Get some coords for the camera angle from the frag coords. Convert to -1..1 range.
    vec2 uv = fragCoord.xy / iResolution.xy;
    uv = uv * 2. - 1.;
    
    // Aspect correction so we don't get oval bokeh
    uv.y *= iResolution.y/iResolution.x;
    
    // Make a camera with ALL NEW AND IMPROVED! camera code :)
    float camTime = iTime;
    vec3 camPos = camPath(camTime);
    vec3 camTarget = camPath(camTime + 1.0);
    camTarget.y = 0.0;
    //camTarget.y -= 3.0;
    Camera cam = setupCam(camPos, camTarget, 0.75, uv);
    
    // Let's raymarch some stuff and inject that into the scene...
    
    // Create an empty colour
    vec3 col = vec3(0);
    
    // Trace that ray!
    marchRay(cam.ray, col);
    
	fragColor = vec4(col,1.0);
}
