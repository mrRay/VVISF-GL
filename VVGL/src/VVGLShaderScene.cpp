#include "VVGLShaderScene.hpp"

//#import <OpenGL/CGLMacro.h>




namespace VVGL
{


using namespace std;




/*	========================================	*/
#pragma mark --------------------- constructor/destructor


VVGLShaderScene::VVGLShaderScene()
: VVGLScene()	{
	//cout << __PRETTY_FUNCTION__ << endl;
}
VVGLShaderScene::VVGLShaderScene(const VVGLContext * inCtx)
: VVGLScene(inCtx)	{

}

void VVGLShaderScene::prepareToBeDeleted()	{
	//cout << __PRETTY_FUNCTION__ << endl;
	
	//	lock, delete the program and shaders if they exist
	{
		lock_guard<recursive_mutex>		lock(renderLock);
		if (context != nullptr)	{
			context->makeCurrentIfNotCurrent();
			if (program > 0)	{
				glDeleteProgram(program);
				GLERRLOG
			}
			if (vs > 0)	{
				glDeleteShader(vs);
				GLERRLOG
			}
			if (fs > 0)	{
				glDeleteShader(fs);
				GLERRLOG
			}
		}
		vsString = string("");
		fsString = string("");
		vsStringUpdated = true;
		fsStringUpdated = true;
	}
	
	{
		lock_guard<mutex>		lock(errDictLock);
		errDict.clear();
	}
	
	//	now call the super, which deletes the context
	VVGLScene::prepareToBeDeleted();
}

VVGLShaderScene::~VVGLShaderScene()	{
	//cout << __PRETTY_FUNCTION__ << endl;
	if (!deleted)
		prepareToBeDeleted();
}


/*	========================================	*/
#pragma mark --------------------- public methods


void VVGLShaderScene::setVertexShaderString(const string & n)	{
	//cout << __PRETTY_FUNCTION__ << endl;
	//cout << "\tstring was:************************\n" << n << "\n************************\n";
	if (n.size()<1)	{
		cout << "\terr: passed a nil string, " << __PRETTY_FUNCTION__ << endl;
		//	throw an exception
		return;
	}
	
	lock_guard<recursive_mutex>		lock(renderLock);
	
	vsString = string(n);
	vsStringUpdated = true;
}
string VVGLShaderScene::getVertexShaderString()	{
	lock_guard<recursive_mutex>		lock(renderLock);
	return vsString;
}
void VVGLShaderScene::setFragmentShaderString(const string & n)	{
	//cout << __PRETTY_FUNCTION__ << endl;
	//cout << "\tstring was:************************\n" << n << "\n************************\n";
	if (n.size()<1)	{
		cout << "\terr: passed a nil string, " << __PRETTY_FUNCTION__ << endl;
		//	throw an exception
		return;
	}
	
	lock_guard<recursive_mutex>		lock(renderLock);
	
	fsString = string(n);
	fsStringUpdated = true;
}
string VVGLShaderScene::getFragmentShaderString()	{
	lock_guard<recursive_mutex>		lock(renderLock);
	return fsString;
}


/*	========================================	*/
#pragma mark --------------------- protected rendering methods


void VVGLShaderScene::_renderPrep()	{
	//cout << __PRETTY_FUNCTION__ << endl;
	
	VVGLScene::_renderPrep();
	
	if (context == nullptr)	{
		cout << "\terr: bailing, ctx null, " << __PRETTY_FUNCTION__ << endl;
		//cout << "\t" << __PRETTY_FUNCTION__ << "- FINISHED" << endl;
		return;
	}
	
	if (vsStringUpdated || fsStringUpdated)	{
		glUseProgram(0);
		GLERRLOG
		
		if (program > 0)	{
			glDeleteProgram(program);
			GLERRLOG
			program = 0;
		}
		if (vs > 0)	{
			glDeleteShader(vs);
			GLERRLOG
			vs = 0;
		}
		if (fs > 0)	{
			glDeleteShader(fs);
			GLERRLOG
			fs = 0;
		}
		
		
		
		{
			lock_guard<mutex>		lock(errDictLock);
			errDict.clear();
		}
		
		bool			encounteredError = false;
		if (vsString.size() > 0)	{
			vs = glCreateShader(GL_VERTEX_SHADER);
			GLERRLOG
			const char		*shaderSrc = vsString.c_str();
			glShaderSource(vs, 1, &shaderSrc, NULL);
			GLERRLOG
			glCompileShader(vs);
			GLERRLOG
			int32_t			compiled;
			glGetShaderiv(vs, GL_COMPILE_STATUS, &compiled);
			GLERRLOG
			if (!compiled)	{
				int32_t			length;
				char			*log;
				glGetShaderiv(vs, GL_INFO_LOG_LENGTH, &length);
				GLERRLOG
				log = new char[length+1];
				glGetShaderInfoLog(vs, length, &length, log);
				GLERRLOG
				cout << "\terr compiling vertex shader in " << __PRETTY_FUNCTION__ << endl;
				cout << "\terr: " << log << endl;
				encounteredError = true;
				
				{
					lock_guard<mutex>		lock(errDictLock);
					errDict.insert(pair<string,string>(string("vertErrLog"), string(log)));
					errDict.insert(pair<string,string>(string("vertSrc"), string(vsString)));
				}
				
				delete [] log;
				glDeleteShader(vs);
				GLERRLOG
				vs = 0;
			}
		}
		if (fsString.size() > 0)	{
			fs = glCreateShader(GL_FRAGMENT_SHADER);
			GLERRLOG
			const char		*shaderSrc = fsString.c_str();
			glShaderSource(fs, 1, &shaderSrc, NULL);
			GLERRLOG
			glCompileShader(fs);
			GLERRLOG
			int32_t			compiled;
			glGetShaderiv(fs, GL_COMPILE_STATUS, &compiled);
			GLERRLOG
			if (!compiled)	{
				int32_t			length;
				char			*log;
				glGetShaderiv(fs, GL_INFO_LOG_LENGTH, &length);
				GLERRLOG
				log = new char[length+1];
				glGetShaderInfoLog(fs, length, &length, log);
				GLERRLOG
				cout << "\terr compiling fragment shader in " << __PRETTY_FUNCTION__ << endl;
				cout << "\terr: " << log << endl;
				encounteredError = true;
				
				{
					lock_guard<mutex>		lock(errDictLock);
					errDict.insert(pair<string,string>(string("fragErrLog"), string(log)));
					errDict.insert(pair<string,string>(string("fragSrc"), string(fsString)));
				}
				
				delete [] log;
				glDeleteShader(fs);
				GLERRLOG
				fs = 0;
			}
		}
		if ((vs>0 || fs>0) && !encounteredError)	{
			program = glCreateProgram();
			GLERRLOG
			if (vs > 0)	{
				glAttachShader(program, vs);
				GLERRLOG
			}
			if (fs > 0)	{
				glAttachShader(program, fs);
				GLERRLOG
			}
			glLinkProgram(program);
			GLERRLOG
			
			int32_t			linked;
			glGetProgramiv(program, GL_LINK_STATUS, &linked);
			GLERRLOG
			if (!linked)	{
				int32_t			length;
				char			*log;
				glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length);
				GLERRLOG
				log = new char(length);
				glGetProgramInfoLog(program, length, &length, log);
				GLERRLOG
				cout << "\terr linking program in " << __PRETTY_FUNCTION__ << endl;
				cout << "\terr: " << log << endl;
				encounteredError = true;
				
				{
					lock_guard<mutex>		lock(errDictLock);
					errDict.insert(pair<string,string>(string("linkErrLog"), string(log)));
				}
				
				delete log;
				glDeleteProgram(program);
				GLERRLOG
				program = 0;
			}
		}
		
		vsStringUpdated = false;
		fsStringUpdated = false;
	}
	
	if (program > 0)	{
		glUseProgram(program);
		GLERRLOG
	}
	
	//cout << "\t" << __PRETTY_FUNCTION__ << "- FINISHED" << endl;
}

void VVGLShaderScene::_initialize()	{
	if (deleted)
		return;
	
	VVGLScene::_initialize();
	
	if (context == nullptr)
		return;
	glDisable(GL_BLEND);
	GLERRLOG
}

void VVGLShaderScene::_renderCleanup()	{
	//cout << __PRETTY_FUNCTION__ << endl;
	if (deleted)
		return;
	
	if (context != nullptr)	{
#if ISF_TARGET_MAC
		glBindTexture(GL_TEXTURE_RECTANGLE_EXT, 0);
		GLERRLOG
#endif
		//glBindTexture(GL_TEXTURE_2D, 0);
		//GLERRLOG
		glUseProgram(0);
		GLERRLOG
	}
	
	VVGLScene::_renderCleanup();
}




}
