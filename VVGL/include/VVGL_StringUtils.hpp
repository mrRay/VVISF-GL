#ifndef VVGL_StringUtils_hpp
#define VVGL_StringUtils_hpp

#include "VVGL_Defines.hpp"

#include <string>
#include <vector>




namespace VVGL
{


using namespace std;


//	functions for doing some basic path manipulation
VVGL_EXPORT vector<string> PathComponents(const string & n);
VVGL_EXPORT string LastPathComponent(const string & n);
VVGL_EXPORT string StringByDeletingLastPathComponent(const string & n);
VVGL_EXPORT string PathFileExtension(const string & n);
VVGL_EXPORT string StringByDeletingExtension(const string & n);
VVGL_EXPORT string StringByDeletingLastAndAddingFirstSlash(const string & n);
VVGL_EXPORT string StringByDeletingLastSlash(const string & n);
VVGL_EXPORT bool CaseInsensitiveCompare(const string & a, const string & b);
//	this function returns a string instance created by passing a c-style format string + any number of arguments
VVGL_EXPORT string FmtString(const char * fmt, ...);
//	this function returns the number of lines in the passed string
VVGL_EXPORT int NumLines(const string & n);



}

#endif /* VVGL_StringUtils_hpp */
