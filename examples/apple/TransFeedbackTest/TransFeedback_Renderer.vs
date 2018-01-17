#version 330 core

in vec3		inXYZ;
in vec4		inRGBA;

uniform mat4	vvglOrthoProj;
out vec4	vsColorOut;

void main()	{
	//	we'll apply the orthogonal model projection in the geometry shader
	gl_Position = vec4(inXYZ, 1.) * vvglOrthoProj;
	vsColorOut = inRGBA;
}
