//stage GL_FRAGMENT_SHADER
//import <GLUP/ShaderToy.h>

/*

	Triangle-Voronoi Graph Weave
	----------------------------

	Expanding on Tomkh's previous work by constructing a dual Voronoi graph from the Delaunay 
	triangulation of random points. It's mainly a proof of concept, but in order to make it
	slightly more interesting, I went to the extra trouble to weave the graph components.

	This turned out to be a trickier exercise than I expected. Obtaining the Voronoi information
	required a little extra work -- due to the necessity to determine neighboring triangle 
	information, but overall, it wasn't too bad. However, creating the weave required unique 
	ID's for neighboring edges, which required a whole bunch of point IDs, etc.

	I wouldn't call this a lot of code, but it's probably a little more than someone with better
	things to do would like to decipher. I thought it would be novel to weave the dual graph, 
	but that complicated things and added a lot of extra code. The extra window dressing also 
	added to the line count. With that in mind, I'll put together a much more concise and easier 
	to consume version pretty soon. In the meantime, it's still possible to use this in 
	conjunction with Tomkh's or my previous example to get the general idea.

	The logic for this seems to be sound, but was rushed, so it wouldn't surprise me if there are
	better ways to go about it. Having said that, the routine consists of just four checks, plus 
	a bit of decision making and variable setting, so it should suffice.

	By the way, I'm pretty sure that the Delaunay triangulation will break with too much of a 
	point spread, so a pretty tight restriction has been set, which in turn, has resulted in boxy
	looking cell sites. At some stage, I'll try to improve on that... or just wait for someone 
	more clever on Shadertoy to do it -- You'd be amazed at how often that strategy works. :D
	

	Based on:

	// I'd been wanting to see a geometric Delaunay triangulation example on Shadertoy for ages,
	// so Tomkh (Tomasz Dobrowolski) was kind enough to whip one up in virtually no time. In
	// addition to helping me out, I really like the way this is presented.
	Random Delaunay Triangulation - Tomkh
	https://www.shadertoy.com/view/4sKyRD

	Another example:
    
	// Really nice screensaver-like example. To my knowledge, Mattz was the first to put up a 
	// quasi-randomized 2D triangle mesh. However, his particular example uses the same diagonal
	// orientation on each quadrilateral.
	ice and fire - mattz
	https://www.shadertoy.com/view/MdfBzl


*/

// Color palette. The default red and gold trim (0), a four-colored pastel palette (1), greyscale with
// color (2), or just greyscale (3).
#define PALETTE 0 
//#define GREY_LINES // Grey triangle lines.

// Fixed unanimated triangles, if you don't like the triangle popping effect. :)
#define FIXED


// A visual aid to show the physical square grid.
//#define SHOW_GRID_CELLS


// Greyscale.
vec3 grey(vec3 col){ return vec3(1)*dot(col, vec3(.299, .587, .114)); }


// 2x2 matrix rotation. Note the absence of "cos." It's there, but in disguise, and comes courtesy
// of Fabrice Neyret's "ouside the box" thinking. :)
mat2 rot2( float a ){ vec2 v = sin(vec2(1.570796, 0) + a);	return mat2(v, -v.y, v.x); }


// vec2 to vec2 hash.
vec2 hash22(vec2 p) { 

    // Faster, but doesn't disperse things quite as nicely. However, when framerate
    // is an issue, and it often is, this is a good one to use. Basically, it's a tweaked 
    // amalgamation I put together, based on a couple of other random algorithms I've 
    // seen around... so use it with caution, because I make a tonne of mistakes. :)
    float n = sin(dot(p, vec2(12, 113)));
    #ifdef FIXED
    return (fract(vec2(262144, 32768)*n) - .5)*2.*.24;
    #else
    // Animated.
    p = fract(vec2(262144, 32768)*n); 
    // Note the ".35," insted of ".5" that you'd expect to see. .
    return sin(p*6.2831853 + iTime/2.)*.24;
    #endif
}


// vec2 to vec2 hash.
float hash21(vec2 p) { 

    // Faster, but doesn't disperse things quite as nicely. However, when framerate
    // is an issue, and it often is, this is a good one to use. Basically, it's a tweaked 
    // amalgamation I put together, based on a couple of other random algorithms I've 
    // seen around... so use it with caution, because I make a tonne of mistakes. :)
    return fract(sin(dot(p, vec2(113.927, 1.763)))*43758.5453);
} 


// The triangle line pattern.
float linePattern(vec2 p, vec2 a, vec2 b){
  
    // Determine the angle between the verticle 12 o'clock vector and the edge
    // we wish to decorate (put lines on), then rotate "p" by that angle prior
    // to decorating. Simple.
    vec2 v1 = vec2(0, 1);
    vec2 v2 = (b - a);

    if(a.x>b.x) v2.y = -v2.y;

    // Angle between vectors.
    //float ang = acos(dot(v1, v2)/(length(v1)*length(v2))); // In general.
    float ang = acos(v2.y/length(v2)); // Trimed down.
    p = rot2(ang - .2)*p; // Putting the angle slightly past 90 degrees is optional.

    float ln = clamp(cos(p.y*64.*2.)*1. - .5, 0., 1.);

    return ln*.25 + clamp(sin(p.y*64.)*3. + 2.95, 0., 1.)*.75 + .15; // Ridges.
 
}


