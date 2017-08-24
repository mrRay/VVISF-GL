#include "VVGLScene.hpp"




namespace VVGL
{


using namespace std;




/*	========================================	*/
#pragma mark --------------------- create/destroy


VVGLScene::VVGLScene()	{
	//cout << __PRETTY_FUNCTION__ << endl;
	VVGLBufferPoolRef		bp = GetGlobalBufferPool();
	if (bp != nullptr)	{
		context = bp->getContext()->newContextSharingMe();
		//cout << "\tcontext is " << *context << endl;
	}
}
VVGLScene::VVGLScene(const VVGLContextRef & inCtx)	{
	//context = new VVGLContext(inCtx);
	context = inCtx;
}

void VVGLScene::prepareToBeDeleted()	{
	//cout << __PRETTY_FUNCTION__ << "->" << this << endl;
	
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
			if (gs > 0)	{
				glDeleteShader(gs);
				GLERRLOG
			}
			if (fs > 0)	{
				glDeleteShader(fs);
				GLERRLOG
			}
		}
		vsString = string("");
		gsString = string("");
		fsString = string("");
		vsStringUpdated = true;
		gsStringUpdated = true;
		fsStringUpdated = true;
	}
	//	lock, delete the error dict
	{
		lock_guard<mutex>		lock(errDictLock);
		errDict.clear();
	}
	
	renderPrepCallback = nullptr;
	renderCallback = nullptr;
	renderCleanupCallback = nullptr;
	
	deleted = true;
}

VVGLScene::~VVGLScene()	{
	//cout << __PRETTY_FUNCTION__ << "->" << this << endl;
	if (!deleted)
		prepareToBeDeleted();
	
	if (context != nullptr)	{
		//delete context;
		context = nullptr;
	}
}


/*	========================================	*/
#pragma mark --------------------- rendering methods




VVGLBufferRef VVGLScene::createAndRenderABuffer(const Size & inSize, const VVGLBufferPoolRef & inPool)	{
	//cout << __PRETTY_FUNCTION__ << endl;
	if (inPool == nullptr || inSize.width <= 0. || inSize.height <= 0.)
		return nullptr;
	
	//	set the orthogonal size
	setOrthoSize(inSize);
	//	make the buffers i'll be rendering into
#if ISF_TARGET_RPI
	RenderTarget		tmpTarget(CreateFBO(false, inPool), CreateRGBATex(orthoSize, false, inPool), nullptr);
#else
	RenderTarget		tmpTarget(CreateFBO(false, inPool), CreateRGBATex(orthoSize, false, inPool), CreateDepthBuffer(orthoSize, false, inPool));
#endif
	//	render
	render(tmpTarget);
	
	return tmpTarget.color;
}
void VVGLScene::renderToBuffer(const VVGLBufferRef & inBuffer)	{
	//cout << __PRETTY_FUNCTION__ << ", passed buffer is " << inBuffer << endl;
	//	set the orthogonal size
	if (inBuffer != nullptr)
		setOrthoSize(inBuffer->srcRect.size);
	//	make the buffers i'll be rendering into
#if ISF_TARGET_RPI
	RenderTarget		tmpTarget(CreateFBO(), inBuffer, nullptr);
#else
	RenderTarget		tmpTarget(CreateFBO(), inBuffer, CreateDepthBuffer(orthoSize));
#endif
	//	render
	render(tmpTarget);
}


void VVGLScene::render(const RenderTarget & inRenderTarget)	{
	//cout << __PRETTY_FUNCTION__ << endl;
	//cout << "\trender target is " << inRenderTarget.fboName() << ", " << inRenderTarget.colorName() << ", " << inRenderTarget.depthName() << endl;
	//	get a lock, set the current GL context
	lock_guard<recursive_mutex>		lock(renderLock);
	if (context == nullptr)	{
		cout << "\terr: bailing, ctx null, " << __PRETTY_FUNCTION__ << endl;
		return;
	}
	context->makeCurrentIfNotCurrent();
	
	//	update the member var for the fbo attachments
	renderTarget = inRenderTarget;
	//	prep for render
	_renderPrep();
	
	//	if there isn't a valid program then we shouldn't execute the client-provided render callback
	bool		looksLikeItShouldHaveAProgram = (vsString.size()>0 || gsString.size()>0 || fsString.size()>0);
	if ((program==0 && !looksLikeItShouldHaveAProgram) ||	(program!=0 && looksLikeItShouldHaveAProgram))	{
		//	execute the render callback
		if (renderCallback != nullptr)
			renderCallback(*this);
	}
	
	//	cleanup after render
	_renderCleanup();
	
	//	update the member vars for the fbo attachments
	renderTarget = RenderTarget();
}


