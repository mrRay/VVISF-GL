#ifndef VVGL_Defines_h
#define VVGL_Defines_h




/*
developers need to define one of these during compilation:

	VVGL_SDK_MAC
	VVGL_SDK_IOS
	VVGL_SDK_RPI
	VVGL_SDK_GLFW
	VVGL_SDK_QT
	VVGL_SDK_WIN

this header file defines a series of macros that describe the availability of various GL 
environments based on the SDK as defined by the user.  these are the possible values- note that 
these are not mutually exclusive- more than one may be defined simultaneously, these are only used 
to block off function calls that won't compile/link against a given SDK's headers/libs.

	VVGL_TARGETENV_GL2
	VVGL_TARGETENV_GL3PLUS
	VVGL_TARGETENV_GLES
	VVGL_TARGETENV_GLES3
*/




//	if you don't define the SDK as a compiler flag, you can define it here, in this file.
//	this approach may be easier in the long run for other devs, as software that links against 
//	the lib will pick up this definition when they include this header.
#include "VVGL_HardCodedDefines.hpp"




//	throw an error with a human-readable explanation if no SDK has been defined yet
#if !defined(VVGL_SDK_MAC) && !defined(VVGL_SDK_IOS) && !defined(VVGL_SDK_RPI) && !defined (VVGL_SDK_GLFW) && !defined(VVGL_SDK_QT) && !defined(VVGL_SDK_WIN)
static_assert(false, "ERR: No SDK defined (eg. VVGL_SDK_XXXX), see VVGL_Defines.hpp for more information.");
#endif




#if defined(VVGL_SDK_MAC)
	#define VVGL_TARGETENV_GL2
	#define VVGL_TARGETENV_GL3PLUS
#elif defined(VVGL_SDK_IOS)
	#define VVGL_TARGETENV_GLES3
#elif defined(VVGL_SDK_RPI)
	#define VVGL_TARGETENV_GLES
#elif defined(VVGL_SDK_GLFW)
	#define VVGL_TARGETENV_GL2
	#define VVGL_TARGETENV_GL3PLUS
#elif defined(VVGL_SDK_QT)
	#define VVGL_TARGETENV_GL2
	#define VVGL_TARGETENV_GL3PLUS
#elif defined(VVGL_SDK_WIN)
	#define VVGL_TARGETENV_GL2
	#define VVGL_TARGETENV_GL3PLUS
#endif




#if defined(VVGL_SDK_QT)
#include "VVGL_Qt_global.h"
#elif defined(VVGL_SDK_WIN)
#include "VVGL_Win_global.hpp"
#else
#define VVGL_EXPORT 
#endif




#if (defined(VVGL_SDK_QT) && defined (_WIN32)) || defined(VVGL_SDK_WIN)
//#define __PRETTY_FUNCTION__ __func__
#define __PRETTY_FUNCTION__ __FUNCSIG__
#pragma warning (disable : 4068)	/*	disables unknown pragma warnings	*/
#endif	//	(VVGL_SDK_QT && _WIN32) || VVGL_SDK_WIN




#if defined(_WIN32)
#define __PRETTY_FUNCTION__ __FUNCSIG__
#endif	//	_WIN32



/*
//	these are declared here to work around a warning that appears when using visual studio to compile a dynamic lib (these types need to be exported with the dll, else a conflict with a different stdlib may cause problems when compiling against this lib)
#if defined(VVGL_SDK_WIN)
#include <memory>
#include <string>
#include <functional>
#include <mutex>
#include <vector>
#include <map>
#include <queue>

namespace VVGL {
	class GLBufferPool;
	class GLBuffer;
	class GLScene;
	class GLContext;
	class GLTexToTexCopier;
}

#pragma warning( push )
#pragma warning( disable: 4251 )
template class VVGL_EXPORT std::basic_string<char, std::char_traits<char>, std::allocator<char>>;
template class VVGL_EXPORT std::function<void(VVGL::GLBuffer&, void*)>;
template class VVGL_EXPORT std::shared_ptr<VVGL::GLBufferPool>;
template class VVGL_EXPORT std::shared_ptr<VVGL::GLBuffer>;
template class VVGL_EXPORT std::shared_ptr<VVGL::GLContext>;
class VVGL_EXPORT std::mutex;
class VVGL_EXPORT std::recursive_mutex;
template class VVGL_EXPORT std::vector<std::shared_ptr<VVGL::GLBuffer>>;
template class VVGL_EXPORT std::map<std::basic_string<char, std::char_traits<char>, std::allocator<char>>, std::basic_string<char, std::char_traits<char>, std::allocator<char>>>;
template class VVGL_EXPORT std::function<void(const VVGL::GLScene &)>;
template class VVGL_EXPORT std::function<void(const VVGL::GLScene &, const bool &, const bool &)>;
template class VVGL_EXPORT std::queue<std::shared_ptr<VVGL::GLBuffer>>;
template class VVGL_EXPORT std::shared_ptr<VVGL::GLTexToTexCopier>;
#pragma warning( pop )
#endif	//	_WIN32
*/




#endif /* VVGL_Defines_h */
