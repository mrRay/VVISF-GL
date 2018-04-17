#include "ISFBase.hpp"

//#include "VVGLBuffer.hpp"
//#include "VVGLBufferPool.hpp"
#include "ISFDoc.hpp"



namespace VVISF
{


using namespace std;





/*	========================================	*/
#pragma mark --------------------- ISF file management methods


#if !ISF_SDK_MAC && !ISF_SDK_IOS

shared_ptr<vector<string>> CreateArrayOfISFsForPath(const string & inPath, const ISFFileType & inType, const bool & inRecursive)	{
	cout << "ERR empty: " << __PRETTY_FUNCTION__ << endl;
	return nullptr;
}
shared_ptr<vector<string>> CreateArrayOfDefaultISFs(const ISFFileType & inType)	{
	cout << "ERR empty: " << __PRETTY_FUNCTION__ << endl;
	return nullptr;
}

#endif


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