void VVGLScene::renderBlackFrame(const RenderTarget & inRenderTarget)	{
	//	get a lock, set the current GL context
	lock_guard<recursive_mutex>		lock(renderLock);
	if (context == nullptr)	{
		cout << "\terr: bailing, ctx null, " << __PRETTY_FUNCTION__ << endl;
		return;
	}
	context->makeCurrentIfNotCurrent();
	
	//	update the member var for the fbo attachments
	renderTarget = inRenderTarget;
	//	prep for render
	_renderPrep();
	
	//	clear the context
	glClearColor(0., 0., 0., 0.);
	GLERRLOG
	uint32_t		mask = GL_COLOR_BUFFER_BIT;
#if !ISF_TARGET_IOS && !ISF_TARGET_RPI
	mask |= GL_DEPTH_BUFFER_BIT;
#endif
	glClear(mask);
	GLERRLOG
	clearColorUpdated = true;
	
	//	cleanup after render
	_renderCleanup();
	
	//	update the member vars for the fbo attachments
	renderTarget = RenderTarget();
}
void VVGLScene::renderOpaqueBlackFrame(const RenderTarget & inRenderTarget)	{
	//	get a lock, set the current GL context
	lock_guard<recursive_mutex>		lock(renderLock);
	if (context == nullptr)	{
		cout << "\terr: bailing, ctx null, " << __PRETTY_FUNCTION__ << endl;
		return;
	}
	context->makeCurrentIfNotCurrent();
	
	//	update the member var for the fbo attachments
	renderTarget = inRenderTarget;
	//	prep for render
	_renderPrep();
	
	//	get the context, clear
	glClearColor(0., 0., 0., 1.);
	GLERRLOG
	uint32_t		mask = GL_COLOR_BUFFER_BIT;
#if !ISF_TARGET_IOS && !ISF_TARGET_RPI
	mask |= GL_DEPTH_BUFFER_BIT;
#endif
	glClear(mask);
	GLERRLOG
	clearColorUpdated = true;
	
	//	cleanup after render
	_renderCleanup();
	
	//	update the member vars for the fbo attachments
	renderTarget = RenderTarget();
}
void VVGLScene::renderRedFrame(const RenderTarget & inRenderTarget)	{
	//	get a lock, set the current GL context
	lock_guard<recursive_mutex>		lock(renderLock);
	if (context == nullptr)	{
		cout << "\terr: bailing, ctx null, " << __PRETTY_FUNCTION__ << endl;
		return;
	}
	context->makeCurrentIfNotCurrent();
	
	//	update the member var for the fbo attachments
	renderTarget = inRenderTarget;
	//	prep for render
	_renderPrep();
	
	//	get the context, clear
	glClearColor(1., 0., 0., 1.);
	GLERRLOG
	uint32_t		mask = GL_COLOR_BUFFER_BIT;
#if !ISF_TARGET_IOS && !ISF_TARGET_RPI
	mask |= GL_DEPTH_BUFFER_BIT;
#endif
	glClear(mask);
	GLERRLOG
	clearColorUpdated = true;
	
	//	cleanup after render
	_renderCleanup();
	
	//	update the member vars for the fbo attachments
	renderTarget = RenderTarget();
}


/*	========================================	*/
#pragma mark --------------------- setter/getter methods


void VVGLScene::setRenderPrepCallback(const RenderPrepCallback & n)	{
	renderPrepCallback = n;
}
void VVGLScene::setRenderPreLinkCallback(const RenderCallback & n)	{
	renderPreLinkCallback = n;
}
void VVGLScene::setRenderCallback(const RenderCallback & n)	{
	renderCallback = n;
}
void VVGLScene::setRenderCleanupCallback(const RenderCallback & n)	{
	renderCleanupCallback = n;
}


