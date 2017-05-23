#ifndef VVBase_h
#define VVBase_h

#include <vector>
#include <memory>
#include "VVGeom.hpp"

#include "VVTime.hpp"
#include "VVRange.hpp"
#include "VVStringUtils.hpp"
#include "VVGLContext.hpp"


namespace VVGL
{


using namespace std;




//	some forward declarations used in this header
class VVGLBuffer;
class VVGLBufferPool;
class VVGLBufferCopier;
//class VVGLContext;




//	VERY COMMON.  VVGLBufferRef is a shared pointer to an VVGLBuffer.  this is how you should store and refer to VVGLBuffers- copying an instance of the VVGLBuffer class is potentially dangerous, as the underlying GL resources aren't duplicated.  working with a shared_ptr ensures that the underlying class instances will be retained as long as it's in use.  if you need to create another instance of a given VVGLBufferRef (copying the accompanying GL resource), you should use VVGLBufferCopy().
using VVGLBufferRef = shared_ptr<VVGLBuffer>;
//	VVGLBufferPoolRef is a shared pointer to an VVGLBufferPoolRef, and is the preferred way to ensure that you don't create/destroy buffer pools unnecessarily.
using VVGLBufferPoolRef = shared_ptr<VVGLBufferPool>;
//	VVGLBufferCopierRef is a shared pointer to an VVGLBufferCopierRef
using VVGLBufferCopierRef = shared_ptr<VVGLBufferCopier>;
//using VVGLContextRef = shared_ptr<VVGLContext>;



//	these structs simplify the process of constructing, referring to, and packing/unpacking GL-based data describing a textured quad
struct GLBufferVertex {
	float		geo[2];
	float		tex[2];
};
struct GLBufferQuad {
	GLBufferVertex	bl;
	GLBufferVertex	br;    
	GLBufferVertex	tl;
	GLBufferVertex	tr;    
};
//	populates the passed VVBufferQuad struct with the passed geometry and texture coords
inline void GLBufferQuadPopulate(GLBufferQuad *b, Rect geoRect, Rect texRect);
//	this struct defines a GL color
struct GLColor	{
	float		r = 0.0;
	float		g = 0.0;
	float		b = 0.0;
	float		a = 0.0;
	GLColor(const float & inR, const float & inG, const float & inB, const float & inA) { r=inR;g=inG;b=inB;a=inA; };
	bool operator==(const GLColor & n) const { return (this->r==n.r && this->g==n.g && this->b==n.b && this->a==n.a); };
	bool operator!=(const GLColor & n) const { return !(*this==n); };
};




#ifdef GL_DEBUG
#define GLERRLOG {if(glGetError()!=0) { std::cout<<"\tglGetError() returned "<<glGetError()<<" in "<<__LINE__<<" of "<<__FILE__<<endl; }}
#else
#define GLERRLOG
#endif




}









#endif /* VVBase_h */
