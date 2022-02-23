//stage GL_FRAGMENT_SHADER
//import <GLUP/ShaderToy.h>

/* "Elliptical arc distance", by mattz

   License: Creative Commons Attribution ShareAlike 4.0
   https://creativecommons.org/licenses/by-sa/4.0/
       
   This is similar to iq's "Ellipse - Distance shader"
   at https://www.shadertoy.com/view/4sS3zz, except that 
   it can also compute distances to elliptical arcs (i.e. 
   circular arcs with nonuniform scaling along the x- and
   y- axes). 

   I'm borrowing iq's awesome color scheme for visualizing 
   signed distance fields -- it really makes things pop and 
   also makes it easy to see when the SDF is incorrect.

   This shader uses the quartic solver demonstrated in 
   https://www.shadertoy.com/view/XdKyRR to find all candidate
   closest points on the ellipse to a given point; they are
   then filtered against the given angular extents.   

   See also "Bezier - Signed Distance" by ajs15822 at 
   https://www.shadertoy.com/view/ltXSDB which works similarly,
   for a quadratic Bezier curve segment.

*/

//////////////////////////////////////////////////////////////////////
// closed-form solver from https://www.shadertoy.com/view/XdKyRR 
// but without special-case checks

bvec4 solve_quartic(in vec4 coeffs,
                    out vec4 roots) {
        
    float p = coeffs[0];
    float q = coeffs[1]; 
    float r = coeffs[2];
    float s = coeffs[3];
    
    ////////////////////////////////////////////////////////////
	// form resolvent cubic and solve it to obtain one real root
        
    float i = -q;
    float j = p*r - 4.*s;
    float k = 4.*q*s - r*r - p*p*s;
    
    // coefficients of normal form
    float a = (3.*j - i*i) / 3.;
    float b = (2.*i*i*i - 9.*i*j + 27.*k) / 27.;
    
    float delta1 = b*b / 4.;
    float delta2 = a*a*a / 27.;
    
    float delta = delta1 + delta2;
    
    float z1;
    
    if (delta >= 0.) {
        vec2 AB = -0.5*b + vec2(1,-1) * sqrt(max(delta, 0.));
        AB = sign(AB) * pow(abs(AB), vec2(1.0/3.0));
        z1 = AB.x + AB.y;
    } else {
        float phi = acos( -sign(b) * sqrt(delta1/-delta2) );
        z1 = 2. * sqrt(-a/3.) * cos( phi / 3.);
    }
    
    // shift back from normal form to root of resolvent cubic
    z1 -= i/3.;
    
    ////////////////////////////////////////////////////////////
	// now form quartic roots from resolvent cubic root

    float R2 = p*p/4. - q + z1; 
        
    bool R_ok = (R2 >= 0.);

    float R = sqrt(max(R2, 0.));
    
    float foo, bar;
    
    if (R == 0.) { 
        float z124s = z1*z1 - 4.*s;
        R_ok = R_ok && (z124s >= 0.);
        foo = 3.*p*p / 4. - 2.*q;
        bar = 2.*sqrt(max(z124s, 0.));
    } else {
        foo = 3.*p*p / 4. - R2 - 2.*q;
        bar = (4.*p*q - 8.*r - p*p*p) / (4.*R);
    }
    
    bool D_ok = R_ok && (foo + bar >= 0.);
    bool E_ok = R_ok && (foo - bar >= 0.);
    
    float D = sqrt(max(foo + bar, 0.));
    float E = sqrt(max(foo - bar, 0.));
    
    roots = vec4(-p/4.) + 0.5 * vec4(R+D, R-D, -(R-E), -(R+E));
    return bvec4(D_ok, D_ok, E_ok, E_ok);

}

//////////////////////////////////////////////////////////////////////
// construct unit vector from angle around circle

vec2 from_angle(float t) {
    return vec2(cos(t), sin(t));
}

//////////////////////////////////////////////////////////////////////
// construct unit vector from cosine of angle

vec2 from_cos(float u) {
    u = clamp(u, -1., 1.);
    return vec2(u, sqrt(1. - u*u));
}

//////////////////////////////////////////////////////////////////////
// construct 2D vector perpendicular to input 

vec2 perp(vec2 v) {
    return vec2(-v.y, v.x);
}

//////////////////////////////////////////////////////////////////////
// return whichever vector has smaller x-coordinate

vec2 smaller_x(vec2 a, vec2 b) {
    return a.x < b.x ? a : b;
}

//////////////////////////////////////////////////////////////////////
// return distance from point p to ellipse centered at origin
// with radii given by ab and angular limits given by alim.
//
// alim.x = start angle
// alim.y = angular delta (use 0 or >2*PI for full ellipse)

