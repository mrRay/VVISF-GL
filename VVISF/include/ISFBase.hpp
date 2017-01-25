#ifndef ISFBase_h
#define ISFBase_h

#include <vector>

#include "VVGL.hpp"
using namespace VVGL;



namespace VVISFKit
{


using namespace std;




//	some forward declarations used in this header
class ISFPassTarget;
struct ISFVal;
class ISFDoc;
class ISFAttr;




//	ISFPassTargetRef is a shared pointer to an ISFPassTarget
using ISFPassTargetRef = shared_ptr<ISFPassTarget>;
//	ISFDocRef is a shared pointer to an ISFDoc
using ISFDocRef = shared_ptr<ISFDoc>;
//	ISFAttrRef is a shared poitner to an ISFAttr
using ISFAttrRef = std::shared_ptr<ISFAttr>;




//	this enum describes the different types of ISF filters- it's a bitmap mask, these vals can be combined
enum ISFFileType	{
	ISFFileType_None = 0,
	ISFFileType_Source = 1,
	ISFFileType_Filter = 2,
	ISFFileType_Transition = 4
};
//	YOU MUST DELETE THE OBJECTS RETURNED BY THESE FUNCTIONS.  these functions are used to verify and discover ISFs in your filesystem
vector<string> * createArrayOfISFsForPath(const string & inPath, const ISFFileType & inType=ISFFileType_None, const bool & inRecursive=true);
vector<string> * createArrayOfDefaultISFs(const ISFFileType & inType=ISFFileType_None);
//	returns a true if the passed file is probably an ISF file
bool fileIsProbablyAnISF(const string & pathToFile);
//	creates a string describing the ISFFileType
string ISFFileTypeString(const ISFFileType & n);




}




#include "ISFConstants.hpp"
#include "ISFErr.hpp"
#include "ISFVal.hpp"
#include "ISFStringUtils.hpp"




#endif /* ISFBase_h */
