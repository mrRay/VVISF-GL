/*
{
    "CATEGORIES": [
        "Automatically Converted",
        "Shadertoy"
    ],
    "DESCRIPTION": "Automatically converted from https://www.shadertoy.com/view/4slGz4 by iq.  Volumetric lines, with a bit of (quickly made) synchronization to JosSs' track.",
    "IMPORTED": {
    },
    "INPUTS": [
        {
            "NAME": "iMouse",
            "TYPE": "point2D"
        },
        {
            "NAME": "iChannel0",
            "TYPE": "audio"
        }
    ]
}

*/


// Created by inigo quilez - iq/2013
// License Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License.


vec3 hash3( float n )
{
    return fract(sin(vec3(n,n+1.0,n+2.0))*vec3(43758.5453123,22578.1459123,19642.3490423));
}

vec3 snoise3( in float x )
{
    float p = floor(x);
    float f = fract(x);

    f = f*f*(3.0-2.0*f);

    return -1.0 + 2.0*mix( hash3(p+0.0), hash3(p+1.0), f );
}

float freqs[16];

vec3 distanceLines( in vec3 ro, in vec3 rd, vec3 pa, vec3 pb )
{
	vec3 ba = pb - pa;
	vec3 oa = ro - pa;
	
	float oad  = dot( oa, rd );
	float dba  = dot( rd, ba );
	float baba = dot( ba, ba );
	float oaba = dot( oa, ba );
	
	vec2 th = vec2( -oad*baba + dba*oaba, oaba - oad*dba ) / (baba - dba*dba);
	
	th.x = max(   th.x, 0.0 );
	th.y = clamp( th.y, 0.0, 1.0 );
	
	vec3 p = pa + ba*th.y;
	vec3 q = ro + rd*th.x;
	
	return vec3( length( p-q ), th );
}


vec3 castRay( vec3 ro, vec3 rd, float linesSpeed )
{
	vec3 col = vec3(0.0);
	
		
	float mindist = 10000.0;
	vec3 p = vec3(0.2);
	float h = 0.0;
	float rad = 0.04 + 0.15*freqs[0];
	float mint = 0.0;
    for( int i=0; i<128; i++ )
	{
		vec3 op = p;
		
		op = p;
		p  = 1.25*1.0*normalize(snoise3( 64.0*h + linesSpeed*0.015*TIME ));
		
		vec3 dis = distanceLines( ro, rd, op, p );
		
		vec3 lcol = 0.6 + 0.4*sin( 10.0*6.2831*h + vec3(0.0,0.6,0.9) );
		
		float m = pow( IMG_NORM_PIXEL(iChannel0,mod(vec2(h*0.5,0.25),1.0)).x, 2.0 )*(1.0+2.0*h);
		
		float f = 1.0 - 4.0*dis.z*(1.0-dis.z);
		float width = 1240.0 - 1000.0*f;
		width *= 0.25;
		float ff = 1.0*exp(-0.06*dis.y*dis.y*dis.y);
		ff *= m;
		col += 0.3*lcol*exp( -0.3*width*dis.x*dis.x )*ff;
		col += 0.5*lcol*exp( -8.0*width*dis.x*dis.x )*ff;
		h += 1.0/128.0;
	}


    return col;
}



void main() {



	vec2 q = gl_FragCoord.xy/RENDERSIZE.xy;
    vec2 p = -1.0+2.0*q;
	p.x *= RENDERSIZE.x/RENDERSIZE.y;
    vec2 mo = iMouse.xy/RENDERSIZE.xy;
		 
	float time = TIME;
	for( int i=0; i<16; i++ )
	    freqs[i] = clamp( 1.9*pow( IMG_NORM_PIXEL(iChannel0,mod(vec2( 0.05 + 0.5*float(i)/16.0, 0.25 ),1.0)).x, 3.0 ), 0.0, 1.0 );
	
	// camera	
	vec3 ta = vec3( 0.0, 0.0, 0.0 );
float isFast = smoothstep( 35.8, 35.81, TIME );
	isFast  -= smoothstep( 61.8, 61.81, TIME );
	isFast  += smoothstep( 78.0, 78.01, TIME );
	isFast  -= smoothstep(103.0,103.01, TIME );
	isFast  += smoothstep(140.0,140.01, TIME );
	isFast  -= smoothstep(204.0,204.01, TIME );
	
    float camSpeed = 1.0 + 40.0*isFast;	
	float beat = floor( max((TIME-35.7+0.4)/0.81,0.0) );
	time += beat*10.0*isFast;
	camSpeed *= mix( 1.0, sign(sin( beat*1.0 )), isFast );
	
float linesSpeed =  smoothstep( 22.7, 22.71, TIME );	
	  linesSpeed -= smoothstep( 61.8, 61.81, TIME );
	  linesSpeed += smoothstep( 78.0, 78.01, TIME );
	  linesSpeed -= smoothstep(140.0,140.01, TIME );
	
	ta  = 0.2*vec3( cos(0.1*time), 0.0*sin(0.1*time), sin(0.07*time) );
	vec3 ro = vec3( 1.0*cos(camSpeed*0.05*time+6.28*mo.x), 0.0, 1.0*sin(camSpeed*0.05*time+6.2831*mo.x) );
	float roll = 0.25*sin(camSpeed*0.01*time);
	
	// camera tx
	vec3 cw = normalize( ta-ro );
	vec3 cp = vec3( sin(roll), cos(roll),0.0 );
	vec3 cu = normalize( cross(cw,cp) );
	vec3 cv = normalize( cross(cu,cw) );
	vec3 rd = normalize( p.x*cu + p.y*cv + 1.2*cw );
	float curve  = smoothstep( 61.8, 71.0, TIME );
	      curve -= smoothstep(103.0,113.0, TIME );
    rd.xy += curve*0.025*vec2( sin(34.0*q.y), cos(34.0*q.x) );
	rd = normalize(rd);
	
	
	ro *= 1.0 - linesSpeed*0.5*freqs[1];
    vec3 col = castRay( ro, rd, 1.0 + 20.0*linesSpeed );
    col = col*col*2.4;
	
	// fade to black
    col *= 1.0 - smoothstep(218.0,228.00, TIME );
    col *=       smoothstep(  0.0,  4.00, TIME );
	if( TIME>61.8 && TIME<65.0 )
	col *= vec3(1.0)*clamp( (TIME-61.8)/(65.0-61.8), 0.0, 1.0 );
   
    col *= 0.15+0.85*pow( 16.0*q.x*q.y*(1.0-q.x)*(1.0-q.y), 0.15 );
    gl_FragColor=vec4( col, 1.0 );
}
