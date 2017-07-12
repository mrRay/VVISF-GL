#include "VVGLCachedUni.hpp"
#include "VVGLContext.hpp"




namespace VVGL
{


using namespace std;




/*
VVGLCachedUni::VVGLCachedUni()	{
	name = string("");
}
*/
VVGLCachedUni::VVGLCachedUni(string & inName)	{
	name = inName;
}
VVGLCachedUni::VVGLCachedUni(const string & inName)	{
	name = inName;
}
void VVGLCachedUni::cacheTheLoc(const int32_t & inPgmToCheck)	{
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
void VVGLCachedUni::purgeCache()	{
	loc = -1;
	prog = -1;
}
*/
void VVGLCachedUni::enable()	{
	if (loc >= 0)	{
		glEnableVertexAttribArray(loc);
		GLERRLOG
	}
}
void VVGLCachedUni::disable()	{
	if (loc >= 0)	{
		glDisableVertexAttribArray(loc);
		GLERRLOG
	}
}




}
