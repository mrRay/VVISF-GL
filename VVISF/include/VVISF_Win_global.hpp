#ifndef VVISF_GLOBAL_H
#define VVISF_GLOBAL_H




//	we're doing this because the 'max' macro Windows defines collides with Range::max().
#define NOMINMAX




//	if we're exporting VVGL as a library then we need to tell the compiler which objects/functions to put in the lib
#if defined(VVISF_LIBRARY)
#  define VVISF_EXPORT __declspec(dllexport)
#else
//#  define VVISF_EXPORT __declspec(dllimport)
#define VVISF_EXPORT 
#endif




#endif // VVISF_GLOBAL_H