// Signed distance to the segment joining "a" and "b." We need this one to determine
// which side of the line a point is on.
//
// From Tomkh's original example. I trimmed it a bit, but for all I know, I might have
// made is slower. :)
float sDistLine(vec2 a, vec2 b) {
       
    b -= a; return dot(a, vec2(-b.y, b.x)/length(b)); //return dot(a, normalize(vec2(-b.y, b.x)));
    
}

// Unsigned distance to the segment joining "a" and "b."
float distLine(vec2 a, vec2 b){
    
	vec2 pa = a;
	vec2 ba = a - b;
	float h = clamp(dot(pa, ba) / dot(ba, ba), 0., 1.);
	//return smoothstep(-thickness*.5, thickness, length(pa - ba * h));
    return length(a - ba*h);
}

// From the the following example:
// Random Delaunay Triangulation - Tomkh
// https://www.shadertoy.com/view/4sKyRD
//
// Use "parabolic lifting" method to calculate if two triangles are about to flip.
// This is actually more reliable than circumscribed circle method.
// The technique is based on duality between Delaunay Triangulation
// and Convex Hull, where DT is just a boundary of convex hull
// of projected seeds onto paraboloid.
// We project (h1 h2 h3) triangle onto paraboloid
// and return the distance of the origin
// to a plane crossing projected triangle.
float flipDistance(vec2 h1, vec2 h2, vec2 h3)
{
   // Projects triangle on paraboloid.
   vec3 g1 = vec3(h1, dot(h1, h1));
   vec3 g2 = vec3(h2, dot(h2, h2));
   vec3 g3 = vec3(h3, dot(h3, h3));
   // Return signed distance of (g1, g2, g3) plane to the origin.
   //#if FLIP_ANIMATION
    // return dot(g1, normalize(cross(g3-g1, g2-g1)));
   //#else
     // If we don't do animation, we are only interested in a sign,
     // so normalization is unnecessary.
   	 return dot(g1, cross(g3-g1, g2-g1));
   //#endif
}


// IQ's triangle hit routine.
bool insideTri(vec2 p, vec2 a, vec2 b, vec2 c){
    
 	// Compute vectors        
    vec2 v0 = c - a;
    vec2 v1 = b - a;
    vec2 v2 = p - a;

    // Compute dot products
    float dot00 = dot(v0, v0);
    float dot01 = dot(v0, v1);
    float dot02 = dot(v0, v2);
    float dot11 = dot(v1, v1);
    float dot12 = dot(v1, v2);

    // Compute barycentric coordinates
    float invDenom = 1./(dot00*dot11 - dot01*dot01);
    float u = (dot11*dot02 - dot01*dot12)*invDenom;
    float v = (dot00*dot12 - dot01*dot02)*invDenom;

    // Check if point is in triangle
    return (u>0. && v>0. && (u + v)<1.)? true : false;  
    
}


float cross2d( in vec2 a, in vec2 b ) { return a.x*b.y - a.y*b.x; }

// IQ's point in a quadrilateral routine -- IQ's original is more sophisticated, but
// I only needed to return a hit, so I hacked at it a bit. There are probably faster 
// routines, especially since the UV coordinates aren't required. However, I might use them
// later, so I'll leave it as is for now. By the way, if someone has a fast "point inside a
// quad" algorithm, I'd like to hear about it.
//
// Given a point p and a quad defined by four points {a,b,c,d}, return the bilinear
// coordinates of p in the quad. Returns (-1,-1) if the point is outside of the quad.
bool insideQuad(in vec2 a, in vec2 b, in vec2 c, in vec2 d){

    vec2 res = vec2(-1.0);

    vec2 e = b-a;
    vec2 f = d-a;
    vec2 g = a-b+c-d;
    vec2 h = -a;
        
    float k2 = cross2d( g, f );
    float k1 = cross2d( e, f ) + cross2d( h, g );
    float k0 = cross2d( h, e );

    // otherwise, it's a quadratic
    float w = k1*k1 - 4.0*k0*k2;
    if( w<0.0 ) return false; //vec2(-1.0);
    w = sqrt( w );


    float ik2 = 0.5/k2;
    float v = (-k1 - w)*ik2; if( v<0.0 || v>1.0 ) v = (-k1 + w)*ik2;
    float u = (h.x - f.x*v)/(e.x + g.x*v);
    if( u<0.0 || u>1.0 || v<0.0 || v>1.0 ) return false;//vec2(-1.0);
    //res = vec2( u, v );
    
    return true;
}

    
// The center of a triangle's circumcircle. Also, the intersection of the perpendicular bisectors.
vec2 circCent(vec2 a, vec2 b, vec2 c){
    
    float d = (a.x*(b.y - c.y) + b.x*(c.y - a.y) + c.x*(a.y - b.y))*2.;

    vec2 a2 = a*a, b2 = b*b, c2 = c*c, u;
    
    u.x =  (a2.x + a2.y)*(b.y - c.y) + (b2.x + b2.y)*(c.y - a.y) + (c2.x + c2.y)*(a.y - b.y);
    u.y =  (a2.x + a2.y)*(c.x - b.x) + (b2.x + b2.y)*(a.x - c.x) + (c2.x + c2.y)*(b.x - a.x);

    return u/d;

}

/*
// Circum radius -- A circle that encloses the triangle and passes through all vertices. 
float circumR(vec2 p0, vec2 p1, vec2 p2){

    // Side lengths.
    float a = length(p0 - p1), b = length(p1 - p2),  c = length(p2 - p0);
    
    float s = (a + b + c)/2.; // Semiperimeter.
    float area = sqrt(s*(s - a)*(s - b)*(s - c)); // Area.
    
    return a*b*c/(4.*area);// Circumradius.
}

*/ 

