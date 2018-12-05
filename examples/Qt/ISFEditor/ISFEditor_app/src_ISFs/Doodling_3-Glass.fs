/*
{
    "CATEGORIES": [
        "Automatically Converted",
        "Shadertoy"
    ],
    "DESCRIPTION": "Automatically converted from https://www.shadertoy.com/view/ldVyzh by lsdlive.  Doodling session.\nThanks to iq, mercury, lj, aiekick, balkhan & all shadertoyers.\nGreets to all the shader showdown paris gang.\n\nPhantom mode from aiekick: https://www.shadertoy.com/view/MtScWW",
    "IMPORTED": {
    },
    "INPUTS": [
    ]
}

*/


mat2 r2d(float a) {
    float c=cos(a),s=sin(a);
    return mat2(c, s, -s, c);
}

float sc(vec3 p, float s) {
    p=abs(p);
    p=max(p,p.yzx);
    return min(p.x,min(p.y,p.z)) - s;
}

vec2 amod(vec2 p, float c) {
    float m = c/6.28;
    float a = mod(atan(p.x,p.y)-m*.5, m)-m*.5;
    return vec2(cos(a), sin(a)) * length(p);
}

void mo(inout vec2 p, vec2 d) {
    p.x = abs(p.x) - d.x;
    p.x = abs(p.x) - d.x;
    if(p.y>p.x)p=p.yx;
}

float g = 0.;
float de(vec3 p) {
    
    vec3 q = p;
    
    p.xy *= r2d(TIME*.1);
    
    
    q.xy += vec2(sin(q.z*2.+1.)*.2+sin(q.z)*.05, sin(q.z)*.4);
    float c = length(max(abs(q+vec2(0,.05).xyx) - vec3(.01, .01, 1e6), 0.));
    
    p.xy *= r2d(p.z*.1);
    p.xy = amod(p.xy, 19.);//8.
    float d1 = 2.;
    p.z = mod(p.z-d1*.5, d1) - d1*.5;
    
    
    mo(p.xy, vec2(.1, .3));
    mo(p.xy, vec2(.8, .9));
    
    p.x = abs(p.x) - .8;

    p.xy *= r2d(.785);
    mo(p.xy, vec2(.2, .2));
    
    
    float d = sc(p, .1);
    d = min(d, c);
    g+=.01/(.01+d*d);
    return d;
}

void main() {



    vec2 uv = gl_FragCoord.xy/RENDERSIZE.xy -.5;
    uv.x*=RENDERSIZE.x/RENDERSIZE.y;
    
    vec3 ro=vec3(0,0,TIME*2.);
    vec3 rd=normalize(vec3(uv,1));
       
    vec3 p;
    float ri,t=0.;
    for(float i=0.;i<1.;i+=.01) {
        ri=i;
        p=ro+rd*t;
        float d = de(p);
        if(t>30.) break;
        d = max(abs(d), .0004);
        t+=d*.5;
    }
    vec3 bg = vec3(.2, .1, .2);
    vec3 col = bg;
    if(t<=30.)
	    col = mix(vec3(.9, .3, .3), bg, uv.x+ri);
    col+=g*.02;
    col = mix(col, bg, 1.-exp(-.1*t*t));
    
    gl_FragColor = vec4(col,1.0);
}
