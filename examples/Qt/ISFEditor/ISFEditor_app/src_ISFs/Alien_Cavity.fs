/*
{
    "CATEGORIES": [
        "Automatically Converted",
        "Shadertoy"
    ],
    "DESCRIPTION": "Automatically converted from https://www.shadertoy.com/view/MdccWr by lsdlive.  Stuff I did for a livecoding session in Atom/Veda, the live version was slightly different, but I don't remember what I edited so I tried to reproduce as it was in my memory.\n\nIt's a tunnel with multiple layer of angular mod & mirror octant.",
    "IMPORTED": {
    },
    "INPUTS": [
    ],
    "PASSES": [
        {
            "FLOAT": true,
            "PERSISTENT": true,
            "TARGET": "BufferA"
        },
        {
        }
    ]
}

*/



// mostly inspired/taken from hglib, but fairly standard now in shadertoy
// http://mercury.sexy/hg_sdf/
float rep(float p, float d) {
	return mod(p - d*.5, d) - d*.5;
}

vec3 rep(vec3 p, float d) {
	return mod(p - d*.5, d) - d*.5;
}

void mo(inout vec2 p, vec2 d) {
	p.x = abs(p.x) - d.x;
	p.y = abs(p.y) - d.y;
	if (p.y > p.x)p = p.yx;
}

void amod(inout vec2 p, float m) {
	float a = rep(atan(p.x, p.y), m);
	p = vec2(cos(a), sin(a)) * length(p);
}

mat2 r2d(float a) {
    float c = cos(a), s = sin(a);
    return mat2(c, s, -s, c);
}
// </hglib>


// Tunnel pattern studied from shane & shau
// i.e. https://www.shadertoy.com/view/4tKXzV
vec2 path(float t) {
	float a = sin(t*.2 + 1.5), b = sin(t*.2);
	return vec2(a*2., a*b);
}

// signed cross (iq, from the menger cube article)
// http://www.iquilezles.org/www/articles/menger/menger.htm
float sc(vec3 p) {
	p = abs(p);
	p = max(p, p.yzx);
	return min(p.x, min(p.y, p.z)) - .02;
}

#define sph(p, r) (length(p) - r)
#define cyl sph

vec3 g;

float de(vec3 p) {
	p.xy -= path(p.z);
    
    p.xy *= r2d(1.57);// pi/2
	mo(p.xy, vec2(.9, 3.));
	mo(p.xy, vec2(.9, .3));
    vec3 q = p;
	
    float d = cyl(p.xy, .13); // cylinder
    p.z = rep(p.z, 4.);
	d = min(d, sph(p, .2));// sphere
    
	amod(p.xy, .785);// pi/4
	mo(p.zy, vec2(1., 1.2));
	p.z = rep(p.z, 1.);
	d = min(d, sc(p));// cross 1
    
    amod(q.xy, 2.09);// pi/1.5
    mo(q.zy, vec2(.2, 3.1));
    mo(q.xy, vec2(.0, .4));
	q.z = rep(q.z, 1.);
    d = min(d, sc(q));// cross 2

    // glow trick from balkhan
    // i.e. https://www.shadertoy.com/view/4t2yW1
	g += vec3(.5, .6, .5) * .025 / (.01 + d*d);
	return d;
}

vec3 camera(vec3 ro, vec2 uv, vec3 ta) {
	vec3 fwd = normalize(ta - ro);
	vec3 left = cross(vec3(0, 1, 0), fwd);
	vec3 up = cross(fwd, left);
	return normalize(fwd + uv.x*left + up*uv.y);
}

void main() {
	if (PASSINDEX == 0)	{
		vec2 uv = (gl_FragCoord.xy - .5*RENDERSIZE.xy) / RENDERSIZE.y;
	
		float dt = TIME * 6.;
		vec3 ro = vec3(0, 0, -4. + dt);
		vec3 ta = vec3(0, 0, dt);
	    vec3 rd;
	    
		ro.xy += path(ro.z);
		ta.xy += path(ta.z);
	    
		rd = camera(ro, uv, ta);
	
		float ri, t = 0.;
		for (float i = 0.; i < 1.; i += .01) {
			ri = i; vec3 p = ro + rd*t;
			float d = de(p);
			if (d<.001 || t>100.) break;
			t += d*.2;
	
		}
	
		vec3 c = mix(vec3(.9, .2, .4), vec3(.3, cos(TIME)*.1, .2), uv.x + ri);
		c.r *= sin(TIME);
		c += g * .015;
		gl_FragColor = vec4(c, 1);
	}
	else if (PASSINDEX == 1)	{


		vec2 uv = gl_FragCoord.xy / RENDERSIZE.xy;
	
	    // standard cheap "glitch" post process
		float s = sin(TIME*10.)*.008;
		float t = tan(TIME)*.002;
		gl_FragColor.r = IMG_NORM_PIXEL(BufferA,mod(uv + vec2(-s, s),1.0)).r;
		gl_FragColor.g = IMG_NORM_PIXEL(BufferA,mod(uv + vec2(-t, t),1.0)).g;
		gl_FragColor.b = IMG_NORM_PIXEL(BufferA,mod(uv + vec2(s, -s),1.0)).b;
	
	}

}
