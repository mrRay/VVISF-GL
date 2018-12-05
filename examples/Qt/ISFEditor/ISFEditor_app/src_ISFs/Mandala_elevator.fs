/*
{
    "CATEGORIES": [
        "Automatically Converted",
        "Shadertoy"
    ],
    "DESCRIPTION": "Automatically converted from https://www.shadertoy.com/view/XtSBRK by Flopine.  Training training training !",
    "IMPORTED": {
    },
    "INPUTS": [
    ]
}

*/


// Code by Flopine
// Thanks to wsmind and leon for teaching me!

#define PI 3.14

mat2 rot(float angle)
{
    float c = cos(angle);
    float s = sin(angle);
    return mat2(c,s,-s,c);
}

vec2 moda (vec2 p, float per)
{
    float angle = atan(p.y,p.x);
    float l = length(p);
    angle = mod(angle-per/2.,per)-per/2.;
    return vec2 (cos(angle),sin(angle))*l;
}

float cylY (vec3 p, float r)
{
    return length(p.xz)-r;
}

float cylZ (vec3 p, float r)
{
    return length(p.xy)-r;
}


float base (vec3 p)
{   
    p.xy *= rot(abs(p.z)-TIME);
    p.xy = moda(p.xy,2.*PI/3.);
    p.x -= 0.4;

    return cylZ(p,0.2);
}


float prim(vec3 p)
{
    p.xz = moda(p.xz, 2.*PI/7.);
    p.x -= 4.;
    return base(p);
}

float elevator (vec3 p)
{
    float per = 8.;
    p.yz = mod (p.yz-per/2.,per)-per/2.;
    float rep1 = prim(p);
    
    float per2 = 15.;
    p.yz *= rot(PI/2.);
     p.x = mod (p.x-per2/2.,per2)-per2/2.;
    float rep2 = prim(p);
    
    return min(rep1,rep2);
}

float background (vec3 p)
{	
    float per = 5.;
    p.yz *= rot(PI/2.);
    p.x = mod (p.x-per/2.,per)-per/2.;
    return prim(p);
}

float SDF(vec3 p)
{
    return min(elevator(p), background(p));
}



void main() {



	vec2 uv = 2.*(gl_FragCoord.xy / RENDERSIZE.xy)-1.;
    uv.x *= RENDERSIZE.x/RENDERSIZE.y;
    
    vec3 p = vec3 (-.001,TIME,-3.);
    vec3 dir = normalize(vec3(uv,1.));
    
    float shad = 0.;
    vec3 color = vec3 (0.);
    
    for (int i=0; i<100;i++)
    {
        float d = elevator(p);
        if (d<0.01)
        {
            shad = float(i)/80.;
           break;
        }
        else shad = .0;
        p+=d*0.2*dir;
    }
    color = vec3(shad)*vec3(0.8,p.z,abs(p.x*0.5));
	gl_FragColor = vec4(color,1.0);
}
