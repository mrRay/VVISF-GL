#include "GLScene.hpp"
#include <cstring>



#if defined(VVGL_SDK_WIN)
#undef far
#undef near
#endif	//	VVGL_SDK_WIN




namespace VVGL
{


using namespace std;




/*	========================================	*/
#pragma mark --------------------- create/destroy


GLScene::GLScene()	{
	//cout << __PRETTY_FUNCTION__ << endl;
	GLBufferPoolRef		bp = GetGlobalBufferPool();
	if (bp != nullptr)	{
		_context = bp->context()->newContextSharingMe();
		//cout << "\tcontext is " << *_context << endl;
	}

#if defined(VVGL_SDK_WIN)
	_alwaysNeedsReshape = true;
#endif	//	VVGL_SDK_WIN
}
GLScene::GLScene(const GLContextRef & inCtx)	{
	//_context = new GLContext(inCtx);
	_context = inCtx;
}

void GLScene::prepareToBeDeleted()	{
	//cout << __PRETTY_FUNCTION__ << "->" << this << endl;
	
	//	lock, delete the program and shaders if they exist
	{
		lock_guard<recursive_mutex>		lock(_renderLock);
		if (_context != nullptr)	{
			_context->makeCurrentIfNotCurrent();
			if (_program > 0)	{
				glDeleteProgram(_program);
				GLERRLOG
			}
			if (_vs > 0)	{
				glDeleteShader(_vs);
				GLERRLOG
			}
			if (_gs > 0)	{
				glDeleteShader(_gs);
				GLERRLOG
			}
			if (_fs > 0)	{
				glDeleteShader(_fs);
				GLERRLOG
			}
		}
		if (_vsString != nullptr)	{
			delete _vsString;
			_vsString = nullptr;
		}
		if (_gsString != nullptr)	{
			delete _gsString;
			_gsString = nullptr;
		}
		if (_fsString != nullptr)	{
			delete _fsString;
			_fsString = nullptr;
		}
		_vsStringUpdated = true;
		_gsStringUpdated = true;
		_fsStringUpdated = true;
	}
	//	lock, delete the error dict
	{
		lock_guard<mutex>		lock(_errDictLock);
		_errDict.clear();
	}
	
	_renderPrepCallback = nullptr;
	_renderCallback = nullptr;
	_renderCleanupCallback = nullptr;
	
	_deleted = true;
}

GLScene::~GLScene()	{
	//cout << __PRETTY_FUNCTION__ << "->" << this << endl;
	if (!_deleted)
		prepareToBeDeleted();
	
	if (_context != nullptr)	{
		//delete _context;
		_context = nullptr;
	}
}


/*	========================================	*/
#pragma mark --------------------- rendering methods




GLBufferRef GLScene::createAndRenderABuffer(const Size & inSize, const GLBufferPoolRef & inPool)	{
	//cout << __PRETTY_FUNCTION__ << endl;
	if (inSize.width <= 0. || inSize.height <= 0.)
		return nullptr;
	
	GLBufferPoolRef		bp = inPool;
	if (bp == nullptr)	{
		if (_privatePool != nullptr)
			bp = _privatePool;
		else
			bp = GetGlobalBufferPool();
	}
	if (bp == nullptr)
		return nullptr;
	
	//	set the orthogonal size
	setOrthoSize(inSize);
	//	make the buffers i'll be rendering into
#if defined(VVGL_SDK_RPI)
	RenderTarget		tmpTarget(CreateFBO(false, bp), CreateRGBATex(_orthoSize, false, bp), nullptr);
#else
	RenderTarget		tmpTarget(CreateFBO(false, bp), CreateRGBATex(_orthoSize, false, bp), CreateDepthBuffer(_orthoSize, false, bp));
#endif
	//	render
	render(tmpTarget);
	
	return tmpTarget.color;
}
void GLScene::renderToBuffer(const GLBufferRef & inBuffer)	{
	//cout << __PRETTY_FUNCTION__ << ", passed buffer is " << inBuffer << endl;
	
	GLBufferPoolRef		bp = _privatePool;
	if (bp == nullptr)
		bp = GetGlobalBufferPool();
	
	if (bp == nullptr)
		return;
	
	//	set the orthogonal size
	if (inBuffer != nullptr)
		setOrthoSize(inBuffer->srcRect.size);
	//	make the buffers i'll be rendering into
#if defined(VVGL_SDK_RPI)
	RenderTarget		tmpTarget(CreateFBO(false, bp), inBuffer, nullptr);
#else
	RenderTarget		tmpTarget(CreateFBO(false, bp), inBuffer, CreateDepthBuffer(_orthoSize, false, bp));
#endif
	//	render
	render(tmpTarget);
}


void GLScene::render(const RenderTarget & inRenderTarget)	{
	//cout << __PRETTY_FUNCTION__ << endl;
	//cout << "\trender target is " << inRenderTarget.fboName() << ", " << inRenderTarget.colorName() << ", " << inRenderTarget.depthName() << endl;
	//	get a lock, set the current GL context
	lock_guard<recursive_mutex>		lock(_renderLock);
	if (_context == nullptr)	{
		cout << "\terr: bailing, ctx null, " << __PRETTY_FUNCTION__ << endl;
		return;
	}
	_context->makeCurrentIfNotCurrent();
	
	//	update the member var for the fbo attachments
	_renderTarget = inRenderTarget;
	//	prep for render
	_renderPrep();
	
	//	if there isn't a valid program then we shouldn't execute the client-provided render callback
	bool		looksLikeItShouldHaveAProgram = ((_vsString!=nullptr && _vsString->size()>0) || (_gsString!=nullptr && _gsString->size()>0) || (_fsString!=nullptr && _fsString->size()>0));
	if ((_program==0 && !looksLikeItShouldHaveAProgram) ||	(_program!=0 && looksLikeItShouldHaveAProgram))	{
		//	execute the render callback
		if (_renderCallback != nullptr)
			_renderCallback(*this);
	}
	
	//	cleanup after render
	_renderCleanup();
	
	//	update the member vars for the fbo attachments
	_renderTarget = RenderTarget();
}


void GLScene::renderBlackFrame(const RenderTarget & inRenderTarget)	{
	//	get a lock, set the current GL context
	lock_guard<recursive_mutex>		lock(_renderLock);
	if (_context == nullptr)	{
		cout << "\terr: bailing, ctx null, " << __PRETTY_FUNCTION__ << endl;
		return;
	}
	_context->makeCurrentIfNotCurrent();
	
	//	update the member var for the fbo attachments
	_renderTarget = inRenderTarget;
	//	prep for render
	_renderPrep();
	
	//	clear the context
	glClearColor(0., 0., 0., 0.);
	GLERRLOG
	uint32_t		mask = GL_COLOR_BUFFER_BIT;
#if !defined(VVGL_SDK_IOS) && !defined(VVGL_SDK_RPI)
	mask |= GL_DEPTH_BUFFER_BIT;
#endif
	glClear(mask);
	GLERRLOG
	_clearColorUpdated = true;
	
	//	cleanup after render
	_renderCleanup();
	
	//	update the member vars for the fbo attachments
	_renderTarget = RenderTarget();
}
void GLScene::renderOpaqueBlackFrame(const RenderTarget & inRenderTarget)	{
	//	get a lock, set the current GL context
	lock_guard<recursive_mutex>		lock(_renderLock);
	if (_context == nullptr)	{
		cout << "\terr: bailing, ctx null, " << __PRETTY_FUNCTION__ << endl;
		return;
	}
	_context->makeCurrentIfNotCurrent();
	
	//	update the member var for the fbo attachments
	_renderTarget = inRenderTarget;
	//	prep for render
	_renderPrep();
	
	//	get the context, clear
	glClearColor(0., 0., 0., 1.);
	GLERRLOG
	uint32_t		mask = GL_COLOR_BUFFER_BIT;
#if !defined(VVGL_SDK_IOS) && !defined(VVGL_SDK_RPI)
	mask |= GL_DEPTH_BUFFER_BIT;
#endif
	glClear(mask);
	GLERRLOG
	_clearColorUpdated = true;
	
	//	cleanup after render
	_renderCleanup();
	
	//	update the member vars for the fbo attachments
	_renderTarget = RenderTarget();
}
void GLScene::renderRedFrame(const RenderTarget & inRenderTarget)	{
	//	get a lock, set the current GL context
	lock_guard<recursive_mutex>		lock(_renderLock);
	if (_context == nullptr)	{
		cout << "\terr: bailing, ctx null, " << __PRETTY_FUNCTION__ << endl;
		return;
	}
	_context->makeCurrentIfNotCurrent();
	
	//	update the member var for the fbo attachments
	_renderTarget = inRenderTarget;
	//	prep for render
	_renderPrep();
	
	//	get the context, clear
	glClearColor(1., 0., 0., 1.);
	GLERRLOG
	uint32_t		mask = GL_COLOR_BUFFER_BIT;
#if !defined(VVGL_SDK_IOS) && !defined(VVGL_SDK_RPI)
	if (inRenderTarget.depth != nullptr)	{
		mask |= GL_DEPTH_BUFFER_BIT;
	}
#endif
	glClear(mask);
	GLERRLOG
	_clearColorUpdated = true;
	
	//	cleanup after render
	_renderCleanup();
	
	//	update the member vars for the fbo attachments
	_renderTarget = RenderTarget();
}


/*	========================================	*/
#pragma mark --------------------- setter/getter methods


void GLScene::setRenderPrepCallback(const RenderPrepCallback & n)	{
	_renderPrepCallback = n;
}
void GLScene::setRenderPreLinkCallback(const RenderCallback & n)	{
	_renderPreLinkCallback = n;
}
void GLScene::setRenderCallback(const RenderCallback & n)	{
	_renderCallback = n;
}
void GLScene::setRenderCleanupCallback(const RenderCallback & n)	{
	_renderCleanupCallback = n;
}


void GLScene::setAlwaysNeedsReshape(const bool & n)	{
	lock_guard<recursive_mutex>		lock(_renderLock);
	_alwaysNeedsReshape = n;
}
void GLScene::setOrthoSize(const Size & n)	{
	lock_guard<recursive_mutex>		lock(_renderLock);
	_orthoSize = n;
	_needsReshape = true;
}
Size GLScene::orthoSize() const	{
	return _orthoSize;
}
void GLScene::setOrthoFlipped(const bool & n)	{
	lock_guard<recursive_mutex>		lock(_renderLock);
	_orthoFlipped = n;
	_needsReshape = true;
}
bool GLScene::orthoFlipped() const	{
	return _orthoFlipped;
}


void GLScene::setClearColor(const GLColor & n)	{
	if (_clearColor == n)
		return;
	_clearColorUpdated = true;
	_clearColor = n;
}
void GLScene::setClearColor(const float & r, const float & g, const float & b, const float & a)	{
	setClearColor(GLColor(r,g,b,a));
}
void GLScene::setClearColor(float * n)	{
	setClearColor(GLColor(*(n+0), *(n+1), *(n+2), *(n+3)));
}
void GLScene::setPerformClear(const bool & n)	{
	_performClear = n;
}
void GLScene::setVertexShaderString(const string & n)	{
	//cout << __PRETTY_FUNCTION__ << endl;
	//cout << "\tstring was:************************\n" << n << "\n************************\n";
	if (n.size()<1)	{
		cout << "\terr: passed a nil string, " << __PRETTY_FUNCTION__ << endl;
		//	throw an exception
		return;
	}
	
	lock_guard<recursive_mutex>		lock(_renderLock);
	
	if (_vsString != nullptr)
		delete _vsString;
	_vsString = new string(n);
	_vsStringUpdated = true;
	_needsReshape = true;
	_orthoUni.purgeCache();
}
string GLScene::vertexShaderString()	{
	lock_guard<recursive_mutex>		lock(_renderLock);
	if (_vsString == nullptr)
		return string("");
	return string(*_vsString);
}
void GLScene::setGeometryShaderString(const string & n)	{
	//cout << __PRETTY_FUNCTION__ << endl;
	//cout << "\tstring was:************************\n" << n << "\n************************\n";
	if (n.size()<1)	{
		cout << "\terr: passed a nil string, " << __PRETTY_FUNCTION__ << endl;
		//	throw an exception
		return;
	}
	
	lock_guard<recursive_mutex>		lock(_renderLock);
	
	if (_gsString != nullptr)
		delete _gsString;
	_gsString = new string(n);
	_gsStringUpdated = true;
	_needsReshape = true;
	_orthoUni.purgeCache();
}
string GLScene::geometryShaderString()	{
	lock_guard<recursive_mutex>		lock(_renderLock);
	if (_gsString == nullptr)
		return string("");
	return string(*_gsString);
}
void GLScene::setFragmentShaderString(const string & n)	{
	//cout << __PRETTY_FUNCTION__ << endl;
	//cout << "\tstring was:************************\n" << n << "\n************************\n";
	if (n.size()<1)	{
		cout << "\terr: passed a nil string, " << __PRETTY_FUNCTION__ << endl;
		//	throw an exception
		return;
	}
	
	lock_guard<recursive_mutex>		lock(_renderLock);
	
	if (_fsString != nullptr)
		delete _fsString;
	_fsString = new string(n);
	_fsStringUpdated = true;
	_needsReshape = true;
	_orthoUni.purgeCache();
}
string GLScene::fragmentShaderString()	{
	lock_guard<recursive_mutex>		lock(_renderLock);
	if (_fsString == nullptr)
		return string("");
	return string(*_fsString);
}
void GLScene::compileProgramIfNecessary()	{
	//	get a lock, set the current GL context
	lock_guard<recursive_mutex>		lock(_renderLock);
	if (_context == nullptr)	{
		cout << "\terr: bailing, ctx null, " << __PRETTY_FUNCTION__ << endl;
		return;
	}
	_context->makeCurrentIfNotCurrent();
	
	//	prep for render
	_renderPrep();
	
	//	cleanup after render
	_renderCleanup();
}


/*	========================================	*/
#pragma mark --------------------- protected rendering methods


void GLScene::_renderPrep()	{
	//cout << __PRETTY_FUNCTION__ << endl;
	//cout << "\tthis is " << this << endl;
	if (_deleted)	{
		//cout << "\t" << __PRETTY_FUNCTION__ << "- FINISHED" << endl;
		return;
	}
	
	bool			needsReshapeFlag = false;
	bool			pgmChangedFlag = false;
	
	//	initialize
	if (!_initialized)
		_initialize();
	
	//	if the vert/frag shader strings have been updated, they need to be recompiled & the program needs to be relinked
	if (_vsStringUpdated || _gsStringUpdated || _fsStringUpdated)	{
		pgmChangedFlag = true;
		
		glUseProgram(0);
		GLERRLOG
		
		if (_program > 0)	{
			glDeleteProgram(_program);
			GLERRLOG
			_program = 0;
		}
		if (_vs > 0)	{
			glDeleteShader(_vs);
			GLERRLOG
			_vs = 0;
		}
		if (_gs > 0)	{
			glDeleteShader(_gs);
			GLERRLOG
			_gs = 0;
		}
		if (_fs > 0)	{
			glDeleteShader(_fs);
			GLERRLOG
			_fs = 0;
		}
		_programReady = false;
		
		
		{
			lock_guard<mutex>		lock(_errDictLock);
			_errDict.clear();
		}
		
		bool			encounteredError = false;
		if (_vsString!=nullptr && _vsString->size() > 0)	{
			_vs = glCreateShader(GL_VERTEX_SHADER);
			GLERRLOG
			const char		*shaderSrc = _vsString->c_str();
			glShaderSource(_vs, 1, &shaderSrc, NULL);
			GLERRLOG
			glCompileShader(_vs);
			GLERRLOG
			int32_t			compiled;
			glGetShaderiv(_vs, GL_COMPILE_STATUS, &compiled);
			GLERRLOG
			if (!compiled)	{
				int32_t			length;
				char			*log;
				glGetShaderiv(_vs, GL_INFO_LOG_LENGTH, &length);
				GLERRLOG
				log = new char[length+1];
				memset(log, 0, sizeof(char) * (length+1));
				glGetShaderInfoLog(_vs, length, &length, log);
				GLERRLOG
				cout << "\terr compiling vertex shader in " << __PRETTY_FUNCTION__ << endl;
				//cout << "\terr: " << log << endl;
				//cout << "\traw shader is:\n" << *_vsString << endl;
				encounteredError = true;
				
				{
					lock_guard<mutex>		lock(_errDictLock);
					_errDict.insert(pair<string,string>(string("vertErrLog"), string(log)));
					_errDict.insert(pair<string,string>(string("vertSrc"), string(*_vsString)));
				}
				
				delete [] log;
				glDeleteShader(_vs);
				GLERRLOG
				_vs = 0;
			}
		}
		if (_gsString!=nullptr && _gsString->size() > 0)	{
#if !defined(VVGL_TARGETENV_GLES) && !defined(VVGL_TARGETENV_GLES3)
			_gs = glCreateShader(GL_GEOMETRY_SHADER);
			GLERRLOG
			const char		*shaderSrc = _gsString->c_str();
			glShaderSource(_gs, 1, &shaderSrc, NULL);
			GLERRLOG
			glCompileShader(_gs);
			GLERRLOG
			int32_t			compiled;
			glGetShaderiv(_gs, GL_COMPILE_STATUS, &compiled);
			GLERRLOG
			if (!compiled)	{
				int32_t			length;
				char			*log;
				glGetShaderiv(_gs, GL_INFO_LOG_LENGTH, &length);
				GLERRLOG
				log = new char[length+1];
				memset(log, 0, sizeof(char) * (length+1));
				glGetShaderInfoLog(_gs, length, &length, log);
				GLERRLOG
				cout << "\terr compiling geo shader in " << __PRETTY_FUNCTION__ << endl;
				//cout << "\terr: " << log << endl;
				//cout << "\traw shader is:\n" << *_gsString << endl;
				encounteredError = true;
				
				{
					lock_guard<mutex>		lock(_errDictLock);
					_errDict.insert(pair<string,string>(string("geoErrLog"), string(log)));
					_errDict.insert(pair<string,string>(string("geoSrc"), string(*_gsString)));
				}
				
				delete [] log;
				glDeleteShader(_gs);
				GLERRLOG
				_gs = 0;
			}
#endif
		}
		if (_fsString!=nullptr && _fsString->size() > 0)	{
			_fs = glCreateShader(GL_FRAGMENT_SHADER);
			GLERRLOG
			const char		*shaderSrc = _fsString->c_str();
			glShaderSource(_fs, 1, &shaderSrc, NULL);
			GLERRLOG
			glCompileShader(_fs);
			GLERRLOG
			int32_t			compiled;
			glGetShaderiv(_fs, GL_COMPILE_STATUS, &compiled);
			GLERRLOG
			if (!compiled)	{
				int32_t			length;
				char			*log;
				glGetShaderiv(_fs, GL_INFO_LOG_LENGTH, &length);
				GLERRLOG
				log = new char[length+1];
				memset(log, 0, sizeof(char) * (length+1));
				glGetShaderInfoLog(_fs, length, &length, log);
				GLERRLOG
				cout << "\terr compiling fragment shader in " << __PRETTY_FUNCTION__ << endl;
				//cout << "\terr: " << log << endl;
				//cout << "\traw shader is:\n" << *_fsString << endl;
				encounteredError = true;
				
				{
					lock_guard<mutex>		lock(_errDictLock);
					_errDict.insert(pair<string,string>(string("fragErrLog"), string(log)));
					_errDict.insert(pair<string,string>(string("fragSrc"), string(*_fsString)));
				}
				
				delete [] log;
				glDeleteShader(_fs);
				GLERRLOG
				_fs = 0;
			}
		}
		if ((_vs>0 || _gs>0 || _fs>0) && !encounteredError)	{
			_program = glCreateProgram();
			GLERRLOG
			if (_vs > 0)	{
				glAttachShader(_program, _vs);
				GLERRLOG
			}
			if (_gs > 0)	{
				glAttachShader(_program, _gs);
				GLERRLOG
			}
			if (_fs > 0)	{
				glAttachShader(_program, _fs);
				GLERRLOG
			}
			if (_renderPreLinkCallback != nullptr)
				_renderPreLinkCallback(*this);
			glLinkProgram(_program);
			GLERRLOG
			
			int32_t			linked;
			glGetProgramiv(_program, GL_LINK_STATUS, &linked);
			GLERRLOG
			if (!linked)	{
				int32_t			length;
				char			*log;
				glGetProgramiv(_program, GL_INFO_LOG_LENGTH, &length);
				GLERRLOG
				log = new char[length+1];
				//log = static_cast<char*>(malloc(sizeof(char) * (length+2)));
				memset(log, 0, sizeof(char) * (length+1));
				glGetProgramInfoLog(_program, length, &length, log);
				GLERRLOG
				cout << "********************\n";
				cout << "\terr linking _program in " << __PRETTY_FUNCTION__ << endl;
				cout << "\terr: " << log << endl;
				cout << "********************\n";
				/*
				cout << "\traw vert shader is:\n";
				if (_vsString == nullptr)
					cout << "<empty>\n";
				else
					cout << *_vsString << endl;
				cout << "********************\n";
				cout << "\traw geo shader is:\n";
				if (_gsString == nullptr)
					cout << "<empty>\n";
				else
					cout << *_gsString << endl;
				cout << "********************\n";
				cout << "\traw frag shader is:\n";
				if (_fsString == nullptr)
					cout << "<empty>\n";
				else
					cout << *_fsString << endl;
				cout << "********************\n";
				*/
				encounteredError = true;
				
				{
					lock_guard<mutex>		lock(_errDictLock);
					_errDict.insert(pair<string,string>(string("linkErrLog"), string(log)));
				}
				
				delete[] log;
				//free(log);
				glDeleteProgram(_program);
				GLERRLOG
				_program = 0;
			}
			else	{
				_programReady = true;
				_orthoUni.cacheTheLoc(_program);
			}
		}
		
		_vsStringUpdated = false;
		_gsStringUpdated = false;
		_fsStringUpdated = false;
		_needsReshape = true;
	}
	
	//	bind the attachments in the render target to the FBO (also in the render target)
	if (_renderTarget.fboName() > 0)	{
		glBindFramebuffer(GL_FRAMEBUFFER, _renderTarget.fboName());
		GLERRLOG
		
		//	attach the depth buffer
		if (_renderTarget.depthName() > 0)	{
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, _renderTarget.depthTarget(), _renderTarget.depthName(), 0);
			GLERRLOG
		}
		else	{
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, _renderTarget.depthTarget(), 0, 0);
			GLERRLOG
		}
		
