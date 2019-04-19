#ifndef ISFStringUtils_hpp
#define ISFStringUtils_hpp

#include <string>
#include <vector>
#include <map>
#include "VVISF_Base.hpp"




namespace VVISF
{




struct ISFVal;
//struct Range;




//	this function parses a std::string as a bool val, and returns either an ISFNullVal (if the std::string couldn't be decisively parsed) or an ISFBoolVal (if it could)
VVISF_EXPORT ISFVal ParseStringAsBool(const std::string & n);
//	this function evaluates the passed std::string and returns a null ISFVal (if the std::string couldn't be evaluated) or a float ISFVal (if it could)
VVISF_EXPORT ISFVal ISFValByEvaluatingString(const std::string & n, const std::map<std::string, double> & inSymbols=std::map<std::string,double>());
//	this function parses a function call from a std::string, dumping the strings of the function arguments 
//	to the provided array.  returns the size of the function std::string (from first char of function call 
//	to the closing parenthesis of the function call)
VVISF_EXPORT VVGL::Range LexFunctionCall(const std::string & inBaseStr, const VVGL::Range & inFuncNameRange, std::vector<std::string> & outVarArray);

VVISF_EXPORT std::string TrimWhitespace(const std::string & inBaseStr);

VVISF_EXPORT void FindAndReplaceInPlace(std::string & inSearch, std::string & inReplace, std::string & inBase);
VVISF_EXPORT void FindAndReplaceInPlace(const char * inSearch, const char * inReplace, std::string & inBase);

VVISF_EXPORT std::string FullPath(const std::string & inRelativePath);


}


#endif /* ISFStringUtils_hpp */

