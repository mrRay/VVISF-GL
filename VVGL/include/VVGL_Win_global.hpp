#ifndef VVGL_WIN_GLOBAL_H
#define VVGL_WIN_GLOBAL_H




//	we're doing this because the 'max' macro Windows defines collides with Range::max().
#define NOMINMAX




//	if we're exporting VVGL as a library then we need to tell the compiler which objects/functions to put in the lib
#if defined(VVGL_LIBRARY)
#  define VVGL_EXPORT __declspec(dllexport)
#else
//#  define VVGL_EXPORT __declspec(dllimport)
#define VVGL_EXPORT 
#endif




#endif // VVGL_WIN_GLOBAL_H
