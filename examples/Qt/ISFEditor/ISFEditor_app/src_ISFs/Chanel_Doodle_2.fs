/*
{
    "CATEGORIES": [
        "Automatically Converted",
        "Shadertoy"
    ],
    "DESCRIPTION": "Automatically converted from https://www.shadertoy.com/view/Mt3fWN by Flopine.  15 minutes improvisation training for a performance at the Chanel Foundation gala",
    "IMPORTED": {
        "iChannel0": {
            "NAME": "iChannel0",
            "PATH": "fb918796edc3d2221218db0811e240e72e340350008338b0c07a52bd353666a6.jpg"
        }
    },
    "INPUTS": [
    ]
}

*/


// Code by Flopine

// Thanks to wsmind, leon, XT95, lsdlive, lamogui and Coyhot for teaching me
// Thanks LJ for giving me the love of shadercoding :3

// Cookie Collective rulz

#define ITER 64.
#define PI 3.141592
#define time TIME
#define BPM 50.
#define tempo BPM/60.

vec3 palette (float t, vec3 a, vec3 b, vec3 c, vec3 d)
{return a+b*cos(2.*PI*(c*t+d));}

vec2 moda (vec2 p, float per)
{
    float a = atan(p.y, p.x);
    float l = length(p);
    a = mod(a-per/2., per)-per/2.;
    return vec2(cos(a),sin(a))*l;
}

vec2 mo(vec2 p, vec2 d)
{
    p = abs(p)-d;
    if (p.y > p.x) p.xy = p.yx;
    return p;
}

float stmin(float a, float b, float k, float n)
{
    float st = k/n;
    float u = b-k;
    return min(min(a,b), 0.5 * (u+a+abs(mod(u-a+st,2.*st)-st)));
}

float smin( float a, float b, float k )
{
    float h = max( k-abs(a-b), 0.0 );
    return min( a, b ) - h*h*0.25/k;
}

mat2 rot(float a)
{return mat2(cos(a),sin(a),-sin(a),cos(a));}

float pulse (float s)
{return exp(-fract(time * tempo) * s);}

float tiktak(float period)
{
    float tik = floor(time*tempo)+pow(fract(time*tempo),3.);
    tik *= 3.*period;
    return tik;
}

float sphe (vec3 p, float r)
{return length(p)-r;}

float od (vec3 p, float d)
{
    p.xy *= rot(time*0.2);
    p.xz *= rot(time*0.5);
    return dot(p, normalize(sign(p)))-d;
}

float cyl (vec2 p, float r)
{return length(p)-r;}

float box( vec3 p, vec3 b )
{
  vec3 d = abs(p) - b;
  return min(max(d.x,max(d.y,d.z)),0.0) + length(max(d,0.0));
}

float prim1(vec3 p)
{
    p.xy *= rot(p.z*0.2);
    p.xy = moda(p.xy, 2.*PI/3.);

    p.x -= 2.;
    p.xz = moda(p.xz, 2.*PI/5.);
    p.x -= 0.8;
    return cyl(p.xz, 0.1);
}

float prim2 (vec3 p)
{
    float per = 3.;
    p.z = mod(p.z-per/2., per)-per/2.;
    return prim1(p);
}

float prim3 (vec3 p)
{
    float pr = prim2(p);
    p.xy *= rot(p.z);
    p.xy = moda(p.xy, 2.*PI/5.);
    p.x -= .7;
    return smin(pr,cyl (p.xy, 0.1), 1.);
}

float g = 0.;
float SDF(vec3 p)
{
  float d = min(prim3(p),max(-od(vec3(p.x,p.y,p.z-time), 0.8),sphe(vec3(p.x,p.y,p.z-time),1.)));
  g+=0.1/(0.1+d*d); 
  return d;
}


void main() {



    vec2 uv = 2.*(gl_FragCoord.xy/RENDERSIZE.xy)-1.;
    uv /= vec2(RENDERSIZE.y / RENDERSIZE.x, 1);
    uv += IMG_NORM_PIXEL(iChannel0,mod(uv+vec2(time*0.2,time*0.1),1.0)).r*0.06;
    vec3 ro = vec3(0.001,0.001,-4. + time); vec3 p = ro;
    vec3 rd = normalize(vec3(uv,1.));
    float shad = 0.;
    for (float i=0.; i<ITER; i++)
    {
        float d = SDF(p);
        if (d<0.001)
        {
            shad = i/ITER;
            break;
        }
        p += d*rd*0.8;
    }
    float t = length(ro-p);
    vec3 pal = palette
    (length(uv),
    vec3(0.5),
    vec3(0.5),
    vec3(0.5),
    vec3(0.,0.3,0.4));
    vec3 c = vec3(1.-shad)*(1.-pal);
    c = mix(c, pal, 1.-exp(-0.001*t*t));
    c+= g*0.016*length(uv)*2.;
    gl_FragColor = vec4(pow(c, vec3(0.45)),1.);
}