// The triangle object. A lot of these variables were hacked in as they were needed, so it lacks
// naming consistency, etc. I'll tidy this up later though.
struct triObj{
    
    vec2 p0, p1, p2; // The triangle vertices.
    
    vec2 id0, id1, id2; // The triangle vertice IDs.
    
    // l - Diagonal opposite, m - top or bottom neighbor, n - left or right neighbor.
    vec2 l, l1, l2, m, m1, m2, n, n1, n2; // Three neighboring triangle vertices.
    
    vec2 lID[3], mID[3], nID[3]; // Three neighboring triangle vertices IDs.
    
    // Unique triangel ID. and one of four triangle cell IDs that identify the 
    // particular triangle's quadrilateral arrangement.
    vec2 id, cID; 
    
};
    
 
    

// The triangle mesh routine: Iterate through the cell and it's neighbors until we hit a quadrilateral, 
// then determine which triangle information to return. By that I mean, triangle vertices and IDs, plus
// the neighboring triangle vertices and IDs. It was exhausting to write out, but logically, not all that
// complicated.
//
// I wrote this from scratch, but basically adapted the logic from Tomkh's Delaunay triangle mesh example.
// It was surprisingly easy to write, but if it were not for his example, I wouldn't have known where to begin. :)
triObj triangulate(in vec2 p){
    
    // I'm declaring the vertices outside the loop, because it looks neater, but I hear it's faster to declare them
    // as locally as possible.
    vec2 o, o1, o2, o3, o4, o5;
    vec2 id0, id1, id2, id3, id4, id5;
	
    // Cell identifier and fractional position.
    vec2 g = floor(p); p -= g + .5;
 
    // Main triangle vertices, and its neighboring vertices. These particular variables are stored in the
    // triangle object. The only reason they're declared here is to save writing "tri.."
    vec2 h = vec2(1e8), h1 = vec2(1e8), h2 = vec2(1e8);
    vec2 l = vec2(1e8), l1 = vec2(1e8), l2 = vec2(1e8);
    vec2 m = vec2(1e8), m1 = vec2(1e8), m2 = vec2(1e8);
    vec2 n = vec2(1e8), n1 = vec2(1e8), n2 = vec2(1e8);
    
    
    
    triObj tri; // The triangle object.
    tri.cID = vec2(-1); // Not necessary, since we're guaranteed a hit, but it's a raytracing habit.
    tri.id0 = tri.id1 = tri.id2 = vec2(1e8); // Main triangle vertice IDs.
    
/*    
    // Precalculating the hash values so as not to recalculate too many in the main loop. Basically, I'm setting 
    // up an extra loop, an array, plus indexing, etc, in order to cut down from a possible 36 hash calculations 
    // to 16. Not to mention, making thing less readable... Therefore, it might be a case of diminishing returns. 
    // I'd like to hear what the experts have to say about this, because I'm on the fence as to whether I should
    // be complicating things and wasting resources with this step. :)
    //
    vec2 aO[16];
    for(int j=0; j<=3; j++){
		for(int i=0; i<=3; i++){
            
            aO[j*4 + i] = vec2(i - 1, j - 1) + hash22(g + vec2(i - 1, j - 1)) - p;            
        }
    }

*/
    
    
    // Iterate through the cell and its neighbors until we hit a quadrilateral, then determine which
    // triangle to return. I've allowed the grid vertices to randomly move further away from their original
    // positions, which requires 9 cell checks, instead of just 4.
    //
    // By the way, once a triangle has been found, we break from the loop to avoid further redundant 
    // calculations. This means fewer than 9 checks are performed on average -- A rough guess would be an
    // average of 5 checks per pass which I'd expect most GPUs can handle in their sleep.
    //
	for(int j=0; j<=1; j++){
		for(int i=0; i<=1; i++){
            
 			// The four quadrilateral vertices for this particular cell. Clockwise arrangement.
            // o -- o1
            // |    |
            // o3-- o2
            id0 = vec2(i - 1, j);
            id1 = vec2(i, j);
            id2 = vec2(i, j - 1);
            id3 = vec2(i - 1, j - 1);
            o = id0 + hash22(g + id0) - p; // Origin -- Top left.
            o1 = id1 + hash22(g + id1) - p; // Top right.
            o2 = id2 + hash22(g + id2) - p; // Bottom right.
            o3 = id3 + hash22(g + id3) - p; // Bottom left.
            //o = aO[(j+1)*4 + i]; // Origin -- Top left.
            //o1 = aO[(j+1)*4 + i + 1]; // Top right.
            //o2 = aO[j*4 + i + 1]; // Bottom right.
            //o3 = aO[j*4 + i]; // Bottom left.

            // Quad and four neighboring arrangements.
            //
            //     4  5

			//	4  0  1  4

			//  5  3  2  5

			//	   4  5
            
    
            // If the point resides in this particular cell's quad, determine which triangle it resides in, then 
            // determine it's neighboring triangles. Return all relevant vertex points, ID, etc.
            if(insideQuad(o, o1, o2, o3)){
                
                // Applying the Delaunay rule to the quad: Basically, split the quad along an arbitrary diagonal to form
                // a triangle. Circumscribe a circle around them, then determine whether the excluded fourth point lies 
                // within the circle. If it does, then flip the diagonal. There's a bit of math and theory behind it, but 
                // thankfully, Tomkh took care of that bit. :)
                //
                // By the way, there's no rule that says you need to do it this way -- You could restric the vertice
                // movement more, then simply flip the diagonal on a random basis. However, the following tends to look 
                // better. Plus, if you wish to put together a Delaunay triangulation for various reasons -- like 
                // constructing the dual Voronoi representation -- this step is necessary.
                float f = flipDistance(o - o2, o1 - o2, o3 - o2)<0.? 1. : -1.;
                 
                
				if(f>0.){ // Diagonal runs from the top right vertex to the bottom left vertex.
                    
                     // Determining which side of the diagonal quadrilateral line the point is on. In other words,
                     // determine which of the two triangles that make up the quad the point is in.
                     if(sDistLine(o1, o3)>0.){
                         // o3, o, o1 triangle.
                         tri.cID = vec2(0); // Red
                         h = o3, h1 = o, h2 = o1;
                         l = o1, l1 = o2, l2 = o3;
                         
                         // Top triangle.
                         id4 = vec2(i - 1, j + 1);
                         id5 = vec2(i, j + 1);
                         o4 = id4 + hash22(g + id4) - p; 
            			 o5 = id5 + hash22(g + id5) - p; 
                         f = flipDistance(o4 - o1, o5 - o1, o - o1)<0.? 1. : -1.;
                         if(f>0.){ m = o, m1 = o5, m2 = o1; tri.mID[0] = id0; tri.mID[1] = id5; tri.mID[2] = id1; }                         
                         else { m = o, m1 = o4, m2 = o1; tri.mID[0] = id0; tri.mID[1] = id4; tri.mID[2] = id1; }
                         
                         // Left triangle.
                         id4 = vec2(i - 2, j);
                         id5 = vec2(i - 2, j);
                         o4 = id4 + hash22(g + id4) - p; 
            			 o5 = id5 + hash22(g + id5) - p;
                         f = flipDistance(o4 - o3, o - o3, o5 - o3)<0.? 1. : -1.;
                         if(f>0.){ n = o3, n1 = o5, n2 = o; tri.nID[0] = id3; tri.nID[1] = id5; tri.nID[2] = id0; }
                         else { n = o3, n1 = o4, n2 = o; tri.nID[0] = id3; tri.nID[1] = id4; tri.nID[2] = id0; }
                         
                         tri.id0 = id3; tri.id1 = id0; tri.id2 = id1; 
                         tri.lID[0] = id1; tri.lID[1] = id2; tri.lID[2] = id3;
                         
                         
                    }
                    else {
                         // o1, o2, o3 triangle.
                         tri.cID = vec2(1); // Blue.
                        
                         h = o1, h1 = o2, h2 = o3;
                         l = o3, l1 = o, l2 = o1;                        
                        
                         // Bottom triangle.
                         id4 = vec2(i - 1, j - 2);
                         id5 = vec2(i, j - 2);
                         o4 = id4 + hash22(g + id4) - p; 
            			 o5 = id5 + hash22(g + id5) - p;
                         f = flipDistance(o3 - o5, o2 - o5, o4 - o5)<0.? 1. : -1.;
                         if(f<0.){ m = o2, m1 = o5, m2 = o3; tri.mID[0] = id2; tri.mID[1] = id5; tri.mID[2] = id3; }
                         else { m = o2, m1 = o4, m2 = o3; tri.mID[0] = id2; tri.mID[1] = id4; tri.mID[2] = id3; }
                        
                         // Right triangle.
                         id4 = vec2(i + 1, j);
                         id5 = vec2(i + 1, j - 1);
                         o4 = id4 + hash22(g + id4) - p; 
            			 o5 = id5 + hash22(g + id5) - p;
                         f = flipDistance(o1 - o5, o4 - o5, o2 - o5)<0.? 1. : -1.;
                         if(f>0.){ n = o1, n1 = o4, n2 = o2; tri.nID[0] = id1; tri.nID[1] = id4; tri.nID[2] = id2; }
                         else { n = o1, n1 = o5, n2 = o2; tri.nID[0] = id1; tri.nID[1] = id5; tri.nID[2] = id2; } 
                        
                         tri.id0 = id1; tri.id1 = id2; tri.id2 = id3; 
                         tri.lID[0] = id3; tri.lID[1] = id0; tri.lID[2] = id1;
                         
                    }
                    
                }
                else { // Diagonal runs from the top left vertex to the bottom right vertex.
                   
                    // If we have the flipped diagonal arrangement, determine which triangle the point is in.
                    if(sDistLine(o, o2)>0.){
                         //o1 = o2; o2 = o3; // o2, o3, o triangle.
                         tri.cID = vec2(2); // Orange.
                         h = o2, h1 = o3, h2 = o;
                         l = o, l1 = o1, l2 = o2;
                        
                         // Bottom triangle.
                         id4 = vec2(i - 1, j - 2);
                         id5 = vec2(i, j - 2);
                         o4 = id4 + hash22(g + id4) - p; 
            			 o5 = id5 + hash22(g + id5) - p;
                         f = flipDistance(o3 - o5, o2 - o5, o4 - o5)<0.? 1. : -1.;
                         if(f<0.){ m = o2, m1 = o5, m2 = o3; tri.mID[0] = id2; tri.mID[1] = id5; tri.mID[2] = id3; }
                         else { m = o2, m1 = o4, m2 = o3; tri.mID[0] = id2; tri.mID[1] = id4; tri.mID[2] = id3; }
                         
                         // Left triangle.
                         id4 = vec2(i - 2, j);
                         id5 = vec2(i - 2, j - 1);
                         o4 = id4 + hash22(g + id4) - p; 
            			 o5 = id5 + hash22(g + id5) - p;
                         f = flipDistance(o4 - o3, o - o3, o5 - o3)<0.? 1. : -1.;
                         if(f>0.){ n = o3, n1 = o5, n2 = o; tri.nID[0] = id3; tri.nID[1] = id5; tri.nID[2] = id0; }
                         else { n = o3, n1 = o4, n2 = o; tri.nID[0] = id3; tri.nID[1] = id4; tri.nID[2] = id0; }
                         
                         tri.id0 = id2; tri.id1 = id3; tri.id2 = id0;
                         tri.lID[0] = id0; tri.lID[1] = id1; tri.lID[2] = id2;
                        
                         
                    }
                    else {
                        
                        // o, o1, o2 triangle.
                         tri.cID = vec2(3); // Green.
                         h = o, h1 = o1, h2 = o2;
                         l = o2, l1 = o3, l2 = o;
                        
                         // Top triangle.
                         id4 = vec2(i - 1, j + 1);
                         id5 = vec2(i, j + 1);
                         o4 = id4 + hash22(g + id4) - p; 
            			 o5 = id5 + hash22(g + id5) - p;
                         f = flipDistance(o4 - o1, o5 - o1, o - o1)<0.? 1. : -1.;
                         if(f>0.){ m = o, m1 = o5, m2 = o1; tri.mID[0] = id0; tri.mID[1] = id5; tri.mID[2] = id1; }
                         else { m = o, m1 = o4, m2 = o1; tri.mID[0] = id0; tri.mID[1] = id4; tri.mID[2] = id1; }
                         
                         // Right triangle.
                         id4 = vec2(i + 1, j);
                         id5 = vec2(i + 1, j - 1);
                         o4 = id4 + hash22(g + id4) - p; 
            			 o5 = id5 + hash22(g + id5) - p;
                         f = flipDistance(o1 - o5, o4 - o5, o2 - o5)<0.? 1. : -1.;
                         if(f>0.){ n = o1, n1 = o4, n2 = o2; tri.nID[0] = id1; tri.nID[1] = id4; tri.nID[2] = id2; }
                         else { n = o1, n1 = o5, n2 = o2; tri.nID[0] = id1; tri.nID[1] = id5; tri.nID[2] = id2; } 
                        
                          
                         tri.id0 = id0; tri.id1 = id1; tri.id2 = id2; 
                         tri.lID[0] = id2; tri.lID[1] = id3; tri.lID[2] = id0;
                        
                    }                  
                }

                
                tri.p0 = h; tri.p1 = h1; tri.p2 = h2; // Triangle vertices.
                tri.id = tri.cID + g + vec2(i - 1, j); // Triangle ID.
                
                tri.id0 += g; tri.id1 += g; tri.id2 += g; // Individual triangle vertex IDs.
                
                // Neighboring triangle vertices.
                tri.l = l; tri.l1 = l1; tri.l2 = l2;
                tri.m = m; tri.m1 = m1; tri.m2 = m2;
                tri.n = n; tri.n1 = n1; tri.n2 = n2;
                
                // Neighboring triangle vertex IDs.
                tri.lID[0] += g; tri.lID[1] += g; tri.lID[2] += g;
                tri.mID[0] += g; tri.mID[1] += g; tri.mID[2] += g;
                tri.nID[0] += g; tri.nID[1] += g; tri.nID[2] += g;


                
                // Once we've effectively hit a triangle, break to save further calculations.
                break;
                
            }
            
                       
		}
	}
    
     
    
    // Return the triangle object -- Vertices, IDs, etc.
    return tri;
}


