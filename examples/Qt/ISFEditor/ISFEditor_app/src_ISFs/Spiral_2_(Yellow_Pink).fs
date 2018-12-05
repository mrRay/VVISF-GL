/*
{
    "CATEGORIES": [
        "Automatically Converted",
        "Shadertoy"
    ],
    "DESCRIPTION": "Automatically converted from https://www.shadertoy.com/view/XtsBDf by lsdlive.  Twisted menger cube.\n\nShort menger formula borrowed from aiekick/coyote.",
    "IMPORTED": {
    },
    "INPUTS": [
    ]
}

*/


mat2 r2d(float a) {
	float c = cos(a), s = sin(a);
	return mat2(c, s, -s, c);
}

float de(vec3 p) {
	p.y += cos(TIME*2.) * .2;

	p.xy *= r2d(TIME + p.z);

	vec3 r;
	float d = 0., s = 1.6;

	for (int i = 0; i < 3; i++)
		r = max(r = abs(mod(p*s + 1., 2.) - 1.), r.yzx),
		d = max(d, (.8 - min(r.x, min(r.y, r.z))) / s),
		s = sqrt(s) + 7.;

	return d;
}

void main() {



	vec2 uv = gl_FragCoord.xy / RENDERSIZE.xy - .5;
	uv.x *= RENDERSIZE.x / RENDERSIZE.y;
	vec3 ro = vec3(.1*cos(TIME), 0, -TIME), p;
	vec3 rd = normalize(vec3(uv, -1));
	p = ro;
	float it = 0.;
	for (float i=0.; i < 1.; i += .01) {
        it = i;
		float d = de(p);
		if (d < .001) break;
		p += rd * d*.5;
	}
	it /= .9 * sqrt(abs(tan(TIME*1.3) + p.x*p.x + p.y*p.y));
	vec3 c = mix(vec3(1., 1., 0), vec3(.9, .2, .6), it*pow(sin(p.z), 1./10.));
	gl_FragColor = vec4(c, 1.0);
}
