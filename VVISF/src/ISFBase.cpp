#include "ISFBase.hpp"

//#include "VVGLBuffer.hpp"
//#include "VVGLBufferPool.hpp"
#include "ISFDoc.hpp"



namespace VVISF
{


using namespace std;





/*	========================================	*/
#pragma mark --------------------- ISF file management methods


vector<string> * createArrayOfISFsForPath(const string & inPath, const ISFFileType & inType, const bool & inRecursive)	{
	return nullptr;
}
vector<string> * createArrayOfDefaultISFs(const ISFFileType & inType)	{
	return nullptr;
}
bool FileIsProbablyAnISF(const string & pathToFile)	{
	//return false;
	bool		returnMe = false;
	try	{
		ISFDoc		tmpDoc(pathToFile);
		returnMe = true;
	}
	catch (ISFErr)	{
	
	}
	return returnMe;
}
string ISFFileTypeString(const ISFFileType & n)	{
	switch (n)	{
	case ISFFileType_None:
		return string("Unknown");
	case ISFFileType_Source:
		return string("Source");
	case ISFFileType_Filter:
		return string("Filter");
	case ISFFileType_Transition:
		return string("Transition");
	}
	return string("?");
}










}
