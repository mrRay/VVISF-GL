#ifndef ISFStringUtils_hpp
#define ISFStringUtils_hpp

#include <string>
#include <vector>
#include <map>
//#include "VVGL_Range.hpp"
#include "VVGL.hpp"

#if defined(ISF_SDK_QT)
#include "vvisf_qt_global.h"
#endif


namespace VVISF
{


using namespace std;
using namespace VVGL;


struct ISFVal;
//struct Range;


//	this function parses a string as a bool val, and returns either an ISFNullVal (if the string couldn't be decisively parsed) or an ISFBoolVal (if it could)
VVISF_EXPORT ISFVal ParseStringAsBool(const string & n);
//	this function evaluates the passed string and returns a null ISFVal (if the string couldn't be evaluated) or a float ISFVal (if it could)
VVISF_EXPORT ISFVal ISFValByEvaluatingString(const string & n, const map<string, double> & inSymbols=map<string,double>());
//	this function parses a function call from a string, dumping the strings of the function arguments 
//	to the provided array.  returns the size of the function string (from first char of function call 
//	to the closing parenthesis of the function call)
VVISF_EXPORT Range LexFunctionCall(const string & inBaseStr, const Range & inFuncNameRange, vector<string> & outVarArray);

VVISF_EXPORT string TrimWhitespace(const string & inBaseStr);

VVISF_EXPORT void FindAndReplaceInPlace(string & inSearch, string & inReplace, string & inBase);
VVISF_EXPORT void FindAndReplaceInPlace(const char * inSearch, const char * inReplace, string & inBase);

VVISF_EXPORT string FullPath(const string & inRelativePath);


}


#endif /* ISFStringUtils_hpp */

