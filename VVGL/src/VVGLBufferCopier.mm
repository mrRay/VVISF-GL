#include "VVGLBufferCopier.hpp"

#if ISF_TARGET_IOS
	#import <GLKit/GLKit.h>
#endif




using namespace std;




namespace VVGL
{




#if ISF_TARGET_IOS
void VVGLBufferCopier::_drawBuffer(const VVGLBufferRef & inBufferRef, const Rect & inGLSrcRect, const Rect & inDstRect)	{
	//	populate my built-in projection effect with the texture data i want to draw
	GLKEffectPropertyTexture		*effectTex = [(GLKBaseEffect *)projMatrixEffect texture2d0];
	[effectTex setEnabled:YES];
	[effectTex setName:inBufferRef->name];
	//	tell the built-in projection effect to prepare to draw
	[(GLKBaseEffect *)projMatrixEffect prepareToDraw];
	
	bool			flipped = inBufferRef->flipped;
	GLfloat			geoCoords[] = {
		(GLfloat)MinX(inDstRect), (GLfloat)MinY(inDstRect),
		(GLfloat)MaxX(inDstRect), (GLfloat)MinY(inDstRect),
		(GLfloat)MinX(inDstRect), (GLfloat)MaxY(inDstRect),
		(GLfloat)MaxX(inDstRect), (GLfloat)MaxY(inDstRect)
	};
	GLfloat			texCoords[] = {
		(GLfloat)MinX(inGLSrcRect), (flipped) ? (GLfloat)MaxY(inGLSrcRect) : (GLfloat)MinY(inGLSrcRect),
		(GLfloat)MaxX(inGLSrcRect), (flipped) ? (GLfloat)MaxY(inGLSrcRect) : (GLfloat)MinY(inGLSrcRect),
		(GLfloat)MinX(inGLSrcRect), (flipped) ? (GLfloat)MinY(inGLSrcRect) : (GLfloat)MaxY(inGLSrcRect),
		(GLfloat)MaxX(inGLSrcRect), (flipped) ? (GLfloat)MinY(inGLSrcRect) : (GLfloat)MaxY(inGLSrcRect)
	};
	//	if i don't have a VBO containing geometry for a quad, make one now
	if (geoXYVBO == nullptr)	{
		geoXYVBO = CreateVBO(geoCoords, 8*sizeof(GLfloat), GL_DYNAMIC_DRAW);
	}
	if (geoSTVBO == nullptr)	{
		geoSTVBO = CreateVBO(texCoords, 8*sizeof(GLfloat), GL_DYNAMIC_DRAW);
	}
	glBindBuffer(GL_ARRAY_BUFFER, geoXYVBO->name);
	GLERRLOG
	glBufferData(GL_ARRAY_BUFFER, 8*(sizeof(GLfloat)), geoCoords, GL_DYNAMIC_DRAW);
	GLERRLOG
	glEnableVertexAttribArray(VertexAttribPosition);
	GLERRLOG
	glVertexAttribPointer(VertexAttribPosition, 2, GL_FLOAT, GL_FALSE, 0, NULL);
	GLERRLOG
	
	glBindBuffer(GL_ARRAY_BUFFER, geoSTVBO->name);
	GLERRLOG
	glBufferData(GL_ARRAY_BUFFER, 8*(sizeof(GLfloat)), texCoords, GL_DYNAMIC_DRAW);
	GLERRLOG
	glEnableVertexAttribArray(VertexAttribTexCoord0);
	GLERRLOG
	glVertexAttribPointer(VertexAttribTexCoord0, 2, GL_FLOAT, GL_FALSE, 0, NULL);
	GLERRLOG
	
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	GLERRLOG
	//	draw
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	GLERRLOG
	
	glDisableVertexAttribArray(VertexAttribPosition);
	GLERRLOG
	glDisableVertexAttribArray(VertexAttribTexCoord0);
	GLERRLOG
}
#endif




}
