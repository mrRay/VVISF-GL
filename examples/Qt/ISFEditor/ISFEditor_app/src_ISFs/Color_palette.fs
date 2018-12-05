/*
{
    "CATEGORIES": [
        "Automatically Converted",
        "Shadertoy"
    ],
    "DESCRIPTION": "Automatically converted from https://www.shadertoy.com/view/4stXDX by anastadunbar.  Colour palette test. Sorry for more zooming shaders, they're just really trippy.",
    "IMPORTED": {
        "iChannel0": {
            "NAME": "iChannel0",
            "PATH": "3083c722c0c738cad0f468383167a0d246f91af2bfa373e9c5c094fb8c8413e0.png"
        }
    },
    "INPUTS": [
    ],
    "PASSES": [
        {
            "FLOAT": true,
            "PERSISTENT": true,
            "TARGET": "BufferA"
        },
        {
        }
    ]
}

*/


#define clamps(x) clamp(x,0.,1.)
#define PI 3.14159265358979323
vec2 rotation(float an,vec2 uv) {
    mat2 rot = mat2(cos(an),-sin(an),sin(an),cos(an));
    return uv*rot;
}
vec3 HueShift(vec3 Color,float Shift)
{
    vec3 P = vec3(0.55735)*dot(vec3(0.55735),Color);
    vec3 U = Color-P;
    vec3 V = cross(vec3(0.55735),U);    
    Color = U*cos(Shift*6.2832) + V*sin(Shift*6.2832) + P;
    return vec3(Color);
}


const mat3 rgb2yiq = mat3( 0.299, 0.595716, 0.211456, 0.587, -0.274453, -0.522591, 0.114, -0.321263, 0.311135 );
const mat3 yiq2rgb = mat3( 1., 1., 1., 0.9563, -0.2721, -1.1070, 0.6210, -0.6474, 1.7046 );
float diff(float a,float b){
    return abs(a-b);
}
float colorDifference(vec3 c1,vec3 c2){
    //return (diff(c1.r,c2.r)+diff(c1.g,c2.g)+diff(c1.b,c2.b))/3.;
    c1 *= rgb2yiq; c2 *= rgb2yiq*.5;
    return sqrt(pow(diff(c1.r,c2.r),2.)+pow(diff(c1.g,c2.g),2.)+pow(diff(c1.b,c2.b),2.))/3.;
}
vec3 draw(vec2 uv){
    return IMG_NORM_PIXEL(BufferA,mod(uv,1.0)).rgb;
}
const int size = 10;
vec3 palette[size];
void main() {
	if (PASSINDEX == 0)	{
	    vec2 position = (gl_FragCoord.xy/RENDERSIZE.xy);
	    vec2 uv = position-.5;
	    uv.x /= RENDERSIZE.y/RENDERSIZE.x;
	    float time = TIME*.3;
	    
	    float fracttime = fract(time);
	    float scale = 5.;
	    
	    vec2 rotuv = rotation(mod(time,PI*2.),uv);
	    
	    vec2 uv2 = rotuv*pow(scale,fracttime);
	    vec2 uv3 = rotuv*pow(scale,fracttime-1.);
	    
	    vec3 o = mix(IMG_NORM_PIXEL(iChannel0,mod(uv2,1.0)).rgb,IMG_NORM_PIXEL(iChannel0,mod(uv3,1.0)).rgb,fracttime);
	    o = HueShift(o,time*.5);
	    o = mix(mix(o,vec3((o.r+o.g+o.b)/3.),-1.5),IMG_NORM_PIXEL(BufferA,mod(position,1.0)).rgb,0.8);
	
	    gl_FragColor = vec4(o,1.);
	}
	else if (PASSINDEX == 1)	{
	    palette[0] = vec3(0.,0.,0.2);
	    palette[1] = vec3(1.,0.8,0.5);
	    palette[2] = vec3(1.,0.1,0.4);
	    palette[3] = vec3(0.9,1.,0.);
	    palette[4] = vec3(0.,1.,0.3);
	    palette[5] = vec3(0.,.6,1.);
	    palette[6] = vec3(0.,0.,1.);
	    palette[7] = vec3(1.,0.,1.);
	    palette[8] = vec3(.5,.5,.55);
	    palette[9] = vec3(.9,.9,1.);
	    vec2 uv = gl_FragCoord.xy / RENDERSIZE.xy;
	    vec3 or = clamp(draw(uv),0.,1.)-(mod(floor(gl_FragCoord.x)+floor(gl_FragCoord.y),2.)*.2); //Original texture
	    vec3 ch = palette[1]; //New
	    int indx = 0; //Palette index
	    for (int i=0;i<size;i++) {
	        if (colorDifference(or,palette[i])<colorDifference(or,ch)) {
	            ch = palette[i];
	        }
	    }
	    gl_FragColor = vec4(ch,1.0);
	}
}
