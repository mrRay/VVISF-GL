/*
{
    "CATEGORIES": [
        "Automatically Converted",
        "Shadertoy"
    ],
    "DESCRIPTION": "Automatically converted from https://www.shadertoy.com/view/lsdBzX by KilledByAPixel.  Anther experiment with something like a Vogel spiral. Can produce some cool spiral patterns and mess with your vision after a while. Really interesting to watch how it continues to develop over time.\n\nmouse.x = skip ahead\nmouse.y = zoom",
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


//////////////////////////////////////////////////////////////////////////////////
// Spiral of Spirals - Copyright 2018 Frank Force
// License Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License.
//////////////////////////////////////////////////////////////////////////////////

const float pi = 3.14159265359;

vec3 hsv2rgb(vec3 c)
{
    float s = c.y * c.z;
    float s_n = c.z - s * .5;
    return vec3(s_n) + vec3(s) * cos(2.0 * pi * (c.x + vec3(1.0, 0.6666, .3333)));
}

void main() {



	vec2 uv = gl_FragCoord.xy;
    uv -= RENDERSIZE.xy / 2.0;
    uv /= RENDERSIZE.x;
   
    vec4 mousePos = (iMouse.xyxy / RENDERSIZE.xyxy);
 	uv *= 100.0;
    if (mousePos.y > 0.0)
    	uv *= 4.0 * mousePos.y;
    
    float a = atan(uv.y, uv.x);
    float d = length(uv);
    
    // make spiral
    float i = d;
    float p = a/(2.0*pi) + 0.5;
    i -= p;
    a += 2.0*pi*floor(i);
    
    // change over time
    float t = .05*(TIME +  400.0*mousePos.x);
    //t = pow(t, 0.4);
    
    float h = 0.5*a;
    h *= t;
    //h *= 0.1*(floor(i)+p);
    h = 0.5*(sin(h) + 1.0);
    h = pow(h, 3.0);
    h += 4.222*t + 0.4;
    
    float s = 2.0*a;
    s *= t;
    s = 0.5*(sin(s) + 1.0);
    s = pow(s, 2.0);
    
    //float h = d*.01 + t*1.33;
    //float s = sin(d*.1 + t*43.11);
    //s = 0.5*(s + 1.0);
    
    // fixed size
    a *= (floor(i)+p);
    
    // apply color
    float v = a;
    v *= t;
    v = sin(v);
    v = 0.5*(v + 1.0);
    v = pow(v, 4.0);
    v *= pow(sin(fract(i)*pi), 0.4); // darken edges
    v *= min(d, 1.0); // dot in center
    
    //vec3 c = vec3(h, s, v);
    vec3 c = vec3(h, s, v);
	gl_FragColor = vec4(hsv2rgb(c), 1.0);
}
