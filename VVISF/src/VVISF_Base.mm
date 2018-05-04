#import <Foundation/Foundation.h>
#include "VVISF_Base.hpp"
#include "ISFDoc.hpp"



namespace VVISF
{


using namespace std;

class ISFDoc;




#if defined(VVGL_SDK_MAC) || defined(VVGL_SDK_IOS)

shared_ptr<vector<string>> CreateArrayOfISFsForPath(const string & inFolderPath, const ISFFileType & inType, const bool & inRecursive)	{
	//cout << __PRETTY_FUNCTION__ << "..." << inFolderPath << endl;
	
	
	
	NSString				*trimmedPath = [NSString stringWithUTF8String:StringByDeletingLastAndAddingFirstSlash(inFolderPath).c_str()];
	//NSString				*folder = [NSString stringWithUTF8String:inFolderPath.c_str()];
	bool					r = inRecursive;
	ISFFileType				func = inType;
	
	if (trimmedPath==nil)
		return nullptr;
	//NSString			*trimmedPath = [folder stringByDeletingLastAndAddingFirstSlash];
	NSFileManager		*fm = [NSFileManager defaultManager];
	BOOL				isDirectory = NO;
	if (![fm fileExistsAtPath:trimmedPath isDirectory:&isDirectory])
		return nullptr;
	if (!isDirectory)
		return nullptr;
	NSMutableArray			*returnMe = [[[NSMutableArray alloc] initWithCapacity:0] autorelease];
	if (r)	{
		NSDirectoryEnumerator	*it = [fm enumeratorAtPath:trimmedPath];
		NSString				*file = nil;
		while (file = [it nextObject])	{
			NSString		*ext = [file pathExtension];
			if (ext!=nil && ([ext isEqualToString:@"fs"] || [ext isEqualToString:@"frag"]))	{
				NSString		*fullPath = [NSString stringWithFormat:@"%@/%@",trimmedPath,file];
				if (func == ISFFileType_All || func == ISFFileType_None)
					[returnMe addObject:fullPath];
				else	{
					string			tmpString([fullPath UTF8String]);
					VVISF::ISFDoc			tmpDoc(tmpString);
					//ISFDoc			tmpDoc(string([fullPath UTF8String]));
					ISFFileType		tmpType = tmpDoc.getType();
					if ((tmpType & func) > 0)
						[returnMe addObject:fullPath];
					
					/*
					if ([self _isAFilter:fullPath])	{
						if (func == ISFF_Filter)
							[returnMe addObject:fullPath];
					}
					else	{
						if (func == ISFF_Source)
							[returnMe addObject:fullPath];
					}
					*/
				}
			}
		}
	}
	//	else non-recursive (shallow) listing
	else	{
		NSArray		*tmpArray = [fm contentsOfDirectoryAtPath:trimmedPath error:nil];
		for (NSString *file in tmpArray)	{
			NSString		*ext = [file pathExtension];
			if (ext!=nil && ([ext isEqualToString:@"fs"] || [ext isEqualToString:@"frag"]))	{
				NSString		*fullPath = [NSString stringWithFormat:@"%@/%@",trimmedPath,file];
				if (func == ISFFileType_All || func == ISFFileType_None)
					[returnMe addObject:fullPath];
				else	{
					string			tmpString([fullPath UTF8String]);
					VVISF::ISFDoc	tmpDoc(tmpString);
					//ISFDoc			tmpDoc(string([fullPath UTF8String]));
					ISFFileType		tmpType = tmpDoc.getType();
					if ((tmpType & func) > 0)
						[returnMe addObject:fullPath];
					
					/*
					if ([self _isAFilter:fullPath])	{
						if (func == ISFF_Filter)
							[returnMe addObject:fullPath];
					}
					else	{
						if (func == ISFF_Source)
							[returnMe addObject:fullPath];
					}
					*/
				}
			}
		}
	}
	//NSLog(@"\t\tpre-assembled array is %@",returnMe);
	shared_ptr<vector<string>>		returnVector = make_shared<vector<string>>();
	for (NSString *tmpNSString in returnMe)	{
		//string			tmpString([tmpNSString UTF8String]);
		returnVector->emplace_back([tmpNSString UTF8String]);
	}
	return returnVector;
}
shared_ptr<vector<string>> CreateArrayOfDefaultISFs(const ISFFileType & inType)	{
	cout << "ERR empty: " << __PRETTY_FUNCTION__ << endl;
	return nullptr;
}

#endif




}