void mainImage(out vec4 fragColor, in vec2 fragCoord){

    
    // COORDINATES AND SETUP
    //
    // Screen coordinates. Note that I've put restrictions on the resolution. I coded this for
    // the 800 by 450 canvas, so the image looks a little bloated in fullscreen. Therefore, I've
    // attempted to counter that by restricting is to 800 pixels... It kind of works. :)
	vec2 uv = (fragCoord.xy - iResolution.xy*.5)/clamp(iResolution.y, 350., 800.);
    
    // Subtle convex screen bulge for a bit of visual variance and that slightly dizzying effect. :)
    vec2 uv2 = uv*vec2(iResolution.y/iResolution.x, 1);
    uv *= .95 + dot(uv2, uv2)*.1;
    
    
    #ifdef FIXED
    // Basic diagonal scrolling.
    vec2 p = uv*5. - vec2(2, 1)*iTime/8.;
    #else 
    // Moving everything down slightly to give the mild impression that the structure is
    // slowly sliding down a wall... or something. I make this up as I go along. :)
    vec2 p = uv*5. - vec2(0, -1)*iTime/8. - vec2(0, 1);
    #endif
    
    

    // DUEL TRIANGLE AND VORONOI PROCESSING
    //
    // Perform the triangulation: This function returns the triangle object struct, which consists of the
    // three triangle vertices, the unique cell ID, and another triangle ID for coloring.
    triObj tri = triangulate(p);
    
    
    // Calculating the circumcenters of the central triangle and its neighbors. Connecting the central
    // circumcenter to the neighboring ones produce the dual Voronoi edges. A few other connections need
    // to made -- due to the grid nature of this example, but that's the gist of it.
    vec2 cC = circCent(tri.p0, tri.p1, tri.p2);
    vec2 cC0 = circCent(tri.l, tri.l1, tri.l2);
    vec2 cC1 = circCent(tri.m, tri.m1, tri.m2);
    vec2 cC2 = circCent(tri.n, tri.n1, tri.n2);
   
/*
    // Distances from our pixel to the three edges. Since we're weaving lines, each have to be accounted
    // for seperately.
    float d0 = distLine(tri.p0, tri.p1); 
    float d1 = distLine(tri.p1, tri.p2);	
    float d2 = distLine(tri.p2, tri.p0);
    
        // Depending on quad arrangement (diagonal arrangement), "d0" and "d1" need to be swapped in order to
    // match the correct edges... Like too many things, I found that out the hard way. :D
    if(tri.cID.x<1.5){
        
        float temp = d0;
        d0 = d1;
        d1 = temp;
        
    }
*/

    // Distances from our pixel to the three edges. Since we're weaving lines, each have to be accounted
    // for seperately. We're using the shared neighboring edge coordinates, since they're oriented 
    // correctly.
    float d2 = distLine(tri.l, tri.l2); 
    float d1 = distLine(tri.n, tri.n2);	
    float d0 = distLine(tri.m, tri.m2);
    

    
    // Triangle line identification. We need to identify which line we're rendering in order to randomly 
    // render it above or below the Voronoi lines.
    float lineID = (d0<d1 && d0<d2)? 0. : d1<d2? 1. : 2.;
    
    // The triangle distance field value, for shading purposes.
    float triDist = min(min(d0, d1), d2);
    

    // Cell color, based on the four triangle arrangements: Top-left, top-right, bottom-left and bottom-right.
    vec3 cellCol;
    
    if(tri.cID.x == 3.) cellCol = vec3(1, 1.3, .6); // Green.
    else if(tri.cID.x == 2.) cellCol = vec3(1.4, 1, .6); // Orangey brown.
    else if(tri.cID.x == 1.) cellCol = vec3(.6, 1, 1.4); // Blue.
    else cellCol = vec3(1.4, .7, .8); // Pinkish red.
    
    //if(hash21(tri.id)>.4) cellCol = grey(cellCol);
    
    #if PALETTE==0
    // The less complicated default palette. For amateurs like myself, fewer colors are easier to work with. :)
    if(hash21(tri.id)>.25) cellCol = grey(cellCol); // Greyscale.
    else cellCol = mix(vec3(1.3, .2, .1), vec3(1.3, .5, .3), dot(sin(p*3. - cos(p.yx*3.)), vec2(.25)) + .5);
    
    /*
    // Blinking color version.
    float blink = smoothstep(.65, .75, sin(hash21(tri.id)*6.283 + iTime));
    vec3 col1 = grey(cellCol); 
    vec3 col2 =  mix(vec3(1.3, .2, .1), vec3(1.3, .5, .3), dot(sin(p*3. - cos(p.yx*3.)), vec2(.25)) + .5);
    cellCol = mix(col1, col2, blink);
    */
        
    #elif PALETTE==2
    if(tri.cID.x == 1. || tri.cID.x == 3.) cellCol = grey(cellCol);
    #elif PALETTE==3
    cellCol = grey(cellCol);
    #endif
    
    // The triangle cell background.
    // Mixing in a bit of the edge color with the cell color to give the impression that some mild lighting 
    // is occurring.
    vec3 bg = mix(cellCol, vec3(1, .9, .7), .25);
    // Mutliplying by a factor of the triangular distance for a bit of shading.
    bg *= (triDist*.7 + .3)*1.55;
    
    // Start the layering process by initating to the triangle cell background.
    vec3 col = bg;
    
        
    // Cell background lines: Apologies for the compiler directive mess below. :) Basically, I wanted
    // a triangle pattern in the colored triangles and diagonal hash lines in the greyscale ones.
    #if PALETTE==1 || PALETTE==3
    //float str = clamp(sin((rot2(tri.id.x)*p).x*6.283*20.)*1.5 + 1.35, 0., 1.); // Cell rotated lines.
    #if PALETTE==1
    float str = clamp(sin((triDist)*6.283*16.)*1.5 + 1.25, 0., 1.); // Triangle lines.
    #else
    float str = clamp(sin((p.y - p.x)*6.283*14.)*1.5 + 1.35, 0., 1.);
    str = max(str, hash21(floor((p)*96.)) - .35);
    #endif
    col *= str*.35 + .65; // Diagonal lines.
    #else
    float str;
    #if PALETTE==0
    if(hash21(tri.id)<=.25)
    #elif PALETTE==2
    if(tri.cID.x == 0. || tri.cID.x == 2.)
    #endif
        str = clamp(sin((triDist)*6.283*16.)*1.5 + 1.25, 0., 1.); // Triangle lines.
    else {
        str = clamp(sin((p.y - p.x)*6.283*14.)*1.5 + 1.35, 0., 1.);
        str = max(str, hash21(floor((p)*96.)) - .35);
    }
    col *= str*.35 + .65; // Diagonal lines.
    #endif
     
  
    
 
////    
    
    // BOTTOM TRIANGLE LINES    
 
    vec3 lCol = vec3(1, .8, .6); // Line color.
    #ifdef GREY_LINES 
    lCol = grey(lCol);
    #endif
    
    // Layering order flags to determine whether to render a particular triangle line under the Voronoi line,
    // or over the top. It took me ages to come up with -- then convince myself -- of this logic: The line 
    // consists of two half edges; The actual triangle edge, and the same shared edge from the neighboring 
    // triangle. The sum of the two point IDs describing the shared edge will be the same for each side of the edge, 
    // which makes it a unique identifier... OK, I'm getting confused again... but it works anyway. :D
    bool lineFirst[3];
    lineFirst[2] = hash21(tri.lID[0] + tri.lID[2])>.5? true : false; // Diagonal rendering.
    lineFirst[1] = hash21(tri.nID[0] + tri.nID[2])>.5? true : false; // Vertical (left\right) rendering.
    lineFirst[0] = hash21(tri.mID[0] + tri.mID[2])>.5? true : false; // Horizonal (top\bottom) rendering.
     
    
    
    
    vec2 p0 = vec2(0), p1 = vec2(0);
    float d = 1e8;
     
    // Triangle line logic.
    if(lineID == 2.) { // Diagonal edge line.
        
        // lCol =  lCol.zyx;  // Debug line color.        
        if(lineFirst[2]){ d = d2; p0 = tri.p0; p1 = tri.p2; }
        
    }
    else if(lineID == 1.){ // Vertical edge line.
        
        // lCol =  lCol.xzy;  // Debug line color.        
        if(lineFirst[1]){ d = d1; p0 = tri.n; p1 = tri.n2; }
 
    }
    else { // Horizontal edge line.
        
        // lCol =  lCol.zxy;  // Debug line color.        
        if(lineFirst[0]){ d = d0; p0 = tri.m; p1 = tri.m2; }
        
    }
    
    // Rendering the triangle lines.
    const float lw = .03;
    float lns = linePattern(p, p0, p1);
    float shade = clamp(1. - d*4., 0., 1.)*.75 + clamp(d*32., 0., 1.)*.4;        
    d -= lw;
    col = mix(col, vec3(0), (1. - smoothstep(0., .1, d - .01))*.65);
    col = mix(col, vec3(0), 1. - smoothstep(0., .015, d - .0225));
    col = mix(col, lCol*lns*shade*.9, 1. - smoothstep(0., .015, d));
    col = mix(col, vec3(0), (1. - smoothstep(0., .015, d + .025))*.5);
     
     
    // VORONOI EDGES AND VERTICES
    
    float vorDist = 1e8;//tri.circ;// - length(cC - tri.p0);
    
    
    // The lines connecting the circumcenter of the hit triangle to the circumcenters of the three 
    // neighboring triangles. These represent Voronoi edges. Normally, you'd need just these three, 
    // but sometimes a triangle's circumcenter will fall outside the triangle, meaning we have to
    // render some extra lines. See below.
    vorDist = min(vorDist, distLine(cC, cC0));
    vorDist = min(vorDist, distLine(cC, cC1));
    vorDist = min(vorDist, distLine(cC, cC2));

    // Opposite bisecting sides of the diagonally opposite neighboring triangle... Don't worry. That confuses
    // me as well, and I wrote it. :D Seriously, just comment out any of the pairs of lines below to see 
    // what they connect, and why they're necessary.
    vorDist = min(vorDist, distLine(cC0, (tri.l + tri.l1)/2.));
    vorDist = min(vorDist, distLine(cC0, (tri.l1 + tri.l2)/2.));
    
    // Opposite bisecting sides of the top or bottom neighboring triangle.
    vorDist = min(vorDist, distLine(cC1, (tri.m + tri.m1)/2.));
    vorDist = min(vorDist, distLine(cC1, (tri.m1 + tri.m2)/2.));
    
    // Opposite bisecting sides of the left or right neighboring triangle.
    vorDist = min(vorDist, distLine(cC2, (tri.n + tri.n1)/2.));
    vorDist = min(vorDist, distLine(cC2, (tri.n1 + tri.n2)/2.));
        
    
    // Rendering the Voronoi lines.
    vec3 vorCol = vec3(1, .9, .8);
    // Dark strip shading.
    float vShade = clamp(vorDist*40. - .5, 0., 1.);
    // Alternate light strip shading.
    //float vShade = clamp(1. - vorDist*8., 0., 1.)*.75 + clamp(.0 + vorDist*32., 0., 1.)*.5;//clamp(vorDist*32. + .5, 0., 1.);
    vorDist -= .035;
    col = mix(col, vec3(0), (1. - smoothstep(0., .1, vorDist - .01))*.65);
    col = mix(col, vec3(0), 1. - smoothstep(0., .015, vorDist - .02));
    col = mix(col, vorCol*vShade, 1. - smoothstep(0., .015, vorDist));
    
    
    // Individual Voronoi edge vertices.    
    float verts = 1e8;
    const float vw = .06; 
    float vertcC = min(verts, length(cC)) - vw;
    float vert0 = min(verts, length(cC0)) - vw;
    float vert1 = min(verts, length(cC1)) - vw;
    float vert2 = min(verts, length(cC2)) - vw;
    
 
    // Rendering the Voronoi edge end-point vertices.
    vec3 vertCol = vec3(1, .96, .92);//vec3(.85, .92, 1)
    float vertSh = min(min(vertcC, vert0), min(vert1, vert2));
    col = mix(col, vec3(0), (1. - smoothstep(0., .1, vertSh - .01))*.75);
     
    col = mix(col, vec3(0), 1. - smoothstep(0., .01, vert2 - .02));
    col = mix(col, vertCol, 1. - smoothstep(0., .01, vert2));
    vert2 += .05;
    col = mix(col, vec3(0), 1. - smoothstep(0., .01, vert2 - .02));
	col = mix(col, vec3(1, .9, .7), 1. - smoothstep(0., .01, vert2));
    
    col = mix(col, vec3(0), 1. - smoothstep(0., .01, vert1 - .02));
    col = mix(col, vertCol, 1. - smoothstep(0., .01, vert1));
    vert1 += .05;
    col = mix(col, vec3(0), 1. - smoothstep(0., .01, vert1 - .02));
	col = mix(col, vec3(1, .9, .7), 1. - smoothstep(0., .01, vert1));
    
    col = mix(col, vec3(0), 1. - smoothstep(0., .01, vert0 - .02));
    col = mix(col, vertCol, 1. - smoothstep(0., .01, vert0));
    vert0 += .05;
    col = mix(col, vec3(0), 1. - smoothstep(0., .01, vert0 - .02));
	col = mix(col, vec3(1, .9, .7), 1. - smoothstep(0., .01, vert0));
    
    col = mix(col, vec3(0), 1. - smoothstep(0., .01, vertcC - .02));
    col = mix(col, vertCol, 1. - smoothstep(0., .01, vertcC));
    vertcC += .05;
    col = mix(col, vec3(0), 1. - smoothstep(0., .01, vertcC - .02));
	col = mix(col, vec3(1, .9, .7), 1. - smoothstep(0., .01, vertcC));
    
    
    
    
   
    
    
    
    
    // TOP TRIANGLE LINES     
    
    // Triangle line logic.
    p0 = vec2(0), p1 = vec2(0);
    d = 1e8;
     
    if(lineID == 2.) { // Diagonal edge line.
        
        // lCol =  lCol.zyx;  // Debug line color.        
        if(!lineFirst[2]){ d = d2; p0 = tri.p0; p1 = tri.p2; }
        
    }
    else if(lineID == 1.){ // Vertical edge line.
        
        // lCol =  lCol.xzy;  // Debug line color.        
        if(!lineFirst[1]){ d = d1; p0 = tri.n; p1 = tri.n2; }
 
    }
    else { // Horizontal edge line.
        
        // lCol =  lCol.zxy;  // Debug line color.        
        if(!lineFirst[0]){ d = d0; p0 = tri.m; p1 = tri.m2; }
        
    }
    
    // Rendering the triangle lines.
    lns = linePattern(p, p0, p1);
    shade = clamp(1. - d*4., 0., 1.)*.75 + clamp(d*32., 0., 1.)*.4;        
    d -= lw;
    col = mix(col, vec3(0), (1. - smoothstep(0., .1, d - .02))*.65);
    col = mix(col, vec3(0), 1. - smoothstep(0., .015, d - .0225));
    col = mix(col, lCol*lns*shade, 1. - smoothstep(0., .015, d));
    col = mix(col, vec3(0), (1. - smoothstep(0., .015, d + .025))*.5);  
    
      
    // Triangle vertices.
    vertCol = vec3(1, .9, .75);//vec3(.85, .92, 1)
    float tVerts = min(min(length(tri.p0), length(tri.p1)), length(tri.p2));
    shade = 1.;//clamp(1.1 - tVerts*4., 0., 1.);
    tVerts -= .09;
    //verts = tri.verts - .04;
    col = mix(col, vec3(0), (1. - smoothstep(0., .1, tVerts - .01))*.75);
    col = mix(col, vec3(0), 1. - smoothstep(0., .01, tVerts - .02));
	col = mix(col, vertCol*shade, 1. - smoothstep(0., .01, tVerts));//vec3(1, .9, .7)
    
    tVerts += .06;
    col = mix(col, vec3(0), 1. - smoothstep(0., .01, tVerts - .02));
	col = mix(col, vec3(1, .8, .6)*.75, 1. - smoothstep(0., .01, tVerts)); 
    
    tVerts += .02;
	col = mix(col, vec3(0), (1. - smoothstep(0., .01, tVerts))*.7); 
    

  
    // SQUARE GRID
    //
    // Square grid -- for a visual guide.
    #ifdef SHOW_GRID_CELLS
    // Cell borders: If you take a look at the triangles overlapping any individual square cell, 
    // you'll see that several partial triangles contribute, and the vertices that make up each 
    // triangle span the 8 surrounding cells. This is the reason why you have to test for
    // contributing triangle intersections from all 9 cells.
    vec2 q = abs(fract(p) - .5);
    float bord = max(q.x, q.y) - .5;
    bord = max(bord, -(bord + .008));
    
    col = mix(col, vec3(0), (1. - smoothstep(0., .1, bord - .01))*.35);
    col = mix(col, vec3(0), (1. - smoothstep(0., .01, bord - .015)));
    col = mix(col, vec3(.8, .9, 1), (1. - smoothstep(0., .01, bord))*1.);
    #endif     
    
    
    // POSTPROCESSING AND SCREEN PRESENTATION 
     
    // Vignette.
    uv = fragCoord/iResolution.xy;
    col = mix(col*1.05, vec3(0), (1. - pow(16.*uv.x*uv.y*(1.-uv.x)*(1.-uv.y), 0.125)));

    
    // Rough gamma correction.
	fragColor = vec4(sqrt(max(col, 0.)), 1);
    
} 
