#include "VVGL.hpp"
#import <Syphon/Syphon.h>


namespace VVGL	{


//	i'm defining this arbitrary constant as the value "100".  the VVBufferBackID type declared in the vvgl lib is a convenience variable- it doesn't affect the functionality of the backend at all, and exists to make it easier to quickly create ad-hoc bridges between this framework and other graphic APIs.
#define VVGLBufferBackingID_Syphon 100


GLBufferRef CreateBufferForSyphonClient(SyphonClient * c, const GLBufferPoolRef & inPoolRef=GetGlobalBufferPool());


}	//	namespace VVGL