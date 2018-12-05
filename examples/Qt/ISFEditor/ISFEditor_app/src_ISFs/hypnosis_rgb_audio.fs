/*
{
    "CATEGORIES": [
        "Automatically Converted",
        "Shadertoy"
    ],
    "DESCRIPTION": "Automatically converted from https://www.shadertoy.com/view/XtdfW2 by ribenno.  outlands 3 wetgenes audio workshop",
    "IMPORTED": {
    },
    "INPUTS": [
        {
            "NAME": "iChannel0",
            "TYPE": "audio"
        }
    ]
}

*/


void main() {



    
    float fft1=IMG_NORM_PIXEL(iChannel0,mod(vec2(0.0/11025.0,0.8),1.0)).r;    
    float fft2=IMG_NORM_PIXEL(iChannel0,mod(vec2(0.0/11025.0,0.25),1.0)).r;   
    float fft3=IMG_NORM_PIXEL(iChannel0,mod(vec2(0.0/11025.0,0.5),1.0)).r;   
    
    // Normalized pixel coordinates (from 0 to 1)
    vec2 uv = gl_FragCoord.xy/RENDERSIZE.xx*12.0;
    // Time varying pixel color
    vec3 col = 0.5+0.5*sin(TIME+uv.x+vec3(0,0.7,4.0));
    col *= 0.5+0.5*cos(-TIME+uv.yyy+vec3(0,2.0,0.2));
	col *= 5.0-abs(tan(-TIME*fft3+uv.xxy+vec3(0.4,1.0,0.5)));
    
    // Output to screen
    gl_FragColor = vec4(col,1.0);
}
