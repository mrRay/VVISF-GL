#ifndef VVGL_StringUtils_hpp
#define VVGL_StringUtils_hpp

#include "VVGL_Defines.hpp"

#include <string>
#include <vector>




namespace VVGL
{




//	functions for doing some basic path manipulation
VVGL_EXPORT std::vector<std::string> PathComponents(const std::string & n);
VVGL_EXPORT std::string LastPathComponent(const std::string & n);
VVGL_EXPORT std::string StringByDeletingLastPathComponent(const std::string & n);
VVGL_EXPORT std::string PathFileExtension(const std::string & n);
VVGL_EXPORT std::string StringByDeletingExtension(const std::string & n);
VVGL_EXPORT std::string StringByDeletingLastAndAddingFirstSlash(const std::string & n);
VVGL_EXPORT std::string StringByDeletingLastSlash(const std::string & n);
VVGL_EXPORT bool CaseInsensitiveCompare(const std::string & a, const std::string & b);
//	this function returns a std::string instance created by passing a c-style format std::string + any number of arguments
VVGL_EXPORT std::string FmtString(const char * fmt, ...);
//	this function returns the number of lines in the passed std::string
VVGL_EXPORT int NumLines(const std::string & n);



}

#endif /* VVGL_StringUtils_hpp */
