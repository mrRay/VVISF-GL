#ifndef VVGL_GLCachedUni_h
#define VVGL_GLCachedUni_h

#include "VVGL_Defines.hpp"

#include <iostream>

#include "VVGL_StringUtils.hpp"




namespace VVGL
{




using namespace std;




/*!
\ingroup VVGL_MISC
\brief Abstract base class that caches the location of an arbitrary GL "object" that we do not own.

\detail A lot of GL "objects" are numbers that we have to keep track of (but not assume ownership of), so this is an abstract base class for an object that will cache the location of any arbitrary GL object in a given program along with a string that can be used to identify it.  the goal of the cache is to eliminate the need to repeatedly query the GL context for the location of attributes/uniforms/other things like that.

Under most circumstances you'll probably want to use one of its concrete subclasses, GLCachedAttrib or GLCachedProperty.  Under most circumstances you'll also probably want to be using Refs (GLCachedAttribRef or GLCachedPropertyRef) because they're easier to maintain than multiple instances and can be copied by value in lambdas while stil referring to the same underlying instance.  				*/

struct VVGL_EXPORT GLCachedProperty	{
	//	these vars are public and that's technically not safe, but practically speaking this works out because everything GL-related has to be serialized such that access is one-context-per-thread anyway...
	public:
		int32_t			loc = -1;	//	the location of the attribute/uniform/etc, derived by quering a GL program (the id of which is also cached).  read-only outside this object!
		string			name;	//	the name of the attribute
		int32_t			prog = -1;	//	the id of the pgm that was checked to create the current loc
	public:
		GLCachedProperty(string & inName);
		GLCachedProperty(const string & inName);
		GLCachedProperty(const GLCachedProperty & n) : loc(n.loc), name(n.name), prog(n.prog) {}
	public:
		//	pure virtual function, subclasses *must* implement this.  this is where the GL stuff specific to the subclass happens.  a valid GL context *must* be current before you call this function.
		virtual void cacheTheLoc(const int32_t & inPgmToCheck) = 0;
		inline void purgeCache() { loc=-1; prog=-1; }
	public:
		//	a valid GL context must be current before you call this function.  caches the loc if it hasn't been done yet, will only return -1 if there's a problem (if the attrib doesn't exist in the current program in use by the current context)
		int32_t location(const int32_t & inGLProgram) { if (loc<0 || inGLProgram<0 || inGLProgram!=prog) cacheTheLoc(inGLProgram); return loc; }
		string getDescriptionString() const { return FmtString("<GLCachedProperty \"%s\", %d>",this->name.c_str(),this->loc); }
		friend ostream & operator<<(ostream & os, const GLCachedProperty & n) { os<<n.getDescriptionString();return os; }
};




/*	this caches the location of an attribute of a given GL program.		*/

struct VVGL_EXPORT GLCachedAttrib : GLCachedProperty	{
	public:
		GLCachedAttrib(string & inName) : GLCachedProperty(inName) {}
		GLCachedAttrib(const string & inName) : GLCachedProperty(inName) {}
		GLCachedAttrib(const GLCachedAttrib & n) : GLCachedProperty(n) {}
	public:
		string getDescriptionString() const { return FmtString("<GLCachedAttrib \"%s\", %d>",this->name.c_str(),this->loc); }
		//	a valid GL context must be current before you call this function.
		void enable();
		//	a valid GL context must be current before you call this function.  protip: don't call this function if you're using VAOs to draw.
		void disable();
		
		virtual void cacheTheLoc(const int32_t & inPgmToCheck);
};




/*	this caches the location of a uniform variable of a given GL program		*/

struct VVGL_EXPORT GLCachedUni : GLCachedProperty	{
	public:
		GLCachedUni(string & inName) : GLCachedProperty(inName) {}
		GLCachedUni(const string & inName) : GLCachedProperty(inName) {}
		GLCachedUni(const GLCachedUni & n) : GLCachedProperty(n) {}
	public:
		string getDescriptionString() const { return FmtString("<GLCachedUni \"%s\", %d>",this->name.c_str(),this->loc); }
		
		virtual void cacheTheLoc(const int32_t & inPgmToCheck);
};




}


#endif /* VVGL_GLCachedUni_h */
