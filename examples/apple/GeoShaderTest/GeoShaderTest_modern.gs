#version 330 core

layout (triangles) in;
layout (triangle_strip, max_vertices=6) out;

in vec4		vsColorOut[3];
out vec4	gsColorOut;

uniform mat4	vvglOrthoProj;

void main()	{
	//	draw the original position
	for (int i=0; i<3; ++i)	{
		gl_Position = gl_in[i].gl_Position * vvglOrthoProj;
		gsColorOut = vsColorOut[i];
		EmitVertex();
	}
	EndPrimitive();
	
	//	draw another triangle shifted 50 pixels up and to the right
	for (int i=0; i<3; ++i)	{
		vec4		tmpCoord = vec4(gl_in[i].gl_Position.x, gl_in[i].gl_Position.y, gl_in[i].gl_Position.z, 1.);
		tmpCoord += vec4(50., 50., 0., 0.);
		//	this is where we apply the orthogonal modelview projection
		gl_Position = tmpCoord * vvglOrthoProj;
		gsColorOut = vsColorOut[i];
		EmitVertex();
	}
	EndPrimitive();
}
