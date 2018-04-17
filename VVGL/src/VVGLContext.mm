#include "VVGLContext.hpp"

#if ISF_SDK_IOS
	#import <OpenGLES/EAGL.h>
	#import <OpenGLES/ES3/glext.h>
	#import <GLKit/GLKit.h>
#endif




using namespace std;




namespace VVGL
{




#if ISF_SDK_IOS


/*	========================================	*/
#pragma mark --------------------- constructor/destructor


VVGLContext::VVGLContext(const void * inEAGLContext)	{
	if (inEAGLContext == nil)
		throw std::invalid_argument("passed nil context");
	ctx = (void *)[(EAGLContext *)inEAGLContext retain];
	generalInit();
}
/*
VVGLContext::VVGLContext(const EAGLContext * inCtx)	{
	
}
VVGLContext::VVGLContext(const EAGLSharegroup * inSharegroup, const EAGLRenderingAPI & inRenderAPI)	{

}
*/
/*
VVGLContext::VVGLContext(const void * inSharegroup)	{
	ctx = (void *)[[EAGLContext alloc] initWithAPI:kEAGLRenderAPIOpenGLES3 sharegroup:(EAGLSharegroup *)inSharegroup];
	generalInit();
}
*/
VVGLContext::VVGLContext()	{
	ctx = (void *)[[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES3 sharegroup:nil];
	generalInit();
}
VVGLContext::~VVGLContext()	{
	if (ctx != nil)	{
		[(EAGLContext *)ctx release];
		ctx = nil;
	}
}


/*	========================================	*/
#pragma mark --------------------- copy constructor


//	copy constructors DO NOT CREATE NEW GL CONTEXTS.  they only retain the contexts they were passed.
VVGLContext::VVGLContext(const VVGLContext * n)	{
	//cout << __PRETTY_FUNCTION__ << endl;
	
	ctx = (n->ctx==nil) ? nil : (void *)[(EAGLContext *)n->ctx retain];
	
	generalInit();
}
VVGLContext::VVGLContext(const VVGLContext & n)	{
	//cout << __PRETTY_FUNCTION__ << endl;
	
	ctx = (n.ctx==nil) ? nil : (void *)[(EAGLContext *)n.ctx retain];
	
	generalInit();
}
VVGLContext::VVGLContext(const VVGLContextRef & n)	{
	//cout << __PRETTY_FUNCTION__ << endl;
	
	ctx = (n->ctx==nil) ? nil : (void *)[(EAGLContext *)n->ctx retain];
	
	generalInit();
}


/*	========================================	*/
#pragma mark --------------------- factory method


/*
VVGLContext * VVGLContext::allocNewContextSharingMe() const	{
	//return new VVGLContext((void *)[ctx sharegroup])
	
	EAGLContext		*newCtx = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES3 sharegroup:[(EAGLContext *)ctx sharegroup]];
	VVGLContext		*returnMe = new VVGLContext((void *)newCtx);
	[newCtx autorelease];
	return returnMe;
}
VVGLContext VVGLContext::newContextSharingMe() const	{
	//return VVGLContext((void *)[ctx sharegroup]);
	
	EAGLContext		*newCtx = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES3 sharegroup:[(EAGLContext *)ctx sharegroup]];
	VVGLContext		returnMe = VVGLContext((void *)newCtx);
	[newCtx autorelease];
	return returnMe;
}
*/
VVGLContextRef VVGLContext::newContextSharingMe() const	{
	EAGLContext		*newCtx = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES3 sharegroup:[(EAGLContext *)ctx sharegroup]];
	if (newCtx == nil)
		return nullptr;
	VVGLContextRef		returnMe = make_shared<VVGLContext>((void *)newCtx);
	[newCtx autorelease];
	return returnMe;
}
VVGLContext & VVGLContext::operator=(const VVGLContext & n)	{
	if (ctx != nil)	{
		[(EAGLContext *)ctx release];
		ctx = nil;
	}
	
	if (n.ctx != nil)
		ctx = (void *)[(EAGLContext *)n.ctx retain];
	
	return *this;
}


/*	========================================	*/
#pragma mark --------------------- general init/delete


void VVGLContext::generalInit()	{
	//	figure out what version of GL we're working with
	calculateVersion();
}


/*	========================================	*/
#pragma mark --------------------- public methods


void VVGLContext::makeCurrent()	{
	//cout << __PRETTY_FUNCTION__ << ", ctx is " << ctx << endl;
	if (ctx != nil)	{
		//[(EAGLContext *)ctx setCurrent];
		[EAGLContext setCurrentContext:(EAGLContext *)ctx];
	}
	
}
void VVGLContext::makeCurrentIfNotCurrent()	{
	//cout << __PRETTY_FUNCTION__ << ", ctx is " << ctx << endl;
	if ([EAGLContext currentContext] != (EAGLContext *)ctx)	{
		//[(EAGLContext *)ctx setCurrent];
		[EAGLContext setCurrentContext:(EAGLContext *)ctx];
	}
}
void VVGLContext::makeCurrentIfNull()	{
	//cout << __PRETTY_FUNCTION__ << ", ctx is " << ctx << endl;
	if ([EAGLContext currentContext] == nil)	{
		//[(EAGLContext *)ctx setCurrent];
		[EAGLContext setCurrentContext:(EAGLContext *)ctx];
	}
}
bool VVGLContext::sameShareGroupAs(const VVGLContextRef & inCtx)	{
	EAGLSharegroup		*inSharegroup = (inCtx==nullptr || inCtx->ctx==nullptr) ? nil : [(EAGLContext *)inCtx->ctx sharegroup];
	EAGLSharegroup		*mySharegroup = (ctx==nullptr) ? nil : [(EAGLContext *)ctx sharegroup];
	if (inSharegroup!=nil && mySharegroup!=nil && [inSharegroup isEqual:mySharegroup])
		return true;
	return false;
}


ostream & operator<<(ostream & os, const VVGLContext & n)	{
	os << &n;
	return os;
}
ostream & operator<<(ostream & os, const VVGLContext * n)	{
	//os << "<VVGLContext " << (void *)n << ">";
	os << "<VVGLContext " << n->ctx << ">";
	return os;
}


#endif	//	ISF_SDK_IOS




}
