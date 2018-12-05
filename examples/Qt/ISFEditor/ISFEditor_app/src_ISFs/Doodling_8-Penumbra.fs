/*
{
    "CATEGORIES": [
        "Automatically Converted",
        "Shadertoy"
    ],
    "DESCRIPTION": "Automatically converted from https://www.shadertoy.com/view/XlKBDR by lsdlive.  Quick doodling yesterday, train session for the GROW's shader showdown.",
    "IMPORTED": {
    },
    "INPUTS": [
    ]
}

*/


// @lsdlive
// CC-BY-NC-SA


mat2 r2d(float a) {
	float c = cos(a), s = sin(a);
	return mat2(c, s, -s, c);
}

// Tunnel pattern studied from shane & shau
// i.e. https://www.shadertoy.com/view/4tKXzV
vec2 path(float t) {
	float a = sin(t*.2 + 1.5), b = sin(t*.2);
	return vec2(a*2., a*b);
}

// http://mercury.sexy/hg_sdf/
// hglib mirrorOctant
void mo(inout vec2 p, vec2 d) {
	p = abs(p) - d;
	if (p.y > p.x)p = p.yx;
}

// hglib pMod1
float re(float p, float d) {
	return mod(p - d * .5, d) - d * .5;
}

// hglib pModPolar
void amod(inout vec2 p, float d) {
	float a = re(atan(p.x, p.y), d);
	p = vec2(cos(a), sin(a)) * length(p);
}

// iq's signed cross sc() - http://iquilezles.org/www/articles/menger/menger.htm
float sc(vec3 p, float d) {
	p = abs(p);
	p = max(p, p.yzx);
	return min(p.x, min(p.y, p.z)) - d;
}

float g = 0.;
float de(vec3 p) {

	p.xy -= path(p.z);

	p.xy *= r2d(p.z*.05);

	vec3 q = p;

    // cylinder section
	amod(q.xy, 6.28 / 7.);
	mo(q.xy, vec2(1, .6));
	q.xy *= r2d(q.z*.5);
	mo(q.xy, vec2(.1, .2));
	amod(q.xy, 6.28 / 3.);
	float cyl2 = length(q.xy) - .05;

    p.z = re(p.z, .4);
    
    // cross structure section
	amod(p.xy, .785*2.);
	mo(p.xz, vec2(3, .01));
	amod(p.xy, .785*.5);
	p.x = abs(p.x) - 2.1;
	mo(p.xy, vec2(.4, 2));
	float d = sc(p, .02);
    
	g += .008 / (.01 + d * d);// glow trick from balkhan https://www.shadertoy.com/view/4t2yW1
	d = min(d, cyl2); // outside of the glow computation for cool effect & better contrast
    
	return d;
}

void main() {



	vec2 uv = (gl_FragCoord.xy - .5*RENDERSIZE.xy) / RENDERSIZE.y;
	float dt = TIME * 3.;
	vec3 ro = vec3(0, 0, -3. + dt);
	vec3 ta = vec3(0, 0, dt);
	ro.xy += path(ro.z);
	ta.xy += path(ta.z);
	vec3 fwd = normalize(ta - ro);
	vec3 right = normalize(cross(fwd, vec3(0, 1, 0)));
	vec3 up = normalize(cross(right, fwd));
	vec3 rd = normalize(fwd + right * uv.x + up * uv.y);
	vec3 p;
	float t = 0., ri;
	for (float i = 0.; i < 1.; i += .02) {
		ri = i;
		p = ro + rd * t;
		float d = de(p);
		d = max(abs(d), .002);// phantom mode trick from aiekick https://www.shadertoy.com/view/MtScWW
		t += d * .8;
	}
	vec3 c = mix(vec3(.5, .4, .3), vec3(.2, .1, .2), uv.x + ri);
	c += g * .034;// glow trick from balkhan https://www.shadertoy.com/view/4t2yW1
	gl_FragColor = vec4(c, 1);
}
