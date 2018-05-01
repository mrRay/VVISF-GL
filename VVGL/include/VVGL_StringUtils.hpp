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
VVGL_EXPORT string StringByDeletingExtension(const string & n);
VVGL_EXPORT string StringByDeletingLastAndAddingFirstSlash(const string & n);
//	this function returns a string instance created by passing a c-style format string + any number of arguments
VVGL_EXPORT string FmtString(const char * fmt, ...);



}

#endif /* VVGL_StringUtils_hpp */
