#ifndef GLShaderScene_hpp
#define GLShaderScene_hpp

#include "VVGLScene.hpp"

#include <map>




namespace VVGL	{


using namespace std;




class VVGLShaderScene : public VVGLScene	{
		
	protected:
		string			vsString = string("");
		string			fsString = string("");
	protected:
		bool			vsStringUpdated = false;
		bool			fsStringUpdated = false;
		
		uint32_t		program = 0;
		uint32_t		vs = 0;
		uint32_t		fs = 0;
		
		mutex			errLock;
		mutex			errDictLock;
		map<string,string>		errDict;
	
	public:
		VVGLShaderScene();
		VVGLShaderScene(const VVGLContextRef & inCtx);
		//VVGLShaderScene(const VVGLContext * inSharedCtx=nullptr, const Size & inSize={640.,480.});
		virtual ~VVGLShaderScene();
		
		virtual void prepareToBeDeleted();
		
		virtual void setVertexShaderString(const string & n);
		virtual string getVertexShaderString();
		virtual void setFragmentShaderString(const string & n);
		virtual string getFragmentShaderString();
		inline uint32_t getProgram() const { return program; }
		inline uint32_t getVS() const { return vs; }
		inline uint32_t getFS() const { return fs; }
		
	protected:
		virtual void _renderPrep();
		virtual void _initialize();
		virtual void _renderCleanup();
};




}


#endif /* GLShaderScene_hpp */
