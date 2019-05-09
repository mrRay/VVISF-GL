#include "ISFScene.hpp"
#include "ISFDoc.hpp"
#include "ISFPassTarget.hpp"




namespace VVISF
{


using namespace VVGL;
using namespace std;


/*	========================================	*/
#pragma mark --------------------- constructor/destructor


ISFScene::ISFScene()
: GLScene()	{
	//cout << __PRETTY_FUNCTION__ << endl;
	_setUpRenderCallback();
}
ISFScene::ISFScene(const GLContextRef & inCtx)
: GLScene(inCtx)	{
	_setUpRenderCallback();
}


void ISFScene::prepareToBeDeleted()	{
	//geoXYVBO = nullptr;
	//	now call the super, which deletes the context
	GLScene::prepareToBeDeleted();
}
ISFScene::~ISFScene()	{
	if (!_deleted)
		prepareToBeDeleted();
	
	if (_compiledInputTypeString != nullptr)	{
		delete _compiledInputTypeString;
		_compiledInputTypeString = nullptr;
	}
	
	//geoXYVBO = nullptr;
#if !defined(VVGL_TARGETENV_GLES)
	_vao = nullptr;
#endif
	_vbo = nullptr;
}


/*	========================================	*/
#pragma mark --------------------- public methods


void ISFScene::useFile() noexcept(false)	{
	lock_guard<recursive_mutex> rlock(_renderLock);
	lock_guard<mutex>	plock(_propertyLock);
	if (_doc != nullptr)
		_doc->setParentScene(nullptr);
	_doc = nullptr;
	
	//	reset the timestamper and render frame index
	//timestamper.reset();
	_baseTime = Timestamp();
	_renderTime = 0.;
	_renderTimeDelta = 0.;
	_renderFrameIndex = 0;
	_passIndex = 0;
	if (_compiledInputTypeString!=nullptr)	{
		delete _compiledInputTypeString;
		_compiledInputTypeString=nullptr;
	}
}
void ISFScene::useFile(const string & inPath, const bool & inThrowExc) noexcept(false)	{
	//cout << __PRETTY_FUNCTION__ << "... " << inPath << endl;
	try	{
		lock_guard<recursive_mutex> rlock(_renderLock);
		lock_guard<mutex>	plock(_propertyLock);
		if (_doc != nullptr)
			_doc->setParentScene(nullptr);
		_doc = nullptr;
		ISFDocRef			newDoc = make_shared<ISFDoc>(inPath, this, inThrowExc);
		_doc = newDoc;
		
		//	reset the timestamper and render frame index
		//timestamper.reset();
		_baseTime = Timestamp();
		_renderTime = 0.;
		_renderTimeDelta = 0.;
		_renderFrameIndex = 0;
		_passIndex = 0;
		if (_compiledInputTypeString!=nullptr)	{
			delete _compiledInputTypeString;
			_compiledInputTypeString=nullptr;
		}
	}
	catch (ISFErr & exc)	{
		cout << "ERR: " << __PRETTY_FUNCTION__ << "-> caught exception: " << exc.getTypeString() << ": " << exc.general << ", " << exc.specific << endl;
		cout << "\texc details are " << exc.details["jsonErrLog"] << endl;
		for (const auto & detail : exc.details)
			_errDict.insert(detail);
		_doc = nullptr;
		//	reset the timestamper and render frame index
		//timestamper.reset();
		_baseTime = Timestamp();
		_renderTime = 0.;
		_renderTimeDelta = 0.;
		_renderFrameIndex = 0;
		_passIndex = 0;
		if (_compiledInputTypeString!=nullptr)	{
			delete _compiledInputTypeString;
			_compiledInputTypeString=nullptr;
		}
		
		//	if i'm supposed to throw the exception then do so now
		if (_throwExceptions)
			throw exc;
	}
	
}
void ISFScene::useDoc(ISFDocRef & inDoc)	{
	lock_guard<recursive_mutex> rlock(_renderLock);
	lock_guard<mutex>	plock(_propertyLock);
	
	_doc = inDoc;
	if (_doc != nullptr)
		_doc->setParentScene(this);
	
	//	reset the timestamper and render frame index
	//timestamper.reset();
	_baseTime = Timestamp();
	_renderTime = 0.;
	_renderTimeDelta = 0.;
	_renderFrameIndex = 0;
	_passIndex = 0;
	if (_compiledInputTypeString!=nullptr)	{
		delete _compiledInputTypeString;
		_compiledInputTypeString=nullptr;
	}
}


void ISFScene::setBufferForInputNamed(const GLBufferRef & inBuffer, const string & inName)	{
	ISFDocRef			tmpDoc = doc();
	if (tmpDoc == nullptr)
		return;
	ISFAttrRef		tmpAttr = tmpDoc->input(inName);
	if (tmpAttr != nullptr)
		tmpAttr->setCurrentImageBuffer(inBuffer);
}
void ISFScene::setFilterInputBuffer(const GLBufferRef & inBuffer)	{
	//cout << __PRETTY_FUNCTION__ << ", buffer is " << inBuffer << endl;
	ISFAttrRef	filterInput = inputNamed(string("inputImage"));
	if (filterInput == nullptr)
		return;
	filterInput->setCurrentImageBuffer(inBuffer);
	//GLBufferRef		checkBuffer = filterInput->getCurrentImageBuffer();
	//cout << "\tcheck buffer is " << checkBuffer;
	//if (checkBuffer==nullptr) cout << "/null" << endl; else cout << "/" << *checkBuffer << endl;
}
void ISFScene::setBufferForInputImageKey(const GLBufferRef & inBuffer, const string & inString)	{
	ISFDocRef			tmpDoc = doc();
	if (tmpDoc == nullptr)
		return;
	for (const auto & attrIt : tmpDoc->imageInputs())	{
		if (attrIt->name() == inString)	{
			attrIt->setCurrentImageBuffer(inBuffer);
			break;
		}
	}
}
void ISFScene::setBufferForAudioInputKey(const GLBufferRef & inBuffer, const string & inString)	{
	ISFDocRef			tmpDoc = doc();
	if (tmpDoc == nullptr)
		return;
	for (const auto & attrIt : tmpDoc->audioInputs())	{
		if (attrIt->name() == inString)	{
			attrIt->setCurrentImageBuffer(inBuffer);
			break;
		}
	}
}
GLBufferRef ISFScene::getBufferForImageInput(const string & inKey)	{
	ISFDocRef			tmpDoc = doc();
	if (tmpDoc == nullptr)
		return nullptr;
	for (const auto & attrIt : tmpDoc->imageInputs())	{
		if (attrIt->name() == inKey)	{
			return attrIt->getCurrentImageBuffer();
		}
	}
	return nullptr;
}
GLBufferRef ISFScene::getBufferForAudioInput(const string & inKey)	{
	ISFDocRef			tmpDoc = doc();
	if (tmpDoc == nullptr)
		return nullptr;
	for (const auto & attrIt : tmpDoc->audioInputs())	{
		if (attrIt->name() == inKey)	{
			return attrIt->getCurrentImageBuffer();
		}
	}
	return nullptr;
}
GLBufferRef ISFScene::getPersistentBufferNamed(const string & inKey)	{
	ISFDocRef			tmpDoc = doc();
	if (tmpDoc == nullptr)
		return nullptr;
	for (const auto & targetIt : tmpDoc->persistentPassTargets())	{
		if (targetIt->name() == inKey)	{
			return targetIt->buffer();
		}
	}
	return nullptr;
}
GLBufferRef ISFScene::getTempBufferNamed(const string & inKey)	{
	ISFDocRef			tmpDoc = doc();
	if (tmpDoc == nullptr)
		return nullptr;
	for (const auto & targetIt : tmpDoc->tempPassTargets())	{
		if (targetIt->name() == inKey)	{
			return targetIt->buffer();
		}
	}
	return nullptr;
}


void ISFScene::setValueForInputNamed(const ISFVal & inVal, const string & inName)	{
	//cout << __FUNCTION__ << "- " << inVal << ", " << inName << endl;
	ISFAttrRef		inputRef = inputNamed(inName);
	if (inputRef == nullptr)
		return;
	
	ISFValType		inValType = inVal.type();
	ISFValType		attrType = inputRef->type();
	bool			reportFailure = false;
	if (inValType == attrType)	{
		inputRef->setCurrentVal(inVal);
	}
	else	{
		switch (attrType)	{
		case ISFValType_None:
			break;
		case ISFValType_Event:
			if (inValType == ISFValType_None)
				inputRef->setCurrentVal(ISFBoolVal(false));
			else
				reportFailure = true;
			break;
		case ISFValType_Bool:
		case ISFValType_Long:
		case ISFValType_Float:
		case ISFValType_Point2D:
		case ISFValType_Color:
		case ISFValType_Cube:
		case ISFValType_Image:
			reportFailure = true;
			break;
		case ISFValType_Audio:
		case ISFValType_AudioFFT:
			if (inValType == ISFValType_Image)
				inputRef->setCurrentVal(inVal);
			else
				reportFailure = true;
			break;
		}
	}
	
	if (reportFailure)	{
		cout << "\tERR: tried to pass val to input of wrong type, " << __PRETTY_FUNCTION__ << endl;
		cout << "\tERR: name was " << inName << " val is " << inVal << endl;
	}
}
ISFVal ISFScene::valueForInputNamed(const string & inName)	{
	ISFAttrRef		inputRef = inputNamed(inName);
	if (inputRef == nullptr)
		return ISFNullVal();
	return inputRef->currentVal();
}


ISFAttrRef ISFScene::inputNamed(const string & inName)	{
	ISFDocRef		tmpDoc = doc();
	if (tmpDoc == nullptr)
		return nullptr;
	for (const auto & attrIt : tmpDoc->inputs())	{
		if (attrIt->name() == inName)
			return attrIt;
	}
	return nullptr;
}
vector<ISFAttrRef> ISFScene::inputs()	{
	ISFDocRef		tmpDoc = doc();
	if (tmpDoc == nullptr)
		return vector<ISFAttrRef>();
	return tmpDoc->inputs();
}
vector<ISFAttrRef> ISFScene::inputsOfType(const ISFValType & inType)	{
	ISFDocRef		tmpDoc = doc();
	if (tmpDoc == nullptr)
		return vector<ISFAttrRef>();
	return tmpDoc->inputsOfType(inType);
}
vector<ISFAttrRef> ISFScene::imageInputs()	{
	ISFDocRef		tmpDoc = doc();
	if (tmpDoc == nullptr)
		return vector<ISFAttrRef>();
	return tmpDoc->imageInputs();
}
vector<ISFAttrRef> ISFScene::audioInputs()	{
	ISFDocRef		tmpDoc = doc();
	if (tmpDoc == nullptr)
		return vector<ISFAttrRef>();
	return tmpDoc->audioInputs();
}
vector<ISFAttrRef> ISFScene::imageImports()	{
	ISFDocRef		tmpDoc = doc();
	if (tmpDoc == nullptr)
		return vector<ISFAttrRef>();
	return tmpDoc->imageImports();
}


/*	========================================	*/
#pragma mark --------------------- public rendering interface


/*
GLBufferRef ISFScene::createAndRenderABuffer(const VVGL::Size & inSize, const GLBufferPoolRef & inPoolRef)	{
	return createAndRenderABuffer(inSize, timestamper.nowTime().getTimeInSeconds(), nullptr, inPoolRef);
}
*/
GLBufferRef ISFScene::createAndRenderABuffer(const VVGL::Size & inSize, map<int32_t,GLBufferRef> * outPassDict, const GLBufferPoolRef & inPoolRef)	{
	return createAndRenderABuffer(inSize, (Timestamp()-_baseTime).getTimeInSeconds(), outPassDict, inPoolRef);
}
/*
GLBufferRef ISFScene::createAndRenderABuffer(const VVGL::Size & inSize, const double & inRenderTime, const GLBufferPoolRef & inPoolRef)	{
	return createAndRenderABuffer(inSize, inRenderTime, nullptr, inPoolRef);
}
*/
GLBufferRef ISFScene::createAndRenderABuffer(const VVGL::Size & inSize, const double & inRenderTime, map<int32_t,GLBufferRef> * outPassDict, const GLBufferPoolRef & inPoolRef)	{
	ISFDocRef		tmpDoc = doc();
	if (tmpDoc == nullptr)
		return nullptr;
	//cout << "\ttmpDoc is " << *tmpDoc << endl;
	
	GLBufferRef			returnMe = nullptr;
	vector<string>			passNames = tmpDoc->renderPasses();
	ISFPassTargetRef		lastPass = nullptr;
	if (passNames.size()>0)	{
		string					lastPassName = passNames.back();
		lastPass = tmpDoc->passTargetForKey(lastPassName);
	}
	
	GLBufferPoolRef		bp = nullptr;
	if (inPoolRef != nullptr)
		bp = inPoolRef;
	else
		bp = (_privatePool != nullptr) ? _privatePool : GetGlobalBufferPool();
	if (bp==nullptr)	{
		cout << "\tERR: bailing, pool null, " << __PRETTY_FUNCTION__ << endl;
		return nullptr;
	}
	
	//returnMe = (lastPass!=nullptr && lastPass->floatFlag())
	//	? CreateBGRAFloatTex(inSize, bp)
	//	: CreateBGRATex(inSize, bp);
	//returnMe = (lastPass!=nullptr && lastPass->floatFlag())
	//	? CreateRGBAFloatTex(inSize, bp)
	//	: CreateRGBATex(inSize, bp);
	
	bool			shouldBeFloat = _alwaysRenderToFloat || (lastPass!=nullptr && lastPass->floatFlag());
#if defined(VVGL_SDK_MAC)
	if (_persistentToIOSurface)
		returnMe = (shouldBeFloat) ? CreateRGBAFloatTexIOSurface(inSize, false, bp) : CreateRGBATexIOSurface(inSize, false, bp);
	else
#endif
		returnMe = (shouldBeFloat) ? CreateRGBAFloatTex(inSize, false, bp) : CreateRGBATex(inSize, false, bp);
	
	renderToBuffer(returnMe, inSize, inRenderTime, outPassDict);
	
	return returnMe;
	/*
	GLBufferRef		returnMe = CreateRGBATex(inSize, bp);
	renderToBuffer(returnMe);
	return returnMe;
	*/
}
void ISFScene::renderToBuffer(const GLBufferRef & inTargetBuffer, const VVGL::Size & inRenderSize, const double & inRenderTime, map<int32_t,GLBufferRef> * outPassDict)	{
	_render(inTargetBuffer, inRenderSize, inRenderTime, outPassDict);
}
void ISFScene::renderToBuffer(const GLBufferRef & inTargetBuffer, const VVGL::Size & inRenderSize, const double & inRenderTime)	{
	_render(inTargetBuffer, inRenderSize, inRenderTime, nullptr);
}
void ISFScene::renderToBuffer(const GLBufferRef & inTargetBuffer, const VVGL::Size & inRenderSize, map<int32_t,GLBufferRef> * outPassDict)	{
	_render(inTargetBuffer, inRenderSize, (Timestamp()-_baseTime).getTimeInSeconds(), outPassDict);
}
void ISFScene::renderToBuffer(const GLBufferRef & inTargetBuffer, const VVGL::Size & inRenderSize)	{
	_render(inTargetBuffer, inRenderSize, (Timestamp()-_baseTime).getTimeInSeconds(), nullptr);
}
void ISFScene::renderToBuffer(const GLBufferRef & inTargetBuffer)	{
	if (inTargetBuffer != nullptr)
		_render(inTargetBuffer, inTargetBuffer->srcRect.size, (Timestamp()-_baseTime).getTimeInSeconds(), nullptr);
	else
		_render(nullptr, _orthoSize, (Timestamp()-_baseTime).getTimeInSeconds(), nullptr);
}


void ISFScene::setSize(const VVGL::Size & n)	{
	//cout << __PRETTY_FUNCTION__ << ", self is " << this << endl;
	//_renderSize = n;	//	do NOT set the _renderSize here (if we do then it will be changed every time a pass is rendered)
	GLScene::setOrthoSize(n);
	//cout << "\tnew size is " << _orthoSize << endl;
}


/*	========================================	*/
#pragma mark --------------------- protected rendering methods


void ISFScene::_setUpRenderCallback()	{
#if defined(VVGL_TARGETENV_GLES)
	setRenderCallback([&](const GLScene & s)	{
		//	make a quad that describes the area we have to draw
		Quad<VertXY>		targetQuad;
		targetQuad.populateGeo(Rect(0,0,_orthoSize.width,_orthoSize.height));
		
		//	get the VBO
		GLBufferRef		myVBO = vbo();
		GLBufferPoolRef	bp = (s.privatePool() != nullptr) ? s.privatePool() : GetGlobalBufferPool();
		
		//	if there's no VBO, or the target quad doesn't match the VBO's contents
		if (myVBO==nullptr || targetQuad!=_vboContents)	{
			//	make the VBO, populate it with vertex data
			myVBO = CreateVBO((void*)&targetQuad, sizeof(targetQuad), GL_STATIC_DRAW, true, bp);
			//	update the instance's copy of the VBO
			setVBO(myVBO);
			//	update the contents of the VBO
			_vboContents = targetQuad;
		}
		if (myVBO == nullptr)	{
			cout << "\terr: VBO still null, bailing, " << __PRETTY_FUNCTION__ << endl;
			return;
		}
		
		//	bind the VBO
		if (myVBO != nullptr)	{
			glBindBuffer(GL_ARRAY_BUFFER, myVBO->name);
			GLERRLOG
		}
		//	configure the attribute pointers to work with the VBO
		if (_vertexAttrib.loc >= 0)	{
			glVertexAttribPointer(_vertexAttrib.loc, 2, GL_FLOAT, GL_FALSE, targetQuad.stride(), BUFFER_OFFSET(0));
			GLERRLOG
			//glVertexAttribPointer(_vertexAttrib.loc, 2, GL_FLOAT, GL_FALSE, targetQuad.stride(), (void*)&targetQuad.bl.geo.x);
			//GLERRLOG
			_vertexAttrib.enable();
		}
		
		//	draw!
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		GLERRLOG
		
		//	disable the relevant attribute pointers
		if (_vertexAttrib.loc >= 0)	{
			_vertexAttrib.disable();
		}
		//	un-bind the VBO
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		GLERRLOG
	});
#else
	setRenderCallback([&](const GLScene & s)	{
		//cout << __PRETTY_FUNCTION__ << endl;
		//CGLContextObj		orig_ctx = CGLGetCurrentContext();
		//cout << "\tin render callback, my context is " << *_context << ", current context is " << orig_ctx << endl;
		//	if we're in GL 2 then we can't use a VAO
		if (s.glVersion() == GLVersion_2)	{
			//	make a quad that describes the area we have to draw
			Quad<VertXY>		targetQuad;
			targetQuad.populateGeo(VVGL::Rect(0,0,_orthoSize.width,_orthoSize.height));
		
			//	get the VBO
			GLBufferRef		myVBO = vbo();
			GLBufferPoolRef	bp = (s.privatePool() != nullptr) ? s.privatePool() : GetGlobalBufferPool();
		
			//	if there's no VBO, or the target quad doesn't match the VBO's contents
			if (myVBO==nullptr || targetQuad!=_vboContents)	{
				//	make the VBO, populate it with vertex data
				myVBO = CreateVBO((void*)&targetQuad, sizeof(targetQuad), GL_STATIC_DRAW, true, bp);
				//	update the instance's copy of the VBO
				setVBO(myVBO);
				//	update the contents of the VBO
				_vboContents = targetQuad;
				//	if the VBO is deleted & its resources destroyed the buffer pool's ctx will be current!
				_context->makeCurrentIfNotCurrent();
			}
		
			//	bind the VBO
			if (myVBO != nullptr)	{
				glBindBuffer(GL_ARRAY_BUFFER, myVBO->name);
				GLERRLOG
			}
			//	configure the attribute pointers to work with the VBO
			if (_vertexAttrib.loc >= 0)	{
				glVertexAttribPointer(_vertexAttrib.loc, 2, GL_FLOAT, GL_FALSE, targetQuad.stride(), BUFFER_OFFSET(0));
				GLERRLOG
				_vertexAttrib.enable();
			}
			
			//	draw!
			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
			GLERRLOG
		
			//	disable the relevant attribute pointers
			if (_vertexAttrib.loc >= 0)	{
				_vertexAttrib.disable();
			}
			//	un-bind the VBO
			glBindBuffer(GL_ARRAY_BUFFER, 0);
			GLERRLOG
		}
		//	else we're in a flavor of GL that has VAOs
		else	{
			//	make a quad that describes the area we have to draw
			Quad<VertXY>		targetQuad;
			targetQuad.populateGeo(VVGL::Rect(0,0,_orthoSize.width,_orthoSize.height));
		
			//	bind the VAO
			GLBufferRef		myVAO = vao();
			if (myVAO == nullptr)
				return;
			glBindVertexArray(myVAO->name);
			GLERRLOG
		
			//	if the target quad doesn't match the contents of the vbo in the vao
			if (targetQuad != _vboContents)	{
				//cout << "\tvbo contents updated, repopulating\n";
				//	make a VBO, populate it with vertex data
				uint32_t		tmpVBO = -1;
				glGenBuffers(1, &tmpVBO);
				GLERRLOG
				glBindBuffer(GL_ARRAY_BUFFER, tmpVBO);
				GLERRLOG
				glBufferData(GL_ARRAY_BUFFER, sizeof(targetQuad), (void*)&targetQuad, GL_STATIC_DRAW);
				GLERRLOG
				//	configure the attribute pointer to work with the VBO
				if (_vertexAttrib.loc >= 0)	{
					glVertexAttribPointer(_vertexAttrib.loc, 2, GL_FLOAT, GL_FALSE, targetQuad.stride(), BUFFER_OFFSET(0));
					GLERRLOG
					_vertexAttrib.enable();
				}
			
				//	un-bind the VAO, we're done configuring it
				glBindVertexArray(0);
				GLERRLOG
				//	delete the VBO we just made (the VAO will retain it)
				glDeleteBuffers(1, &tmpVBO);
				GLERRLOG
				//	update my local copy of the vbo contents
				_vboContents = targetQuad;
				//	re-enable the VAO!
				glBindVertexArray(myVAO->name);
				GLERRLOG
			}
		
			//	...draw!
			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
			GLERRLOG
		
			//	un-bind the VAO
			glBindVertexArray(0);
			GLERRLOG
		}
	});
#endif
	
	
	
	
	
	/*
	setRenderCallback([&](const GLScene & s)	{
		//cout << __PRETTY_FUNCTION__ << endl;
		VVGL::Rect				tmpRect(0,0,0,0);
		tmpRect.size = static_cast<const ISFScene&>(s)._orthoSize;
		//cout << "\tverts based on rect " << tmpRect << endl;
#if defined(VVGL_SDK_MAC) || defined(VVGL_SDK_GLFW)
		glColor4f(1., 1., 1., 1.);
		GLERRLOG
		glEnableClientState(GL_VERTEX_ARRAY);
		GLERRLOG
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		GLERRLOG
		glDisableClientState(GL_COLOR_ARRAY);
		GLERRLOG
		float				verts[] = {
			(float)tmpRect.minX(), (float)tmpRect.minY(), 0.0,
			(float)tmpRect.minX(), (float)tmpRect.maxY(), 0.0,
			(float)tmpRect.maxX(), (float)tmpRect.maxY(), 0.0,
			(float)tmpRect.maxX(), (float)tmpRect.minY(), 0.0
		};
		glVertexPointer(3, GL_FLOAT, 0, verts);
		GLERRLOG
		glDrawArrays(GL_QUADS, 0, 4);
		GLERRLOG
#elif defined(VVGL_SDK_IOS) || defined(VVGL_SDK_RPI)
		GLfloat			geoCoords[] = {
			(GLfloat)tmpRect.minX(), (GLfloat)tmpRect.minY(),
			(GLfloat)tmpRect.maxX(), (GLfloat)tmpRect.minY(),
			(GLfloat)tmpRect.minX(), (GLfloat)tmpRect.maxY(),
			(GLfloat)tmpRect.maxX(), (GLfloat)tmpRect.maxY()
		};
		if (_vertexAttrib.loc >= 0)	{
			glEnableVertexAttribArray(_vertexAttrib.loc);
			GLERRLOG
			glVertexAttribPointer(_vertexAttrib.loc, 2, GL_FLOAT, GL_FALSE, 0, geoCoords);
			GLERRLOG
		}
		
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		GLERRLOG
		
		if (_vertexAttrib.loc >= 0)	{
			glDisableVertexAttribArray(_vertexAttrib.loc);
			GLERRLOG
		}
		
#endif
	});
	*/
}
void ISFScene::_renderPrep()	{
	//cout << __PRETTY_FUNCTION__ << endl;
	if (_deleted)
		return;
	if (_doc == nullptr)
		return;
	
	//	check the texture type string- if it's changed, we'll have to recompile the shaders!
	string		currentTextureTypeString = _doc->generateTextureTypeString();
	if (_compiledInputTypeString==nullptr || (currentTextureTypeString!=*_compiledInputTypeString))	{
		if (_compiledInputTypeString != nullptr)
			delete _compiledInputTypeString;
		_compiledInputTypeString = new string(currentTextureTypeString);
		
		//	update the shader strings
		string		tmpFrag;
		string		tmpVert;
		GLVersion	tmpVersion = (_context==nullptr) ? GLVersion_2 : _context->version;
		_doc->generateShaderSource(&tmpFrag, &tmpVert, tmpVersion);
		setVertexShaderString(tmpVert);
		setFragmentShaderString(tmpFrag);
	}
	
	//	store these values, then check them after the super's "_renderPrep"...
	bool		vShaderUpdatedFlag = _vsStringUpdated;
	bool		fShaderUpdatedFlag = _fsStringUpdated;
	
	//	tell the super to do its _renderPrep, which will compile the shader and get it all set up if necessary
	GLScene::_renderPrep();
	
	/*
	//	if i don't have a VBO containing geometry for a quad, make one now
	if (geoXYVBO == nullptr)	{
		GLfloat			geo[] = {
			-1., -1.,
			1., -1.,
			-1, 1.,
			1., 1.
		};
		geoXYVBO = CreateVBO(geo, sizeof(GLfloat)*8, GL_STATIC_DRAW);
	}
	*/
	
	//	make sure there's a VAO
#if !defined(VVGL_TARGETENV_GLES)
	if (glVersion() != GLVersion_2)	{
		if (vao() == nullptr)
			setVAO(CreateVAO(true, (_privatePool!=nullptr) ? _privatePool : GetGlobalBufferPool()));
	}
#endif
	
	//	...if either of these values have changed, the program has been recompiled and i need to find new uniform locations for all the attributes (the uniforms in the GLSL programs)
	bool		findNewUniforms = false;
	if (vShaderUpdatedFlag!=_vsStringUpdated || fShaderUpdatedFlag!=_fsStringUpdated)
		findNewUniforms = true;
	
	//	need a GL context
	if (_context == nullptr)	{
		cout << "\terr: bailing, _context null, " << __PRETTY_FUNCTION__ << endl;
		return;
	}
	
	//	need a GL program
	if (_program <= 0)	{
		//cout << "\terr: no program, " << __PRETTY_FUNCTION__ << endl;
		return;
	}
	
	//	set up some vars and some blocks that we're going to use to cache the locations of uniforms in the attributes of the ISFDoc instance, and eventually push those vals to GL
	GLint				samplerLoc = 0;
	GLint				textureCount = 0;
	GLBufferRef		tmpBuffer = nullptr;
	VVGL::Rect				tmpRect;
	char				tmpCString[64];
	memset(tmpCString, 0, 64);
	
	//	this block retrieves and stores the uniform location from the passed attribute for simple val-based attributes
	auto		setAttrUniformsSimpleValBlock = [&](const ISFAttrRef & inAttr)	{
		const char *	tmpAttrName = inAttr->name().c_str();
		samplerLoc = (_program<=0) ? -1 : glGetUniformLocation(_program, tmpAttrName);
		GLERRLOG
		inAttr->setUniformLocation(0, samplerLoc);
	};
	//	this block retrieves and stores the uniform locations from the passed attribute for cube-based attributes
	auto		setAttrUniformsCubeBlock = [&](const ISFAttrRef & inAttr)	{
		const char *		tmpAttrName = inAttr->name().c_str();
		samplerLoc = (_program<=0) ? -1 : glGetUniformLocation(_program, tmpAttrName);
		GLERRLOG
		inAttr->setUniformLocation(0, samplerLoc);
		
		sprintf(tmpCString,"_%s_imgSize",tmpAttrName);
		samplerLoc = (_program<=0) ? -1 : glGetUniformLocation(_program, tmpCString);
		GLERRLOG
		inAttr->setUniformLocation(2, samplerLoc);
	};
	//	this block retrieves and stores the uniform locations from the passed attribute for all other image-based attributes
	auto		setAttrUniformsImageBlock = [&](const ISFAttrRef & inAttr)	{
		const char *		tmpAttrName = inAttr->name().c_str();
		samplerLoc = (_program<=0) ? -1 : glGetUniformLocation(_program, tmpAttrName);
		GLERRLOG
		inAttr->setUniformLocation(0, samplerLoc);
		
		sprintf(tmpCString,"_%s_imgRect",tmpAttrName);
		samplerLoc = (_program<=0) ? -1 : glGetUniformLocation(_program, tmpCString);
		GLERRLOG
		inAttr->setUniformLocation(1, samplerLoc);
		
		sprintf(tmpCString,"_%s_imgSize",tmpAttrName);
		samplerLoc = (_program<=0) ? -1 : glGetUniformLocation(_program, tmpCString);
		GLERRLOG
		inAttr->setUniformLocation(2, samplerLoc);
		
		sprintf(tmpCString,"_%s_flip",tmpAttrName);
		samplerLoc = (_program<=0) ? -1 : glGetUniformLocation(_program, tmpCString);
		GLERRLOG
		inAttr->setUniformLocation(3, samplerLoc);
	};
	//	this block gets a buffer for a cube texture from the passed attrib and pushes it to the gl program
	auto		pushAttrUniformsCubeBlock = [&](const ISFAttrRef & inAttr)	{
		tmpBuffer = inAttr->getCurrentImageBuffer();
		//if (tmpBuffer != nullptr && inAttr->getUniformLocation(0)>=0)	{
			//	pass the actual texture to the program
			if (tmpBuffer != nullptr)	{
				glActiveTexture(GL_TEXTURE0 + textureCount);
				GLERRLOG
				if (_context->version <= GLVersion_2)	{
					glEnable(tmpBuffer->desc.target);
					GLERRLOG
				}
				glBindTexture(tmpBuffer->desc.target, tmpBuffer->name);
				GLERRLOG
			}
			samplerLoc = inAttr->getUniformLocation(0);
			if (samplerLoc >= 0)	{
				glUniform1i(samplerLoc, textureCount);
				GLERRLOG
			}
			++textureCount;
			//	pass the size to the program
			tmpRect = (tmpBuffer==nullptr) ? VVGL::Rect(0,0,1,1) : tmpBuffer->srcRect;
			samplerLoc = inAttr->getUniformLocation(2);
			if (samplerLoc >= 0)	{
				glUniform2f(samplerLoc, float(tmpRect.size.width), float(tmpRect.size.height));
				GLERRLOG
			}
		//}
	};
	//	this block gets a buffer for a non-cube texture from the passed attrib and pushes it to the gl program
	auto		pushAttrUniformsImageBlock = [&](const ISFAttrRef & inAttr)	{
		tmpBuffer = inAttr->getCurrentImageBuffer();
		//if (tmpBuffer != nullptr && inAttr->getUniformLocation(0)>=0)	{
			//	pass the actual texture to the program
			if (tmpBuffer != nullptr)	{
				glActiveTexture(GL_TEXTURE0 + textureCount);
				GLERRLOG
				if (_context->version <= GLVersion_2)	{
					glEnable(tmpBuffer->desc.target);
					GLERRLOG
				}
				glBindTexture(tmpBuffer->desc.target, tmpBuffer->name);
				GLERRLOG
			}
			samplerLoc = inAttr->getUniformLocation(0);
			if (samplerLoc >= 0)	{
				glUniform1i(samplerLoc,textureCount);
				GLERRLOG
			}
			++textureCount;
			//	pass the img rect to the program
			tmpRect = (tmpBuffer==nullptr) ? VVGL::Rect(0,0,1,1) : tmpBuffer->glReadySrcRect();
			samplerLoc = inAttr->getUniformLocation(1);
			if (samplerLoc >= 0)	{
				glUniform4f(samplerLoc, float(tmpRect.origin.x), float(tmpRect.origin.y), float(tmpRect.size.width), float(tmpRect.size.height));
				GLERRLOG
			}
			//	pass the size to the program
			tmpRect = (tmpBuffer==nullptr) ? VVGL::Rect(0,0,1,1) : tmpBuffer->srcRect;
			samplerLoc = inAttr->getUniformLocation(2);
			if (samplerLoc >= 0)	{
				glUniform2f(samplerLoc, float(tmpRect.size.width), float(tmpRect.size.height));
				GLERRLOG
			}
			//	pass the flippedness to the program
			samplerLoc = inAttr->getUniformLocation(3);
			if (samplerLoc >= 0)	{
				glUniform1i(samplerLoc,((tmpBuffer!=nullptr && tmpBuffer->flipped)?1:0));
				GLERRLOG
			}
		//}
	};
	/*
	auto		setTargetUniformsCubeBlock = [&](const ISFPassTargetRef & inTarget)	{
		//const char *		tmpTargetName = inTarget->name().c_str();
		//samplerLoc = (_program<=0) ? -1 : glGetUniformLocation(_program, tmpTargetName);
		//GLERRLOG
		//if (samplerLoc >= 0)
		//	inTarget->setUniformLocation(0, samplerLoc);
		//
		//sprintf(tmpCString,"_%s_imgSize",tmpTargetName);
		//samplerLoc = (_program<=0) ? -1 : glGetUniformLocation(_program, tmpCString);
		//GLERRLOG
		//if (samplerLoc >= 0)
		//	inTarget->setUniformLocation(2, samplerLoc);
		//
		
		inTarget->cacheUniformLocations(_program);
	};
	*/
	auto		setTargetUniformsImageBlock = [&](const ISFPassTargetRef & inTarget)	{
		//const char *		tmpTargetName = inTarget->name().c_str();
		//samplerLoc = (_program<=0) ? -1 : glGetUniformLocation(_program, tmpTargetName);
		//GLERRLOG
		//inTarget->setUniformLocation(0, samplerLoc);
		//
		//sprintf(tmpCString,"_%s_imgRect",tmpTargetName);
		//samplerLoc = (_program<=0) ? -1 : glGetUniformLocation(_program, tmpCString);
		//GLERRLOG
		//inTarget->setUniformLocation(1, samplerLoc);
		//
		//sprintf(tmpCString,"_%s_imgSize",tmpTargetName);
		//samplerLoc = (_program<=0) ? -1 : glGetUniformLocation(_program, tmpCString);
		//GLERRLOG
		//inTarget->setUniformLocation(2, samplerLoc);
		//
		//sprintf(tmpCString,"_%s_flip",tmpTargetName);
		//samplerLoc = (_program<=0) ? -1 : glGetUniformLocation(_program, tmpCString);
		//GLERRLOG
		//inTarget->setUniformLocation(3, samplerLoc);
		
		inTarget->cacheUniformLocations(_program);
	};
	/*
	auto		pushTargetUniformsCubeBlock = [&](const ISFPassTargetRef & inTarget)	{
		tmpBuffer = inTarget->buffer();
		if (tmpBuffer != nullptr)	{
			//	pass the actual texture to the program
			glActiveTexture(GL_TEXTURE0 + textureCount);
			GLERRLOG
			if (_context->version <= GLVersion_2)	{
				glEnable(tmpBuffer->desc.target);
				GLERRLOG
			}
			glBindTexture(tmpBuffer->desc.target, tmpBuffer->name);
			GLERRLOG
			
			samplerLoc = inTarget->getUniformLocation(0);
			if (samplerLoc >= 0)	{
				glUniform1i(samplerLoc, textureCount);
				GLERRLOG
			}
			++textureCount;
			//	pass the size to the program
			tmpRect = tmpBuffer->srcRect;
			samplerLoc = inTarget->getUniformLocation(2);
			if (samplerLoc >= 0)	{
				glUniform2f(samplerLoc, tmpRect.size.width, tmpRect.size.height);
				GLERRLOG
			}
		}
	};
	*/
	auto		pushTargetUniformsImageBlock = [&](const ISFPassTargetRef & inTarget)	{
		tmpBuffer = inTarget->buffer();
		if (tmpBuffer != nullptr)	{
			//	pass the actual texture to the program
			glActiveTexture(GL_TEXTURE0 + textureCount);
			GLERRLOG
			if (_context->version <= GLVersion_2)	{
				glEnable(tmpBuffer->desc.target);
				GLERRLOG
			}
			glBindTexture(tmpBuffer->desc.target, tmpBuffer->name);
			GLERRLOG
			
			samplerLoc = inTarget->getUniformLocation(0);
			if (samplerLoc >= 0)	{
				glUniform1i(samplerLoc,textureCount);
				GLERRLOG
			}
			++textureCount;
			//	pass the img rect to the program
			tmpRect = tmpBuffer->glReadySrcRect();
			samplerLoc = inTarget->getUniformLocation(1);
			if (samplerLoc >= 0)	{
				glUniform4f(samplerLoc, float(tmpRect.origin.x), float(tmpRect.origin.y), float(tmpRect.size.width), float(tmpRect.size.height));
				GLERRLOG
			}
			//	pass the size to the program
			tmpRect = tmpBuffer->srcRect;
			samplerLoc = inTarget->getUniformLocation(2);
			if (samplerLoc >= 0)	{
				glUniform2f(samplerLoc, float(tmpRect.size.width), float(tmpRect.size.height));
				GLERRLOG
			}
			//	pass the flippedness to the program
			samplerLoc = inTarget->getUniformLocation(3);
			if (samplerLoc >= 0)	{
				glUniform1i(samplerLoc,((tmpBuffer->flipped)?1:0));
				GLERRLOG
			}
		}
	};
	
	
	//	run through the inputs, applying the current values to the program
	vector<ISFAttrRef> &	inputs = _doc->inputs();
	for (const auto & attribRef : inputs)	{
		if (attribRef == nullptr)	{
			//cout << "\tERR: attrib NULL in " << __PRETTY_FUNCTION__ << endl;
			continue;
		}
		ISFValType			attribType = attribRef->type();
		ISFVal &			currentVal = attribRef->currentVal();
		
		switch (attribType)	{
		case ISFValType_None:
			break;
		case ISFValType_Event:
			{
			if (findNewUniforms)
				setAttrUniformsSimpleValBlock(attribRef);
			samplerLoc = attribRef->getUniformLocation(0);
			
			//GLint			tmpInt = currentVal.getBoolVal();
			//cout << "\tuploading event val as " << tmpInt << endl;
			if (samplerLoc >= 0)	{
				glUniform1i(samplerLoc, currentVal.getBoolVal());
				GLERRLOG
			}
			
			//currentVal.val.boolVal = false;
			//currentVal = ISFBoolVal(false);
			//tmpInt = currentVal.getBoolVal();
			//attribRef->setCurrentVal(ISFBoolVal(false));
			//cout << "\tchecking: " << attribRef->currentVal().getBoolVal() << endl;
			break;
			}
		case ISFValType_Bool:
			{
			//cout << "\tprocessing bool-type attr: " << *attribRef << endl;
			//attribRef->lengthyDescription();
			if (findNewUniforms)
				setAttrUniformsSimpleValBlock(attribRef);
			samplerLoc = attribRef->getUniformLocation(0);
			if (samplerLoc >= 0)	{
				glUniform1i(samplerLoc, currentVal.getBoolVal());
				GLERRLOG
			}
			break;
			}
		case ISFValType_Long:
			if (findNewUniforms)
				setAttrUniformsSimpleValBlock(attribRef);
			samplerLoc = attribRef->getUniformLocation(0);
			if (samplerLoc >= 0)	{
				glUniform1i(samplerLoc, (int32_t)(currentVal.getDoubleVal()));
				GLERRLOG
			}
			break;
		case ISFValType_Float:
			//cout << "\tprocessing float-type input named " << attribRef->name() << ", val is " << currentVal.getDoubleVal() << endl;
			if (findNewUniforms)
				setAttrUniformsSimpleValBlock(attribRef);
			samplerLoc = attribRef->getUniformLocation(0);
			if (samplerLoc >= 0)	{
				glUniform1f(samplerLoc, (float)(currentVal.getDoubleVal()));
				GLERRLOG
			}
			break;
		case ISFValType_Point2D:
			if (findNewUniforms)
				setAttrUniformsSimpleValBlock(attribRef);
			samplerLoc = attribRef->getUniformLocation(0);
			if (samplerLoc >= 0)	{
				//glUniform2f(samplerLoc, currentVal.val.pointVal[0], currentVal.val.pointVal[1]);
				//GLERRLOG
				double		*pointVals = currentVal.getPointValPtr();
				if (pointVals == nullptr)	{
					glUniform2f(samplerLoc, 0., 0.);
					GLERRLOG
				}
				else	{
					glUniform2f(samplerLoc, float(pointVals[0]), float(pointVals[1]));
					GLERRLOG
				}
			}
			break;
		case ISFValType_Color:
			if (findNewUniforms)
				setAttrUniformsSimpleValBlock(attribRef);
			samplerLoc = attribRef->getUniformLocation(0);
			if (samplerLoc >= 0)	{
				//glUniform4f(samplerLoc, currentVal.val.colorVal[0], currentVal.val.colorVal[1], currentVal.val.colorVal[2], currentVal.val.colorVal[3]);
				//GLERRLOG
				double		*colorVals = currentVal.getColorValPtr();
				if (colorVals == nullptr)	{
					glUniform4f(samplerLoc, 0., 0., 0., 0.);
					GLERRLOG
				}
				else	{
					glUniform4f(samplerLoc, float(colorVals[0]), float(colorVals[1]), float(colorVals[2]), float(colorVals[3]));
					GLERRLOG
				}
			}
			break;
		case ISFValType_Cube:
			if (findNewUniforms)
				setAttrUniformsCubeBlock(attribRef);
			pushAttrUniformsCubeBlock(attribRef);
			break;
		case ISFValType_Image:
		case ISFValType_Audio:
		case ISFValType_AudioFFT:
			{
			//GLBufferRef		tmpBuffer = attribRef->getCurrentImageBuffer();
			//cout << "\tprocessing image-type input named " << attribRef->name();
			//cout << ", buffer is " << tmpBuffer;
			//if (tmpBuffer==nullptr) cout << "/null" << endl; else cout << "/" << *tmpBuffer << endl;
			if (findNewUniforms)
				setAttrUniformsImageBlock(attribRef);
			pushAttrUniformsImageBlock(attribRef);
			break;
			}
		}
	}
	
	//	run through the imported images, applying the current values to the program
	vector<ISFAttrRef> &	imageImports = _doc->imageImports();
	for (const auto & attribRef : imageImports)	{
		if (attribRef == nullptr)	{
			cout << "\tERR: attrib NULL in " << __PRETTY_FUNCTION__ << endl;
			continue;
		}
		
		if (attribRef->type() == ISFValType_Cube)	{
			if (findNewUniforms)
				setAttrUniformsCubeBlock(attribRef);
			pushAttrUniformsCubeBlock(attribRef);
		}
		else if (attribRef->shouldHaveImageBuffer())	{
			if (findNewUniforms)
				setAttrUniformsImageBlock(attribRef);
			pushAttrUniformsImageBlock(attribRef);
		}
	}
	
	//	run through the persistent buffers, applying the current values to the program
	const vector<ISFPassTargetRef>	persistentBuffers = _doc->persistentPassTargets();
	for (const auto & targetRef : persistentBuffers)	{
		if (targetRef == nullptr)
			return;
		
		if (findNewUniforms)
			setTargetUniformsImageBlock(targetRef);
		pushTargetUniformsImageBlock(targetRef);
	}
	
	//	run through the temp buffers, applying the current values to the program
	const vector<ISFPassTargetRef>	tempBuffers = _doc->tempPassTargets();
	for (const auto & targetRef : tempBuffers)	{
		if (targetRef == nullptr)
			continue;
		
		if (findNewUniforms)
			setTargetUniformsImageBlock(targetRef);
		pushTargetUniformsImageBlock(targetRef);
	}
	
	//	if we're finding new uniforms then we also have to update the uniform locations of some standard inputs
	if (findNewUniforms)	{
		_vertexAttrib.cacheTheLoc(_program);
		_renderSizeUni.cacheTheLoc(_program);
		_passIndexUni.cacheTheLoc(_program);
		_timeUni.cacheTheLoc(_program);
		_timeDeltaUni.cacheTheLoc(_program);
		_dateUni.cacheTheLoc(_program);
		_renderFrameIndexUni.cacheTheLoc(_program);
	}
	//	push the standard inputs to the program
	if (_renderSizeUni.loc >= 0)	{
		glUniform2f(_renderSizeUni.loc, float(_orthoSize.width), float(_orthoSize.height));
		GLERRLOG
	}
	if (_passIndexUni.loc >= 0)	{
		glUniform1i(_passIndexUni.loc, _passIndex-1);
		GLERRLOG
	}
	if (_timeUni.loc >= 0)	{
		glUniform1f(_timeUni.loc, (float)_renderTime);
		GLERRLOG
	}
	if (_timeDeltaUni.loc >= 0)	{
		glUniform1f(_timeDeltaUni.loc, (float)_renderTimeDelta);
		GLERRLOG
	}
	if (_dateUni.loc >= 0)	{
		time_t		now = time(0);
		tm			*localTime = localtime(&now);
		double		timeInSeconds = 0.;
		timeInSeconds += localTime->tm_sec;
		timeInSeconds += localTime->tm_min * 60.;
		timeInSeconds += localTime->tm_hour * 60. * 60.;
		glUniform4f(_dateUni.loc, float(localTime->tm_year+1900.), float(localTime->tm_mon+1), float(localTime->tm_mday), float(timeInSeconds));
		GLERRLOG
	}
	if (_renderFrameIndexUni.loc >= 0)	{
		glUniform1i(_renderFrameIndexUni.loc, _renderFrameIndex);
		GLERRLOG
	}
	
	//cout << "\t" << __PRETTY_FUNCTION__ << "- FINISHED" << endl;
}
void ISFScene::_initialize()	{
	if (_deleted)
		return;
	
	GLScene::_initialize();
#if defined(VVGL_TARGETENV_GL2)
	if (glVersion() < GLVersion_2)	{
		glDisable(GL_BLEND);
		GLERRLOG
	}
#endif
	
	//if (_context == nullptr)	{
	//	cout << "\terr: bailing, ctx null, " << __PRETTY_FUNCTION__ << endl;
	//	return;
	//}
}
void ISFScene::_renderCleanup()	{
	if (_deleted)
		return;
	
	GLScene::_renderCleanup();
	
	//if (_context == nullptr)	{
	//	cout << "\terr: bailing, ctx null, " << __PRETTY_FUNCTION__ << endl;
	//	return;
	//}
}
void ISFScene::_render(const GLBufferRef & inTargetBuffer, const VVGL::Size & inSize, const double & inTime, map<int32_t,GLBufferRef> * outPassDict)	{
	//cout << __FUNCTION__ << ", self is " << doc()->name() << ", time is " << inTime << endl;
	//cout << "\tinSize is " << inSize << ", inTargetBuffer is " << *inTargetBuffer << endl;
	//if (inTargetBuffer != nullptr)
	//	cout << "/" << *inTargetBuffer << endl;
	//else
	//	cout << endl;
	
	//	get the doc before we hold any other locks- bail if there's no doc
	ISFDocRef			tmpDoc = doc();
	if (tmpDoc == nullptr)
		return;
	
#if defined(VVGL_SDK_IOS)
	glPushGroupMarkerEXT(0, "All ISF-specific rendering");
	GLERRLOG
#endif
	
	{
		lock_guard<recursive_mutex> lock(_renderLock);
		
		//	update the render size and time vars
		_renderSize = inSize;
		_renderTimeDelta = (inTime<=0.) ? 0. : fabs(inTime-_renderTime);
		_renderTime = inTime;
		
		//	get the buffer pool we're going to use to generate the buffers
		GLBufferPoolRef		bp = _privatePool;
		if (bp==nullptr && inTargetBuffer!=nullptr)
			bp = inTargetBuffer->parentBufferPool;
		if (bp==nullptr)
			bp = GetGlobalBufferPool();
		if (bp==nullptr)	{
			cout << "\tERR: bailing, pool null, " << __PRETTY_FUNCTION__ << endl;
			return;
		}
		
		GLBufferRef				tmpFBO = CreateFBO(false, bp);
		
		_context->makeCurrentIfNotCurrent();
		
		//	tell the doc to evaluate its buffers with the passed render size
		tmpDoc->evalBufferDimensionsWithRenderSize(_renderSize);
		
		//	clear the passed dict of render passes (we'll be placing the rendered content from each pass in it as we progress)
		if (outPassDict != nullptr)
			outPassDict->clear();
		
		


		
		bool					shouldBeFloat = _alwaysRenderToFloat;
		bool					shouldBeIOSurface;
		shouldBeIOSurface = _persistentToIOSurface;

		//_context->makeCurrentIfNotCurrent();

		//	run through the array of passes, rendering each of them
		vector<string>			passes = tmpDoc->renderPasses();
		_passIndex = 1;
		for (const auto & pass : passes)	{
			//cout << "\trendering pass " << _passIndex << endl;
			//	get the name of the target buffer for this pass (if there is a name)
			//string				targetName = passIt.getTargetName();
			ISFPassTargetRef		targetBuffer = nullptr;
			bool					isPersistentBuffer = false;
			bool					isTempBuffer = false;
			//RenderTarget			tmpRenderTarget = RenderTarget(CreateFBO(), nullptr, nullptr);
			RenderTarget			tmpRenderTarget;
			//if (inTargetBuffer != nullptr)	{
				tmpRenderTarget.fbo = tmpFBO;
				_context->makeCurrentIfNotCurrent();
			//}
			
			//	if there's a target buffer name, i need to find the target buffer
			if (pass.size()>0)	{
				//	try to find a persistent buffer matching the target name
				targetBuffer = tmpDoc->persistentPassTargetForKey(pass);
				if (targetBuffer != nullptr)
					isPersistentBuffer = true;
				//	else i couldn't find a persistent buffer matching the target name
				else	{
					//	try to find a temp buffer matching the target name
					targetBuffer = tmpDoc->tempPassTargetForKey(pass);
					if (targetBuffer != nullptr)
						isTempBuffer = true;
					else	{
						cout << "\tERR: failed to locate pers/temp buffer named " << pass << " in " << __PRETTY_FUNCTION__ << endl;
					}
				}
			}
			VVGL::Size			targetBufferSize = (targetBuffer==nullptr) ? inSize : targetBuffer->targetSize();
			if (_passIndex >= passes.size())
				tmpRenderTarget.color = inTargetBuffer;
			else	{
				//tmpRenderTarget.color = (targetBuffer->floatFlag()) ? CreateBGRAFloatTex(targetBufferSize, bp) : CreateBGRATex(targetBufferSize, bp);
				//tmpRenderTarget.color = (targetBuffer->floatFlag()) ? CreateRGBAFloatTex(targetBufferSize, bp) : CreateRGBATex(targetBufferSize, bp);
				
#if defined(VVGL_SDK_MAC)
				if (shouldBeIOSurface)
					tmpRenderTarget.color = (shouldBeFloat || (targetBuffer!=nullptr && targetBuffer->floatFlag())) ? CreateRGBAFloatTexIOSurface(targetBufferSize, true, bp) : CreateRGBATexIOSurface(targetBufferSize, true, bp);
				else
#endif
					tmpRenderTarget.color = (shouldBeFloat || (targetBuffer!=nullptr && targetBuffer->floatFlag())) ? CreateRGBAFloatTex(targetBufferSize, true, bp) : CreateRGBATex(targetBufferSize, true, bp);
				
				//_context->makeCurrentIfNotCurrent();
			}
			//cout << "\ttargetBufferSize is " << targetBufferSize << ", and has target color buffer " << *(tmpRenderTarget.color) << endl;
			
			setSize(targetBufferSize);
			
			//_context->makeCurrentIfNotCurrent();
			
			render(tmpRenderTarget);
			
			//	if there's an out pass dict, add the frame i just rendered into to it at the appropriate key
			if (outPassDict!=nullptr && tmpRenderTarget.color!=nullptr)	{
				//(*outPassDict)[FmtString("%d",_passIndex-1)] = tmpRenderTarget.color;
				(*outPassDict)[_passIndex-1] = tmpRenderTarget.color;
				//cout << "\tstoring " << *tmpRenderTarget.color << " at " << _passIndex-1 << endl;
			}
			
			//	increment the pass index for next time
			++_passIndex;
			
			//	if this was a persistent or temp buffer, store the frame i just rendered
			if (isPersistentBuffer || isTempBuffer)	{
				targetBuffer->setBuffer(tmpRenderTarget.color);
			}
		}
		
		//	now we have to run through the inputs, and set the value of any 'event'-type inputs that were YES to NO
		vector<ISFAttrRef> &	inputs = tmpDoc->inputs();
		for (const auto & attribRef : inputs)	{
			ISFValType		attribType = attribRef->type();
			ISFVal &		currentVal = attribRef->currentVal();
			
			switch (attribType)	{
			case ISFValType_Event:
				if (currentVal.getBoolVal())	{
					attribRef->setCurrentVal(ISFBoolVal(false));
				}
				break;
			default:	//	intentionally blank, only disabling events
				break;
			}
		}
		
		//	increment the rendered frame index!
		++_renderFrameIndex;
		
		//	if there's a pass dict...
		if (outPassDict != nullptr)	{
			//	add the buffer i rendered into (this is the "output" buffer, and is stored at key "-1")
			//(*outPassDict)[string("-1")] = inTargetBuffer;
			(*outPassDict)[-1] = inTargetBuffer;
			//cout << "\tstoring " << *inTargetBuffer << " at " << -1 << endl;
			//	add the buffers for the various image inputs at keys going from 100-199
			int			i=0;
			for (const auto & attrIt : tmpDoc->imageInputs())	{
				GLBufferRef		tmpBuffer = attrIt->getCurrentImageBuffer();
				if (tmpBuffer != nullptr)	{
					//(*outPassDict)[FmtString("%d",100+i)] = tmpBuffer;
					(*outPassDict)[100+i] = tmpBuffer;
					//cout << "\tstoring " << *tmpBuffer << " at " << 100+i << endl;
				}
				
				++i;
			}
			//	add the buffers for the various audio inputs at keys going from 200-299
			i=0;
			for (const auto & attrIt : tmpDoc->audioInputs())	{
				GLBufferRef		tmpBuffer = attrIt->getCurrentImageBuffer();
				if (tmpBuffer != nullptr)	{
					//(*outPassDict)[FmtString("%d",200+i)] = tmpBuffer;
					(*outPassDict)[200+i] = tmpBuffer;
					//cout << "\tstoring " << *tmpBuffer << " at " << 200+i << endl;
				}
				
				++i;
			}
		}
		
		//	run through and delete all the buffers in the temp buffer array
		for (const auto & attrIt : tmpDoc->tempPassTargets())	{
			attrIt->clearBuffer();
		}
	}
	
	//	if i'm supposed to throw any exceptions and there was a GLSL error compiling/linking the program, throw an exception now
	{
		lock_guard<mutex>		lock(_errDictLock);
		if (_throwExceptions && _errDict.size()>0)	{
			ISFErr		tmpErr = ISFErr(ISFErrType_ErrorCompilingGLSL, "Shader Problem", "check error dict for more info", _errDict);
			_errDict.clear();
			throw tmpErr;
		}
	}
	
#if defined(VVGL_SDK_IOS)
	glPopGroupMarkerEXT();
	GLERRLOG
#endif
	
}
void ISFScene::setVertexShaderString(const string & n)	{
	//cout << "*******************************\n";
	//cout << __PRETTY_FUNCTION__ << endl << "\tstring is:\n" << n << endl;
	GLScene::setVertexShaderString(n);
	
	ISFDocRef			tmpDoc = doc();
	if (tmpDoc != nullptr)	{
		for (const auto & attrIt : tmpDoc->inputs())	{
			attrIt->clearUniformLocations();
		}
		for (const auto & attrIt : tmpDoc->imageImports())	{
			attrIt->clearUniformLocations();
		}
	}
}
void ISFScene::setFragmentShaderString(const string & n)	{
	//cout << "*******************************\n";
	//cout << __PRETTY_FUNCTION__ << endl << "\tstring is:\n" << n << endl;
	GLScene::setFragmentShaderString(n);
	
	ISFDocRef			tmpDoc = doc();
	if (tmpDoc != nullptr)	{
		for (const auto & attrIt : tmpDoc->inputs())	{
			attrIt->clearUniformLocations();
		}
		for (const auto & attrIt : tmpDoc->imageImports())	{
			attrIt->clearUniformLocations();
		}
	}
}




}

