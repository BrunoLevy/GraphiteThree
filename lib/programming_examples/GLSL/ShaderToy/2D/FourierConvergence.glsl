//stage GL_FRAGMENT_SHADER
//import <GLUP/ShaderToy.h>

// Fourier series:
// https://en.wikipedia.org/wiki/Fourier_series#Convergence
//
// ========================================
// Square wave:
// http://mathworld.wolfram.com/FourierSeriesSquareWave.html
//
// Triangle wave:
// http://mathworld.wolfram.com/TriangleWave.html
//
// Sawtooth wave:
// http://mathworld.wolfram.com/SawtoothWave.html
//
// ========================================
// distance formula used to plot waves smoothly
// d = |fx-y|/sqrt(1+(dfx/dx)^2)
// http://www.iquilezles.org/www/articles/distance/distance.htm
// ========================================
//
// hue color picker taken from :
// https://www.shadertoy.com/view/ll2cDc
//
// following function is used for smooth transition:
// https://math.stackexchange.com/questions/2746958/smooth-floor-function

const float pi = 3.14159265359;
const float scale = 2.0;
const float thickness = 3.0*scale;

const float duration = 12.0; // duration visualizing each wave in seconds.
const float transition = 5.0; // transition speed. (dont know the unit) 
                              // but 1.0 means transition takes half of duration.
                              // so it takes half of duration to make "n" terms
                              // and another half of duration to remove "n" terms

const int terms = 10; // number of terms to produce
const float freq = 0.795; // frequency in hz
const float len = 2.0; // length of the wave. (period)
const float amp = 1.0; // amplitude of the wave.

vec2 uvmap(vec2 uv)
{
    return (2.0*uv - iResolution.xy)/iResolution.y;
}

// color picker:
// https://www.shadertoy.com/view/ll2cDc
vec3 pickColor(float n) {
    return 0.6+0.6*cos(6.3*n+vec3(0,23,21));
}

float smoothout(float dist){
    return smoothstep(thickness/iResolution.y,0.0,dist);
}

float circle(vec2 uv, vec2 C, float r, bool fill)
{
    vec2 p = uv-C;
    float fx = length(p)-r;
    float dist = fill? fx:abs(fx);
    return smoothout(dist);
}

float line(vec2 p, vec2 a, vec2 b)
{
    vec2 pa = p - a, ba = b - a; 
    float h = clamp(dot(pa, ba) / dot(ba, ba), 0.0, 1.0);
    float dist = length(pa - ba * h);
    return smoothout(dist);
}

// ==================Square Wave===================== 
float squarewave(float n, float x, float l, float phase){
    n = n*2.0+1.0;
    return amp*4.0/(n*pi)*sin(n*pi*x/l+phase);
}

// derivative of series terms.
float dsquarewave(float n, float x, float l, float phase){
    n = n*2.0+1.0;
    return amp*4.0/l*cos(n*pi*x/l+phase);
}

// ===================Triangle Wave==================
float trianglewave(float n, float x, float l, float phase){
    float k = n*2.0+1.0;
    return amp*8.0/(pi*pi)/(k*k)*cos(pi*n)*sin(k*pi*x/l+phase);
}

float dtrianglewave(float n, float x, float l, float phase){
    float k = n*2.0+1.0;
    return amp*8.0/pi/(k*l)*cos(pi*n)*cos(k*pi*x/l+phase);
}

// ====================Sawtooth Wave===================
float sawtoothwave(float n, float x, float l, float phase){
    n++;
    return amp*2.0/(pi*n)*sin(n*pi*x/l+phase);
}

float dsawtoothwave(float n, float x, float l, float phase){
    n++;
    return amp*2.0/l*cos(n*pi*x/l+phase);
}

// ====================Wave switch===================
float wave(float n, float x, float l, float phase){
    switch(int(mod(iTime, duration*3.0) / duration))
    {
        case 0: return squarewave(n,x,l,phase);
        case 1: return sawtoothwave(n,x,l,phase);
        case 2: return trianglewave(n,x,l,phase);
    }
}

// derivative of series terms.
float dwave(float n, float x, float l, float phase){
    switch(int(mod(iTime, duration*3.0) / duration))
    {
        case 0: return dsquarewave(n,x,l,phase);
        case 1: return dsawtoothwave(n,x,l,phase);
        case 2: return dtrianglewave(n,x,l,phase);
    }
}
// =======================================

// used for smooth transitions
float smoothfloor(float x) {
    return x - sin(2.0*pi*x)/(2.0*pi);
}

// maps sin wave with amplitude [-1,1]    and period 2*pi
// to   sin wave with amplitude [0,terms] and period duration
float clock(){
    return float(terms)/2.0*(1.0 - cos(iTime*2.0*pi/duration));
}

void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
    vec2 uv = uvmap(fragCoord.xy)*scale;
    vec3 col = vec3(0);
        
    float l = len/2.0; // wave length divided by two.
    vec2 c = vec2(0); // center of the circles
    float sum = 0.0; // fourier series sum
    float dsum = 0.0; // derivative of the sum
    float tsum = 0.0; // sum for red line
    
    float smfloor = smoothfloor(clock()*transition);
    float time = iTime*freq;
    
    for(int i=0; i<terms; i++) {
        float n = float(i);
        vec3 color = pickColor(n/float(terms));
        
        float mul = clamp(smfloor - n,0.0,1.0);
        
        // calculate fourier series terms for circles
        float term = wave(n, time, l, 0.0)*mul;
        float cterm = wave(n, time, l, pi/2.0)*mul;
        vec2 r = vec2(cterm,term);
        
        // plot circles
        col += circle(uv,c,length(r),false)*color;
        col += line(uv,c, c += r)*color;
        
        // calculate fourier series terms for wave plot
        sum += wave(n, time-uv.x+len/2.0, l, 0.0)*mul;
        dsum += dwave(n, time-uv.x+len/2.0, l, 0.0)*mul;
        tsum += term;
    }
    
    // wave plot
    float dist = abs(uv.y-sum)/sqrt(1.0+dsum*dsum);
    col+=smoothout(dist);

    // red line
    col+=(line(uv,c,vec2(+l,c.y))
        + line(uv,c,vec2(-l,c.y))
        + circle(uv,vec2(+l,tsum),0.01,true)
        + circle(uv,vec2(-l,tsum),0.01,true))*vec3(1,0,0);
    
    // grid lines
    col+=(smoothout(abs(uv.y+amp))
        + smoothout(abs(uv.y-amp))
        + smoothout(abs(uv.x+l))
        + smoothout(abs(uv.x-l)))*vec3(0.25);
    
    // output to screen
    fragColor = vec4(col,1.0);
}