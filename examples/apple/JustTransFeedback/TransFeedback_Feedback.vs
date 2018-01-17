#version 330 core

in vec3		inXYZ;
in vec4		inRGBA;

out vec3 outXYZ;
out vec4 outRGBA;

void main()	{
	outXYZ = inXYZ + vec3(0.345);
	outRGBA = inRGBA + vec4(0.123);
}
