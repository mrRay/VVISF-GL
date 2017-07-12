#include "VVGLCachedAttrib.hpp"
#include "VVGLContext.hpp"




namespace VVGL
{


using namespace std;




/*
VVGLCachedAttrib::VVGLCachedAttrib()	{
	name = string("");
}
*/
VVGLCachedAttrib::VVGLCachedAttrib(string & inName)	{
	name = inName;
}
VVGLCachedAttrib::VVGLCachedAttrib(const string & inName)	{
	name = inName;
}
void VVGLCachedAttrib::cacheTheLoc(const int32_t & inPgmToCheck)	{
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
void VVGLCachedAttrib::purgeCache()	{
	loc = -1;
	prog = -1;
}
*/
void VVGLCachedAttrib::enable()	{
	if (loc >= 0)	{
		glEnableVertexAttribArray(loc);
		GLERRLOG
	}
}
void VVGLCachedAttrib::disable()	{
	if (loc >= 0)	{
		glDisableVertexAttribArray(loc);
		GLERRLOG
	}
}




}
