/*
{
    "CATEGORIES": [
        "Automatically Converted",
        "Shadertoy"
    ],
    "DESCRIPTION": "Automatically converted from https://www.shadertoy.com/view/4lGyW1 by tdhooper.  Variation of [url=https://www.shadertoy.com/view/XtGyWh]Shaderdough x-ray[/url]",
    "IMPORTED": {
    },
    "INPUTS": [
        {
            "NAME": "iMouse",
            "TYPE": "point2D"
        }
    ]
}

*/


// --------------------------------------------------------
// OPTIONS
// --------------------------------------------------------

// Disable to see more colour variety
#define SEAMLESS_LOOP
#define COLOUR_CYCLE
#define HIGH_QUALITY

// --------------------------------------------------------
// http://www.neilmendoza.com/glsl-rotation-about-an-arbitrary-axis/
// --------------------------------------------------------

mat3 rotationMatrix(vec3 axis, float angle)
{
    axis = normalize(axis);
    float s = sin(angle);
    float c = cos(angle);
    float oc = 1.0 - c;

    return mat3(
        oc * axis.x * axis.x + c,           oc * axis.x * axis.y - axis.z * s,  oc * axis.z * axis.x + axis.y * s,
        oc * axis.x * axis.y + axis.z * s,  oc * axis.y * axis.y + c,           oc * axis.y * axis.z - axis.x * s,
        oc * axis.z * axis.x - axis.y * s,  oc * axis.y * axis.z + axis.x * s,  oc * axis.z * axis.z + c
    );
}


// --------------------------------------------------------
// HG_SDF
// https://www.shadertoy.com/view/Xs3GRB
// --------------------------------------------------------

#define PI 3.14159265359
#define PHI (1.618033988749895)


float t;


float vmax(vec3 v) {
    return max(max(v.x, v.y), v.z);
}

float sgn(float x) {
    return (x<0.)?-1.:1.;
}

// Rotate around a coordinate axis (i.e. in a plane perpendicular to that axis) by angle <a>.
// Read like this: R(p.xz, a) rotates "x towards z".
// This is fast if <a> is a compile-time constant and slower (but still practical) if not.
void pR(inout vec2 p, float a) {
    p = cos(a)*p + sin(a)*vec2(p.y, -p.x);
}

// Reflect space at a plane
float pReflect(inout vec3 p, vec3 planeNormal, float offset) {
    float t = dot(p, planeNormal)+offset;
    if (t < 0.) {
        p = p - (2.*t)*planeNormal;
    }
    return sign(t);
}

// Repeat around the origin by a fixed angle.
// For easier use, num of repetitions is use to specify the angle.
float pModPolar(inout vec2 p, float repetitions) {
    float angle = 2.*PI/repetitions;
    float a = atan(p.y, p.x) + angle/2.;
    float r = length(p);
    float c = floor(a/angle);
    a = mod(a,angle) - angle/2.;
    p = vec2(cos(a), sin(a))*r;
    // For an odd number of repetitions, fix cell index of the cell in -x direction
    // (cell index would be e.g. -5 and 5 in the two halves of the cell):
    if (abs(c) >= (repetitions/2.)) c = abs(c);
    return c;
}


// --------------------------------------------------------
// IQ
// https://www.shadertoy.com/view/ll2GD3
// --------------------------------------------------------

vec3 pal( in float t, in vec3 a, in vec3 b, in vec3 c, in vec3 d ) {
    return a + b*cos( 6.28318*(c*t+d) );
}

vec3 spectrum(float n) {
    return pal( n, vec3(0.5,0.5,0.5),vec3(0.5,0.5,0.5),vec3(1.0,1.0,1.0),vec3(0.0,0.33,0.67) );
}


// --------------------------------------------------------
// knighty
// https://www.shadertoy.com/view/MsKGzw
// --------------------------------------------------------

