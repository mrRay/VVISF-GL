#ifndef ISFErr_h
#define ISFErr_h

#include "VVISF_Base.hpp"




namespace VVISF
{


using namespace std;


/*!
\ingroup VVISF_BASIC
\brief Enumerates the different kinds of ISF errors.
*/
enum ISFErrType	{
	//!<	An unknown error type
	ISFErrType_Unknown = 0,
	//!<	Resources required by the ISF file cannot be located.
	ISFErrType_MissingResource,
	//!<	The JSON blob that defines the ISF file was malformed.
	ISFErrType_MalformedJSON,
	//!<	There was an error loading one of the resources required by the ISF file.
	ISFErrType_ErrorLoading,
	//!<	There was an error parsing the JSON blob that defines the ISF file.
	ISFErrType_ErrorParsingFS,
	//!<	There was an error compiling the ISF file's GLSL program.  Consult the ISFErr's "details" map for more information.
	ISFErrType_ErrorCompilingGLSL
};


/*!
\ingroup VVISF_BASIC
\brief The base error class utilized by VVISF.
*/
class VVISF_EXPORT ISFErr	{
	public:
		//!	The type of the ISFErr
		ISFErrType		type;
		//!	A string that describes the general problem.
		string		general;
		//!	A string that describes the problem more specifically.
		string		specific;
		//!	A map that offers a more structured way to store more information that describes the error.  Typically used to embed GLSL compiler errors in the ISFErr instance.
		map<string,string>		details;
	public:
		ISFErr(const ISFErrType & inType, const char * inGeneral, const char * inSpecific, const map<string,string> & inDetails=map<string,string>()) : type(inType), general(inGeneral), specific(inSpecific), details(inDetails) {};
		ISFErr(const ISFErrType & inType, const char * inGeneral, const string & inSpecific, const map<string,string> & inDetails=map<string,string>()) : type(inType), general(inGeneral), specific(inSpecific), details(inDetails) {};
		ISFErr(const ISFErrType & inType, const string & inGeneral, const char * inSpecific, const map<string,string> & inDetails=map<string,string>()) : type(inType), general(inGeneral), specific(inSpecific), details(inDetails) {};
		ISFErr(const ISFErrType & inType, const string & inGeneral, const string & inSpecific, const map<string,string> & inDetails=map<string,string>()) : type(inType), general(inGeneral), specific(inSpecific), details(inDetails) {};
		
		ISFErr(const ISFErr & n) = default;
		ISFErr & operator=(const ISFErr & n) = default;
		
		//!	Returns a human-readable string describing the receiver's error.
		string getTypeString() {
			switch (type)	{
			case ISFErrType_Unknown:
				return string("Unknown");
			case ISFErrType_MissingResource:
				return string("Missing Resource");
			case ISFErrType_MalformedJSON:
				return string("JSON error");
			case ISFErrType_ErrorLoading:
				return string("Error loading");
			case ISFErrType_ErrorParsingFS:
				return string("FS Error");
			case ISFErrType_ErrorCompilingGLSL:
				return string("GLSL Error");
			}
			return string("");
		};
};




}


#endif /* ISFErr_h */
