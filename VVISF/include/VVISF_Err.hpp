#ifndef ISFErr_h
#define ISFErr_h

#include "VVISF_Base.hpp"




namespace VVISF
{




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
	//!<	There was an error compiling the ISF file's GLSL program.  Consult the ISFErr's "details" std::map for more information.
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
		//!	A std::string that describes the general problem.
		std::string		general;
		//!	A std::string that describes the problem more specifically.
		std::string		specific;
		//!	A std::map that offers a more structured way to store more information that describes the error.  Typically used to embed GLSL compiler errors in the ISFErr instance.
		std::map<std::string,std::string>		details;
	public:
		ISFErr(const ISFErrType & inType, const char * inGeneral, const char * inSpecific, const std::map<std::string,std::string> & inDetails=std::map<std::string,std::string>()) : type(inType), general(inGeneral), specific(inSpecific), details(inDetails) {};
		ISFErr(const ISFErrType & inType, const char * inGeneral, const std::string & inSpecific, const std::map<std::string,std::string> & inDetails=std::map<std::string,std::string>()) : type(inType), general(inGeneral), specific(inSpecific), details(inDetails) {};
		ISFErr(const ISFErrType & inType, const std::string & inGeneral, const char * inSpecific, const std::map<std::string,std::string> & inDetails=std::map<std::string,std::string>()) : type(inType), general(inGeneral), specific(inSpecific), details(inDetails) {};
		ISFErr(const ISFErrType & inType, const std::string & inGeneral, const std::string & inSpecific, const std::map<std::string,std::string> & inDetails=std::map<std::string,std::string>()) : type(inType), general(inGeneral), specific(inSpecific), details(inDetails) {};
		
		ISFErr(const ISFErr & n) = default;
		ISFErr & operator=(const ISFErr & n) = default;
		
		//!	Returns a human-readable std::string describing the receiver's error.
		std::string getTypeString() {
			switch (type)	{
			case ISFErrType_Unknown:
				return std::string("Unknown");
			case ISFErrType_MissingResource:
				return std::string("Missing Resource");
			case ISFErrType_MalformedJSON:
				return std::string("JSON error");
			case ISFErrType_ErrorLoading:
				return std::string("Error loading");
			case ISFErrType_ErrorParsingFS:
				return std::string("FS Error");
			case ISFErrType_ErrorCompilingGLSL:
				return std::string("GLSL Error");
			}
			return std::string("");
		};
};




}


#endif /* ISFErr_h */
