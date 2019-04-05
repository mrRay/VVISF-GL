/*{
	"CREDIT": "by mojovideotech",
	"DESCRIPTION": "",
	"CATEGORIES": [
		"XXX"
	],
	"INPUTS": [
	 	{
      		"NAME" : "center",
      		"TYPE" : "point2D",
      		"MAX" : [
        		2,
        		2
      		],
      		"MIN" : [
        		-2,
         		-2
      		]
    	},
 		{
            "NAME": "mult",
            "TYPE": "float",
           "DEFAULT": 2.75,
            "MIN": 0.1,
            "MAX": 5.0
          },
           {
            "NAME": "mod1",
            "TYPE": "float",
           "DEFAULT": 4.5,
            "MIN": 2.0,
            "MAX": 10.0
          },
          {
            "NAME": "mod2",
            "TYPE": "float",
           "DEFAULT": 4.75,
            "MIN": 2.0,
            "MAX": 10.0
          },
           {
            "NAME": "phase1",
            "TYPE": "float",
           "DEFAULT": 0.25,
            "MIN": 0.01,
            "MAX": 0.99
          },
          {
            "NAME": "phase2",
            "TYPE": "float",
           "DEFAULT": -0.5,
            "MIN": -1.5,
            "MAX": 1.5
          },
          {
            "NAME": "phase3",
            "TYPE": "float",
           "DEFAULT": 48.0,
            "MIN": 0.1,
            "MAX": 60.0
          },
          {
          	 "NAME": "mash",
            "TYPE": "float",
           "DEFAULT": 3.25,
            "MIN": 0.1,
            "MAX": 5.0
          },
           {
            "NAME": "freq",
            "TYPE": "float",
           "DEFAULT": 8.0,
            "MIN": 2.0,
            "MAX": 10.0
          },
          {
            "NAME": "push",
            "TYPE": "float",
           "DEFAULT": -6.75,
            "MIN": -10.0,
            "MAX": 0.0
          },
           {
            "NAME": "pull",
            "TYPE": "float",
           "DEFAULT": 3.25,
            "MIN": 0.0,
            "MAX": 4.5
          },
          {
            "NAME": "tint",
            "TYPE": "float",
           "DEFAULT": -0.15,
            "MIN": -0.5,
            "MAX": 0.5
          },
           {
            "NAME": "rate",
            "TYPE": "float",
           "DEFAULT": -0.01,
            "MIN": -0.1,
            "MAX": 0.1
          }
	]
}*/

// CellMod by mojovideotech
// Remix of http://glslsandbox.com/e#32728.2
// Based on https://www.shadertoy.com/view/Xd2GR3
// Created by inigo quilez - iq/2014
// License Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License.

#ifdef GL_ES
precision mediump float;
#endif

vec4 hexagon( vec2 p ) 
{
	vec2 q = vec2( p.x*2.0*phase1, p.y + p.x*phase2 );
	vec2 pi = floor(q);
	vec2 pf = fract(q);
	float v = mod(pi.x + pi.y, freq);
	float ca = step(mod(2.0*mod1,4.0*(mod1*mod2)),v);
	float cb = step(mod(7.0*(mod2*mod1),5.0*(mod2)),v);
	vec2  ma = step(pf.xy,pf.yx);
    
	return vec4(pi + ca - cb * ma, 0.0, 0.0 );
}

float hash1( vec2  p ) {return fract( p.x  * mash + p.y ); }

void main( void ) 
{
	float TT = TIME*rate;
	vec2 pos = ((-RENDERSIZE.xy + 2.0*gl_FragCoord.xy)/RENDERSIZE.y)+center;
	pos *= 1.0 + push*length(pos);
	
    	vec4 h = hexagon((5.0-pull)*pos + 0.0125*TT);

	float d = hash1(h.xy * mod(mult,fract(sin(TT))*(mult-0.0001)) + phase3);
	gl_FragColor = vec4( vec3(0.67 * (1.0-d) + tint,0.67 * d,0.67 * (d+1.0)), 1.0 );
}