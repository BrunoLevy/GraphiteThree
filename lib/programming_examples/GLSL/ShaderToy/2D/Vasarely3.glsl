//stage GL_FRAGMENT_SHADER
//import <GLUP/ShaderToy.h>

void mainImage( out vec4 O,  vec2 U )
{
	vec2 R = iResolution.xy, P;
         U = (U+U-R)/R.y * 12.;
  
    O -= O;
    if (abs(U.x) > 12.) return;
    float l;
  
#define S(x,y) P = vec2(x,y); l = length(U-P); if (l < 5.5) U = (U-P) / (3.-l*l/15.) + P
    S( 0, -5);                                                                 // bubbles
    S(-6,5.6);
    S( 6,5.6);
    
    U *= 6.28;
    float h = cos(U.y/1.5), 
          s = U.x, e=100./R.y, X, Y;

#define F(x,y,h,c1,c2,c3) X=sin(U.r/x),Y=sin(U.g/y); O= X*Y*h>0. ? s>-10.?c1:c2 :c3; O*= min(1.,8.*min(abs(X),abs(Y)))

    F( .87, 1.5, 1., vec4(1,0,0,1), vec4(.7,.4,0,1), vec4(.4+.4*cos(s/12.)) ); // red & white faces

    if (abs(h)>.5) {
        U *= mat2(.575,1,.575,-1); // U = mat2(.5,.5,.87,-.87)*U/.87;
    	F( 1.,1., h, vec4(0,0,1,1), vec4(.4,0,.7,1), O );                      // blue faces
   }

/*  
    X = sin(U.x/.87), Y = sin(U.y/1.5)                                        // red & white faces
    O = X*Y > 0.
        ? s>-10. ? vec4(1,0,0,1) : vec4(.7,.4,0,1)  
        :  vec4(.4+.4*cos(s/12.));
    
    O *= min(1.,8.*min(abs(X),abs(Y)));  // black line
    
   if (abs(h)>.5) {                                                            // blue faces
        U = mat2(.5,.5,.87,-.87)*U/.87;
    	X = sin(U.x); Y = sin(U.y);
        O = X*Y*h > 0. 
        	? s>-10. ? vec4(0,0,1,1) :  vec4(.4,0,.7,1) 
            : O;
     	O *= min(1.,8.*min(abs(X),abs(Y)));     // black line
    }
*/
}