float ellipse_arc_dist(vec2 p, vec2 ab, vec2 alim) {
    
	// constants    
    const float PI = 3.141592653589793;
    
    // accept roots from quartic solver just a bit below -1 or above +1
    const float COS_TOL = 1e-3;

    // distance and sign
    vec2 ds = vec2(1e5, -1);
    
    // needed for deciding sign for distance later
    float orig_sign = -1.;
    
    // are we going all the way around ellipse?
    bool full_ellipse = (alim[1] == 0. || abs(alim[1]) >= 2.*PI);

    // construct unit vectors corresp. to start and end angles
    vec2 n0 = from_angle(alim.x);
    vec2 n1 = from_angle(alim.x + alim.y);

    // determine some tangent vectors along ellipse at 
    // endpoints
    float delta_sign = sign(alim.y);
    vec2 t0 = -perp(n0)*delta_sign;
    vec2 t1 = perp(n1)*delta_sign;
    
    // angular extents bigger than 1/2 rotation are
    // treated differently than those less 
    float wedge_sign = 1.;
    
    if (abs(alim[1]) > PI) {
        t0 = -t0;
        t1 = -t1;
        wedge_sign = -1.;
    }

    // determine endpoints of ellipse 
    vec2 p0 = ab * n0;
    vec2 p1 = ab * n1;

    // determine whether we are inside the wedge
    // formed by the normal vectors at the endpoints
    if (!full_ellipse) {
        orig_sign = sign(max(dot(p - p0, ab.yx*n0), dot(p - p1, ab.yx*n1)));
        ds = vec2(min(length(p - p0), length(p - p1)), orig_sign);
    }
    
    
    // circles need special-case handling because the quartic
    // degenerates into a quadratic.    
    if (abs((ab.x - ab.y) / ab.x) < 1e-2) {
        
        // a and b are pratically equal, so treat this as a circle
        
        float dc = length(p) - ab.x;
        
        if (full_ellipse || max(dot(p, t0), dot(p, t1))*wedge_sign <= .0) {
            ds = smaller_x(ds, vec2(abs(dc), dc));
        }
        
    } else {
        
        // general ellipse case
        
        // the quartic is numerically ill-conditioned
        // near the y-axis, so keep on the nicely behaved
        // side of the diagonal
        if (abs(p.x) < abs(p.y)) {
            p.xy = p.yx;
            ab.xy = ab.yx;
            t0 = t0.yx;
            t1 = t1.yx;
        }
        
        // formulate the quartic polynomial that represents
        // this ellipse. we are solving for the position 
        // u = cos(theta) along the ellipse such that the
        // tangent at the point [a*u, b*sqrt(1-u^2)] is 
        // perpendicular to the displacement between p
        // and the point itself.
        //
        // there may be multiple valid solutions for this 
        // polynomial -- for a full ellipse we could just 
        // grab the maximal root but we actually need
        // to inspect each of them in the arc case.
        
        float l = ab.y*ab.y - ab.x*ab.x;

        float ax = ab.x*p.x / l;
        float by = ab.y*p.y / l;

        float a2x2 = ax*ax;
        float b2y2 = by*by;

        // vector of polynomial coefficients
        vec4 coeffs = vec4(2.*ax, (a2x2 + b2y2) - 1., -2.*ax, -a2x2);

        // solve for up to 4 roots 
        vec4 roots;
        solve_quartic(coeffs, roots);

        // for each root
        for (int i=0; i<4; ++i) {
            
            // if root is not in domain of cos(theta), discard this root
            if ( abs(roots[i]) > 1. + COS_TOL ) { continue; }
            
            // construct point on unit circle
            vec2 uv = from_cos(roots[i]);
            
            // need to check this point both above and below x-axis
            for (int j=0; j<2; ++j) {
                
                // if we are considering the entire ellipse or
                // the given uv is inside the angle clip region,
                if (full_ellipse ||
                    max(dot(uv, t0), dot(uv, t1))*wedge_sign <= .0) {
                    
                    // get the absolute distance to the closest point on 
                    // the ellipse, as well as its sign
                    vec2 pc = ab*uv;
                    ds = smaller_x(ds, vec2(length(p - pc), dot(p-pc, pc)));
                    
                }
                
                // mirror the root point across the x-axis
                uv.y *= -1.;
                
            }
        }
        
    }
    
    // combine the absolute distance with the sign of the 
    // distance, respecting original sign classification
    // before root finding
    return ds.x*max(sign(ds.y), orig_sign);
    
}

//////////////////////////////////////////////////////////////////////
// our main function

void mainImage( out vec4 fragColor, in vec2 fragCoord ) {
    
	float scl = 2.0 / iResolution.y;
    
    vec2 p = (fragCoord.xy - 0.5*iResolution.xy) * scl;
    
    vec2 ab = vec2(cos(iTime + 1.2)*.6 + .8, sin(iTime*0.1 - 0.5)*.2 + .6);
    vec2 alim = vec2(0.5*iTime, 7.*cos(0.3*iTime));
    
    if (max(iMouse.x, iMouse.y) > 20.) {
        
        ab = abs(iMouse.xy - 0.5*iResolution.xy) * scl;
        ab = max(ab, vec2(.125));    
        alim = 16. * iMouse.xy / iResolution.xy - 8.;

    }
    
    float d = ellipse_arc_dist(p, ab, alim);
    
    // iq's lovely color scheme
    vec3 col = vec3(1.0) - sign(d)*vec3(0.1,0.4,0.7);
	col *= 1.0 - exp(-2.0*abs(d));
	col *= 0.8 + 0.2*cos(120.0*d);
	col = mix( col, vec3(1.0), 1.0-smoothstep(0.0,0.02,abs(d)) );

    fragColor = vec4(col, 1);
    
}

