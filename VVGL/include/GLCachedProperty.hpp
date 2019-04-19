#ifndef VVGL_GLCachedUni_h
#define VVGL_GLCachedUni_h

#include "VVGL_Defines.hpp"

#include <iostream>

#include "VVGL_StringUtils.hpp"




namespace VVGL
{




//! Abstract base class that caches the location of an arbitrary GL "object" that we do not own.
/*!
\ingroup VVGL_MISC
A lot of GL "objects"- like attributes, uniforms, etc- are numbers that we have to keep track of (but not assume ownership of).  Querying the context for the location of these "objects" every render pass is a performance sink.  The goal of this class is to eliminate the need to repeatedly query the GL context by caching the location along with the relevant program and a std::string that can be used to look up the property.

This is an abstract base class, so you should never create an instance of it directly- instead, use one of its subclasses (or write another subclass) which cache specific kinds of data: GLCachedAttrib caches the location of an attribute in a GLSL program, and GLCachedUni caches the location of a uniform in a GLSL program.

Notes on use:
- You should strive to work almost exclusively with #GLCachedAttribRef and #GLCachedUniRef, which are std::shared_ptrs around GLCachedAttrib/GLCachedUni.
*/
struct VVGL_EXPORT GLCachedProperty	{
	//	these vars are public and that's technically not safe in multi-threaded situations, but practically speaking this works out because everything GL-related has to be serialized such that access is one-context-per-thread anyway...
	public:
		//	The location of the property (attribute/uniform/etc), derived by quering a GL program (the id of which is also cached).  read-only outside this object!
		int32_t			loc = -1;
		//	The name of the attribute
		std::string		name;
		//	The id of the GLSL program that was checked to create the current loc
		int32_t			prog = -1;
	public:
		GLCachedProperty(std::string & inName) : name(inName)	{}
		GLCachedProperty(const std::string & inName) : name(inName)	{}
		GLCachedProperty(const GLCachedProperty & n) : loc(n.loc), name(n.name), prog(n.prog) {}
		virtual ~GLCachedProperty();
	public:
		//!	Pure virtual function, subclasses *must* implement this.  This is where the GL stuff specific to the subclass is performed.  A valid GL context *must* be current before you call this function- subclasses may have additional requirements.
		virtual void cacheTheLoc(const int32_t & inPgmToCheck);
		inline void purgeCache() { loc=-1; prog=-1; }
	public:
		//!	Returns the location of the property cached by the receiver.  A valid GL context must be current before you call this function.  Caches the location if it hasn't been looked up/cached yet.  Will only return -1 if there's a problem (if the attrib doesn't exist in the current program in use by the current context).
		int32_t location(const int32_t & inGLProgram) { if (loc<0 || inGLProgram<0 || inGLProgram!=prog) cacheTheLoc(inGLProgram); return loc; }
		std::string getDescriptionString() const { return FmtString("<GLCachedProperty \"%s\", %d>",this->name.c_str(),this->loc); }
		friend std::ostream & operator<<(std::ostream & os, const GLCachedProperty & n) { os<<n.getDescriptionString();return os; }
};




/*!
\ingroup VVGL_MISC
\brief Subclass of GLCachedProperty that caches the location of an attribute in a GLSL program.
*/
struct VVGL_EXPORT GLCachedAttrib : GLCachedProperty	{
	public:
		GLCachedAttrib(std::string & inName) : GLCachedProperty(inName) {}
		GLCachedAttrib(const std::string & inName) : GLCachedProperty(inName) {}
		GLCachedAttrib(const GLCachedAttrib & n) : GLCachedProperty(n) {}
	public:
		std::string getDescriptionString() const { return FmtString("<GLCachedAttrib \"%s\", %d>",this->name.c_str(),this->loc); }
		//!	Enables the attribute.  A valid GL context must be current and the program this attribute refers to must be bound before you call this function!
		void enable();
		//!	Disables the attribute.  A valid GL context must be current and the program this attribute refers to must be bound before you call this function!  Protip: don't call this function if you're using VAOs to draw.
		void disable();
		
		//!	Caches the location of the receiver's attribute in the passed program.  A valid GL context must be current and the program this attribute refers to must be bound before you call this function!
		void cacheTheLoc(const int32_t & inPgmToCheck) override;
};




/*!
\ingroup VVGL_MISC
\brief Subclass of GLCachedProperty that caches the location of a uniform variable in a GLSL program.
*/
struct VVGL_EXPORT GLCachedUni : GLCachedProperty	{
	public:
		GLCachedUni(std::string & inName) : GLCachedProperty(inName) {}
		GLCachedUni(const std::string & inName) : GLCachedProperty(inName) {}
		GLCachedUni(const GLCachedUni & n) : GLCachedProperty(n) {}
		//~GLCachedUni() {}
	public:
		std::string getDescriptionString() const { return FmtString("<GLCachedUni \"%s\", %d>",this->name.c_str(),this->loc); }
		
		//!	Caches the location of the receiver's uniform in the passed program.  A valid GL context must be current and the program this uniform refers to must be bound before you call this function!
		void cacheTheLoc(const int32_t & inPgmToCheck) override;
};




}


#endif /* VVGL_GLCachedUni_h */
