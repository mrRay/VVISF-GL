#include "GLCachedUni.hpp"
#include "GLContext.hpp"




namespace VVGL
{


using namespace std;




/*
GLCachedUni::GLCachedUni()	{
	name = string("");
}
*/
GLCachedUni::GLCachedUni(string & inName)	{
	name = inName;
}
GLCachedUni::GLCachedUni(const string & inName)	{
	name = inName;
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
/*
void GLCachedUni::purgeCache()	{
	loc = -1;
	prog = -1;
}
*/
/*
void GLCachedUni::enable()	{
	if (loc >= 0)	{
		glEnableVertexAttribArray(loc);
		GLERRLOG
	}
}
void GLCachedUni::disable()	{
	if (loc >= 0)	{
		glDisableVertexAttribArray(loc);
		GLERRLOG
	}
}
*/



}
