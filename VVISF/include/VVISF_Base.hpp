#ifndef ISFBase_h
#define ISFBase_h

#include <vector>

#if defined(VVGL_SDK_QT)
#include "VVISF_Qt_global.hpp"
#elif defined(VVGL_SDK_WIN)
#include "VVISF_Win_global.hpp"
#else
#define VVISF_EXPORT 
#endif

//#if defined(VVGL_SDK_MAC)
//#include <VVGL/VVGL.hpp>
//#else
//#include "VVGL.hpp"
//#endif
#include <memory>
#include <string>
#include <map>
#include <iostream>

#include "VVGL_Base.hpp"
#include "VVGL_Geom.hpp"

/*!
\file
*/




namespace VVISF
{




//	some forward declarations used in this header
class ISFPassTarget;
//struct ISFVal;
class ISFDoc;
class ISFAttr;
class ISFScene;




/*!
\brief ISFPassTargetRef is a shared pointer around an ISFPassTarget instance.
\relates VVISF::ISFPassTarget
*/
using ISFPassTargetRef = std::shared_ptr<ISFPassTarget>;
//! ISFDocRef is a shared pointer around an ISFDoc instance.
/*!
\relates VVISF::ISFDoc
ISFDocRef is the preferred means of working with ISFDoc instances, which can be extremely simple with no overhead or can potentially contain a variety of data values including GL resources (texture, buffers, etc).
*/
using ISFDocRef = std::shared_ptr<ISFDoc>;
/*!
\brief ISFAttrRef is a shared pointer around an ISFAttr instance.
\relates VVISF::ISFAttr
*/
using ISFAttrRef = std::shared_ptr<ISFAttr>;
//! ISFSceneRef is a shared pointer around an ISFScene instance.
/*!
\relates VVISF::ISFScene
ISFScene is a subclass of GLScene, and like its parent, you should strive to work exclusively with ISFSceneRef instead of ISFScene directly.
*/
using ISFSceneRef = std::shared_ptr<ISFScene>;




/*!
The basic functionality offered by the ISF file loaded by an ISFDoc instance can be described by one or more of these enums (it's a bitmask)
*/
enum ISFFileType	{
	ISFFileType_None = 0,	//!<	No or unrecognized file type
	ISFFileType_Source = 1,	//!<	The file is a "source"- it generates images
	ISFFileType_Filter = 2,	//!<	The file is a "filter"- it defines an image-type input under the name "inputImage", which it modifies.
	ISFFileType_Transition = 4,	//!<	The file is a "transition"- it defines two image-type inputs ("startImage" and "endImage") in addition to a normalized float-type input ("progress"), which is used to "mix" from the start image to the end image.
	ISFFileType_All = 7	//!<	Convenience enumeration, should always evaluate to "all types simultaneously".
};
//!	Returns a std::string describing the passed ISFFileType
std::string ISFFileTypeString(const ISFFileType & n);




/*!
\brief Scans the passed path for valid ISF files, returns an array of strings/paths to the detected files.
\param inFolderPath The path of the directory to scan.
\param inType The type of ISFs to scan for.  Set to 0 or ISFFileType_All to return all valid ISFs in the passed folder- anything else will only return ISFs that match the passed type.
\param inRecursive Whether or not the scan should be recursive.
*/
std::shared_ptr<std::vector<std::string>> CreateArrayOfISFsForPath(const std::string & inFolderPath, const ISFFileType & inType=ISFFileType_None, const bool & inRecursive=true);
/*!
\brief Returns an array of strings/paths to the default ISF files.
\param inType The type of ISFs to scan for.  Set to 0 or ISFFileType_All to return all valid ISFs in the passed folder- anything else will only return ISFs that match the passed type.
*/
std::shared_ptr<std::vector<std::string>> CreateArrayOfDefaultISFs(const ISFFileType & inType=ISFFileType_None);
/*!
Returns 'true' if there is a file at the passed path, and that file appears to contain a valid ISF file.
*/
bool FileIsProbablyAnISF(const std::string & pathToFile);




}




#include "VVISF_Constants.hpp"
#include "VVISF_Err.hpp"
#include "ISFVal.hpp"
#include "VVISF_StringUtils.hpp"




#endif /* ISFBase_h */
