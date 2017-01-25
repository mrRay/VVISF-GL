#ifndef ISFConstants_h
#define ISFConstants_h


namespace VVISFKit
{


//	string constants used by the ISFScene to compiled the ISFs
static const std::string		ISF_ES_Compatibility = std::string("\
	\n\
#ifdef GL_ES	\n\
precision highp float;	\n\
precision highp int;	\n\
#endif	\n\
	\n\
");
static const std::string		ISFGLMacro2D = std::string("\
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
static const std::string		ISFGLMacro2DBias = std::string("\
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
static const std::string		ISFGLMacro2DRect = std::string("	\n\
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
static const std::string		ISFGLMacro2DRectBias = std::string("	\n\
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
static const std::string		ISFVertPassthru = std::string("	\n\
	\n\
void main(void)	{	\n\
	isf_vertShaderInit();	\n\
}	\n\
	\n\
");
static const std::string		ISFVertInitFunc = std::string("	\n\
	\n\
#ifdef GL_ES	\n\
	//	gl_Position should be equal to gl_ProjectionMatrix * gl_ModelViewMatrix * gl_Vertex	\n\
	gl_Position = a_position;	\n\
	mat4			projectionMatrix = mat4(2./RENDERSIZE.x, 0., 0., -1.,	\n\
		0., 2./RENDERSIZE.y, 0., -1.,	\n\
		0., 0., -1., 0.,	\n\
		0., 0., 0., 1.);	\n\
	gl_Position *= projectionMatrix;	\n\
	//vv_VertNorm = 	\n\
	//vv_VertPos = 	\n\
#else	\n\
	gl_Position = ftransform();	\n\
	//vv_VertNorm = gl_Normal.xyz;	\n\
	//vv_VertPos = gl_Vertex.xyz;	\n\
#endif	\n\
	//vv_FragNormCoord = vec2((gl_Position.x+1.0)/2.0, (gl_Position.y+1.0)/2.0);	\n\
	isf_FragNormCoord = vec2((gl_Position.x+1.0)/2.0, (gl_Position.y+1.0)/2.0);	\n\
	//vec2	vv_fragCoord = floor(vv_FragNormCoord * RENDERSIZE);	\n\
	vec2	isf_fragCoord = floor(isf_FragNormCoord * RENDERSIZE);	\n\
	\n\
");
static const std::string		ISFVertVarDec = std::string("	\n\
	\n\
#ifdef GL_ES	\n\
attribute vec4		a_position;	\n\
#endif	\n\
void isf_vertShaderInit();	\n\
	\n\
");


}


#endif /* ISFConstants_h */
