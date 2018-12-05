/*
{
    "CATEGORIES": [
        "Automatically Converted",
        "Shadertoy"
    ],
    "DESCRIPTION": "Automatically converted from https://www.shadertoy.com/view/XtGfRG by yx.  Coded live on twitch stream in Bonzomatic, and tidied/ported to shadertoy.\nUnedited Bonzomatic shader here: [url]https://gist.github.com/willkirkby/ca8710328097693d58baa86f153875cc[/url]",
    "IMPORTED": {
    },
    "INPUTS": [
    ]
}

*/


float phi = (1.+sqrt(5.))*.5;

float noise(vec3 p)
{
    return fract(
        sin(
            dot(p, vec3(12.4536728,432.45673828,32.473682))
        )*43762.342);
}

vec2 rotate(vec2 a, float b)
{
    float c = cos(b);
    float s = sin(b);
    return vec2(
        a.x * c - a.y * s,
        a.x * s + a.y * c
    );
}

float sdIcosahedron(vec3 p, float r)
{
    float q = (sqrt(5.)+3.)/2.;

    vec3 n1 = normalize(vec3(q,1,0));
    vec3 n2 = vec3(sqrt(3.)/3.);

    p = abs(p/r);
    float a = dot(p, n1.xyz);
    float b = dot(p, n1.zxy);
    float c = dot(p, n1.yzx);
    float d = dot(p, n2.xyz)-n1.x;
    return max(max(max(a,b),c)-n1.x,d)*r; // turn into (...)/r  for weird refractive effects when you subtract this shape
}

float sdDodecahedron(vec3 p, float r)
{
    vec3 n = normalize(vec3(phi,1,0));

    p = abs(p/r);
    float a = dot(p,n.xyz);
    float b = dot(p,n.zxy);
    float c = dot(p,n.yzx);
    return (max(max(a,b),c)-n.x)*r;
}

float scene(vec3 p)
{
    p.xy = rotate(p.xy, p.z*.05);

    float n = noise(floor((p)/4.));
    float shape = fract((floor(p.x/4.)+floor(p.y/4.)*2.)/4.);
    float spinOffset1 = floor(p.z/4.);
    float spinOffset2 = floor(p.z/4.+2.);
    float spinOffset3 = floor(p.z/4.+4.);


    p = mod(p,4.)-2.;
    p.xy = rotate(p.xy, TIME+spinOffset1);
    p.yz = rotate(p.yz, TIME+spinOffset2);
    p.zx = rotate(p.zx, TIME+spinOffset3);

    if (shape < .25) {
        return min(
            sdDodecahedron(p,1.),
            sdIcosahedron(p.zyx,1.)
        );
    } else if (shape < .5) {
        return max(
            sdDodecahedron(p,1.),
            -sdIcosahedron(p.zyx,.9)
        );
    } else if (shape < .75) {
        return max(
            -sdDodecahedron(p,.95),
            sdIcosahedron(p.zyx,1.)
        );
    } else {
        return max(
            sdDodecahedron(p,.95),
            sdIcosahedron(p.zyx,1.)
        );
    }
}

void main() {



    vec2 uv = gl_FragCoord.xy / RENDERSIZE.xy - .5;
    uv.x *= RENDERSIZE.x / RENDERSIZE.y;
    uv *= 1.+length(uv)*.5;
    vec3 cam = vec3(0,0,0);
    vec3 dir = normalize(vec3(uv,1));
    //cam.yz = rotate(cam.yz, .3);
    //dir.yz = rotate(dir.yz, .3);
    cam.z = TIME * 4.;
    dir.xy = rotate(dir.xy, TIME*.1);
    //dir.yz = rotate(dir.yz, TIME*.3);
    //dir.zx = rotate(dir.zx, TIME*.3);
    float t = 0.;
    float k = 0.;
    int i;
    for(i=0;i<100;++i)
    {
        k = scene(cam+dir*t)*.55;
        t += k;
        if (k < .001) break;
    }
    vec3 h = cam+dir*t;
    vec2 o = vec2(.001, 0);
    vec3 n = normalize(vec3(
        scene(h+o.xyy)-scene(h-o.xyy),
        scene(h+o.yxy)-scene(h-o.yxy),
        scene(h+o.yyx)-scene(h-o.yyx)
    ));
    float iterFog = pow(1.-(float(i)/100.), 2.);
    float light = pow(max(0.,n.x*.5+.5),2.);
    float vignette = smoothstep(2.,0.,length(uv));
    vec3 a = mix(vec3(.01,.01,.1),vec3(0,1,1),iterFog);
    vec3 b = mix(vec3(0,0,0),vec3(1,sin(TIME*.4)*.5+.5,cos(TIME*.4)*.5+.5),light*iterFog*4.);
    gl_FragColor.rgb = a + b;
    gl_FragColor *= vignette;
}