void VVGLScene::setAlwaysNeedsReshape(const bool & n)	{
	lock_guard<recursive_mutex>		lock(renderLock);
	alwaysNeedsReshape = n;
}
void VVGLScene::setOrthoSize(const Size & n)	{
	lock_guard<recursive_mutex>		lock(renderLock);
	orthoSize = n;
	needsReshape = true;
}
Size VVGLScene::getOrthoSize() const	{
	return orthoSize;
}
void VVGLScene::setOrthoFlipped(const bool & n)	{
	lock_guard<recursive_mutex>		lock(renderLock);
	orthoFlipped = n;
	needsReshape = true;
}
bool VVGLScene::getOrthoFlipped() const	{
	return orthoFlipped;
}


void VVGLScene::setClearColor(const GLColor & n)	{
	if (clearColor == n)
		return;
	clearColorUpdated = true;
	clearColor = n;
}
void VVGLScene::setClearColor(const float & r, const float & g, const float & b, const float & a)	{
	setClearColor(GLColor(r,g,b,a));
}
void VVGLScene::setClearColor(float * n)	{
	setClearColor(GLColor(*(n+0), *(n+1), *(n+2), *(n+3)));
}
void VVGLScene::setPerformClear(const bool & n)	{
	performClear = n;
}
void VVGLScene::setVertexShaderString(const string & n)	{
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
	needsReshape = true;
	orthoUni.purgeCache();
}
string VVGLScene::getVertexShaderString()	{
	lock_guard<recursive_mutex>		lock(renderLock);
	return vsString;
}
void VVGLScene::setGeometryShaderString(const string & n)	{
	//cout << __PRETTY_FUNCTION__ << endl;
	//cout << "\tstring was:************************\n" << n << "\n************************\n";
	if (n.size()<1)	{
		cout << "\terr: passed a nil string, " << __PRETTY_FUNCTION__ << endl;
		//	throw an exception
		return;
	}
	
	lock_guard<recursive_mutex>		lock(renderLock);
	
	gsString = string(n);
	gsStringUpdated = true;
	needsReshape = true;
	orthoUni.purgeCache();
}
string VVGLScene::getGeometryShaderString()	{
	lock_guard<recursive_mutex>		lock(renderLock);
	return gsString;
}
void VVGLScene::setFragmentShaderString(const string & n)	{
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
	needsReshape = true;
	orthoUni.purgeCache();
}
string VVGLScene::getFragmentShaderString()	{
	lock_guard<recursive_mutex>		lock(renderLock);
	return fsString;
}


/*	========================================	*/
#pragma mark --------------------- protected rendering methods