		//	attach the color buffer
		if (_renderTarget.colorName() > 0)	{
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, _renderTarget.colorTarget(), _renderTarget.colorName(), 0);
			GLERRLOG
		}
		else	{
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, _renderTarget.colorTarget(), 0, 0);
			GLERRLOG
		}
		
		/*
		GLenum		check = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		GLERRLOG
		if (check == GL_FRAMEBUFFER_COMPLETE)	{
			//	intentionally blank, FBO complete is good!
		}
		else	{
			cout << "ERR: framebuffer check failed in " << __PRETTY_FUNCTION__ << endl;
			switch (check)	{
			case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:	cout << "ERR: incomplete attachment framebuffer\n"; break;
			//case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS:	NSLog(@"\t\terr: incomplete dimensions framebuffer"); break;
			case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:	cout << "ERR: incomplete missing attachment framebuffer\n"; break;
			case GL_FRAMEBUFFER_UNSUPPORTED:	cout << "ERR: unsupported framebuffer\n"; break;
			default:	cout << "ERR: unrecognized framebuffer error, " << check << endl; break;
			}
		}
		*/
	}
	
	//	clear
	if (_clearColorUpdated)	{
		glClearColor(_clearColor.r, _clearColor.g, _clearColor.b, _clearColor.a);
		GLERRLOG
		_clearColorUpdated = false;
	}
	if (_performClear)	{
		uint32_t		mask = GL_COLOR_BUFFER_BIT;
#if !defined(VVGL_SDK_IOS) && !defined(VVGL_SDK_RPI)
		mask |= GL_DEPTH_BUFFER_BIT;
#endif
		glClear(mask);
		GLERRLOG
	}
	
	//	if there's a program, use it
	if (_program > 0)	{
		glUseProgram(_program);
		GLERRLOG
	}
	
	//	reshape as needed (must do this after compiling the program, which caches the ortho uniform location)
	if (_alwaysNeedsReshape || _needsReshape)	{
		needsReshapeFlag = true;
		_reshape();
	}
	
	//	if there's a render prep callback, call it now- pass in flags indicating whether or not the scene has been reshaped (render size changed) and whether or not the program has been recompiled
	if (_renderPrepCallback != nullptr)
		_renderPrepCallback(*this, needsReshapeFlag, pgmChangedFlag);
	//cout << "\t" << __PRETTY_FUNCTION__ << "- FINISHED" << endl;
}

