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




//	some forward declarations used in this header
class GLBuffer;
class GLBufferPool;
class GLTexToTexCopier;
class GLContext;
class GLScene;
struct GLCachedAttrib;
struct GLCachedUni;
class GLTexToCPUCopier;
class GLCPUToTexCopier;
struct Timestamp;
class GLContextWindowBacking;

//!	Very common- GLBufferRef is a shared pointer around a GLBuffer.
/*!
\relates VVGL::GLBuffer
This is the preferred means of working with GLBuffer- copying an instance of the GLBuffer class is potentially dangerous, as the underlying GL resource will not be duplicated.  Working with a std::shared_ptr ensures that the underlying class instances will be retained as long as it's in use (and when the class instance is deleted, its corresponding GL resource will either be deleted or pooled).
*/
using GLBufferRef = std::shared_ptr<GLBuffer>;
/*!
\brief A GLBufferPoolRef is a shared pointer around a GLBufferPool.
\relates VVGL::GLBufferPool
*/
using GLBufferPoolRef = std::shared_ptr<GLBufferPool>;
/*!
\brief A GLTexToTexCopierRef is a shared pointer around a GLTexToTexCopier.
\relates VVGL::GLTexToTexCopier
*/
using GLTexToTexCopierRef = std::shared_ptr<GLTexToTexCopier>;

/*!
\brief A GLContextRef is a shared pointer around a GLContext.
\relates VVGL::GLContext
*/
using GLContextRef = std::shared_ptr<GLContext>;
/*!
\brief A GLContextWeakRef is a weak pointer to a GLContext
\relates VVGL::GLContext
*/
using GLContextWeakRef = std::weak_ptr<GLContext>;

#if defined(VVGL_SDK_WIN)
/*!
\brief A GLContextWindowBackingRef is a shared pointer around a GLContextWindowBacking.
\relates VVGL::GLContext
*/
using GLContextWindowBackingRef = std::shared_ptr<GLContextWindowBacking>;
#endif	//	VVGL_SDK_WIN

/*!
\brief	A GLSceneRef is a shared pointer around a GLScene.
\relates VVGL::GLScene
*/
using GLSceneRef = std::shared_ptr<GLScene>;
/*!
\brief	A GLCachedAttribRef is a shared pointer around a GLCachedAttrib.
\relates VVGL::GLCachedAttrib
*/
using GLCachedAttribRef = std::shared_ptr<GLCachedAttrib>;
/*!
\brief	A GLCachedUniRef is a shared pointer around a GLCachedUni.
\relates VVGL::GLCachedUni
*/
using GLCachedUniRef = std::shared_ptr<GLCachedUni>;
/*!
\brief	A GLTexToCPUCopierRef is a shared pointer around a GLTexToCPUCopier.
\relates VVGL::GLTexToCPUCopier
*/
using GLTexToCPUCopierRef = std::shared_ptr<GLTexToCPUCopier>;
/*!
\brief	A GLCPUToTexCopierRef is a shared pointer around a GLCPUToTexCopier.
\relates VVGL::GLCPUToTexCopier
*/
using GLCPUToTexCopierRef = std::shared_ptr<GLCPUToTexCopier>;




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
inline const std::string GLVersionToString(const GLVersion & v)	{ switch (v) { case GLVersion_Unknown: return std::string("Unknown"); case GLVersion_2: return std::string("2"); case GLVersion_ES: return std::string("ES"); case GLVersion_ES2: return std::string("ES2"); case GLVersion_ES3: return std::string("ES3"); case GLVersion_33: return std::string("3.3"); case GLVersion_4: return std::string("4"); } return std::string("err"); }




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
	GLColor(const float & inR, const float & inG, const float & inB, const float & inA) { r=inR;g=inG;b=inB;a=inA; }
	bool operator==(const GLColor & n) const { return (this->r==n.r && this->g==n.g && this->b==n.b && this->a==n.a); }
	bool operator!=(const GLColor & n) const { return !(*this==n); }
};


//	this macro is a crude but effective way of logging GL errors on platforms that don't have dedicated GL debugging utilities
#ifdef GL_DEBUG
#define GLERRLOG { \
	int __ASDFQWERZXCV__=glGetError(); \
	if (__ASDFQWERZXCV__!=0){ \
		std::string		humanReadable("???"); \
		switch(__ASDFQWERZXCV__){ \
		case 0x0: humanReadable=std::string("no error"); break; \
		case 0x500: humanReadable=std::string("invalid enum"); break; \
		case 0x501: humanReadable=std::string("invalid value"); break; \
		case 0x502: humanReadable=std::string("invalid operation"); break; \
		case 0x503: humanReadable=std::string("stack overflow"); break; \
		case 0x504: humanReadable=std::string("stack underflow"); break; \
		case 0x505: humanReadable=std::string("out of memory"); break; \
		case 0x506: humanReadable=std::string("invalid framebuffer"); break; \
		case 0x507: humanReadable=std::string("context lost"); break; \
		case 0x8031: humanReadable=std::string("table too large"); break; \
		default: break; \
		} \
		std::cout<<"\tglGetError() returned "<<__ASDFQWERZXCV__<<" ("<<humanReadable<<") in "<<__LINE__<<" of "<<__FILE__<<endl; \
	} \
}
#else
#define GLERRLOG
#endif


//	this function dumps a 4cc to chars
inline void VVUnpackFourCC_toChar(unsigned long fourCC, char *destCharPtr) { if (destCharPtr==nullptr) return; for (int i=0; i<4; ++i) destCharPtr[i] = (char)((fourCC >> ((3-i)*8)) & 0xFF); }




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




#endif /* VVGL_Base_h */