void VVGLScene::_renderPrep()	{
	//cout << __PRETTY_FUNCTION__ << endl;
	//cout << "\tthis is " << this << endl;
	if (deleted)	{
		//cout << "\t" << __PRETTY_FUNCTION__ << "- FINISHED" << endl;
		return;
	}
	
	bool			needsReshapeFlag = false;
	bool			pgmChangedFlag = false;
	
	//	initialize
	if (!initialized)
		_initialize();
	
	//	if the vert/frag shader strings have been updated, they need to be recompiled & the program needs to be relinked
	if (vsStringUpdated || gsStringUpdated || fsStringUpdated)	{
		pgmChangedFlag = true;
		
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
		if (gs > 0)	{
			glDeleteShader(gs);
			GLERRLOG
			gs = 0;
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
				cout << "\traw shader is:\n" << vsString << endl;
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
		if (gsString.size() > 0)	{
#if !ISF_TARGET_GLES && !ISF_TARGET_GLES3
			gs = glCreateShader(GL_GEOMETRY_SHADER);
			GLERRLOG
			const char		*shaderSrc = gsString.c_str();
			glShaderSource(gs, 1, &shaderSrc, NULL);
			GLERRLOG
			glCompileShader(gs);
			GLERRLOG
			int32_t			compiled;
			glGetShaderiv(gs, GL_COMPILE_STATUS, &compiled);
			GLERRLOG
			if (!compiled)	{
				int32_t			length;
				char			*log;
				glGetShaderiv(gs, GL_INFO_LOG_LENGTH, &length);
				GLERRLOG
				log = new char[length+1];
				glGetShaderInfoLog(gs, length, &length, log);
				GLERRLOG
				cout << "\terr compiling geo shader in " << __PRETTY_FUNCTION__ << endl;
				cout << "\terr: " << log << endl;
				cout << "\traw shader is:\n" << gsString << endl;
				encounteredError = true;
				
				{
					lock_guard<mutex>		lock(errDictLock);
					errDict.insert(pair<string,string>(string("geoErrLog"), string(log)));
					errDict.insert(pair<string,string>(string("geoSrc"), string(gsString)));
				}
				
				delete [] log;
				glDeleteShader(gs);
				GLERRLOG
				gs = 0;
			}
#endif
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
				cout << "\traw shader is:\n" << fsString << endl;
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
		if ((vs>0 || gs>0 || fs>0) && !encounteredError)	{
			program = glCreateProgram();
			GLERRLOG
			if (vs > 0)	{
				glAttachShader(program, vs);
				GLERRLOG
			}
			if (gs > 0)	{
				glAttachShader(program, gs);
				GLERRLOG
			}
			if (fs > 0)	{
				glAttachShader(program, fs);
				GLERRLOG
			}
			if (renderPreLinkCallback != nullptr)
				renderPreLinkCallback(*this);
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
				cout << "********************\n";
				cout << "\terr linking program in " << __PRETTY_FUNCTION__ << endl;
				cout << "\terr: " << log << endl;
				cout << "********************\n";
				cout << "\traw vert shader is:\n" << vsString << endl;
				cout << "********************\n";
				cout << "\traw geo shader is:\n" << gsString << endl;
				cout << "********************\n";
				cout << "\traw frag shader is:\n" << fsString << endl;
				cout << "********************\n";
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
			else	{
				orthoUni.cacheTheLoc(program);
			}
		}
		
		vsStringUpdated = false;
		gsStringUpdated = false;
		fsStringUpdated = false;
		needsReshape = true;
	}
	
	//	bind the attachments in the render target to the FBO (also in the render target)
	if (renderTarget.fboName() > 0)	{
		glBindFramebuffer(GL_FRAMEBUFFER, renderTarget.fboName());
		GLERRLOG
		
		//	attach the depth buffer
		if (renderTarget.depthName() > 0)	{
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, renderTarget.depthTarget(), renderTarget.depthName(), 0);
			GLERRLOG
		}
		else	{
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, renderTarget.depthTarget(), 0, 0);
			GLERRLOG
		}
		
		//	attach the color buffer
		if (renderTarget.colorName() > 0)	{
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, renderTarget.colorTarget(), renderTarget.colorName(), 0);
			GLERRLOG
		}
		else	{
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, renderTarget.colorTarget(), 0, 0);
			GLERRLOG
		}
	}
	
	//	clear
	if (clearColorUpdated)	{
		glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
		GLERRLOG
		clearColorUpdated = false;
	}
	if (performClear)	{
		uint32_t		mask = GL_COLOR_BUFFER_BIT;
#if !ISF_TARGET_IOS && !ISF_TARGET_RPI
		mask |= GL_DEPTH_BUFFER_BIT;
#endif
		glClear(mask);
		GLERRLOG
	}
	
	//	if there's a program, use it
	if (program > 0)	{
		glUseProgram(program);
		GLERRLOG
	}
	
	//	reshape as needed (must do this after compiling the program, which caches the ortho uniform location)
	if (alwaysNeedsReshape || needsReshape)	{
		needsReshapeFlag = true;
		_reshape();
	}
	
	//	if there's a render prep callback, call it now- pass in flags indicating whether or not the scene has been reshaped (render size changed) and whether or not the program has been recompiled
	if (renderPrepCallback != nullptr)
		renderPrepCallback(*this, needsReshapeFlag, pgmChangedFlag);
	//cout << "\t" << __PRETTY_FUNCTION__ << "- FINISHED" << endl;
}

