#version 120


uniform mat4		vvglOrthoProj;


void main(void)	{	
	//	we'll apply the orthogonal model projection in the geometry shader
	//gl_Position = gl_Vertex;
	//gl_Position = ftransform();
	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
	//gl_Position = vvglOrthoProj * tmpVec;
	
	gl_FrontColor = gl_Color;
}	
