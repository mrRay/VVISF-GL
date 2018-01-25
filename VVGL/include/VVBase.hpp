#ifndef VVBase_h
#define VVBase_h

#include <vector>
#include <memory>

#include "VVTime.hpp"
#include "VVRange.hpp"
#include "VVStringUtils.hpp"


namespace VVGL
{


using namespace std;


//	some forward declarations used in this header
class VVGLBuffer;
class VVGLBufferPool;
class VVGLBufferCopier;
class VVGLContext;
class VVGLScene;
struct VVGLCachedAttrib;
struct VVGLCachedUni;


//	VERY COMMON.  VVGLBufferRef is a shared pointer to an VVGLBuffer.  this is how you should store and refer to VVGLBuffers- copying an instance of the VVGLBuffer class is potentially dangerous, as the underlying GL resources aren't duplicated.  working with a shared_ptr ensures that the underlying class instances will be retained as long as it's in use.  if you need to create another instance of a given VVGLBufferRef (copying the accompanying GL resource), you should use VVGLBufferCopy().
using VVGLBufferRef = shared_ptr<VVGLBuffer>;
//	VVGLBufferPoolRef is a shared pointer to an VVGLBufferPool, and is the preferred way to ensure that you don't create/destroy buffer pools unnecessarily.
using VVGLBufferPoolRef = shared_ptr<VVGLBufferPool>;
using VVGLBufferCopierRef = shared_ptr<VVGLBufferCopier>;
using VVGLContextRef = shared_ptr<VVGLContext>;
using VVGLSceneRef = shared_ptr<VVGLScene>;
using VVGLCachedAttribRef = shared_ptr<VVGLCachedAttrib>;
using VVGLCachedUniRef = shared_ptr<VVGLCachedUni>;


//	this enum is used to describe the GL environment of a GL context
enum GLVersion	{
	GLVersion_Unknown,
	GLVersion_2,
	GLVersion_ES2,
	GLVersion_ES3,
	GLVersion_33,
	GLVersion_4
};


//	this struct defines a GL color
struct GLColor	{
	float		r = 0.0;
	float		g = 0.0;
	float		b = 0.0;
	float		a = 0.0;
	GLColor() {}
	GLColor(const float & inR, const float & inG, const float & inB, const float & inA) { r=inR;g=inG;b=inB;a=inA; };
	bool operator==(const GLColor & n) const { return (this->r==n.r && this->g==n.g && this->b==n.b && this->a==n.a); };
	bool operator!=(const GLColor & n) const { return !(*this==n); };
};


//	this macro is a crude but effective way of logging GL errors on platforms that don't have dedicated GL debugging utilities
#ifdef GL_DEBUG
#define GLERRLOG {if(glGetError()!=0) { std::cout<<"\tglGetError() returned "<<glGetError()<<" in "<<__LINE__<<" of "<<__FILE__<<endl; }}
#else
#define GLERRLOG
#endif


//	this function dumps a 4cc to chars
inline void VVUnpackFourCC_toChar(unsigned long fourCC, char *destCharPtr) { if (destCharPtr==nullptr) return; for (int i=0; i<4; ++i) destCharPtr[i] = (fourCC >> ((3-i)*8)) & 0xFF; }


}




#if ISF_TARGET_QT
#include <QThread>
#include <QCoreApplication>
// 		this template establishes a function for asynchronously performing a lambda on the passed 
// 		object's thread (or the main thread if there's no passed object).
// 	
// 	usage:
// 		perform_async([]() { qDebug() << __PRETTY_FUNCTION__; });
// 	
// 		perform_async([&]{ o.setObjectName("hello"); }, &o);
// 		perform_async(std::bind(&QObject::setObjectName, &o, "hello"), &o);
template <typename FTYPE>
static void perform_async(FTYPE && inFunc, QObject * inObj=qApp)
{
	struct Event : public QEvent
	{
		using Fun = typename std::decay<FTYPE>::type;
		Fun			varFunc;
		Event(Fun && declInFunc) : QEvent(QEvent::None), varFunc(std::move(declInFunc)) {}
		Event(const Fun & declInFunc) : QEvent(QEvent::None), varFunc(declInFunc) {}
		~Event() { this->varFunc(); }
	};
	QCoreApplication::postEvent(inObj, new Event(std::forward<FTYPE>(inFunc)));
}
#endif




//	load some basic geometry structs and functions
#include "VVGeom.hpp"




#endif /* VVBase_h */
