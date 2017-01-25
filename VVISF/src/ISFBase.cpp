#include "ISFBase.hpp"

//#include "VVGLBuffer.hpp"
//#include "VVGLBufferPool.hpp"




namespace VVISFKit
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
bool fileIsProbablyAnISF(const string & pathToFile)	{
	return false;
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
