/*
{
    "CATEGORIES": [
        "Automatically Converted",
        "Shadertoy"
    ],
    "DESCRIPTION": "Automatically converted from https://www.shadertoy.com/view/MtySz1 by hhhzzzsss.  This is a combination of two orbit traps on a mandelbrot set. One is based on a line, and the other is based on a point. The line and the point is constantly changing, resulting in the animation shown.",
    "IMPORTED": {
    },
    "INPUTS": [
    ]
}

*/


#define iterations 200

vec3 hsv2rgb(vec3 c)
{
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

float calctrap(vec2 z) {
    vec2 center = 0.3*vec2(sin(0.82342*TIME+2.254), sin(0.41423*TIME+331.234));
    vec2 direction = normalize(vec2(sin(0.794875*TIME+3.2534), sin(1.0985734*TIME+5.253)));
	float stalk1 = abs(dot( z - center, direction ));
    float stalk2 = abs(dot( z - center, vec2(direction.y,-direction.x) ));
    float stalk = min(stalk1, stalk2);
    
    center = 0.3*vec2(sin(1.7509*TIME+23.423), sin(2.41423*TIME+623.724));
    float point = length(z-center)/3.0;
	
    return min(point,stalk);
}

void main() {



	vec2 uv = gl_FragCoord.xy / RENDERSIZE.xy;
    uv = 2.0*uv-1.0;
    uv.x *= RENDERSIZE.x/RENDERSIZE.y;
    uv *= 1.3;
    
    vec2 z = uv;
    vec2 c = uv;
    float trap = calctrap(z);
    for (int i = 0; i < iterations; ++i) {
        //if (dot(z,z) > 4.0) break;
        z = vec2(z.x*z.x - z.y*z.y, 2.0*z.x*z.y) + c;
        trap = min(trap, calctrap(z));
    }
	gl_FragColor = vec4(hsv2rgb(vec3(1.0/(1.0+trap*50.0) - 0.3,1.0,1.0)),1.0);
    
}
