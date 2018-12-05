/*
{
    "CATEGORIES": [
        "Automatically Converted",
        "Shadertoy"
    ],
    "DESCRIPTION": "Automatically converted from https://www.shadertoy.com/view/lslyD2 by hhhzzzsss.  This shows the electric fields resulting from charged particles. The brightness represents intensity, and the colors indicate direction. The particles move around to make a cool show.",
    "IMPORTED": {
    },
    "INPUTS": [
    ]
}

*/


#define TAU 6.2831853071

vec2 uv;

vec3 hsv2rgb(vec3 c) {
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

void updateField(inout vec2 f, vec3 p) {
    vec2 dispvec = uv-p.xy;
    vec2 dir = normalize(dispvec);
    float len = length(dispvec);
    float mag = p.z/(len*len);
    f += dir*mag;
}

void main() {



	uv = gl_FragCoord.xy / RENDERSIZE.xy;
    uv = uv * 2.0 - 1.0;
    uv.x *= RENDERSIZE.x / RENDERSIZE.y;
    
    vec2 field = vec2(0.0);
    
    vec3 p1 = vec3(sin(TIME*4.78904365), cos(TIME*5.142093847), -0.2);
    vec3 p2 = vec3(sin(TIME*3.32252), cos(TIME*2.70329481), 0.2);
    vec3 p3 = vec3(sin(TIME*1.23947), cos(TIME*0.6598234), -0.25);
    vec3 p4 = vec3(sin(TIME*0.72783957), cos(TIME*1.2034242), 0.25);
    vec3 p5 = vec3(sin(TIME*3.234058), cos(TIME*3.7583429), 0.1);
    vec3 p6 = vec3(sin(TIME*2.82094583), cos(TIME*3.40534298), 0.1);
    updateField(field, p1);
    updateField(field, p2);
    updateField(field, p3);
    updateField(field, p4);
    updateField(field, p5);
    updateField(field, p6);
    
    float direction = atan(field.y, field.x)/TAU;
    float intensity = 1.0-1.0/(1.0+length(field));
    
    vec3 col = hsv2rgb( vec3(direction, 1.5+0.5*sin(TIME)-intensity, intensity) );
    col = pow(col, vec3(0.45));
    
	gl_FragColor = vec4(col,1.0);
}
