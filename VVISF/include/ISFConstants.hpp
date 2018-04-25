#ifndef ISFConstants_h
#define ISFConstants_h

#if defined(ISF_SDK_QT)
#include "vvisf_qt_global.h"
#endif




namespace VVISF
{


//	string constants used by the ISFScene to compiled the ISFs
static const std::string		ISF_ES_Compatibility = std::string("\
	\n\
precision highp float;	\n\
precision highp int;	\n\
	\n\
");
static const std::string		ISFGLMacro2D_GL2 = std::string("\
vec4 VVSAMPLER_2DBYPIXEL(sampler2D sampler, vec4 samplerImgRect, vec2 samplerImgSize, bool samplerFlip, vec2 loc)	{	\n\
	return (samplerFlip)	\n\
		? texture2D		(sampler,vec2(((loc.x/samplerImgSize.x*samplerImgRect.z)+samplerImgRect.x), (samplerImgRect.w-(loc.y/samplerImgSize.y*samplerImgRect.w)+samplerImgRect.y)))	\n\
		: texture2D		(sampler,vec2(((loc.x/samplerImgSize.x*samplerImgRect.z)+samplerImgRect.x), ((loc.y/samplerImgSize.y*samplerImgRect.w)+samplerImgRect.y)));	\n\
}	\n\
vec4 VVSAMPLER_2DBYNORM(sampler2D sampler, vec4 samplerImgRect, vec2 samplerImgSize, bool samplerFlip, vec2 normLoc)	{	\n\
	vec4		returnMe = VVSAMPLER_2DBYPIXEL(		sampler,samplerImgRect,samplerImgSize,samplerFlip,vec2(normLoc.x*samplerImgSize.x, normLoc.y*samplerImgSize.y));	\n\
	return returnMe;	\n\
}	\n\
");
static const std::string		ISFGLMacro2DBias_GL2 = std::string("\
vec4 VVSAMPLER_2DBYPIXEL(sampler2D sampler, vec4 samplerImgRect, vec2 samplerImgSize, bool samplerFlip, vec2 loc, float bias)	{	\n\
	return (samplerFlip)	\n\
		? texture2D		(sampler,vec2(((loc.x/samplerImgSize.x*samplerImgRect.z)+samplerImgRect.x), (samplerImgRect.w-(loc.y/samplerImgSize.y*samplerImgRect.w)+samplerImgRect.y)), bias)	\n\
		: texture2D		(sampler,vec2(((loc.x/samplerImgSize.x*samplerImgRect.z)+samplerImgRect.x), ((loc.y/samplerImgSize.y*samplerImgRect.w)+samplerImgRect.y)), bias);	\n\
}	\n\
vec4 VVSAMPLER_2DBYNORM(sampler2D sampler, vec4 samplerImgRect, vec2 samplerImgSize, bool samplerFlip, vec2 normLoc, float bias)	{	\n\
	vec4		returnMe = VVSAMPLER_2DBYPIXEL(		sampler,samplerImgRect,samplerImgSize,samplerFlip,vec2(normLoc.x*samplerImgSize.x, normLoc.y*samplerImgSize.y), bias);	\n\
	return returnMe;	\n\
}	\n\
");
static const std::string		ISFGLMacro2DRect_GL2 = std::string("	\n\
vec4 VVSAMPLER_2DRECTBYPIXEL(sampler2DRect sampler, vec4 samplerImgRect, vec2 samplerImgSize, bool samplerFlip, vec2 loc)	{	\n\
	return (samplerFlip)	\n\
		? texture2DRect	(sampler,vec2(((loc.x/samplerImgSize.x*samplerImgRect.z)+samplerImgRect.x), (samplerImgRect.w-(loc.y/samplerImgSize.y*samplerImgRect.w)+samplerImgRect.y)))	\n\
		: texture2DRect	(sampler,vec2(((loc.x/samplerImgSize.x*samplerImgRect.z)+samplerImgRect.x), ((loc.y/samplerImgSize.y*samplerImgRect.w)+samplerImgRect.y)));	\n\
}	\n\
vec4 VVSAMPLER_2DRECTBYNORM(sampler2DRect sampler, vec4 samplerImgRect, vec2 samplerImgSize, bool samplerFlip, vec2 normLoc)	{	\n\
	vec4		returnMe = VVSAMPLER_2DRECTBYPIXEL(	sampler,samplerImgRect,samplerImgSize,samplerFlip,vec2(normLoc.x*samplerImgRect.z, normLoc.y*samplerImgRect.w));	\n\
	return returnMe;	\n\
}	\n\
");
static const std::string		ISFGLMacro2DRectBias_GL2 = std::string("	\n\
vec4 VVSAMPLER_2DRECTBYPIXEL(sampler2DRect sampler, vec4 samplerImgRect, vec2 samplerImgSize, bool samplerFlip, vec2 loc, float bias)	{	\n\
	return (samplerFlip)	\n\
		? texture2DRect	(sampler,vec2(((loc.x/samplerImgSize.x*samplerImgRect.z)+samplerImgRect.x), (samplerImgRect.w-(loc.y/samplerImgSize.y*samplerImgRect.w)+samplerImgRect.y)))	\n\
		: texture2DRect	(sampler,vec2(((loc.x/samplerImgSize.x*samplerImgRect.z)+samplerImgRect.x), ((loc.y/samplerImgSize.y*samplerImgRect.w)+samplerImgRect.y)));	\n\
}	\n\
vec4 VVSAMPLER_2DRECTBYNORM(sampler2DRect sampler, vec4 samplerImgRect, vec2 samplerImgSize, bool samplerFlip, vec2 normLoc, float bias)	{	\n\
	vec4		returnMe = VVSAMPLER_2DRECTBYPIXEL(	sampler,samplerImgRect,samplerImgSize,samplerFlip,vec2(normLoc.x*samplerImgRect.z, normLoc.y*samplerImgRect.w), bias);	\n\
	return returnMe;	\n\
}	\n\
");




static const std::string		ISFGLMacro2D_GL3 = std::string("\
vec4 VVSAMPLER_2DBYPIXEL(sampler2D sampler, vec4 samplerImgRect, vec2 samplerImgSize, bool samplerFlip, vec2 loc)	{	\n\
	return (samplerFlip)	\n\
		? texture		(sampler,vec2(((loc.x/samplerImgSize.x*samplerImgRect.z)+samplerImgRect.x), (samplerImgRect.w-(loc.y/samplerImgSize.y*samplerImgRect.w)+samplerImgRect.y)))	\n\
		: texture		(sampler,vec2(((loc.x/samplerImgSize.x*samplerImgRect.z)+samplerImgRect.x), ((loc.y/samplerImgSize.y*samplerImgRect.w)+samplerImgRect.y)));	\n\
}	\n\
vec4 VVSAMPLER_2DBYNORM(sampler2D sampler, vec4 samplerImgRect, vec2 samplerImgSize, bool samplerFlip, vec2 normLoc)	{	\n\
	vec4		returnMe = VVSAMPLER_2DBYPIXEL(		sampler,samplerImgRect,samplerImgSize,samplerFlip,vec2(normLoc.x*samplerImgSize.x, normLoc.y*samplerImgSize.y));	\n\
	return returnMe;	\n\
}	\n\
");
static const std::string		ISFGLMacro2DBias_GL3 = std::string("\
vec4 VVSAMPLER_2DBYPIXEL(sampler2D sampler, vec4 samplerImgRect, vec2 samplerImgSize, bool samplerFlip, vec2 loc, float bias)	{	\n\
	return (samplerFlip)	\n\
		? texture		(sampler,vec2(((loc.x/samplerImgSize.x*samplerImgRect.z)+samplerImgRect.x), (samplerImgRect.w-(loc.y/samplerImgSize.y*samplerImgRect.w)+samplerImgRect.y)), bias)	\n\
		: texture		(sampler,vec2(((loc.x/samplerImgSize.x*samplerImgRect.z)+samplerImgRect.x), ((loc.y/samplerImgSize.y*samplerImgRect.w)+samplerImgRect.y)), bias);	\n\
}	\n\
vec4 VVSAMPLER_2DBYNORM(sampler2D sampler, vec4 samplerImgRect, vec2 samplerImgSize, bool samplerFlip, vec2 normLoc, float bias)	{	\n\
	vec4		returnMe = VVSAMPLER_2DBYPIXEL(		sampler,samplerImgRect,samplerImgSize,samplerFlip,vec2(normLoc.x*samplerImgSize.x, normLoc.y*samplerImgSize.y), bias);	\n\
	return returnMe;	\n\
}	\n\
");
static const std::string		ISFGLMacro2DRect_GL3 = std::string("	\n\
vec4 VVSAMPLER_2DRECTBYPIXEL(sampler2DRect sampler, vec4 samplerImgRect, vec2 samplerImgSize, bool samplerFlip, vec2 loc)	{	\n\
	return (samplerFlip)	\n\
		? texture		(sampler,vec2(((loc.x/samplerImgSize.x*samplerImgRect.z)+samplerImgRect.x), (samplerImgRect.w-(loc.y/samplerImgSize.y*samplerImgRect.w)+samplerImgRect.y)))	\n\
		: texture		(sampler,vec2(((loc.x/samplerImgSize.x*samplerImgRect.z)+samplerImgRect.x), ((loc.y/samplerImgSize.y*samplerImgRect.w)+samplerImgRect.y)));	\n\
}	\n\
vec4 VVSAMPLER_2DRECTBYNORM(sampler2DRect sampler, vec4 samplerImgRect, vec2 samplerImgSize, bool samplerFlip, vec2 normLoc)	{	\n\
	vec4		returnMe = VVSAMPLER_2DRECTBYPIXEL(	sampler,samplerImgRect,samplerImgSize,samplerFlip,vec2(normLoc.x*samplerImgRect.z, normLoc.y*samplerImgRect.w));	\n\
	return returnMe;	\n\
}	\n\
");
static const std::string		ISFGLMacro2DRectBias_GL3 = std::string("	\n\
vec4 VVSAMPLER_2DRECTBYPIXEL(sampler2DRect sampler, vec4 samplerImgRect, vec2 samplerImgSize, bool samplerFlip, vec2 loc, float bias)	{	\n\
	return (samplerFlip)	\n\
		? texture		(sampler,vec2(((loc.x/samplerImgSize.x*samplerImgRect.z)+samplerImgRect.x), (samplerImgRect.w-(loc.y/samplerImgSize.y*samplerImgRect.w)+samplerImgRect.y)))	\n\
		: texture		(sampler,vec2(((loc.x/samplerImgSize.x*samplerImgRect.z)+samplerImgRect.x), ((loc.y/samplerImgSize.y*samplerImgRect.w)+samplerImgRect.y)));	\n\
}	\n\
vec4 VVSAMPLER_2DRECTBYNORM(sampler2DRect sampler, vec4 samplerImgRect, vec2 samplerImgSize, bool samplerFlip, vec2 normLoc, float bias)	{	\n\
	vec4		returnMe = VVSAMPLER_2DRECTBYPIXEL(	sampler,samplerImgRect,samplerImgSize,samplerFlip,vec2(normLoc.x*samplerImgRect.z, normLoc.y*samplerImgRect.w), bias);	\n\
	return returnMe;	\n\
}	\n\
");




static const std::string		ISFVertPassthru_GL2 = std::string("	\n\
	\n\
void main(void)	{	\n\
	isf_vertShaderInit();	\n\
}	\n\
	\n\
");
static const std::string		ISFVertInitFunc = std::string("	\n\
	\n\
	//	gl_Position should be equal to gl_ProjectionMatrix * gl_ModelViewMatrix * gl_Vertex	\n\
	mat4			projectionMatrix = mat4(2./RENDERSIZE.x, 0., 0., -1.,	\n\
		0., 2./RENDERSIZE.y, 0., -1.,	\n\
		0., 0., -1., 0.,	\n\
		0., 0., 0., 1.);	\n\
	gl_Position = VERTEXDATA * projectionMatrix;	\n\
	isf_FragNormCoord = vec2((gl_Position.x+1.0)/2.0, (gl_Position.y+1.0)/2.0);	\n\
	vec2	isf_fragCoord = floor(isf_FragNormCoord * RENDERSIZE);	\n\
	\n\
");
static const std::string		ISFVertVarDec_GLES2 = std::string("	\n\
	\n\
attribute vec4		VERTEXDATA;	\n\
void isf_vertShaderInit();	\n\
	\n\
");
static const std::string		ISFVertVarDec_GL3 = std::string("	\n\
	\n\
in vec4		VERTEXDATA;	\n\
void isf_vertShaderInit();	\n\
	\n\
");


}


#endif /* ISFConstants_h */