int Type=5;
vec3 nc;
vec3 pbc;
vec3 pca;
void initIcosahedron() {//setup folding planes and vertex
    float cospin=cos(PI/float(Type)), scospin=sqrt(0.75-cospin*cospin);
    nc=vec3(-0.5,-cospin,scospin);//3rd folding plane. The two others are xz and yz planes
    pbc=vec3(scospin,0.,0.5);//No normalization in order to have 'barycentric' coordinates work evenly
    pca=vec3(0.,scospin,cospin);
    pbc=normalize(pbc); pca=normalize(pca);//for slightly better DE. In reality it's not necesary to apply normalization :) 

}

void pModIcosahedron(inout vec3 p) {
    p = abs(p);
    pReflect(p, nc, 0.);
    p.xy = abs(p.xy);
    pReflect(p, nc, 0.);
    p.xy = abs(p.xy);
    pReflect(p, nc, 0.);
}

float splitPlane(float a, float b, vec3 p, vec3 plane) {
    float split = max(sign(dot(p, plane)), 0.);
    return mix(a, b, split);
}

float icosahedronIndex(inout vec3 p) {
    vec3 sp, plane;
    float x, y, z, idx;

    sp = sign(p);
    x = sp.x * .5 + .5;
    y = sp.y * .5 + .5;
    z = sp.z * .5 + .5;

    plane = vec3(-1. - PHI, -1, PHI);

    idx = x + y * 2. + z * 4.;
    idx = splitPlane(idx, 8. + y + z * 2., p, plane * sp);
    idx = splitPlane(idx, 12. + x + y * 2., p, plane.yzx * sp);
    idx = splitPlane(idx, 16. + z + x * 2., p, plane.zxy * sp);

    return idx;
}

vec3 icosahedronVertex(vec3 p) {
    vec3 sp, v, v1, v2, v3, result, plane;
    float split;
    v = vec3(PHI, 1, 0);
    sp = sign(p);
    v1 = v.xyz * sp;
    v2 = v.yzx * sp;
    v3 = v.zxy * sp;

    plane = vec3(1, PHI, -PHI - 1.);

    split = max(sign(dot(p, plane.xyz * sp)), 0.);
    result = mix(v2, v1, split);
    plane = mix(plane.yzx * -sp, plane.zxy * sp, split);
    split = max(sign(dot(p, plane)), 0.);
    result = mix(result, v3, split);

    return normalize(result);
}

// Nearest vertex and distance.
// Distance is roughly to the boundry between the nearest and next
// nearest icosahedron vertices, ensuring there is always a smooth
// join at the edges, and normalised from 0 to 1
vec4 icosahedronAxisDistance(vec3 p) {
    vec3 iv = icosahedronVertex(p);
    vec3 originalIv = iv;

    vec3 pn = normalize(p);
    pModIcosahedron(pn);
    pModIcosahedron(iv);

    float boundryDist = dot(pn, vec3(1, 0, 0));
    float boundryMax = dot(iv, vec3(1, 0, 0));
    boundryDist /= boundryMax;

    float roundDist = length(iv - pn);
    float roundMax = length(iv - vec3(0, 0, 1.));
    roundDist /= roundMax;
    roundDist = -roundDist + 1.;

    float blend = 1. - boundryDist;
    blend = pow(blend, 6.);
    
    float dist = mix(roundDist, boundryDist, blend);

    return vec4(originalIv, dist);
}

// Twists p around the nearest icosahedron vertex
void pTwistIcosahedron(inout vec3 p, float amount) {
    vec4 a = icosahedronAxisDistance(p);
    vec3 axis = a.xyz;
    float dist = a.a;
    mat3 m = rotationMatrix(axis, dist * amount);
    p *= m;
}


// --------------------------------------------------------
// MAIN
// --------------------------------------------------------

