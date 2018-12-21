#include "GLContext.hpp"

#include <iostream>
//#include <cassert>
#include <regex>



#if defined(VVGL_SDK_MAC)
#include "GLContext_Mac.txt"
#elif defined(VVGL_SDK_GLFW)
#include "GLContext_GLFW.txt"
#elif defined(VVGL_SDK_RPI)
#include "GLContext_RPI.txt"
#elif defined(VVGL_SDK_QT)
#include "GLContext_Qt.txt"
#endif




namespace VVGL
{


using namespace std;


#pragma mark ******************************************** COMMON


void GLContext::calculateVersion()	{
	//cout << __PRETTY_FUNCTION__ << endl;
	version = GLVersion_Unknown;
#if defined(VVGL_SDK_MAC) || defined(VVGL_SDK_IOS) || defined(VVGL_SDK_RPI) || defined(VVGL_SDK_QT)
	if (ctx == nullptr)
		return;
#elif defined(VVGL_SDK_GLFW)
	if (win == nullptr)
		return;
#endif
	makeCurrentIfNotCurrent();
	const unsigned char			*versString = glGetString(GL_VERSION);
	GLERRLOG
	//cout << "\tversion string is " << versString << endl;
	switch (*versString)	{
	case '2': version = GLVersion_2; break;
	case '3': version = GLVersion_33; break;
	case '4': version = GLVersion_4; break;
	default:
		{
			string			baseString = string((const char *)versString);
			regex			regexJustES("[gG][lL][\\s]*[eE][sS]");
			//	if the base string looks like an OpenGL ES string...
			if (regex_search(baseString, regexJustES))	{
				regex			regexESVsn("[gG][lL][\\s]*[eE][sS][\\s]*([0-9]+)");
				smatch			matches;
				//	if we were able to extract some kind of vsn for GL ES
				if (regex_search(baseString, matches, regexESVsn) && matches.size()>=2)	{
					int				majorVsn = atoi(matches[1].str().c_str());
					switch (majorVsn)	{
					case 2:
						version = GLVersion_ES2;
						break;
					case 3:
						version = GLVersion_ES3;
						break;
					default:
						version = GLVersion_ES2;
						break;
					}
				}
				//	else we weren't able to extract any kind of vsn for GL ES- something's wrong, the full regex isn't matched
				else	{
					//cout << "\terr: matched base string (" << baseString << ") didn't match expanded\n";
					version = GLVersion_ES2;
				}
			}
			//	else the base string doesn't look like a GL ES string
			else	{
				//cout << "\terr: base string (" << baseString << ") not recognized\n";
				//	...i have no idea what goes here, need to test more hardware.  for now, fall back to GL 2.
				version = GLVersion_2;
			}
		}
		break;
	}
	const char			*rendererString = reinterpret_cast<const char *>(glGetString(GL_RENDERER));
	GLERRLOG
	//cout << "\traw renderer string is " << rendererString << endl;
	_renderer = std::string(rendererString);
}




}