void VVGLScene::_initialize()	{
	//cout << __PRETTY_FUNCTION__ << endl;
	if (deleted)
		return;
	if (context == nullptr)
		return;
	
	//glEnable(GL_TEXTURE_RECTANGLE);
	//glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	if (getGLVersion() == GLVersion_2)	{
#if !ISF_TARGET_GLES && !ISF_TARGET_GLES3
		glEnable(GL_BLEND);
		GLERRLOG
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		GLERRLOG
//#if !ISF_TARGET_IOS && !ISF_TARGET_RPI
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
		GLERRLOG
//#endif
#endif
	}
	//const int32_t		swap = 0;
	//CGLSetParameter(cgl_ctx, kCGLCPSwapInterval, &swap);
	
	//glHint(GL_CLIP_VOLUME_CLIPPING_HINT_EXT, GL_FASTEST);
	//glHint(GL_PERSPECTIVE_CORRECTION_HINT,GL_NICEST);
	//glDisable(GL_DEPTH_TEST);
	
	//glClearColor(0.0, 0.0, 0.0, 0.0);
	
	//glDisable(GL_BLEND);
	//GLERRLOG
	
	initialized = true;
	clearColorUpdated = true;
	
}

void VVGLScene::_reshape()	{
	//cout << __PRETTY_FUNCTION__ << endl;
	//cout << "\tthis is " << this << endl;
	
	if (orthoSize.width>0. && orthoSize.height>0.)	{
		if (this->getGLVersion() == GLVersion_2)	{
			glMatrixMode(GL_MODELVIEW);
			glLoadIdentity();
			glMatrixMode(GL_PROJECTION);
			glLoadIdentity();
			if (!orthoFlipped)
				glOrtho(0, orthoSize.width, 0, orthoSize.height, 1.0, -1.0);
			else
				glOrtho(0, orthoSize.width, orthoSize.height, 0, 1.0, -1.0);
		}
		
		if (program > 0)	{
			GLint			orthoUniLoc = orthoUni.location(program);
			if (orthoUniLoc >= 0)	{
				//	calculate the orthographic projection transform for a viewport with the given size
				Rect		tmpRect(0., 0., orthoSize.width, orthoSize.height);
				//cout << "\treshaping with orthogonal rect " << tmpRect << endl;
				float		right = MaxX(tmpRect);
				float		left = MinX(tmpRect);
				float		top = MaxY(tmpRect);
				float		bottom = MinY(tmpRect);
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
				//cout << "\tERR: need to reshape, but no orhogonal uniform! " << __PRETTY_FUNCTION__ << endl;
				//cout << "************* vsString is:\n" << vsString << endl;
			}
		}
		else	{
			//cout << "\tERR: need to reshape, but no pgm! " << __PRETTY_FUNCTION__ << endl;
		}
		
		
		glViewport(0,0,orthoSize.width,orthoSize.height);
		GLERRLOG
	}
	
	if (getGLVersion() == GLVersion_2)	{
#if !ISF_TARGET_GLES && !ISF_TARGET_GLES3
//#if !ISF_TARGET_IOS && !ISF_TARGET_RPI
		glMatrixMode(GL_MODELVIEW);
		GLERRLOG
		glLoadIdentity();
		GLERRLOG
		glMatrixMode(GL_PROJECTION);
		GLERRLOG
		glLoadIdentity();
		GLERRLOG
		if (orthoSize.width>0 && orthoSize.height>0)	{
			if (!orthoFlipped)	{
				glOrtho(0, orthoSize.width, 0, orthoSize.height, 1.0, -1.0);
				GLERRLOG
			}
			else	{
				glOrtho(0, orthoSize.width, orthoSize.height, 0, 1.0, -1.0);
				GLERRLOG
			}
		}
//#endif
#endif
		glViewport(0, 0, orthoSize.width, orthoSize.height);
		GLERRLOG
	}
	
	needsReshape = false;
}

void VVGLScene::_renderCleanup()	{
	if (context != nullptr)	{
		glUseProgram(0);
		GLERRLOG
	}
	//	flush
	glFlush();
	GLERRLOG
	//	unbind the render target's attachments
	if (renderTarget.fboName() > 0)	{
		if (renderTarget.depthName() > 0)	{
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, 0, 0, 0);
			GLERRLOG
		}
		if (renderTarget.colorName() > 0)	{
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, 0, 0, 0);
			GLERRLOG
		}
		//	unbind the framebuffer
		glBindFramebuffer(GL_FRAMEBUFFER,0);
		GLERRLOG
	}
	
	//	if there's a render cleanup callback, call it now
	if (renderCleanupCallback != nullptr)
		renderCleanupCallback(*this);
}


}