struct Model {
    float dist;
    vec3 colour;
    float id;
};
     
     
Model fInflatedIcosahedron(vec3 p) {
    float d = 1000.;

    // Slightly inflated icosahedron
    float idx = icosahedronIndex(p);
    d = min(d, dot(p, pca) - .9);
    d = mix(d, length(p) - .9, 1.0);

    // Colour each icosahedron face differently
    # ifdef SEAMLESS_LOOP
        if (idx == 3.) {
            idx = 2.;
        }
        idx /= 10.;
    # else
        idx /= 20.;
    # endif
    # ifdef COLOUR_CYCLE
        idx = mod(idx + t*1., 1.);
    # endif
    vec3 colour = spectrum(idx);
    
    d *= .6;
    return Model(d, colour, 1.);
}

Model model(vec3 p) {
    float rate = PI/6.;

    float a = atan(1., PHI + 1.);
    pR(p.yz, a);

    pR(p.yx, t * 2.1 + rate);
    pR(p.yz, a);

    vec3 twistCenter = vec3(.7, 0, 0);
    pR(twistCenter.yx, t * 2.1 + rate);
    pR(twistCenter.yz, a);

    p += twistCenter;
    pTwistIcosahedron(p, 10.5);
    p -= twistCenter;

    # ifdef SEAMLESS_LOOP
        pR(p.yz, -a);
        pR(p.xy, -PI/2.);
        pModPolar(p.xy, 3.);
        pR(p.xy, -PI/2.);
        pR(p.yz, -a);
    #endif

    return fInflatedIcosahedron(p);
}


Model map(vec3 p) {
    return model(p);
}


mat3 calcLookAtMatrix(in vec3 ro, in vec3 ta, in float roll) {
    vec3 ww = normalize( ta - ro );
    vec3 uu = normalize( cross(ww,vec3(sin(roll),cos(roll),0.0) ) );
    vec3 vv = normalize( cross(uu,ww));
    return mat3( uu, vv, ww );
}

const float MAX_TRACE_DISTANCE = 6.0;
const float INTERSECTION_PRECISION = 0.001;
#ifdef HIGH_QUALITY
    const float FUDGE_FACTOR = .2;
#else
    const float FUDGE_FACTOR = .4;
#endif

void main() {



    initIcosahedron();
    t = TIME - .25;
    t /= 2.;
    //t = mod(t, 1.);
    //t *= 2.5;
    
    vec2 p = (-RENDERSIZE.xy + 2.0*gl_FragCoord.xy)/RENDERSIZE.y;
    vec2 m = iMouse.xy / RENDERSIZE.xy;
    
    vec3 camPos = vec3(-1.5,1.6,0);
    vec3 camTar = -camPos + vec3(0,.1,0);
    float camRoll = 0.;
    // camera matrix
    mat3 camMat = calcLookAtMatrix( camPos, camTar, camRoll );  // 0.0 is the camera roll
    // create view ray
    vec3 rd = normalize( camMat * vec3(p.xy,1.0) ); // 2.0 is the lens length
    vec3 color = pow(vec3(.15,0,.2), vec3(2.2));    
    
    vec3 ro = camPos;
    float t = 0.0;
    float h = INTERSECTION_PRECISION * 2.0;
    float res = -1.0;
    vec3 colour;
    
    int iter = int(20. / FUDGE_FACTOR);
    for( int i=0; i < iter; i++ ){
        if( t > MAX_TRACE_DISTANCE ) break;
        Model m = map( ro+rd*t );
        h = abs(m.dist);
        t += max(INTERSECTION_PRECISION, h * FUDGE_FACTOR);
        color += m.colour * pow(max(0., (.02 - h)) * 19.5, 10.) * 150.;
        color += m.colour * .001 * FUDGE_FACTOR;
    }
    
    color = pow(color, vec3(1./1.8)) * 1.5;
    color = pow(color, vec3(1.5));
    color *= 3.5;
    
    gl_FragColor = vec4(color,1.0);
}
