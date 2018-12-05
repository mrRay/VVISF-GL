/*{
	"CREDIT": "by mojovideotech",
  "IMPORTED" : [

  ],
  "CATEGORIES" : [
    "2d",
    "noise",
    "fire",
    "turbulence",
    "Automatically Converted"
  ],
  "DESCRIPTION" : "Automatically converted from https:\/\/www.shadertoy.com\/view\/Xt2GRh by Passion.  fire like effect",
  "INPUTS" : [
  	 	{
            "NAME": "offsetx",
            "TYPE": "float",
           "DEFAULT": 0.5,
            "MIN": -2.0,
            "MAX": 2.0
         },
          {
            "NAME": "offsety",
            "TYPE": "float",
           "DEFAULT": 0.5,
            "MIN": -2.0,
            "MAX": 2.0
         },
          {
			"NAME": "freq1",
			"TYPE": "float",
			"DEFAULT": 6.23,
			"MIN": -15,
			"MAX": 15
		 },
		  {
			"NAME": "freq2",
			"TYPE": "float",
			"DEFAULT": 0.46,
			"MIN": -12,
			"MAX": 12
		},
		 {
			"NAME": "shift",
			"TYPE": "float",
			"DEFAULT": 0.5,
			"MIN": -0.1,
			"MAX": 1.1
		},
		 {
			"NAME": "turbulent",
			"TYPE": "float",
			"DEFAULT": 27,
			"MIN": 1,
			"MAX": 30
		},
	  	 { 
			"NAME": "rate",
			"TYPE": "float",
			"DEFAULT": 1,
			"MIN": -2,
			"MAX": 2
		},
		 {
			"NAME": "randomize",
			"TYPE": "float",
			"DEFAULT": 1.0,
			"MIN": 0.1,
			"MAX": 3.0
		},
		 {
		 	"NAME": "contrast",
			"TYPE": "float",
			"DEFAULT": 0.5,
			"MIN": 0.1,
			"MAX": 1.5
		},
     	 {
			"NAME": "zoom",
			"TYPE": "float",
			"DEFAULT": 4.0,
			"MIN": 2.0,
			"MAX": 6.0
     	},
     	 {
			"NAME": "wobble",
			"TYPE": "bool",
	        "DEFAULT": "FALSE"
		}
  ]
}
*/

// RingsOfFire by mojovideotech
// based on :
// https://www.shadertoy.com/view/Xt2GRh

float t = TIME*rate;

vec2 hash( vec2 p ) 
{                       
    p = vec2( dot(p,vec2(159.7,676.5)),
              dot(p,vec2(258.4,418.1)) );
    return -1. + 2.*fract(sin(p+randomize)*53316.291173);
}  

float noise( in vec2 p ) 
{
    vec2 i = floor((p)), f = fract((p));
    vec2 u = f*f*(3.-2.*f);
    return mix( mix( dot( hash( i + vec2(0.,0.) ), f - vec2(0.,0.) ), 
                     dot( hash( i + vec2(1.,0.) ), f - vec2(1.,0.) ), u.x),
                mix( dot( hash( i + vec2(0.,1.) ), f - vec2(0.,1.) ), 
                     dot( hash( i + vec2(1.,1.) ), f - vec2(1.,1.) ), u.x), u.y);
}

float turbulence(vec3 p, int octaves, float lacunarity, float gain)
{
    float sum = 0.0;
    float amp = 1.0;
    for(int i=0; i<4; i++) 
    {
        sum += amp * abs(noise(p.xy));
        p *= lacunarity;
        amp *= gain;
    }
    return sum;
}

void main()
{
    vec2 pos = ( gl_FragCoord.xy / RENDERSIZE.xy ) -vec2(.5);    
	vec2 uv = pos * (6.0 - zoom) + vec2 (offsetx, offsety);
    uv.x *= RENDERSIZE.x/RENDERSIZE.y;
    if (wobble) uv += vec2(sin(t*.5)/15.+noise(vec2(t,t*.2))/5.,cos(t)/12.);
    float tb3= turbulence(vec3((uv*(15.))-t*5.,1.0),0,1.0-shift,0.0);
    uv=length((uv)-.5)-vec2(.3,.8+noise(vec2(uv.x,uv.y+tb3))/(31.-turbulent));
    float tb = turbulence(vec3((uv*(3.*freq1))-t,1.0),1,shift,contrast); 
    float tb2= turbulence(vec3((uv*(2.*freq2))-t*.5,1.0),2,1.0+shift,contrast);
	tb=tb-tb2;
	
	gl_FragColor = vec4(sqrt(vec3(uv+tb,0.0)),1.0);
}