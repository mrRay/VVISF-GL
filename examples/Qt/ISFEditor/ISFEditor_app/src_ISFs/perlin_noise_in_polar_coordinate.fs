/*
{
    "CATEGORIES": [
        "Automatically Converted",
        "Shadertoy"
    ],
    "DESCRIPTION": "Automatically converted from https://www.shadertoy.com/view/ltGfRG by ukeyshima.  perlin noise in polar coordinate",
    "IMPORTED": {
    },
    "INPUTS": [
    ]
}

*/


#define PI 3.141592

float random1(vec3 p){
    return fract(sin(dot(p.xyz,vec3(12.9898,46.2346,78.233)))*43758.5453123)*2.0-1.0;
}
float random2(vec3 p){
    return fract(sin(dot(p.xyz,vec3(73.6134,21.6712,51.5781)))*51941.3781931)*2.0-1.0;
}
float random3(vec3 p){
    return fract(sin(dot(p.xyz,vec3(39.1831,85.3813,16.2981)))*39183.4971731)*2.0-1.0;
}
float perlinNoise(vec3 p){
    vec3 i1=floor(p);    
    vec3 i2=i1+vec3(1.0,0.0,0.0);
    vec3 i3=i1+vec3(0.0,1.0,0.0);
    vec3 i4=i1+vec3(1.0,1.0,0.0);
    vec3 i5=i1+vec3(0.0,0.0,1.0);
    vec3 i6=i1+vec3(1.0,0.0,1.0);
    vec3 i7=i1+vec3(0.0,1.0,1.0);
    vec3 i8=i1+vec3(1.0,1.0,1.0);
    vec3 f1=vec3(random1(i1),random2(i1),random3(i1));
    vec3 f2=vec3(random1(i2),random2(i2),random3(i2));
    vec3 f3=vec3(random1(i3),random2(i3),random3(i3));
    vec3 f4=vec3(random1(i4),random2(i4),random3(i4));
    vec3 f5=vec3(random1(i5),random2(i5),random3(i5));
    vec3 f6=vec3(random1(i6),random2(i6),random3(i6));
    vec3 f7=vec3(random1(i7),random2(i7),random3(i7));
    vec3 f8=vec3(random1(i8),random2(i8),random3(i8));
    vec3 k1=p-i1;
    vec3 k2=p-i2;
    vec3 k3=p-i3;
    vec3 k4=p-i4;
    vec3 k5=p-i5;
    vec3 k6=p-i6;
    vec3 k7=p-i7;
    vec3 k8=p-i8;
    vec3 j=fract(p);
    j=j*j*(3.0-2.0*j);
    return (((dot(f1,k1)*(1.0-j.x)+dot(f2,k2)*j.x)*(1.0-j.y)+(dot(f3,k3)*(1.0-j.x)+dot(f4,k4)*j.x)*j.y)*(1.0-j.z)+((dot(f5,k5)*(1.0-j.x)+dot(f6,k6)*j.x)*(1.0-j.y)+(dot(f7,k7)*(1.0-j.x)+dot(f8,k8)*j.x)*j.y)*j.z)*0.95+0.05;
}

float octavePerlinNoise(vec3 p){
float value=0.0;
float maxValue=0.0;
for(float i=0.0;i<10.0;i++){
    value+=pow(0.5,i)*perlinNoise(vec3(p.x*pow(2.0,i),p.y*pow(2.0,i),p.z*pow(2.0,i)));
    maxValue+=pow(0.5,i);
}
return value/maxValue;
}

vec3 polarCoordinate(vec3 p){
    float r=length(p);
    float theta1=atan(p.x,p.z);
    float theta2=atan(p.y,p.z);
    return vec3(r,theta1,theta2);
}

void main() {

    vec2 p = (gl_FragCoord.xy*2.0 - RENDERSIZE.xy) / min(RENDERSIZE.x,RENDERSIZE.y);   
    p*=10.0; 
    vec3 c=polarCoordinate(vec3(p,TIME));
    vec3 color=vec3(octavePerlinNoise(c))*vec3(p,1.0);
    gl_FragColor=vec4(color,1.0);
}
