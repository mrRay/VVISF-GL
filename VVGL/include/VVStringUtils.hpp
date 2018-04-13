#ifndef VVStringUtils_hpp
#define VVStringUtils_hpp

#if ISF_TARGET_QT
#include "vvgl_qt_global.h"
#endif

#include <string>
#include <vector>




namespace VVGL
{


using namespace std;




//	functions for doing some basic path manipulation
vector<string> PathComponents(const string & n);
string LastPathComponent(const string & n);
string StringByDeletingLastPathComponent(const string & n);
string StringByDeletingExtension(const string & n);
string StringByDeletingLastAndAddingFirstSlash(const string & n);
//	this function returns a string instance created by passing a c-style format string + any number of arguments
string FmtString(const char * fmt, ...);



}

#endif /* VVStringUtils_hpp */
