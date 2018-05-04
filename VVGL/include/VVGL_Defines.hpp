#ifndef VVGL_Defines_h
#define VVGL_Defines_h



/*

developers need to define one of these during compilation:

	VVGL_SDK_MAC
	VVGL_SDK_IOS
	VVGL_SDK_RPI
	VVGL_SDK_GLFW
	VVGL_SDK_QT

this header file defines a series of macros that describe the availability of various GL 
environments based on the SDK as defined by the user.  these are the possible values- note that 
these are not mutually exclusive- more than one may be defined simultaneously, these are only used 
to block of function calls that won't compile/link against a given SDK's headers/libs.

	VVGL_TARGETENV_GL2
	VVGL_TARGETENV_GL3PLUS
	VVGL_TARGETENV_GLES
	VVGL_TARGETENV_GLES3



*/




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
#endif




#if defined(VVGL_SDK_QT)
#include "VVGL_Qt_global.h"
#else
#define VVGL_EXPORT 
#endif




#if defined (_WIN32)
//#define __PRETTY_FUNCTION__ __func__
#define __PRETTY_FUNCTION__ __FUNCSIG__
#pragma warning (disable : 4068)	/*	disables unknown pragma warnings	*/
#endif	//	_WIN32




#endif /* VVGL_Defines_h */
