/*
{
    "CATEGORIES": [
        "Automatically Converted",
        "Shadertoy"
    ],
    "DESCRIPTION": "Automatically converted from https://www.shadertoy.com/view/4lSGz3 by anastadunbar.  Generated music.",
    "IMPORTED": {
    },
    "INPUTS": [
    ],
    "PASSES": [
        {
        },
        {
        }
    ]
}

*/


float rand(vec2 co){
    return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

vec2 mainSound(float time)
{
    float beat = mod(time,0.5);
    //Kick
    float kick = sin(beat*(500.*(1.-(beat*1.4))));
    //Synth
    float randomnote = rand(vec2(258.,floor(mod(time*2.,8.))));
    float synth = sin(time*(500.+(randomnote*1000.)))/pow(mod(time,0.5)+1.,10.);
    synth += sin(time*(1700.+(randomnote*1000.)))/pow(mod(time,0.25)+1.,10.);
    synth += (sin(time*(3780.+(randomnote*1500.)))/pow(mod(time,0.125)+1.,20.))/2.;
    synth = synth*(beat*2.);
    //Hihat
    float hihat = rand(vec2(time,time))/pow(mod(time,0.125)+1.,20.);
    return vec2( (synth*0.4)+(kick*0.5)+(hihat*0.2) );
}
vec3 hsl2rgb( in vec3 c )
{
    vec3 rgb = clamp( abs(mod(c.x*6.0+vec3(0.0,4.0,2.0),6.0)-3.0)-1.0, 0.0, 1.0 );

    return c.z + c.y * (rgb-0.5)*(1.0-abs(2.0*c.z-1.0));
}

//https://www.shadertoy.com/view/MsjXRt
vec3 HueShift (in vec3 Color, in float Shift)
{
    vec3 P = vec3(0.55735)*dot(vec3(0.55735),Color);
    
    vec3 U = Color-P;
    
    vec3 V = cross(vec3(0.55735),U);    

    Color = U*cos(Shift*6.2832) + V*sin(Shift*6.2832) + P;
    
    return vec3(Color);
}


vec2 rotate(in float angle,in vec2 position)
{
    //Function seen from https://www.shadertoy.com/view/XlsGWf
    float rot = radians(angle);
    mat2 rotation = mat2(cos(rot), -sin(rot), sin(rot), cos(rot));
    return vec2(position*rotation);
}

vec2 scale(vec2 pos, float size)
{
 	return vec2((pos-0.5)*size);   
}

float clamps(float val)
{
    return clamp(val,0.,1.);
}

void main() {
	if (PASSINDEX == 0)	{
	}
	else if (PASSINDEX == 1)	{


	    float time = TIME;
	    
	    float beat = 1.-(mod(time,0.5));
		vec2 uv = (((gl_FragCoord.xy / RENDERSIZE.xy)-vec2(0.,0.5))*vec2(1.,0.58))+vec2(0.,0.5);
	    vec2 muv = fract(scale(rotate(time*10.,uv),20.));
	    float a = clamps(1.-((length(muv-0.5)-(beat/3.))*70.));
	    float b = clamps(sin(((uv.x+uv.y)*40.)+(time*5.))*20.);
	    //https://www.shadertoy.com/view/llX3zX
	    float c = smoothstep(float(mod(float(int(muv.x*2.02) + int(muv.y*2.02)), 2.0) > 0.0),float(mod(float(int(muv.x*2.) + int(muv.y*2.)), 2.0) > 0.0),0.5);
	    //float c = clamps((sin(uv.x*80.+time)*cos(uv.y*80.+time))*80.);
	    vec3 colors = HueShift(vec3(b,c,a),(uv.x+(time/20.)))+0.1;
	    colors = clamp(colors,0.,1.);
	    float d = (length(uv-0.5)-0.2)*2.;
	    gl_FragColor = vec4(colors-d,1.0);
	}
}
