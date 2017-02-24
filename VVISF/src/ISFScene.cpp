#include "ISFScene.hpp"
#include "ISFDoc.hpp"




namespace VVISF
{




/*	========================================	*/
#pragma mark --------------------- constructor/destructor


ISFScene::ISFScene()
: VVGLShaderScene()	{
	cout << __PRETTY_FUNCTION__ << endl;
	_setUpRenderCallback();
}
ISFScene::ISFScene(const VVGLContext * inCtx)
: VVGLShaderScene(inCtx)	{
	_setUpRenderCallback();
}


void ISFScene::prepareToBeDeleted()	{
	geoXYVBO = nullptr;
	//	now call the super, which deletes the context
	VVGLShaderScene::prepareToBeDeleted();
}
ISFScene::~ISFScene()	{
	if (!deleted)
		prepareToBeDeleted();
	
	if (compiledInputTypeString != nullptr)	{
		delete compiledInputTypeString;
		compiledInputTypeString = nullptr;
	}
	
	geoXYVBO = nullptr;
}


/*	========================================	*/
#pragma mark --------------------- public methods


void ISFScene::useFile(const string & inPath)	{
	//cout << __PRETTY_FUNCTION__ << "... " << inPath << endl;
	try	{
		lock_guard<recursive_mutex> rlock(renderLock);
		lock_guard<mutex>	plock(propertyLock);
		if (doc != nullptr)
			doc->setParentScene(nullptr);
		doc = nullptr;
		ISFDocRef			newDoc = make_shared<ISFDoc>(inPath, this);
		doc = newDoc;
		
		//	reset the timestamper and render frame index
		timestamper.reset();
		renderTime = 0.;
		renderTimeDelta = 0.;
		passIndex = 0;
		if (compiledInputTypeString!=nullptr)	{
			delete compiledInputTypeString;
			compiledInputTypeString=nullptr;
		}
	}
	catch (ISFErr & exc)	{
		cout << "ERR: " << __PRETTY_FUNCTION__ << "-> caught exception: " << exc.getTypeString() << ": " << exc.general << ", " << exc.specific << endl;
		doc = nullptr;
		//	reset the timestamper and render frame index
		timestamper.reset();
		renderTime = 0.;
		renderTimeDelta = 0.;
		passIndex = 0;
		if (compiledInputTypeString!=nullptr)	{
			delete compiledInputTypeString;
			compiledInputTypeString=nullptr;
		}
		
		//	if i'm supposed to throw the exception then do so now
		if (throwExceptions)
			throw exc;
	}
	
}
string ISFScene::getFilePath()	{
	lock_guard<mutex>		lock(propertyLock);
	return doc->getPath();
}
string ISFScene::getFileName()	{
	lock_guard<mutex>		lock(propertyLock);
	return doc->getName();
}
string ISFScene::getFileDescription()	{
	lock_guard<mutex>		lock(propertyLock);
	return doc->getDescription();
}
string ISFScene::getFileCredit()	{
	lock_guard<mutex>		lock(propertyLock);
	return doc->getCredit();
}
ISFFileType ISFScene::getFileType()	{
	lock_guard<mutex>		lock(propertyLock);
	return doc->getType();
}
vector<string> ISFScene::getFileCategories()	{
	lock_guard<mutex>		lock(propertyLock);
	return doc->getCategories();
}


void ISFScene::setBufferForInputNamed(const VVGLBufferRef & inBuffer, const string & inName)	{
	ISFDocRef			tmpDoc = getDoc();
	ISFAttrRef		tmpAttr = tmpDoc->getInput(inName);
	tmpAttr->setCurrentImageBuffer(inBuffer);
}
void ISFScene::setFilterInputBuffer(const VVGLBufferRef & inBuffer)	{
	//cout << __PRETTY_FUNCTION__ << ", buffer is " << inBuffer << endl;
	ISFAttrRef	filterInput = getInputNamed(string("inputImage"));
	if (filterInput == nullptr)
		return;
	filterInput->setCurrentImageBuffer(inBuffer);
	VVGLBufferRef		checkBuffer = filterInput->getCurrentImageBuffer();
	//cout << "\tcheck buffer is " << checkBuffer;
	//if (checkBuffer==nullptr) cout << "/null" << endl; else cout << "/" << *checkBuffer << endl;
}
void ISFScene::setBufferForInputImageKey(const VVGLBufferRef & inBuffer, const string & inString)	{
	ISFDocRef			tmpDoc = getDoc();
	for (const auto & attrIt : tmpDoc->getImageInputs())	{
		if (attrIt->getName() == inString)	{
			attrIt->setCurrentImageBuffer(inBuffer);
			break;
		}
	}
}
void ISFScene::setBufferForAudioInputKey(const VVGLBufferRef & inBuffer, const string & inString)	{
	ISFDocRef			tmpDoc = getDoc();
	for (const auto & attrIt : tmpDoc->getAudioInputs())	{
		if (attrIt->getName() == inString)	{
			attrIt->setCurrentImageBuffer(inBuffer);
			break;
		}
	}
}
VVGLBufferRef ISFScene::getBufferForImageInput(const string & inKey)	{
	ISFDocRef			tmpDoc = getDoc();
	for (const auto & attrIt : tmpDoc->getImageInputs())	{
		if (attrIt->getName() == inKey)	{
			return attrIt->getCurrentImageBuffer();
		}
	}
	return nullptr;
}
VVGLBufferRef ISFScene::getBufferForAudioInput(const string & inKey)	{
	ISFDocRef			tmpDoc = getDoc();
	for (const auto & attrIt : tmpDoc->getAudioInputs())	{
		if (attrIt->getName() == inKey)	{
			return attrIt->getCurrentImageBuffer();
		}
	}
	return nullptr;
}
VVGLBufferRef ISFScene::getPersistentBufferNamed(const string & inKey)	{
	ISFDocRef			tmpDoc = getDoc();
	for (const auto & targetIt : tmpDoc->getPersistentBuffers())	{
		if (targetIt->getName() == inKey)	{
			return targetIt->getBuffer();
		}
	}
	return nullptr;
}
VVGLBufferRef ISFScene::getTempBufferNamed(const string & inKey)	{
	ISFDocRef			tmpDoc = getDoc();
	for (const auto & targetIt : tmpDoc->getTempBuffers())	{
		if (targetIt->getName() == inKey)	{
			return targetIt->getBuffer();
		}
	}
	return nullptr;
}


void ISFScene::setValueForInputNamed(const ISFVal & inVal, const string & inName)	{
	//cout << __FUNCTION__ << "- " << inVal << ", " << inName << endl;
	ISFAttrRef		inputRef = getInputNamed(inName);
	if (inputRef == nullptr)
		return;
	if (inVal.getType() != inputRef->getType())	{
		cout << "\tERR: tried to pass val to input of wrong type, " << __PRETTY_FUNCTION__ << endl;
		return;
	}
	inputRef->setCurrentVal(inVal);
}
ISFVal ISFScene::valueForInputNamed(const string & inName)	{
	ISFAttrRef		inputRef = getInputNamed(inName);
	if (inputRef == nullptr)
		return ISFNullVal();
	return inputRef->getCurrentVal();
}


ISFAttrRef ISFScene::getInputNamed(const string & inName)	{
	ISFDocRef		tmpDoc = getDoc();
	for (const auto & attrIt : tmpDoc->getInputs())	{
		if (attrIt->getName() == inName)
			return attrIt;
	}
	return nullptr;
}
vector<ISFAttrRef> ISFScene::getImageInputs()	{
	ISFDocRef		tmpDoc = getDoc();
	return tmpDoc->getImageInputs();
}
vector<ISFAttrRef> ISFScene::getAudioInputs()	{
	ISFDocRef		tmpDoc = getDoc();
	return tmpDoc->getAudioInputs();
}
vector<ISFAttrRef> ISFScene::getImageImports()	{
	ISFDocRef		tmpDoc = getDoc();
	return tmpDoc->getImageImports();
}


/*	========================================	*/
#pragma mark --------------------- public rendering interface


VVGLBufferRef ISFScene::createAndRenderABuffer(const VVGL::Size & inSize, const VVGLBufferPoolRef & inPoolRef)	{
	return createAndRenderABuffer(inSize, timestamper.nowTime().getTimeInSeconds(), inPoolRef);
}
VVGLBufferRef ISFScene::createAndRenderABuffer(const VVGL::Size & inSize, const double & inRenderTime, const VVGLBufferPoolRef & inPoolRef)	{
	ISFDocRef		tmpDoc = getDoc();
	if (tmpDoc == nullptr)
		return nullptr;
	//cout << "\ttmpDoc is " << *tmpDoc << endl;
	VVGLBufferRef			returnMe = nullptr;
	vector<string>			passNames = tmpDoc->getRenderPasses();
	ISFPassTargetRef		lastPass = nullptr;
	if (passNames.size()>0)	{
		string					lastPassName = passNames.back();
		lastPass = tmpDoc->getPersistentTargetForKey(lastPassName);
		if (lastPass == nullptr)
			lastPass = tmpDoc->getTempTargetForKey(lastPassName);
	}
	
	VVGLBufferPoolRef		bp = inPoolRef;
	if (bp == nullptr && privatePool!=nullptr) bp = privatePool;
	if (bp == nullptr) bp = GetGlobalBufferPool();
	
	returnMe = (lastPass!=nullptr && lastPass->getFloatFlag())
		? CreateRGBAFloatTex(inSize, bp)
		: CreateRGBATex(inSize, bp);
	//returnMe = (lastPass!=nullptr && lastPass->getFloatFlag())
	//	? CreateBGRAFloatTex(inSize, bp)
	//	: CreateBGRATex(inSize, bp);
	
	renderToBuffer(returnMe, inSize, inRenderTime);
	
	return returnMe;
	/*
	VVGLBufferRef		returnMe = CreateRGBATex(inSize, bp);
	renderToBuffer(returnMe);
	return returnMe;
	*/
}
void ISFScene::renderToBuffer(const VVGLBufferRef & inTargetBuffer, const VVGL::Size & inRenderSize, const double & inRenderTime, map<int32_t,VVGLBufferRef> * outPassDict)	{
	_render(inTargetBuffer, inRenderSize, inRenderTime, outPassDict);
}
void ISFScene::renderToBuffer(const VVGLBufferRef & inTargetBuffer, const VVGL::Size & inRenderSize, const double & inRenderTime)	{
	_render(inTargetBuffer, inRenderSize, inRenderTime, nullptr);
}
void ISFScene::renderToBuffer(const VVGLBufferRef & inTargetBuffer, const VVGL::Size & inRenderSize)	{
	_render(inTargetBuffer, inRenderSize, timestamper.nowTime().getTimeInSeconds(), nullptr);
}
void ISFScene::renderToBuffer(const VVGLBufferRef & inTargetBuffer)	{
	if (inTargetBuffer != nullptr)
		_render(inTargetBuffer, inTargetBuffer->srcRect.size, timestamper.nowTime().getTimeInSeconds(), nullptr);
	else
		_render(nullptr, size, timestamper.nowTime().getTimeInSeconds(), nullptr);
}


void ISFScene::setSize(const VVGL::Size & n)	{
	//cout << __PRETTY_FUNCTION__ << ", self is " << this << endl;
	//renderSize = n;	//	do NOT set the renderSize here (if we do then it will be changed every time a pass is rendered)
	VVGLScene::setSize(n);
	//cout << "\tnew size is " << size << endl;
}


/*	========================================	*/
#pragma mark --------------------- protected rendering methods


void ISFScene::_setUpRenderCallback()	{
	setRenderCallback([&](const VVGLScene & s)	{
		//cout << __PRETTY_FUNCTION__ << endl;
		VVGL::Rect				tmpRect(0,0,0,0);
		tmpRect.size = static_cast<const ISFScene&>(s).size;
		//cout << "\tverts based on rect " << tmpRect << endl;
#if ISF_TARGET_MAC || ISF_TARGET_GLFW
		glColor4f(1., 1., 1., 1.);
		GLERRLOG
		glEnableClientState(GL_VERTEX_ARRAY);
		GLERRLOG
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		GLERRLOG
		glDisableClientState(GL_COLOR_ARRAY);
		GLERRLOG
		float				verts[] = {
			(float)MinX(tmpRect), (float)MinY(tmpRect), 0.0,
			(float)MinX(tmpRect), (float)MaxY(tmpRect), 0.0,
			(float)MaxX(tmpRect), (float)MaxY(tmpRect), 0.0,
			(float)MaxX(tmpRect), (float)MinY(tmpRect), 0.0
		};
		glVertexPointer(3, GL_FLOAT, 0, verts);
		GLERRLOG
		glDrawArrays(GL_QUADS, 0, 4);
		GLERRLOG
#elif ISF_TARGET_IOS || ISF_TARGET_RPI
		GLfloat			geoCoords[] = {
			(GLfloat)MinX(tmpRect), (GLfloat)MinY(tmpRect),
			(GLfloat)MaxX(tmpRect), (GLfloat)MinY(tmpRect),
			(GLfloat)MinX(tmpRect), (GLfloat)MaxY(tmpRect),
			(GLfloat)MaxX(tmpRect), (GLfloat)MaxY(tmpRect)
		};
		if (vertexAttribLoc >= 0)	{
			glEnableVertexAttribArray(vertexAttribLoc);
			GLERRLOG
			glVertexAttribPointer(vertexAttribLoc, 2, GL_FLOAT, GL_FALSE, 0, geoCoords);
			GLERRLOG
		}
		
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		
		if (vertexAttribLoc >= 0)	{
			glDisableVertexAttribArray(vertexAttribLoc);
		}
		
		/*
		if (geoXYVBO != nullptr)	{
			glBindBuffer(GL_ARRAY_BUFFER, geoXYVBO->name);
			GLERRLOG
			glEnableVertexAttribArray(0);
			GLERRLOG
			glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, NULL);
			GLERRLOG
			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
			GLERRLOG
			glBindBuffer(GL_ARRAY_BUFFER, 0);
			GLERRLOG
		}
		else
			cout << "\terr: geoXYVBO null in " << __PRETTY_FUNCTION__ << endl;
		*/
#endif
	});
}
void ISFScene::_renderPrep()	{
	//cout << __PRETTY_FUNCTION__ << endl;
	if (deleted)
		return;
	if (doc == nullptr)
		return;
	
	//	check the texture type string- if it's changed, we'll have to recompile the shaders!
	string		currentTextureTypeString = doc->generateTextureTypeString();
	if (compiledInputTypeString==nullptr || (currentTextureTypeString!=*compiledInputTypeString))	{
		if (compiledInputTypeString != nullptr)
			delete compiledInputTypeString;
		compiledInputTypeString = new string(currentTextureTypeString);
		
		//	update the shader strings
		string		tmpFrag;
		string		tmpVert;
		doc->generateShaderSource(&tmpFrag, &tmpVert);
		setVertexShaderString(tmpVert);
		setFragmentShaderString(tmpFrag);
	}
	
	//	store these values, then check them after the super's "_renderPrep"...
	bool		vShaderUpdatedFlag = vsStringUpdated;
	bool		fShaderUpdatedFlag = fsStringUpdated;
	
	//	tell the super to do its _renderPrep, which will compile the shader and get it all set up if necessary
	VVGLShaderScene::_renderPrep();
	
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
	
	//	...if either of these values have changed, the program has been recompiled and i need to find new uniform locations for all the attributes (the uniforms in the GLSL programs)
	bool		findNewUniforms = false;
	if (vShaderUpdatedFlag!=vsStringUpdated || fShaderUpdatedFlag!=fsStringUpdated)
		findNewUniforms = true;
	
	//	need a GL context
	if (context == nullptr)	{
		cout << "\terr: bailing, context null, " << __PRETTY_FUNCTION__ << endl;
		return;
	}
	
	//	set up some vars and some blocks that we're going to use to cache the locations of uniforms in the attributes of the ISFDoc instance, and eventually push those vals to GL
	GLint				samplerLoc = 0;
	GLint				textureCount = 0;
	VVGLBufferRef		tmpBuffer = nullptr;
	VVGL::Rect				tmpRect;
	char				tmpCString[64];
	memset(tmpCString, 0, 64);
	
	//	this block retrieves and stores the uniform location from the passed attribute for simple val-based attributes
	auto		setAttrUniformsSimpleValBlock = [&](const ISFAttrRef & inAttr)	{
		const char *	tmpAttrName = inAttr->getName().c_str();
		samplerLoc = (program<=0) ? -1 : glGetUniformLocation(program, tmpAttrName);
		GLERRLOG
		inAttr->setUniformLocation(0, samplerLoc);
	};
	//	this block retrieves and stores the uniform locations from the passed attribute for cube-based attributes
	auto		setAttrUniformsCubeBlock = [&](const ISFAttrRef & inAttr)	{
		const char *		tmpAttrName = inAttr->getName().c_str();
		samplerLoc = (program<=0) ? -1 : glGetUniformLocation(program, tmpAttrName);
		GLERRLOG
		inAttr->setUniformLocation(0, samplerLoc);
		
		sprintf(tmpCString,"_%s_imgSize",tmpAttrName);
		samplerLoc = (program<=0) ? -1 : glGetUniformLocation(program, tmpCString);
		GLERRLOG
		inAttr->setUniformLocation(2, samplerLoc);
	};
	//	this block retrieves and stores the uniform locations from the passed attribute for all other image-based attributes
	auto		setAttrUniformsImageBlock = [&](const ISFAttrRef & inAttr)	{
		const char *		tmpAttrName = inAttr->getName().c_str();
		samplerLoc = (program<=0) ? -1 : glGetUniformLocation(program, tmpAttrName);
		GLERRLOG
		inAttr->setUniformLocation(0, samplerLoc);
		
		sprintf(tmpCString,"_%s_imgRect",tmpAttrName);
		samplerLoc = (program<=0) ? -1 : glGetUniformLocation(program, tmpCString);
		GLERRLOG
		inAttr->setUniformLocation(1, samplerLoc);
		
		sprintf(tmpCString,"_%s_imgSize",tmpAttrName);
		samplerLoc = (program<=0) ? -1 : glGetUniformLocation(program, tmpCString);
		GLERRLOG
		inAttr->setUniformLocation(2, samplerLoc);
		
		sprintf(tmpCString,"_%s_flip",tmpAttrName);
		samplerLoc = (program<=0) ? -1 : glGetUniformLocation(program, tmpCString);
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
				glUniform2f(samplerLoc, tmpRect.size.width, tmpRect.size.height);
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
				glUniform4f(samplerLoc,tmpRect.origin.x,tmpRect.origin.y,tmpRect.size.width,tmpRect.size.height);
				GLERRLOG
			}
			//	pass the size to the program
			tmpRect = (tmpBuffer==nullptr) ? VVGL::Rect(0,0,1,1) : tmpBuffer->srcRect;
			samplerLoc = inAttr->getUniformLocation(2);
			if (samplerLoc >= 0)	{
				glUniform2f(samplerLoc,tmpRect.size.width,tmpRect.size.height);
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
		const char *		tmpTargetName = inTarget->getName().c_str();
		samplerLoc = (program<=0) ? -1 : glGetUniformLocation(program, tmpTargetName);
		if (samplerLoc >= 0)
			inTarget->setUniformLocation(0, samplerLoc);
		
		sprintf(tmpCString,"_%s_imgSize",tmpTargetName);
		samplerLoc = (program<=0) ? -1 : glGetUniformLocation(program, tmpCString);
		if (samplerLoc >= 0)
			inTarget->setUniformLocation(2, samplerLoc);
	};
	*/
	auto		setTargetUniformsImageBlock = [&](const ISFPassTargetRef & inTarget)	{
		const char *		tmpTargetName = inTarget->getName().c_str();
		samplerLoc = (program<=0) ? -1 : glGetUniformLocation(program, tmpTargetName);
		GLERRLOG
		inTarget->setUniformLocation(0, samplerLoc);
		
		sprintf(tmpCString,"_%s_imgRect",tmpTargetName);
		samplerLoc = (program<=0) ? -1 : glGetUniformLocation(program, tmpCString);
		GLERRLOG
		inTarget->setUniformLocation(1, samplerLoc);
		
		sprintf(tmpCString,"_%s_imgSize",tmpTargetName);
		samplerLoc = (program<=0) ? -1 : glGetUniformLocation(program, tmpCString);
		GLERRLOG
		inTarget->setUniformLocation(2, samplerLoc);
		
		sprintf(tmpCString,"_%s_flip",tmpTargetName);
		samplerLoc = (program<=0) ? -1 : glGetUniformLocation(program, tmpCString);
		GLERRLOG
		inTarget->setUniformLocation(3, samplerLoc);
	};
	/*
	auto		pushTargetUniformsCubeBlock = [&](const ISFPassTargetRef & inTarget)	{
		tmpBuffer = inTarget->getBuffer();
		if (tmpBuffer != nullptr)	{
			//	pass the actual texture to the program
			glActiveTexture(GL_TEXTURE0 + textureCount);
			glBindTexture(tmpBuffer->desc.target, tmpBuffer->name);
			
			samplerLoc = inTarget->getUniformLocation(0);
			if (samplerLoc >= 0)
				glUniform1i(samplerLoc, textureCount);
			++textureCount;
			//	pass the size to the program
			tmpRect = tmpBuffer->srcRect;
			samplerLoc = inTarget->getUniformLocation(2);
			if (samplerLoc >= 0)
				glUniform2f(samplerLoc, tmpRect.size.width, tmpRect.size.height);
		}
	};
	*/
	auto		pushTargetUniformsImageBlock = [&](const ISFPassTargetRef & inTarget)	{
		tmpBuffer = inTarget->getBuffer();
		if (tmpBuffer != nullptr)	{
			//	pass the actual texture to the program
			glActiveTexture(GL_TEXTURE0 + textureCount);
			GLERRLOG
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
				glUniform4f(samplerLoc,tmpRect.origin.x,tmpRect.origin.y,tmpRect.size.width,tmpRect.size.height);
				GLERRLOG
			}
			//	pass the size to the program
			tmpRect = tmpBuffer->srcRect;
			samplerLoc = inTarget->getUniformLocation(2);
			if (samplerLoc >= 0)	{
				glUniform2f(samplerLoc,tmpRect.size.width,tmpRect.size.height);
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
	vector<ISFAttrRef> &	inputs = doc->getInputs();
	for (const auto attribRef : inputs)	{
		ISFValType			attribType = attribRef->getType();
		ISFVal &			currentVal = attribRef->getCurrentVal();
		
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
			attribRef->setCurrentVal(ISFBoolVal(false));
			//cout << "\tchecking: " << attribRef->getCurrentVal().getBoolVal() << endl;
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
			//cout << "\tprocessing float-type input named " << attribRef->getName() << ", val is " << currentVal.getDoubleVal() << endl;
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
				double		*pointVals = currentVal.getPointValPtr();
				if (pointVals == nullptr)	{
					glUniform2f(samplerLoc, 0., 0.);
					GLERRLOG
				}
				else	{
					glUniform2f(samplerLoc, pointVals[0], pointVals[1]);
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
				double		*colorVals = currentVal.getColorValPtr();
				if (colorVals == nullptr)	{
					glUniform4f(samplerLoc, 0., 0., 0., 0.);
					GLERRLOG
				}
				else	{
					glUniform4f(samplerLoc, colorVals[0], colorVals[1], colorVals[2], colorVals[3]);
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
			//VVGLBufferRef		tmpBuffer = attribRef->getCurrentImageBuffer();
			//cout << "\tprocessing image-type input named " << attribRef->getName();
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
	vector<ISFAttrRef> &	imageImports = doc->getImageImports();
	for (const auto attribRef : imageImports)	{
		if (attribRef->getType() == ISFValType_Cube)	{
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
	const vector<ISFPassTargetRef>	persistentBuffers = doc->getPersistentBuffers();
	for (const auto targetRef : persistentBuffers)	{
		if (findNewUniforms)
			setTargetUniformsImageBlock(targetRef);
		pushTargetUniformsImageBlock(targetRef);
	}
	
	//	run through the temp buffers, applying the current values to the program
	const vector<ISFPassTargetRef>	tempBuffers = doc->getTempBuffers();
	for (const auto targetRef : tempBuffers)	{
		if (findNewUniforms)
			setTargetUniformsImageBlock(targetRef);
		pushTargetUniformsImageBlock(targetRef);
	}
	
	//	if we're finding new uniforms then we also have to update the uniform locations of some standard inputs
	if (findNewUniforms)	{
		vertexAttribLoc = (program<=0) ? -1 : glGetAttribLocation(program, "a_position");
		GLERRLOG
		renderSizeUniformLoc = (program<=0) ? -1 : glGetUniformLocation(program, "RENDERSIZE");
		GLERRLOG
		passIndexUniformLoc = (program<=0) ? -1 : glGetUniformLocation(program, "PASSINDEX");
		GLERRLOG
		timeUniformLoc = (program<=0) ? -1 : glGetUniformLocation(program, "TIME");
		GLERRLOG
		timeDeltaUniformLoc = (program<=0) ? -1 : glGetUniformLocation(program, "TIMEDELTA");
		GLERRLOG
		dateUniformLoc = (program<=0) ? -1 : glGetUniformLocation(program, "DATE");
		GLERRLOG
		renderFrameIndexUniformLoc = (program<=0) ? -1 : glGetUniformLocation(program, "FRAMEINDEX");
		GLERRLOG
	}
	//	push the standard inputs to the program
	if (renderSizeUniformLoc >= 0)	{
		glUniform2f(renderSizeUniformLoc, size.width, size.height);
		GLERRLOG
	}
	if (passIndexUniformLoc >= 0)	{
		glUniform1i(passIndexUniformLoc, passIndex-1);
		GLERRLOG
	}
	if (timeUniformLoc >= 0)	{
		glUniform1f(timeUniformLoc, (float)renderTime);
		GLERRLOG
	}
	if (timeDeltaUniformLoc >= 0)	{
		glUniform1f(timeDeltaUniformLoc, (float)renderTimeDelta);
		GLERRLOG
	}
	if (dateUniformLoc >= 0)	{
		time_t		now = time(0);
		tm			*localTime = localtime(&now);
		double		timeInSeconds = 0.;
		timeInSeconds += localTime->tm_sec;
		timeInSeconds += localTime->tm_min * 60.;
		timeInSeconds += localTime->tm_hour * 60. * 60.;
		glUniform4f(dateUniformLoc, localTime->tm_year+1900., localTime->tm_mon+1, localTime->tm_mday, timeInSeconds);
		GLERRLOG
	}
	if (renderFrameIndexUniformLoc >= 0)	{
		glUniform1i(renderFrameIndexUniformLoc, renderFrameIndex);
		GLERRLOG
	}
	
	//cout << "\t" << __PRETTY_FUNCTION__ << "- FINISHED" << endl;
}
void ISFScene::_initialize()	{
	if (deleted)
		return;
	
	VVGLShaderScene::_initialize();
	
	//if (context == nullptr)	{
	//	cout << "\terr: bailing, ctx null, " << __PRETTY_FUNCTION__ << endl;
	//	return;
	//}
}
void ISFScene::_renderCleanup()	{
	if (deleted)
		return;
	
	VVGLShaderScene::_renderCleanup();
	
	//if (context == nullptr)	{
	//	cout << "\terr: bailing, ctx null, " << __PRETTY_FUNCTION__ << endl;
	//	return;
	//}
}
void ISFScene::_render(const VVGLBufferRef & inTargetBuffer, const VVGL::Size & inSize, const double & inTime, map<int32_t,VVGLBufferRef> * outPassDict)	{
	//cout << __FUNCTION__ << ", self is " << getFileName() << ", time is " << inTime << endl;
	//cout << "\tinSize is " << inSize << ", inTargetBuffer is " << *inTargetBuffer << endl;
	//if (inTargetBuffer != nullptr)
	//	cout << "/" << *inTargetBuffer << endl;
	//else
	//	cout << endl;
	
	//	get the doc before we hold any other locks- bail if there's no doc
	ISFDocRef			tmpDoc = getDoc();
	if (tmpDoc == nullptr)
		return;
	
#if ISF_TARGET_IOS
	glPushGroupMarkerEXT(0, "All ISF-specific rendering");
#endif
	
	{
		lock_guard<recursive_mutex> lock(renderLock);
		
		//	update the render size and time vars
		renderSize = inSize;
		renderTimeDelta = (inTime<=0.) ? 0. : fabs(inTime-renderTime);
		renderTime = inTime;
		
		//	tell the doc to evaluate its buffers with the passed render size
		tmpDoc->evalBufferDimensionsWithRenderSize(renderSize);
		
		//	clear the passed dict of render passes (we'll be placing the rendered content from each pass in it as we progress)
		if (outPassDict != nullptr)
			outPassDict->clear();
		
		//	get the buffer pool we're going to use to generate the buffers
		VVGLBufferPoolRef		bp = privatePool;
		if (bp==nullptr && inTargetBuffer!=nullptr) bp = inTargetBuffer->parentBufferPool;
		if (bp==nullptr) bp = GetGlobalBufferPool();
		
		//	run through the array of passes, rendering each of them
		vector<string>			passes = tmpDoc->getRenderPasses();
		passIndex = 1;
		for (const auto & pass : passes)	{
			//cout << "\trendering pass " << passIndex << endl;
			//	get the name of the target buffer for this pass (if there is a name)
			//string				targetName = passIt.getTargetName();
			ISFPassTargetRef		targetBuffer = nullptr;
			bool					isPersistentBuffer = false;
			bool					isTempBuffer = false;
			//RenderTarget			tmpRenderTarget = RenderTarget(CreateFBO(), nullptr, nullptr);
			RenderTarget			tmpRenderTarget;
			if (inTargetBuffer != nullptr)	{
				tmpRenderTarget.fbo = CreateFBO(bp);
				context->makeCurrentIfNotCurrent();
			}
			
			//	if there's a target buffer name, i need to find the target buffer
			if (pass.size()>0)	{
				//	try to find a persistent buffer matching the target name
				targetBuffer = tmpDoc->getPersistentTargetForKey(pass);
				if (targetBuffer != nullptr)
					isPersistentBuffer = true;
				//	else i couldn't find a persistent buffer matching the target name
				else	{
					//	try to find a temp buffer matching the target name
					targetBuffer = tmpDoc->getTempTargetForKey(pass);
					if (targetBuffer != nullptr)
						isTempBuffer = true;
					else	{
						cout << "\tERR: failed to locate pers/temp buffer named " << pass << " in " << __PRETTY_FUNCTION__ << endl;
					}
				}
			}
			VVGL::Size			targetBufferSize = (targetBuffer==nullptr) ? inSize : targetBuffer->targetSize();
			if (passIndex >= passes.size())
				tmpRenderTarget.color = inTargetBuffer;
			else	{
				tmpRenderTarget.color = (targetBuffer->getFloatFlag()) ? CreateRGBAFloatTex(targetBufferSize, bp) : CreateRGBATex(targetBufferSize, bp);
				//tmpRenderTarget.color = (targetBuffer->getFloatFlag()) ? CreateBGRAFloatTex(targetBufferSize, bp) : CreateBGRATex(targetBufferSize, bp);
				context->makeCurrentIfNotCurrent();
			}
			//cout << "\ttargetBufferSize is " << targetBufferSize << ", and has target color buffer " << tmpRenderTarget.color << " and size " << targetBufferSize << endl;
			
			setSize(targetBufferSize);
			
			render(tmpRenderTarget);
			
			//	if there's an out pass dict, add the frame i just rendered into to it at the appropriate key
			if (outPassDict!=nullptr && tmpRenderTarget.color!=nullptr)	{
				//(*outPassDict)[FmtString("%d",passIndex-1)] = tmpRenderTarget.color;
				(*outPassDict)[passIndex-1] = tmpRenderTarget.color;
			}
			
			//	increment the pass index for next time
			++passIndex;
			
			//	if this was a persistent or temp buffer, store the frame i just rendered
			if (isPersistentBuffer || isTempBuffer)	{
				targetBuffer->setBuffer(tmpRenderTarget.color);
			}
		}
		
		//	increment the rendered frame index!
		++renderFrameIndex;
		
		//	if there's a pass dict...
		if (outPassDict != nullptr)	{
			//	add the buffer i rendered into (this is the "output" buffer, and is stored at key "-1")
			//(*outPassDict)[string("-1")] = inTargetBuffer;
			(*outPassDict)[-1] = inTargetBuffer;
			//	add the buffers for the various image inputs at keys going from 100-199
			int			i=0;
			for (const auto & attrIt : tmpDoc->getImageInputs())	{
				VVGLBufferRef		tmpBuffer = attrIt->getCurrentImageBuffer();
				if (tmpBuffer != nullptr)	{
					//(*outPassDict)[FmtString("%d",100+i)] = tmpBuffer;
					(*outPassDict)[100+i] = tmpBuffer;
				}
				
				++i;
			}
			//	add the buffers for the various audio inputs at keys going from 200-299
			i=0;
			for (const auto & attrIt : tmpDoc->getAudioInputs())	{
				VVGLBufferRef		tmpBuffer = attrIt->getCurrentImageBuffer();
				if (tmpBuffer != nullptr)	{
					//(*outPassDict)[FmtString("%d",200+i)] = tmpBuffer;
					(*outPassDict)[200+i] = tmpBuffer;
				}
				
				++i;
			}
		}
		
		//	run through and delete all the buffers in the temp buffer array
		for (const auto & attrIt : tmpDoc->getTempBuffers())	{
			attrIt->clearBuffer();
		}
	}
	
	//	if i'm supposed to throw any exceptions and there was a GLSL error compiling/linking the program, throw an exception now
	{
		lock_guard<mutex>		lock(errDictLock);
		if (throwExceptions && errDict.size()>0)	{
			ISFErr		tmpErr = ISFErr(ISFErrType_ErrorCompilingGLSL, "Shader Problem", "check error dict for more info", errDict);
			errDict.clear();
			throw tmpErr;
		}
	}
	
#if ISF_TARGET_IOS
	glPopGroupMarkerEXT();
#endif
	
}
void ISFScene::setVertexShaderString(const string & n)	{
	//cout << __PRETTY_FUNCTION__ << endl << "\tstring is:\n" << n << endl;
	VVGLShaderScene::setVertexShaderString(n);
	
	ISFDocRef			tmpDoc = getDoc();
	for (const auto & attrIt : tmpDoc->getInputs())	{
		attrIt->clearUniformLocations();
	}
	for (const auto & attrIt : tmpDoc->getImageImports())	{
		attrIt->clearUniformLocations();
	}
}
void ISFScene::setFragmentShaderString(const string & n)	{
	//cout << __PRETTY_FUNCTION__ << endl << "\tstring is:\n" << n << endl;
	VVGLShaderScene::setFragmentShaderString(n);
	
	ISFDocRef			tmpDoc = getDoc();
	for (const auto & attrIt : tmpDoc->getInputs())	{
		attrIt->clearUniformLocations();
	}
	for (const auto & attrIt : tmpDoc->getImageImports())	{
		attrIt->clearUniformLocations();
	}
}




}

