#include "GLCachedAttrib.hpp"
#include "GLContext.hpp"




namespace VVGL
{


using namespace std;




/*
GLCachedAttrib::GLCachedAttrib()	{
	name = string("");
}
*/
GLCachedAttrib::GLCachedAttrib(string & inName)	{
	name = inName;
}
GLCachedAttrib::GLCachedAttrib(const string & inName)	{
	name = inName;
}
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
/*
void GLCachedAttrib::purgeCache()	{
	loc = -1;
	prog = -1;
}
*/
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




}