void GLScene::_initialize()	{
	//cout << __PRETTY_FUNCTION__ << endl;
	if (_deleted)
		return;
	if (_context == nullptr)
		return;
	
	//glEnable(GL_TEXTURE_RECTANGLE);
	//GLERRLOG
	//glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	//GLERRLOG
	if (glVersion() == GLVersion_2)	{
#if defined(VVGL_TARGETENV_GL2)
		glEnable(GL_BLEND);
		GLERRLOG
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		GLERRLOG
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
		GLERRLOG
#endif
	}
	//const int32_t		swap = 0;
	//CGLSetParameter(cgl_ctx, kCGLCPSwapInterval, &swap);
	
	//glHint(GL_CLIP_VOLUME_CLIPPING_HINT_EXT, GL_FASTEST);
	//GLERRLOG
	//glHint(GL_PERSPECTIVE_CORRECTION_HINT,GL_NICEST);
	//GLERRLOG
	//glDisable(GL_DEPTH_TEST);
	//GLERRLOG
	
	//glClearColor(0.0, 0.0, 0.0, 0.0);
	//GLERRLOG
	
	//glDisable(GL_BLEND);
	//GLERRLOG
	
	_initialized = true;
	_clearColorUpdated = true;
	
}

void GLScene::_reshape()	{
	//cout << __PRETTY_FUNCTION__ << endl;
	//cout << "\tthis is " << this << endl;
	
	if (_orthoSize.width>0. && _orthoSize.height>0.)	{
		if (glVersion() == GLVersion_2)	{
#if defined(VVGL_TARGETENV_GL2)
			glMatrixMode(GL_MODELVIEW);
			GLERRLOG
			glLoadIdentity();
			GLERRLOG
			glMatrixMode(GL_PROJECTION);
			GLERRLOG
			glLoadIdentity();
			GLERRLOG
			if (_orthoSize.width>0 && _orthoSize.height>0)	{
				if (!_orthoFlipped)	{
					glOrtho(0, _orthoSize.width, 0, _orthoSize.height, 1.0, -1.0);
					GLERRLOG
				}
				else	{
					glOrtho(0, _orthoSize.width, _orthoSize.height, 0, 1.0, -1.0);
					GLERRLOG
				}
			}
#endif
		}
		else	{
#if defined(VVGL_TARGETENV_GLES) || defined(VVGL_TARGETENV_GLES3) || defined(VVGL_TARGETENV_GL3PLUS)
			if (_program > 0)	{
				GLint			orthoUniLoc = _orthoUni.location(_program);
				if (orthoUniLoc >= 0)	{
					//	calculate the orthographic projection transform for a viewport with the given size
					Rect		tmpRect(0., 0., _orthoSize.width, _orthoSize.height);
					//cout << "\treshaping with orthogonal rect " << tmpRect << endl;
					float		right = static_cast<float>(tmpRect.maxX());
					float		left = static_cast<float>(tmpRect.minX());
					float		top = static_cast<float>(tmpRect.maxY());
					float		bottom = static_cast<float>(tmpRect.minY());
					float		near = -1.;
					float		far = 1.;
					GLfloat		projMatrix[] = {
						2.f/(right - left), 0., 0., -1.f*(right+left)/(right-left),
						0.,	2.f/(top-bottom), 0., -1.f*(top+bottom)/(top-bottom),
						0., 0., -2.f/(far-near), -1.f*(far+near)/(far-near),
						0., 0., 0., 1.
					};
					//	TODO: right now, flipped isn't supported!
					glUniformMatrix4fv(orthoUniLoc, 1, GL_FALSE, projMatrix);
					GLERRLOG
				}
				else	{
					//cout << "\tERR: need to reshape, but no orthogonal uniform! " << __PRETTY_FUNCTION__ << endl;
					//cout << "************* _vsString is:\n" << *_vsString << endl;
				}
			}
			else	{
				//cout << "\tERR: need to reshape, but no pgm! " << __PRETTY_FUNCTION__ << endl;
			}
#endif
		}
		
		glViewport(0,0,static_cast<GLsizei>(_orthoSize.width),static_cast<GLsizei>(_orthoSize.height));
		GLERRLOG
	}
	
	_needsReshape = false;
}

void GLScene::_renderCleanup()	{
	if (_context != nullptr)	{
		glUseProgram(0);
		GLERRLOG
	}
	//	flush
	glFlush();
	GLERRLOG
	//	unbind the render target's attachments
	if (_renderTarget.fboName() > 0)	{
		/*
		if (_renderTarget.depthName() > 0)	{
			glFramebufferTexture2D(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, 0, 0, 0);
			GLERRLOG
		}
		if (_renderTarget.colorName() > 0)	{
			glFramebufferTexture2D(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, 0, 0, 0);
			GLERRLOG
		}
		*/
		//	unbind the framebuffer
		glBindFramebuffer(GL_FRAMEBUFFER,0);
		GLERRLOG
	}
	
	//	if there's a render cleanup callback, call it now
	if (_renderCleanupCallback != nullptr)
		_renderCleanupCallback(*this);
}


}
