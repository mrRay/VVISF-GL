#ifndef ISFErr_h
#define ISFErr_h


namespace VVISF
{


using namespace std;




enum ISFErrType	{
	ISFErrType_Unknown = 0,
	ISFErrType_MissingResource,
	ISFErrType_MalformedJSON,
	ISFErrType_ErrorLoading,
	ISFErrType_ErrorParsingFS,
	ISFErrType_ErrorCompilingGLSL
};
//	base exception class
class ISFErr	{
	public:
		ISFErrType		type;
		string		general;
		string		specific;
		map<string,string>		details;
	public:
		/*
		ISFErr(const ISFErrType & inType, const char * inGeneral, const char * inSpecific) : type(inType), general(inGeneral), specific(inSpecific) {};
		ISFErr(const ISFErrType & inType, const char * inGeneral, const string & inSpecific) : type(inType), general(inGeneral), specific(inSpecific) {};
		ISFErr(const ISFErrType & inType, const string & inGeneral, const char * inSpecific) : type(inType), general(inGeneral), specific(inSpecific) {};
		ISFErr(const ISFErrType & inType, const string & inGeneral, const string & inSpecific) : type(inType), general(inGeneral), specific(inSpecific) {};
		*/
		ISFErr(const ISFErrType & inType, const char * inGeneral, const char * inSpecific, const map<string,string> & inDetails=map<string,string>()) : type(inType), general(inGeneral), specific(inSpecific), details(inDetails) {};
		ISFErr(const ISFErrType & inType, const char * inGeneral, const string & inSpecific, const map<string,string> & inDetails=map<string,string>()) : type(inType), general(inGeneral), specific(inSpecific), details(inDetails) {};
		ISFErr(const ISFErrType & inType, const string & inGeneral, const char * inSpecific, const map<string,string> & inDetails=map<string,string>()) : type(inType), general(inGeneral), specific(inSpecific), details(inDetails) {};
		ISFErr(const ISFErrType & inType, const string & inGeneral, const string & inSpecific, const map<string,string> & inDetails=map<string,string>()) : type(inType), general(inGeneral), specific(inSpecific), details(inDetails) {};
		
		
		ISFErr(const ISFErr & n) = default;
		ISFErr & operator=(const ISFErr & n) = default;
		
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
