#include "GLCachedProperty.hpp"
#include "GLContext.hpp"




namespace VVGL
{


using namespace std;




GLCachedProperty::GLCachedProperty(string & inName)	{
	name = inName;
}
GLCachedProperty::GLCachedProperty(const string & inName)	{
	name = inName;
}
/*
void GLCachedProperty::cacheTheLoc(const int32_t & inPgmToCheck)	{
	//	subclasses of this should override this member function and cache the location and program here as appropriate
}
*/



void GLCachedAttrib::cacheTheLoc(const int32_t & inPgmToCheck)	{
	//cout << __PRETTY_FUNCTION__ << endl;
	if (inPgmToCheck < 0)	{
		//cout << "\terr: cannot cache, no program!\n";
		prog = -1;
		loc = -1;
		return;
	}
	prog = inPgmToCheck;
	loc = glGetAttribLocation(prog, name.c_str());
	GLERRLOG
	if (loc < 0)	{
		//cout << "\terr: checked, attrib for \"" << name << "\" not present.\n";
		prog = -1;
	}
}
void GLCachedAttrib::enable()	{
	if (loc >= 0)	{
		glEnableVertexAttribArray(loc);
		GLERRLOG
	}
	else
		cout << "\terr: can't enable, loc is " << loc << " in " << __PRETTY_FUNCTION__ << endl;
}
void GLCachedAttrib::disable()	{
	if (loc >= 0)	{
		glDisableVertexAttribArray(loc);
		GLERRLOG
	}
	else
		cout << "\terr: can't disable, loc is " << loc << " in " << __PRETTY_FUNCTION__ << endl;
}




void GLCachedUni::cacheTheLoc(const int32_t & inPgmToCheck)	{
	if (inPgmToCheck < 0)	{
		//cout << "\terr: cannot cache, no program!\n";
		prog = -1;
		loc = -1;
		return;
	}
	prog = inPgmToCheck;
	loc = glGetUniformLocation(prog, name.c_str());
	GLERRLOG
	if (loc < 0)	{
		//cout << "\terr: checked, uni for \"" << name << "\" not present.\n";
		prog = -1;
	}
}




}
