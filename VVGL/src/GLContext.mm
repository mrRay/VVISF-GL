#include "GLContext.hpp"

#if defined(ISF_SDK_IOS)
	#import <OpenGLES/EAGL.h>
	#import <OpenGLES/ES3/glext.h>
	#import <GLKit/GLKit.h>
#endif




using namespace std;




namespace VVGL
{




#if defined(ISF_SDK_IOS)


/*	========================================	*/
#pragma mark --------------------- constructor/destructor


GLContext::GLContext(const void * inEAGLContext)	{
	if (inEAGLContext == nil)
		throw std::invalid_argument("passed nil context");
	ctx = (void *)[(EAGLContext *)inEAGLContext retain];
	generalInit();
}
/*
GLContext::GLContext(const EAGLContext * inCtx)	{
	
}
GLContext::GLContext(const EAGLSharegroup * inSharegroup, const EAGLRenderingAPI & inRenderAPI)	{

}
*/
/*
GLContext::GLContext(const void * inSharegroup)	{
	ctx = (void *)[[EAGLContext alloc] initWithAPI:kEAGLRenderAPIOpenGLES3 sharegroup:(EAGLSharegroup *)inSharegroup];
	generalInit();
}
*/
GLContext::GLContext()	{
	//ctx = (void *)[[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES3 sharegroup:nil];
	ctx = (void *)[[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES3];
	generalInit();
}
GLContext::~GLContext()	{
	if (ctx != nil)	{
		[(EAGLContext *)ctx release];
		ctx = nil;
	}
}


/*	========================================	*/
#pragma mark --------------------- factory method


/*
GLContext * GLContext::allocNewContextSharingMe() const	{
	//return new GLContext((void *)[ctx sharegroup])
	
	EAGLContext		*newCtx = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES3 sharegroup:[(EAGLContext *)ctx sharegroup]];
	GLContext		*returnMe = new GLContext((void *)newCtx);
	[newCtx autorelease];
	return returnMe;
}
GLContext GLContext::newContextSharingMe() const	{
	//return GLContext((void *)[ctx sharegroup]);
	
	EAGLContext		*newCtx = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES3 sharegroup:[(EAGLContext *)ctx sharegroup]];
	GLContext		returnMe = GLContext((void *)newCtx);
	[newCtx autorelease];
	return returnMe;
}
*/
GLContextRef GLContext::newContextSharingMe() const	{
	EAGLContext		*newCtx = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES3 sharegroup:[(EAGLContext *)ctx sharegroup]];
	if (newCtx == nil)
		return nullptr;
	GLContextRef		returnMe = make_shared<GLContext>((void *)newCtx);
	[newCtx autorelease];
	return returnMe;
}
GLContext & GLContext::operator=(const GLContext & n)	{
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


void GLContext::generalInit()	{
	//	figure out what version of GL we're working with
	calculateVersion();
}


/*	========================================	*/
#pragma mark --------------------- public methods


void GLContext::makeCurrent()	{
	//cout << __PRETTY_FUNCTION__ << ", ctx is " << ctx << endl;
	if (ctx != nil)	{
		//[(EAGLContext *)ctx setCurrent];
		[EAGLContext setCurrentContext:(EAGLContext *)ctx];
	}
	
}
void GLContext::makeCurrentIfNotCurrent()	{
	//cout << __PRETTY_FUNCTION__ << ", ctx is " << ctx << endl;
	if ([EAGLContext currentContext] != (EAGLContext *)ctx)	{
		//[(EAGLContext *)ctx setCurrent];
		[EAGLContext setCurrentContext:(EAGLContext *)ctx];
	}
}
void GLContext::makeCurrentIfNull()	{
	//cout << __PRETTY_FUNCTION__ << ", ctx is " << ctx << endl;
	if ([EAGLContext currentContext] == nil)	{
		//[(EAGLContext *)ctx setCurrent];
		[EAGLContext setCurrentContext:(EAGLContext *)ctx];
	}
}
bool GLContext::sameShareGroupAs(const GLContextRef & inCtx)	{
	EAGLSharegroup		*inSharegroup = (inCtx==nullptr || inCtx->ctx==nullptr) ? nil : [(EAGLContext *)inCtx->ctx sharegroup];
	EAGLSharegroup		*mySharegroup = (ctx==nullptr) ? nil : [(EAGLContext *)ctx sharegroup];
	if (inSharegroup!=nil && mySharegroup!=nil && [inSharegroup isEqual:mySharegroup])
		return true;
	return false;
}


ostream & operator<<(ostream & os, const GLContext & n)	{
	os << &n;
	return os;
}
ostream & operator<<(ostream & os, const GLContext * n)	{
	//os << "<GLContext " << (void *)n << ">";
	os << "<GLContext " << n->ctx << ">";
	return os;
}


#endif	//	ISF_SDK_IOS




}
