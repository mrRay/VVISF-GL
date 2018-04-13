#ifndef ISFStringUtils_hpp
#define ISFStringUtils_hpp

#include <string>
#include <vector>
#include <map>
//#include "VVRange.hpp"
#include "VVGL.hpp"

#if ISF_TARGET_QT
#include "vvisf_qt_global.h"
#endif


namespace VVISF
{


using namespace std;
using namespace VVGL;


struct ISFVal;
//struct VVRange;


//	this function parses a string as a bool val, and returns either an ISFNullVal (if the string couldn't be decisively parsed) or an ISFBoolVal (if it could)
ISFVal ParseStringAsBool(const string & n);
//	this function evaluates the passed string and returns a null ISFVal (if the string couldn't be evaluated) or a float ISFVal (if it could)
ISFVal ISFValByEvaluatingString(const string & n, const map<string, double> & inSymbols=map<string,double>());
//	this function parses a function call from a string, dumping the strings of the function arguments 
//	to the provided array.  returns the size of the function string (from first char of function call 
//	to the closing parenthesis of the function call)
VVRange LexFunctionCall(const string & inBaseStr, const VVRange & inFuncNameRange, vector<string> & outVarArray);

string TrimWhitespace(const string & inBaseStr);

void FindAndReplaceInPlace(string & inSearch, string & inReplace, string & inBase);
void FindAndReplaceInPlace(const char * inSearch, const char * inReplace, string & inBase);

string FullPath(const string & inRelativePath);


}


#endif /* ISFStringUtils_hpp */

