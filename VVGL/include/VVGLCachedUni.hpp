#ifndef VVGLCachedUni_h
#define VVGLCachedUni_h

#include <iostream>

#include "VVStringUtils.hpp"




namespace VVGL
{




using namespace std;




/*	the goal of this class is to create an object that fetches & caches the location of a uniform in 
a GL program.  i want to minimize the number of times i need to query the GL environment for this 
value, and i can either implement it as a class or i can write the same basic logic every time i 
want to track the location of a uniform.				*/


struct VVGLCachedUni	{
	//	these vars are public and that's technically not safe, but practically speaking this works out because everything GL-related has to be serialized such that access is one-context-per-thread anyway...
	public:
		int32_t			loc = -1;	//	the location of the attribute, derived by quering a GL program (the id of which is also cached).  read-only outside this object!
		string			name;	//	the name of the attribute
		int32_t			prog = -1;	//	the id of the pgm that was checked to create the current loc
	public:
		//VVGLCachedUni();
		VVGLCachedUni(string & inName);
		VVGLCachedUni(const string & inName);
		VVGLCachedUni(const VVGLCachedUni & n) : loc(n.loc), name(n.name), prog(n.prog) {}
	public:
		//	a valid GL context must be current before you call this function.
		void cacheTheLoc(const int32_t & inPgmToCheck);
		inline void purgeCache() { loc=-1; prog=-1; }
	public:
		//inline int32_t loc() const { return loc; }
		//	a valid GL context must be current before you call this function.  caches the loc if it hasn't been done yet, will only return -1 if there's a problem (if the attrib doesn't exist in the current program in use by the current context)
		inline int32_t location(const int32_t & inGLProgram) { if (loc<0 || inGLProgram<0 || inGLProgram!=prog) cacheTheLoc(inGLProgram); return loc; }
		//	a valid GL context must be current before you call this function.
		//void enable();
		//	a valid GL context must be current before you call this function.  protip: don't call this function if you're using VAOs to draw.
		//void disable();
		inline string getDescriptionString() const { return FmtString("<VVGLCachedUni \"%s\", %d>",this->name.c_str(),this->loc); }
		friend ostream & operator<<(ostream & os, const VVGLCachedUni & n) { os<<n.getDescriptionString();return os; }
};




}


#endif /* VVGLCachedUni_h */
