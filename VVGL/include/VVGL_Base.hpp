#ifndef VVGL_Base_h
#define VVGL_Base_h

#include "VVGL_Defines.hpp"

#include <vector>
#include <memory>

#include "VVGL_Time.hpp"
#include "VVGL_Range.hpp"
#include "VVGL_StringUtils.hpp"


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


//	VERY COMMON.  GLBufferRef is a shared pointer to an GLBuffer.  this is how you should store and refer to GLBuffers- copying an instance of the GLBuffer class is potentially dangerous, as the underlying GL resources aren't duplicated.  working with a shared_ptr ensures that the underlying class instances will be retained as long as it's in use.  if you need to create another instance of a given GLBufferRef (copying the accompanying GL resource), you should use GLBufferCopy().
using GLBufferRef = shared_ptr<GLBuffer>;
using GLBufferPoolRef = shared_ptr<GLBufferPool>;
using GLBufferCopierRef = shared_ptr<GLBufferCopier>;
using GLContextRef = shared_ptr<GLContext>;
using GLSceneRef = shared_ptr<GLScene>;
using GLCachedAttribRef = shared_ptr<GLCachedAttrib>;
using GLCachedUniRef = shared_ptr<GLCachedUni>;


//	this enum is used to describe the GL environment of a GL context
enum GLVersion	{
	GLVersion_Unknown,
	GLVersion_2,
	GLVersion_ES,
	GLVersion_ES2,
	GLVersion_ES3,
	GLVersion_33,
	GLVersion_4
};
inline const string GLVersionToString(const GLVersion & v)	{ switch (v) { case GLVersion_Unknown: return string("Unknown"); case GLVersion_2: return string("2"); case GLVersion_ES: return string("ES"); case GLVersion_ES2: return string("ES2"); case GLVersion_ES3: return string("ES3"); case GLVersion_33: return string("3.3"); case GLVersion_4: return string("4"); default: return string("???"); } return string("err"); }


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




}	//	namespace VVGL




#if defined(ISF_SDK_QT)
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
