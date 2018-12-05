/*
{
    "CATEGORIES": [
        "Automatically Converted",
        "Shadertoy"
    ],
    "DESCRIPTION": "Automatically converted from https://www.shadertoy.com/view/MsGBDh by iridule.  Metamorphosis ",
    "IMPORTED": {
    },
    "INPUTS": [
    ]
}

*/


#define TWO_PI 6.28318530718
#define rotate(a) mat2(cos(a), sin(a), -sin(a), cos(a))
#define spiral(u, a, r, t, d) abs(sin(t + r * length(u) + a * (d * atan(u.y, u.x))))
#define flower(u, a, r, t) spiral(u, a, r, t, 1.) * spiral(u, a, r, t, -1.)
#define sinp(a) .5 + sin(a) * .5
#define polar(a, t) a * vec2(cos(t), sin(t))


void main() {

	
    vec2 st = (2. * gl_FragCoord.xy - RENDERSIZE.xy) / RENDERSIZE.y;
 	st = rotate(-TIME / 10.) * st;
	
	vec3 col;
    vec2 v;
    
    float t = TIME;
    float f;
    
    for (int i = 0; i < 3; i++) {
        for (float i = 0.; i < TWO_PI; i += TWO_PI / 16.) {
            t += .6 * 
                
                flower(vec2(st + polar(1., i)), 8., 8. * 
                             sinp(TIME / 5.), TIME / 10.);
                    
        }
		col[i] = sin(5. * t + length(st) * 8. * sinp(t));
	}
	
	gl_FragColor = vec4(mix(col, vec3(1., .7, 0.), col.r), 1.0);
    
}
