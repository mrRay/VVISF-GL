/*
{
    "CATEGORIES": [
        "Automatically Converted",
        "Shadertoy"
    ],
    "DESCRIPTION": "Automatically converted from https://www.shadertoy.com/view/XsGBWh by iridule.  rainbow?",
    "IMPORTED": {
    },
    "INPUTS": [
    ]
}

*/


#define map(x, a, b, c, d) c + ((v - a) / (b - a)) * (d - c)

#define TWO_PI 6.28318530718
#define AMOUNT 500.
#define STEP AMOUNT / 1e5

vec3 hsv2rgb(vec3 c) {
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

void main() {

    vec2 uv = (2. * gl_FragCoord.xy - RENDERSIZE.xy) / RENDERSIZE.y;
    vec3 color;
    float f = 0.;
    for (float i = 0.; i < AMOUNT; i++) {
        vec2 p = vec2(
            cos(2. * TIME + i * STEP * 2.) * .6,
        	sin(TIME + i * STEP * 2.) * .6);
        float ii = (i * i) / 500.;
        f += 1e-5 / abs(length(uv + p) - i / 5e6) * ii;
        color = hsv2rgb(vec3(clamp(f /8., 0., 1.), 1., clamp(f, 0., 1.)));
    }
    gl_FragColor = vec4(color, 1.);
}
