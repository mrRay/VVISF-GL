#ifndef VVGL_Base_h
#define VVGL_Base_h

#include "VVGL_Defines.hpp"

#include <vector>
#include <memory>

#include "VVGL_Time.hpp"
#include "VVGL_Range.hpp"
#include "VVGL_StringUtils.hpp"

/*!
\file 
*/




namespace VVGL
{


using namespace std;


//	some forward declarations used in this header
class GLBuffer;
class GLBufferPool;
class GLBufferCopier;
class GLContext;
class GLScene;
struct GLCachedAttrib;
struct GLCachedUni;
class GLTexToCPUCopier;
class GLCPUToTexCopier;

//!	Very common- GLBufferRef is a shared pointer around a GLBuffer.
/*!
\relates VVGL::GLBuffer
This is the preferred means of working with GLBuffer- copying an instance of the GLBuffer class is potentially dangerous, as the underlying GL resource will not be duplicated.  Working with a shared_ptr ensures that the underlying class instances will be retained as long as it's in use (and when the class instance is deleted, its corresponding GL resource will either be deleted or pooled).
*/
using GLBufferRef = shared_ptr<GLBuffer>;
/*!
\brief A GLBufferPoolRef is a shared pointer around a GLBufferPool.
\relates VVGL::GLBufferPool
*/
using GLBufferPoolRef = shared_ptr<GLBufferPool>;
/*!
\brief A GLBufferCopierRef is a shared pointer around a GLBufferCopier.
\relates VVGL::GLBufferCopier
*/
using GLBufferCopierRef = shared_ptr<GLBufferCopier>;

/*!
\brief A GLContextRef is a shared pointer around a GLContext.
\relates VVGL::GLContext
*/
using GLContextRef = shared_ptr<GLContext>;
/*!
\brief	A GLSceneRef is a shared pointer around a GLScene.
\relates VVGL::GLScene
*/
using GLSceneRef = shared_ptr<GLScene>;
/*!
\brief	A GLCachedAttribRef is a shared pointer around a GLCachedAttrib.
\relates VVGL::GLCachedAttrib
*/
using GLCachedAttribRef = shared_ptr<GLCachedAttrib>;
/*!
\brief	A GLCachedUniRef is a shared pointer around a GLCachedUni.
\relates VVGL::GLCachedUni
*/
using GLCachedUniRef = shared_ptr<GLCachedUni>;
/*!
\brief	A GLTexToCPUCopierRef is a shared pointer around a GLTexToCPUCopier.
\relates VVGL::GLTexToCPUCopier
*/
using GLTexToCPUCopierRef = shared_ptr<GLTexToCPUCopier>;
/*!
\brief	A GLCPUToTexCopierRef is a shared pointer around a GLCPUToTexCopier.
\relates VVGL::GLCPUToTexCopier
*/
using GLCPUToTexCopierRef = shared_ptr<GLCPUToTexCopier>;




/*!
\ingroup VVGL_BASIC
\brief	This enum is used to describe the GL environment of a GL context, which is determined at runtime.
*/
enum GLVersion	{
	GLVersion_Unknown,
	GLVersion_2,
	GLVersion_ES,
	GLVersion_ES2,
	GLVersion_ES3,
	GLVersion_33,
	GLVersion_4
};
/*!
\ingroup VVGL_BASIC
\brief Returns a std::string describing the passed GLVersion.
*/
inline const string GLVersionToString(const GLVersion & v)	{ switch (v) { case GLVersion_Unknown: return string("Unknown"); case GLVersion_2: return string("2"); case GLVersion_ES: return string("ES"); case GLVersion_ES2: return string("ES2"); case GLVersion_ES3: return string("ES3"); case GLVersion_33: return string("3.3"); case GLVersion_4: return string("4"); default: return string("???"); } return string("err"); }




/*!
\ingroup VVGL_BASIC
\brief This struct describes an RGBA color.
*/
struct GLColor	{
	//!	The red component.
	float		r = 0.0;
	//!	The green component.
	float		g = 0.0;
	//!	The blue component.
	float		b = 0.0;
	//!	The alpha component.
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




}	//	namespace VVGL




#if defined(VVGL_SDK_QT)
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
#include "VVGL_Geom.hpp"




#endif /* VVGL_Base_h */
