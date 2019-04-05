#include "ISFDoc.hpp"
#include "ISFAttr.hpp"

#include <iostream>
#include <fstream>

#include "ISFPassTarget.hpp"
#include "ISFScene.hpp"

#include "nlohmann_json/json.hpp"
using json = nlohmann::json;




namespace VVISF
{




using namespace std;
using namespace VVGL;




/*	========================================	*/
#pragma mark *************** ISFDoc class ***************

/*	========================================	*/
#pragma mark --------------------- constructor/destructor


ISFDoc::ISFDoc(const string & inFSContents, const string & inVSContents, const string & inImportsDir, ISFScene * inParentScene, const bool & throwExcept)	{
	_parentScene = inParentScene;
	
	//_path = new string("");
	if (inImportsDir.length() < 1)
		_path = new string("/XXX.fs");
	else	{
		_path = new string( FmtString("%s/XXX.fs", StringByDeletingLastSlash(inImportsDir).c_str()) );
	}
	_name = new string("");
	_throwExcept = throwExcept;
	
	//_vertShaderSource = new string(ISFVertPassthru_GL2);
	_vertShaderSource = new string(inVSContents);
	
	_initWithRawFragShaderString(inFSContents);
}
ISFDoc::ISFDoc(const string & inPath, ISFScene * inParentScene, const bool & throwExcept) noexcept(false)	{
	//cout << __PRETTY_FUNCTION__ << endl;
	//cout << "\t" << inPath << endl;
	
	_parentScene = inParentScene;
	
	//	set the local path and name variables
	_path = new string(inPath);
	_name = new string(LastPathComponent(inPath));
	_throwExcept = throwExcept;
	//cout << "\tpath is " << *_path << endl;
	//cout << "\tname is " << *_name << endl;
	
	
	//	read the passed file into a string
	ifstream		fin;
	fin.open(inPath);
	if (!fin.is_open())	{
		if (_throwExcept)
			throw ISFErr(ISFErrType_MissingResource, "cannot create ISFDoc from passed path", inPath);
		else
			return;
	}
	string			rawFile( static_cast<stringstream const &>(stringstream() << fin.rdbuf()).str() );
	fin.close();
	/*
	cout << "\trawFile is:\n";
	cout << "**************************" << endl;
	cout << rawFile << endl;
	cout << "**************************" << endl;
	*/


	//	look for a vert shader that matches the name of the frag shader
	string			noExtPath = StringByDeletingExtension(inPath);
	string			tmpPath;
	fin.open(noExtPath+".vs");
	//	if the file's not open, try opening a file with a different path
	if (!fin.is_open())	{
		fin.open(noExtPath+".vert");
	}
	//	if the file's still not open then it just didn't exist- i have to use the passthru shader
	if (!fin.is_open())	{
		_vertShaderSource = new string(ISFVertPassthru_GL2);
	}
	//	else the file's open- read it into a string
	else	{
		_vertShaderSource = new string( static_cast<stringstream const &>(stringstream() << fin.rdbuf()).str() );
		fin.close();
	}
	
	
	//	call the init method with the contents of the file we read in
	_initWithRawFragShaderString(rawFile);
}
ISFDoc::~ISFDoc()	{
	//cout << __PRETTY_FUNCTION__ << endl;
	
	lock_guard<recursive_mutex>		lock(_propLock);
	
	if (_path != nullptr)	{
		delete _path;
		_path = nullptr;
	}
	if (_name != nullptr)	{
		delete _name;
		_name = nullptr;
	}
	if (_description != nullptr)	{
		delete _description;
		_description = nullptr;
	}
	if (_credit != nullptr)	{
		delete _credit;
		_credit = nullptr;
	}
	if (_vsn != nullptr)	{
		delete _vsn;
		_vsn = nullptr;
	}
	
	
	if (_jsonSourceString != nullptr)	{
		delete _jsonSourceString;
		_jsonSourceString = nullptr;
	}
	if (_jsonString != nullptr)	{
		delete _jsonString;
		_jsonString = nullptr;
	}
	if (_vertShaderSource != nullptr)	{
		delete _vertShaderSource;
		_vertShaderSource = nullptr;
	}
	if (_fragShaderSource != nullptr)	{
		delete _fragShaderSource;
		_fragShaderSource = nullptr;
	}
}


/*	========================================	*/
#pragma mark --------------------- getters


vector<ISFAttrRef> ISFDoc::inputsOfType(const ISFValType & n)	{
	auto		returnMe = vector<ISFAttrRef>(0);
	lock_guard<recursive_mutex>		lock(_propLock);
	
	for (auto it=_inputs.begin(); it!=_inputs.end(); ++it)	{
		ISFAttrRef		&tmpAttrib = *it;
		if ((tmpAttrib->type() & n) == n)
			returnMe.push_back(tmpAttrib);
	}
	
	return returnMe;
}
ISFAttrRef ISFDoc::input(const string & inAttrName)	{
	lock_guard<recursive_mutex>		lock(_propLock);
	
	for (auto it=_inputs.begin(); it!=_inputs.end(); ++it)	{
		ISFAttrRef		&tmpAttrib = *it;
		if (tmpAttrib->name() == inAttrName)
			return tmpAttrib;
	}
	return nullptr;
}
const GLBufferRef ISFDoc::getBufferForKey(const string & n)	{
	lock_guard<recursive_mutex>		lock(_propLock);
	
	for (const auto & attribRefIt : _imageInputs)	{
		if (attribRefIt->name() == n)
			return attribRefIt->currentVal().imageBuffer();
	}
	for (const auto & attribRefIt : _imageImports)	{
		if (attribRefIt->name() == n)
			return attribRefIt->currentVal().imageBuffer();
	}
	for (const auto & attribRefIt : _audioInputs)	{
		if (attribRefIt->name() == n)
			return attribRefIt->currentVal().imageBuffer();
	}
	
	for (const auto & targetBufIt : _persistentPassTargets)	{
		if (targetBufIt->name() == n)
			return targetBufIt->buffer();
	}
	for (const auto & targetBufIt : _tempPassTargets)	{
		if (targetBufIt->name() == n)
			return targetBufIt->buffer();
	}
	return nullptr;
}
const GLBufferRef ISFDoc::getPersistentBufferForKey(const string & n)	{
	lock_guard<recursive_mutex>		lock(_propLock);
	
	for (const auto & targetBufIt : _persistentPassTargets)	{
		if (targetBufIt->name() == n)
			return targetBufIt->buffer();
	}
	return nullptr;
}
const GLBufferRef ISFDoc::getTempBufferForKey(const string & n)	{
	lock_guard<recursive_mutex>		lock(_propLock);
	
	for (const auto & targetBufIt : _tempPassTargets)	{
		if (targetBufIt->name() == n)
			return targetBufIt->buffer();
	}
	return nullptr;
}
const ISFPassTargetRef ISFDoc::passTargetForKey(const string & n)	{
	ISFPassTargetRef		returnMe = persistentPassTargetForKey(n);
	if (returnMe == nullptr)
		returnMe = tempPassTargetForKey(n);
	return returnMe;
}
const ISFPassTargetRef ISFDoc::persistentPassTargetForKey(const string & n)	{
	//cout << __PRETTY_FUNCTION__ << ", key is \"" << n << "\"" << endl;
	lock_guard<recursive_mutex>		lock(_propLock);
	
	for (const auto & targetBufIt : _persistentPassTargets)	{
		//cout << "\tchecking persistent buffer named \"" << targetBufIt->name() << "\"" << endl;
		if (targetBufIt->name() == n)	{
			//cout << "\tmatch found! returning " << targetBufIt << endl;
			return targetBufIt;
		}
	}
	return nullptr;
}
const ISFPassTargetRef ISFDoc::tempPassTargetForKey(const string & n)	{
	//cout << __PRETTY_FUNCTION__ << ", key is \"" << n << "\"" << endl;
	lock_guard<recursive_mutex>		lock(_propLock);
	
	for (const auto & targetBufIt : _tempPassTargets)	{
		//cout << "\tchecking temp buffer named \"" << targetBufIt->name() << "\"" << endl;
		if (targetBufIt->name() == n)	{
			//cout << "\tmatch found! returning " << targetBufIt << endl;
			return targetBufIt;
		}
	}
	return nullptr;
}


void ISFDoc::jsonSourceString(string & outStr)	{
	lock_guard<recursive_mutex>		lock(_propLock);
	if (_jsonSourceString==nullptr)	{
		outStr = "";
		return;
	}
	outStr.reserve(_jsonSourceString->length());
	outStr.clear();
	outStr.append(*_jsonSourceString);
}



/*
vector<ISFAttrRef> ISFDoc::inputs(const ISFValType & n)	{
	auto		returnMe = vector<ISFAttrRef>(0);
	lock_guard<recursive_mutex>		lock(_propLock);
	
	for (auto it=_inputs.begin(); it!=_inputs.end(); ++it)	{
		ISFAttrRef		&tmpAttrib = *it;
		if ((tmpAttrib->type() & n) == n)
			returnMe.push_back(tmpAttrib);
	}
	
	return returnMe;
}
*/
string ISFDoc::generateTextureTypeString()	{
	string		returnMe("");
	lock_guard<recursive_mutex>		lock(_propLock);
	
	for (const auto & attribRefIt : _imageInputs)	{
		if (attribRefIt->shouldHaveImageBuffer())	{
			GLBufferRef		tmpBuffer = attribRefIt->getCurrentImageBuffer();
			if (tmpBuffer==nullptr || tmpBuffer->desc.target==GLBuffer::Target_2D)
				returnMe.append("2");
#if defined(VVGL_SDK_MAC) || defined(VVGL_SDK_WIN)
			else if (tmpBuffer->desc.target==GLBuffer::Target_Rect)
				returnMe.append("R");
#endif
#if !defined(VVGL_SDK_RPI)
			else if (tmpBuffer->desc.target==GLBuffer::Target_Cube)
				returnMe.append("C");
#endif
		}
	}
	for (const auto & attribRefIt : _audioInputs)	{
		if (attribRefIt->shouldHaveImageBuffer())	{
			GLBufferRef		tmpBuffer = attribRefIt->getCurrentImageBuffer();
			if (tmpBuffer==nullptr || tmpBuffer->desc.target==GLBuffer::Target_2D)
				returnMe.append("2");
#if defined(VVGL_SDK_MAC) || defined(VVGL_SDK_WIN)
			else if (tmpBuffer->desc.target==GLBuffer::Target_Rect)
				returnMe.append("R");
#endif
#if !defined(VVGL_SDK_RPI)
			else if (tmpBuffer->desc.target==GLBuffer::Target_Cube)
				returnMe.append("C");
#endif
		}
	}
	for (const auto & attribRefIt : _imageImports)	{
		if (attribRefIt->shouldHaveImageBuffer())	{
			GLBufferRef		tmpBuffer = attribRefIt->getCurrentImageBuffer();
			if (tmpBuffer==nullptr || tmpBuffer->desc.target==GLBuffer::Target_2D)
				returnMe.append("2");
#if defined(VVGL_SDK_MAC) || defined(VVGL_SDK_WIN)
			else if (tmpBuffer->desc.target==GLBuffer::Target_Rect)
				returnMe.append("R");
#endif
#if !defined(VVGL_SDK_RPI)
			else if (tmpBuffer->desc.target==GLBuffer::Target_Cube)
				returnMe.append("C");
#endif
		}
	}
	for (const auto & targetBufIt : _persistentPassTargets)	{
		GLBufferRef		tmpBuffer = targetBufIt->buffer();
		if (tmpBuffer==nullptr || tmpBuffer->desc.target==GLBuffer::Target_2D)
			returnMe.append("2");
#if defined(VVGL_SDK_MAC) || defined(VVGL_SDK_WIN)
		else if (tmpBuffer->desc.target==GLBuffer::Target_Rect)
			returnMe.append("R");
#endif
#if !defined(VVGL_SDK_RPI)
		else if (tmpBuffer->desc.target==GLBuffer::Target_Cube)
			returnMe.append("C");
#endif
	}
	for (const auto & targetBufIt : _tempPassTargets)	{
		GLBufferRef		tmpBuffer = targetBufIt->buffer();
		if (tmpBuffer==nullptr || tmpBuffer->desc.target==GLBuffer::Target_2D)
			returnMe.append("2");
#if defined(VVGL_SDK_MAC) || defined(VVGL_SDK_WIN)
		else if (tmpBuffer->desc.target==GLBuffer::Target_Rect)
			returnMe.append("R");
#endif
#if !defined(VVGL_SDK_RPI)
		else if (tmpBuffer->desc.target==GLBuffer::Target_Cube)
			returnMe.append("C");
#endif
	}
	return returnMe;
}
bool ISFDoc::generateShaderSource(string * outFragSrc, string * outVertSrc, GLVersion & inGLVers, const bool & inVarsAsUBO)	{
	//cout << __PRETTY_FUNCTION__ << ", vers is " << inGLVers << endl;
	lock_guard<recursive_mutex>		lock(_propLock);
	
	if (outFragSrc==nullptr || outVertSrc==nullptr || _vertShaderSource==nullptr || _fragShaderSource==nullptr)	{
		if (_throwExcept)
			throw ISFErr(ISFErrType_ErrorParsingFS, "Preflight failed", __PRETTY_FUNCTION__);
		else
			return false;
	}
	//	assemble the variable declarations
	string		vsVarDeclarations = string("");
	string		fsVarDeclarations = string("");
	if (!_assembleShaderSource_VarDeclarations(&vsVarDeclarations, &fsVarDeclarations, inGLVers, inVarsAsUBO))	{
		if (_throwExcept)
			throw ISFErr(ISFErrType_ErrorParsingFS, "Var Dec failed", __PRETTY_FUNCTION__);
		else
			return false;
	}
	//cout << "vs var declarations:\n*******************\n" << vsVarDeclarations << "*******************\n";
	//cout << "fs var declarations:\n*******************\n" << fsVarDeclarations << "*******************\n";
	
	/*	stores names of the images/buffers that are accessed via IMG_THIS_PIXEL (which is replaced 
	in the frag shader, but the names are then needed to declare vars in the vert shader)		*/
	vector<string>		imgThisPixelSamplerNames;
	vector<string>		imgThisNormPixelSamplerNames;
	
	//	check the source string to see if it requires any of the macro functions, add them if necessary
	//bool			requiresMacroFunctions = false;
	bool			requires2DMacro = false;
	bool			requires2DBiasMacro = false;
	bool			requires2DRectMacro = false;
	bool			requires2DRectBiasMacro = false;
	//size_t			findIndex;
	Range		tmpRange(0,0);
	string			searchString("");
	GLBufferRef	imgBuffer = nullptr;
	string			modSrcString("");
	string			newString("");
	size_t			tmpIndex;
	
	//	put together a new frag shader string from the raw shader source
	string			newFragShaderSrc = string("");
	newFragShaderSrc.reserve( uint32_t(1.5 * (fsVarDeclarations.size()+_fragShaderSource->size())) );
	{
		//	remove any lines containing #version tags
		searchString = string("#version");
		tmpRange = Range(newFragShaderSrc.find(searchString), searchString.size());
		do	{
			if (tmpRange.loc != string::npos)	{
				tmpIndex = modSrcString.find_first_of("\n\r\f", tmpRange.max());
				if (tmpIndex != string::npos)	{
					tmpRange.len = tmpIndex - tmpRange.loc;
					newFragShaderSrc.erase(tmpRange.loc, tmpRange.len);
					
					tmpRange = Range(newFragShaderSrc.find(searchString), searchString.size());
				}
			}
		} while (tmpRange.loc != string::npos);
		//	add the #version tag for the min version of GLSL supported by this major vsn of openGL
		switch (inGLVers)	{
		case GLVersion_Unknown:
		case GLVersion_2:
			newFragShaderSrc.insert(0, string("#version 110\n"));
			break;
		case GLVersion_ES:
		case GLVersion_ES2:
			newFragShaderSrc.insert(0, string("#version 100\n"));
			break;
		case GLVersion_ES3:
			newFragShaderSrc.insert(0, string("#version 300 es\n"));
			break;
		case GLVersion_33:
			newFragShaderSrc.insert(0, string("#version 330\n"));
			break;
		case GLVersion_4:
			newFragShaderSrc.insert(0, string("#version 400\n"));
			break;
		}
		
		//	add the compatibility define
		switch (inGLVers)	{
		case GLVersion_Unknown:
		case GLVersion_2:
			//	intentionally blank
			break;
		case GLVersion_ES:
		case GLVersion_ES2:
		case GLVersion_ES3:
		case GLVersion_33:
		case GLVersion_4:
			newFragShaderSrc.append(ISF_ES_Compatibility);
			break;
		}
		//	copy the variable declarations to the frag shader src
		newFragShaderSrc.append(fsVarDeclarations);
		
		//	now i have to find-and-replace the shader source for various things- make a copy of the raw source and work from that.
		modSrcString.reserve(newFragShaderSrc.capacity());
		modSrcString.append(*_fragShaderSource);
		
		//	find-and-replace vv_FragNormCoord (v1 of the ISF spec) with isf_FragNormCoord (v2 of the ISF spec)
		searchString = string("vv_FragNormCoord");
		newString = string("isf_FragNormCoord");
		tmpRange = Range(0, searchString.size());
		do	{
			tmpRange.loc = modSrcString.find(searchString);
			if (tmpRange.loc != string::npos)
				modSrcString.replace(tmpRange.loc, tmpRange.len, newString, 0, newString.size());
		} while (tmpRange.loc != string::npos);
		
		
		//	now find-and-replace IMG_PIXEL
		searchString = string("IMG_PIXEL");
		imgBuffer = nullptr;
		tmpRange = Range(0, searchString.size());
		do	{
			tmpRange = Range(modSrcString.find(searchString), searchString.size());
			if (tmpRange.loc != string::npos)	{
				vector<string>		varArray(0);
				varArray.reserve(5);
				Range			fullFuncRangeToReplace = LexFunctionCall(modSrcString, tmpRange, varArray);
				size_t				varArrayCount = varArray.size();
				if (varArrayCount!=2 && varArrayCount!=3)	{
					if (_throwExcept)	{
						if (_path != nullptr)
							throw ISFErr(ISFErrType_ErrorParsingFS, "IMG_PIXEL has wrong number of arguments", *_path);
						else
							throw ISFErr(ISFErrType_ErrorParsingFS, "IMG_PIXEL has wrong number of args", "");
					}
				}
				else	{
					string			newFuncString("");
					string &		samplerName = varArray[0];
					const char *	samplerNameC = samplerName.c_str();
					string &		samplerCoord = varArray[1];
					const char *	samplerCoordC = samplerCoord.c_str();
					imgBuffer = getBufferForKey(samplerName);
					if (imgBuffer==nullptr || imgBuffer->desc.target==GLBuffer::Target_2D)	{
						if (varArrayCount==3)	{
							newFuncString = FmtString("VVSAMPLER_2DBYPIXEL(%s, _%s_imgRect, _%s_imgSize, _%s_flip, %s, %s)",samplerNameC,samplerNameC,samplerNameC,samplerNameC,samplerCoordC,varArray[2].c_str());
							requires2DBiasMacro = true;
						}
						else	{
							newFuncString = FmtString("VVSAMPLER_2DBYPIXEL(%s, _%s_imgRect, _%s_imgSize, _%s_flip, %s)",samplerNameC,samplerNameC,samplerNameC,samplerNameC,samplerCoordC);
							requires2DMacro = true;
						}
					}
					else	{
						if (varArrayCount==3)	{
							newFuncString = FmtString("VVSAMPLER_2DRECTBYPIXEL(%s, _%s_imgRect, _%s_imgSize, _%s_flip, %s, %s)",samplerNameC,samplerNameC,samplerNameC,samplerNameC,samplerCoordC,varArray[2].c_str());
							requires2DRectBiasMacro = true;
						}
						else	{
							newFuncString = FmtString("VVSAMPLER_2DRECTBYPIXEL(%s, _%s_imgRect, _%s_imgSize, _%s_flip, %s)",samplerNameC,samplerNameC,samplerNameC,samplerNameC,samplerCoordC);
							requires2DRectMacro = true;
						}
					}
					
					modSrcString.replace(fullFuncRangeToReplace.loc, fullFuncRangeToReplace.len, newFuncString, 0, newFuncString.size());
				}
			}
		} while (tmpRange.loc != string::npos);
		
		//	now find-and-replace IMG_NORM_PIXEL
		searchString = string("IMG_NORM_PIXEL");
		imgBuffer = nullptr;
		tmpRange = Range(0, searchString.size());
		do	{
			tmpRange = Range(modSrcString.find(searchString), searchString.size());
			if (tmpRange.loc != string::npos)	{
				vector<string>		varArray(0);
				varArray.reserve(5);
				Range			fullFuncRangeToReplace = LexFunctionCall(modSrcString, tmpRange, varArray);
				size_t				varArrayCount = varArray.size();
				if (varArrayCount!=2 && varArrayCount!=3)	{
					if (_throwExcept)	{
						if (_path != nullptr)
							throw ISFErr(ISFErrType_ErrorParsingFS, "IMG_NORM_PIXEL has wrong number of arguments", *_path);
						else
							throw ISFErr(ISFErrType_ErrorParsingFS, "IMG_NORM_PIXEL has wrong number of arguments", "");
					}
				}
				else	{
					string			newFuncString("");
					string &		samplerName = varArray[0];
					const char *	samplerNameC = samplerName.c_str();
					string &		samplerCoord = varArray[1];
					const char *	samplerCoordC = samplerCoord.c_str();
					imgBuffer = getBufferForKey(samplerName);
					if (imgBuffer==nullptr || imgBuffer->desc.target==GLBuffer::Target_2D)	{
						if (varArrayCount==3)	{
							newFuncString = FmtString("VVSAMPLER_2DBYNORM(%s, _%s_imgRect, _%s_imgSize, _%s_flip, %s, %s)",samplerNameC,samplerNameC,samplerNameC,samplerNameC,samplerCoordC,varArray[2].c_str());
							requires2DBiasMacro = true;
						}
						else	{
							newFuncString = FmtString("VVSAMPLER_2DBYNORM(%s, _%s_imgRect, _%s_imgSize, _%s_flip, %s)",samplerNameC,samplerNameC,samplerNameC,samplerNameC,samplerCoordC);
							requires2DMacro = true;
						}
					}
					else	{
						if (varArrayCount==3)	{
							newFuncString = FmtString("VVSAMPLER_2DRECTBYNORM(%s, _%s_imgRect, _%s_imgSize, _%s_flip, %s, %s)",samplerNameC,samplerNameC,samplerNameC,samplerNameC,samplerCoordC,varArray[2].c_str());
							requires2DRectBiasMacro = true;
						}
						else	{
							newFuncString = FmtString("VVSAMPLER_2DRECTBYNORM(%s, _%s_imgRect, _%s_imgSize, _%s_flip, %s)",samplerNameC,samplerNameC,samplerNameC,samplerNameC,samplerCoordC);
							requires2DRectMacro = true;
						}
					}
					
					modSrcString.replace(fullFuncRangeToReplace.loc, fullFuncRangeToReplace.len, newFuncString, 0, newFuncString.size());
				}
			}
		} while (tmpRange.loc != string::npos);
		
		//	now find-and-replace IMG_THIS_PIXEL
		searchString = string("IMG_THIS_PIXEL");
		imgBuffer = nullptr;
		tmpRange = Range(0, searchString.size());
		do	{
			tmpRange = Range(modSrcString.find(searchString), searchString.size());
			if (tmpRange.loc != string::npos)	{
				vector<string>		varArray(0);
				varArray.reserve(5);
				Range			fullFuncRangeToReplace = LexFunctionCall(modSrcString, tmpRange, varArray);
				size_t				varArrayCount = varArray.size();
				if (varArrayCount!=1)	{
					if (_throwExcept)	{
						if (_path != nullptr)
							throw ISFErr(ISFErrType_ErrorParsingFS, "IMG_THIS_PIXEL has wrong number of arguments", *_path);
						else
							throw ISFErr(ISFErrType_ErrorParsingFS, "IMG_THIS_PIXEL has wrong number of arguments", "");
					}
				}
				else	{
					string			newFuncString("");
					string &		samplerName = varArray[0];
					const char *	samplerNameC = samplerName.c_str();
					
					if (find(imgThisPixelSamplerNames.begin(), imgThisPixelSamplerNames.end(), samplerName) == imgThisPixelSamplerNames.end())
						imgThisPixelSamplerNames.push_back(samplerName);
					
					imgBuffer = getBufferForKey(samplerName);
					
					switch (inGLVers)	{
					case GLVersion_Unknown:
					case GLVersion_2:
					case GLVersion_ES:
					case GLVersion_ES2:
						if (imgBuffer==nullptr || imgBuffer->desc.target==GLBuffer::Target_2D)	{
							newFuncString = FmtString("texture2D(%s, _%s_texCoord)",samplerNameC,samplerNameC);
						}
						else	{
							newFuncString = FmtString("texture2DRect(%s, _%s_texCoord)",samplerNameC,samplerNameC);
						}
						break;
					case GLVersion_ES3:
					case GLVersion_33:
					case GLVersion_4:
						newFuncString = FmtString("texture(%s, _%s_texCoord)",samplerNameC,samplerNameC);
						break;
					}
					
					modSrcString.replace(fullFuncRangeToReplace.loc, fullFuncRangeToReplace.len, newFuncString, 0, newFuncString.size());
				}
			}
		} while (tmpRange.loc != string::npos);
		//	add the IMG_THIS_PIXEL variable declarations to the frag shader
		if (imgThisPixelSamplerNames.size() > 0)	{
			for (const auto & it : imgThisPixelSamplerNames)	{
				switch (inGLVers)	{
				case GLVersion_Unknown:
				case GLVersion_2:
				case GLVersion_ES:
				case GLVersion_ES2:
					newFragShaderSrc.append(FmtString("varying vec2\t\t_%s_texCoord;\n",it.c_str()));
					break;
				case GLVersion_ES3:
				case GLVersion_33:
				case GLVersion_4:
					newFragShaderSrc.append(FmtString("in vec2\t\t_%s_texCoord;\n",it.c_str()));
					break;
				}
			}
		}
		
		//	now find-and-replace IMG_THIS_NORM_PIXEL
		searchString = string("IMG_THIS_NORM_PIXEL");
		imgBuffer = nullptr;
		tmpRange = Range(0, searchString.size());
		do	{
			tmpRange = Range(modSrcString.find(searchString), searchString.size());
			if (tmpRange.loc != string::npos)	{
				vector<string>		varArray(0);
				varArray.reserve(5);
				Range			fullFuncRangeToReplace = LexFunctionCall(modSrcString, tmpRange, varArray);
				size_t				varArrayCount = varArray.size();
				if (varArrayCount!=1)	{
					if (_throwExcept)	{
						if (_path != nullptr)
							throw ISFErr(ISFErrType_ErrorParsingFS, "IMG_THIS_NORM_PIXEL has wrong number of arguments", *_path);
						else
							throw ISFErr(ISFErrType_ErrorParsingFS, "IMG_THIS_NORM_PIXEL has wrong number of arguments", "");
					}
				}
				else	{
					string			newFuncString("");
					string &		samplerName = varArray[0];
					const char *	samplerNameC = samplerName.c_str();
					
					if (find(imgThisNormPixelSamplerNames.begin(), imgThisNormPixelSamplerNames.end(), samplerName) == imgThisNormPixelSamplerNames.end())
						imgThisNormPixelSamplerNames.push_back(samplerName);
					
					imgBuffer = getBufferForKey(samplerName);
					switch (inGLVers)	{
					case GLVersion_Unknown:
					case GLVersion_2:
					case GLVersion_ES:
					case GLVersion_ES2:
						if (imgBuffer==nullptr || imgBuffer->desc.target==GLBuffer::Target_2D)	{
							newFuncString = FmtString("texture2D(%s, _%s_normTexCoord)",samplerNameC,samplerNameC);
						}
						else	{
							newFuncString = FmtString("texture2DRect(%s, _%s_normTexCoord)",samplerNameC,samplerNameC);
						}
						break;
					case GLVersion_ES3:
					case GLVersion_33:
					case GLVersion_4:
						newFuncString = FmtString("texture(%s, _%s_normTexCoord)", samplerNameC, samplerNameC);
						break;
					}
					
					modSrcString.replace(fullFuncRangeToReplace.loc, fullFuncRangeToReplace.len, newFuncString, 0, newFuncString.size());
				}
			}
		} while (tmpRange.loc != string::npos);
		//	add the IMG_THIS_NORM_PIXEL variable declarations to the frag shader
		if (imgThisNormPixelSamplerNames.size() > 0)	{
			for (const auto & it : imgThisNormPixelSamplerNames)	{
				switch (inGLVers)	{
				case GLVersion_Unknown:
				case GLVersion_2:
				case GLVersion_ES:
				case GLVersion_ES2:
					newFragShaderSrc.append(FmtString("varying vec2\t\t_%s_normTexCoord;\n",it.c_str()));
					break;
				case GLVersion_ES3:
				case GLVersion_33:
				case GLVersion_4:
					newFragShaderSrc.append(FmtString("in vec2\t\t_%s_normTexCoord;\n",it.c_str()));
					break;
				}
			}
		}
		
		//	now find-and-replace IMG_SIZE
		searchString = string("IMG_SIZE");
		imgBuffer = nullptr;
		tmpRange = Range(0, searchString.size());
		do	{
			tmpRange = Range(modSrcString.find(searchString), searchString.size());
			if (tmpRange.loc != string::npos)	{
				vector<string>		varArray(0);
				varArray.reserve(5);
				Range		fullFuncRangeToReplace = LexFunctionCall(modSrcString, tmpRange, varArray);
				size_t			varArrayCount = varArray.size();
				if (varArrayCount != 1)	{
					if (_throwExcept)	{
						if (_path != nullptr)
							throw ISFErr(ISFErrType_ErrorParsingFS, "IMG_SIZE has wrong number of arguments", *_path);
						else
							throw ISFErr(ISFErrType_ErrorParsingFS, "IMG_SIZE has wrong number of arguments", "");
					}
				}
				else	{
					string &		samplerName = varArray[0];
					const char *	samplerNameC = samplerName.c_str();
					string			newFuncString = FmtString("(_%s_imgSize.xy)",samplerNameC);
					modSrcString.replace(fullFuncRangeToReplace.loc, fullFuncRangeToReplace.len, newFuncString, 0, newFuncString.size());
				}
			}
		} while (tmpRange.loc != string::npos);
		
		newFragShaderSrc.append("\n");
		
		//	if the frag shader requires macro functions, add them now that i'm done declaring the variables
		switch (inGLVers)	{
		case GLVersion_Unknown:
		case GLVersion_2:
		case GLVersion_ES:
		case GLVersion_ES2:
			if (requires2DMacro)
				newFragShaderSrc.append(ISFGLMacro2D_GL2);
			if (requires2DBiasMacro)
				newFragShaderSrc.append(ISFGLMacro2DBias_GL2);
			if (requires2DRectMacro)
				newFragShaderSrc.append(ISFGLMacro2DRect_GL2);
			if (requires2DRectBiasMacro)
				newFragShaderSrc.append(ISFGLMacro2DRectBias_GL2);
			break;
		case GLVersion_ES3:
		case GLVersion_33:
		case GLVersion_4:
			if (requires2DMacro)
				newFragShaderSrc.append(ISFGLMacro2D_GL3);
			if (requires2DBiasMacro)
				newFragShaderSrc.append(ISFGLMacro2DBias_GL3);
			if (requires2DRectMacro)
				newFragShaderSrc.append(ISFGLMacro2DRect_GL3);
			if (requires2DRectBiasMacro)
				newFragShaderSrc.append(ISFGLMacro2DRectBias_GL3);
			break;
		}
		//	add the shader source that has been find-and-replaced
		newFragShaderSrc.append(modSrcString);
		//cout << "newFragShaderSrc is:\n******************\n" << newFragShaderSrc << "\n******************\n";
	}
	
	
	//	put together a new vert shader string from the raw shader source
	string			newVertShaderSrc = string("");
	newVertShaderSrc.reserve( uint32_t(2.5 * (vsVarDeclarations.size()+_vertShaderSource->size())) );
	{
		//	remove any lines containing #version tags
		searchString = string("#version");
		tmpRange = Range(newVertShaderSrc.find(searchString), searchString.size());
		do	{
			if (tmpRange.loc != string::npos)	{
				tmpIndex = modSrcString.find_first_of("\n\r\f", tmpRange.max());
				if (tmpIndex != string::npos)	{
					tmpRange.len = tmpIndex - tmpRange.loc;
					newVertShaderSrc.erase(tmpRange.loc, tmpRange.len);
					
					tmpRange = Range(newVertShaderSrc.find(searchString), searchString.size());
				}
			}
		} while (tmpRange.loc != string::npos);
		//	add the #version tag for the min version of GLSL supported by this major vsn of openGL
		switch (inGLVers)	{
		case GLVersion_Unknown:
		case GLVersion_2:
			newVertShaderSrc.insert(0, string("#version 110\n"));
			break;
		case GLVersion_ES:
		case GLVersion_ES2:
			newVertShaderSrc.insert(0, string("#version 100\n"));
			break;
		case GLVersion_ES3:
			newVertShaderSrc.insert(0, string("#version 300 es\n"));
			break;
		case GLVersion_33:
			newVertShaderSrc.insert(0, string("#version 330\n"));
			break;
		case GLVersion_4:
			newVertShaderSrc.insert(0, string("#version 400\n"));
			break;
		}
		
		//	add the compatibility define
		switch (inGLVers)	{
		case GLVersion_Unknown:
		case GLVersion_2:
			//	intentionally blank
			break;
		case GLVersion_ES:
		case GLVersion_ES2:
		case GLVersion_ES3:
		case GLVersion_33:
		case GLVersion_4:
			newVertShaderSrc.append(ISF_ES_Compatibility);
			break;
		}
		//	load any specific vars or function declarations for the vertex shader from an included file
		switch (inGLVers)	{
		case GLVersion_Unknown:
		case GLVersion_2:
		case GLVersion_ES:
		case GLVersion_ES2:
			newVertShaderSrc.append(ISFVertVarDec_GLES2);
			break;
		case GLVersion_ES3:
		case GLVersion_33:
		case GLVersion_4:
			newVertShaderSrc.append(ISFVertVarDec_GL3);
			break;
		}
		//	append the variable declarations i assembled earlier with the frag shader
		newVertShaderSrc.append(vsVarDeclarations);
		
		//	add the variables for values corresponding to buffers from IMG_THIS_PIXEL and IMG_THIS_NORM_PIXEL in the frag shader
		if (imgThisPixelSamplerNames.size()>0 || imgThisNormPixelSamplerNames.size()>0)	{
			if (imgThisPixelSamplerNames.size() > 0)	{
				for (const auto & it : imgThisPixelSamplerNames)	{
					switch (inGLVers)	{
					case GLVersion_Unknown:
					case GLVersion_2:
					case GLVersion_ES:
					case GLVersion_ES2:
						newVertShaderSrc.append(FmtString("varying vec2\t\t_%s_texCoord;\n",it.c_str()));
						break;
					case GLVersion_ES3:
					case GLVersion_33:
					case GLVersion_4:
						newVertShaderSrc.append(FmtString("out vec2\t\t_%s_texCoord;\n",it.c_str()));
						break;
					}
				}
			}
			if (imgThisNormPixelSamplerNames.size() > 0)	{
				for (const auto & it : imgThisNormPixelSamplerNames)	{
					switch (inGLVers)	{
					case GLVersion_Unknown:
					case GLVersion_2:
					case GLVersion_ES:
					case GLVersion_ES2:
						newVertShaderSrc.append(FmtString("varying vec2\t\t_%s_normTexCoord;\n",it.c_str()));
						break;
					case GLVersion_ES3:
					case GLVersion_33:
					case GLVersion_4:
						newVertShaderSrc.append(FmtString("out vec2\t\t_%s_normTexCoord;\n",it.c_str()));
						break;
					}
				}
			}
		}
		//	check the source string to see if it requires any of the macro functions, add them if necessary
		requires2DMacro = false;
		requires2DBiasMacro = false;
		requires2DRectMacro = false;
		requires2DRectBiasMacro = false;
		
		//	now i have to find-and-replace the shader source for various things- make a copy of the raw source and work from that.
		modSrcString = string("");
		modSrcString.reserve(newVertShaderSrc.capacity());
		modSrcString.append(*_vertShaderSource);
		
		//	find-and-replace vv_FragNormCoord (v1 of the ISF spec) with isf_FragNormCoord (v2 of the ISF spec)
		searchString = string("vv_FragNormCoord");
		newString = string("isf_FragNormCoord");
		tmpRange = Range(0, searchString.size());
		do	{
			tmpRange.loc = modSrcString.find(searchString);
			if (tmpRange.loc != string::npos)
				modSrcString.replace(tmpRange.loc, tmpRange.len, newString, 0, newString.size());
		} while (tmpRange.loc != string::npos);
		
		//	find-and-replace vv_vertShaderInit (v1 of the ISF spec) with isf_vertShaderInit (v2 of the ISF spec)
		searchString = string("vv_vertShaderInit");
		newString = string("isf_vertShaderInit");
		tmpRange = Range(0, searchString.size());
		do	{
			tmpRange.loc = modSrcString.find(searchString);
			if (tmpRange.loc != string::npos)
				modSrcString.replace(tmpRange.loc, tmpRange.len, newString, 0, newString.size());
		} while (tmpRange.loc != string::npos);
		
		//	now find-and-replace IMG_PIXEL
		searchString = string("IMG_PIXEL");
		imgBuffer = nullptr;
		tmpRange = Range(0, searchString.size());
		do	{
			tmpRange = Range(modSrcString.find(searchString), searchString.size());
			if (tmpRange.loc != string::npos)	{
				vector<string>		varArray(0);
				varArray.reserve(5);
				Range			fullFuncRangeToReplace = LexFunctionCall(modSrcString, tmpRange, varArray);
				size_t				varArrayCount = varArray.size();
				if (varArrayCount!=2 && varArrayCount!=3)	{
					if (_throwExcept)	{
						if (_path != nullptr)
							throw ISFErr(ISFErrType_ErrorParsingFS, "IMG_PIXEL has wrong number of arguments", *_path);
						else
							throw ISFErr(ISFErrType_ErrorParsingFS, "IMG_PIXEL has wrong number of arguments", "");
					}
				}
				else	{
					string			newFuncString("");
					string &		samplerName = varArray[0];
					const char *	samplerNameC = samplerName.c_str();
					string &		samplerCoord = varArray[1];
					const char *	samplerCoordC = samplerCoord.c_str();
					imgBuffer = getBufferForKey(samplerName);
					if (imgBuffer==nullptr || imgBuffer->desc.target==GLBuffer::Target_2D)	{
						if (varArrayCount==3)	{
							newFuncString = FmtString("VVSAMPLER_2DBYPIXEL(%s, _%s_imgRect, _%s_imgSize, _%s_flip, %s, %s)",samplerNameC,samplerNameC,samplerNameC,samplerNameC,samplerCoordC,varArray[2].c_str());
							requires2DBiasMacro = true;
						}
						else	{
							newFuncString = FmtString("VVSAMPLER_2DBYPIXEL(%s, _%s_imgRect, _%s_imgSize, _%s_flip, %s)",samplerNameC,samplerNameC,samplerNameC,samplerNameC,samplerCoordC);
							requires2DMacro = true;
						}
					}
					else	{
						if (varArrayCount==3)	{
							newFuncString = FmtString("VVSAMPLER_2DRECTBYPIXEL(%s, _%s_imgRect, _%s_imgSize, _%s_flip, %s, %s)",samplerNameC,samplerNameC,samplerNameC,samplerNameC,samplerCoordC,varArray[2].c_str());
							requires2DRectBiasMacro = true;
						}
						else	{
							newFuncString = FmtString("VVSAMPLER_2DRECTBYPIXEL(%s, _%s_imgRect, _%s_imgSize, _%s_flip, %s)",samplerNameC,samplerNameC,samplerNameC,samplerNameC,samplerCoordC);
							requires2DRectMacro = true;
						}
					}
					
					modSrcString.replace(fullFuncRangeToReplace.loc, fullFuncRangeToReplace.len, newFuncString, 0, newFuncString.size());
				}
			}
		} while (tmpRange.loc != string::npos);
		
		//	now find-and-replace IMG_NORM_PIXEL
		searchString = string("IMG_NORM_PIXEL");
		imgBuffer = nullptr;
		tmpRange = Range(0, searchString.size());
		do	{
			tmpRange = Range(modSrcString.find(searchString), searchString.size());
			if (tmpRange.loc != string::npos)	{
				vector<string>		varArray(0);
				varArray.reserve(5);
				Range			fullFuncRangeToReplace = LexFunctionCall(modSrcString, tmpRange, varArray);
				size_t				varArrayCount = varArray.size();
				if (varArrayCount!=2 && varArrayCount!=3)	{
					if (_throwExcept)	{
						if (_path != nullptr)
							throw ISFErr(ISFErrType_ErrorParsingFS, "IMG_NORM_PIXEL has wrong number of arguments", *_path);
						else
							throw ISFErr(ISFErrType_ErrorParsingFS, "IMG_NORM_PIXEL has wrong number of arguments", "");
					}
				}
				else	{
					string			newFuncString("");
					string &		samplerName = varArray[0];
					const char *	samplerNameC = samplerName.c_str();
					string &		samplerCoord = varArray[1];
					const char *	samplerCoordC = samplerCoord.c_str();
					imgBuffer = getBufferForKey(samplerName);
					if (imgBuffer==nullptr || imgBuffer->desc.target==GLBuffer::Target_2D)	{
						if (varArrayCount==3)	{
							newFuncString = FmtString("VVSAMPLER_2DBYNORM(%s, _%s_imgRect, _%s_imgSize, _%s_flip, %s, %s)",samplerNameC,samplerNameC,samplerNameC,samplerNameC,samplerCoordC,varArray[2].c_str());
							requires2DBiasMacro = true;
						}
						else	{
							newFuncString = FmtString("VVSAMPLER_2DBYNORM(%s, _%s_imgRect, _%s_imgSize, _%s_flip, %s)",samplerNameC,samplerNameC,samplerNameC,samplerNameC,samplerCoordC);
							requires2DMacro = true;
						}
					}
					else	{
						if (varArrayCount==3)	{
							newFuncString = FmtString("VVSAMPLER_2DRECTBYNORM(%s, _%s_imgRect, _%s_imgSize, _%s_flip, %s, %s)",samplerNameC,samplerNameC,samplerNameC,samplerNameC,samplerCoordC,varArray[2].c_str());
							requires2DRectBiasMacro = true;
						}
						else	{
							newFuncString = FmtString("VVSAMPLER_2DRECTBYNORM(%s, _%s_imgRect, _%s_imgSize, _%s_flip, %s)",samplerNameC,samplerNameC,samplerNameC,samplerNameC,samplerCoordC);
							requires2DRectMacro = true;
						}
					}
					
					modSrcString.replace(fullFuncRangeToReplace.loc, fullFuncRangeToReplace.len, newFuncString, 0, newFuncString.size());
				}
			}
		} while (tmpRange.loc != string::npos);
		
		//	now find-and-replace IMG_SIZE
		searchString = string("IMG_SIZE");
		imgBuffer = nullptr;
		tmpRange = Range(0, searchString.size());
		do	{
			tmpRange = Range(modSrcString.find(searchString), searchString.size());
			if (tmpRange.loc != string::npos)	{
				vector<string>		varArray(0);
				varArray.reserve(5);
				Range		fullFuncRangeToReplace = LexFunctionCall(modSrcString, tmpRange, varArray);
				size_t			varArrayCount = varArray.size();
				if (varArrayCount != 1)	{
					if (_throwExcept)	{
						if (_path != nullptr)
							throw ISFErr(ISFErrType_ErrorParsingFS, "IMG_SIZE has wrong number of arguments", *_path);
						else
							throw ISFErr(ISFErrType_ErrorParsingFS, "IMG_SIZE has wrong number of arguments", "");
					}
				}
				else	{
					string &		samplerName = varArray[0];
					const char *	samplerNameC = samplerName.c_str();
					string			newFuncString = FmtString("(_%s_imgSize.xy)",samplerNameC);
					modSrcString.replace(fullFuncRangeToReplace.loc, fullFuncRangeToReplace.len, newFuncString, 0, newFuncString.size());
				}
			}
		} while (tmpRange.loc != string::npos);
		
		newVertShaderSrc.append("\n");
		
		//	if the frag shader requires macro functions, add them now that i'm done declaring the variables
		if (requires2DMacro)
			newVertShaderSrc.append(ISFGLMacro2D_GL2);
		if (requires2DBiasMacro)
			newVertShaderSrc.append(ISFGLMacro2DBias_GL2);
		if (requires2DRectMacro)
			newVertShaderSrc.append(ISFGLMacro2DRect_GL2);
		if (requires2DRectBiasMacro)
			newVertShaderSrc.append(ISFGLMacro2DRectBias_GL2);
		
		//	add the shader source that has been find-and-replaced
		newVertShaderSrc.append(modSrcString);
		
		//	add the isf_vertShaderInit() method to the vertex shader
		newVertShaderSrc.append(string("\nvoid isf_vertShaderInit(void)\t{"));
		newVertShaderSrc.append(ISFVertInitFunc);
		//	run through the IMG_THIS_PIXEL sampler names, populating the varying vec2 variables i declared
		for (const auto & it : imgThisPixelSamplerNames)	{
			const char *	samplerName = it.c_str();
			//newVertShaderSrc.append(FmtString("\t_%s_texCoord = (_%s_flip) ? vec2(((isf_fragCoord.x/(_%s_imgSize.x)*_%s_imgRect.z)+_%s_imgRect.x), (_%s_imgRect.w-(isf_fragCoord.y/(_%s_imgSize.y)*_%s_imgRect.w)+_%s_imgRect.y)) : vec2(((isf_fragCoord.x/(_%s_imgSize.x)*_%s_imgRect.z)+_%s_imgRect.x), (isf_fragCoord.y/(_%s_imgSize.y)*_%s_imgRect.w)+_%s_imgRect.y);\n",samplerName,samplerName,samplerName,samplerName,samplerName,samplerName,samplerName,samplerName,samplerName,samplerName,samplerName,samplerName,samplerName,samplerName,samplerName));
			//	the floor(XXX + 0.5) trick used here is meant to be equivalent to round(XXX), which doesn't exist in GLSL
			newVertShaderSrc.append(FmtString("\t_%s_texCoord = (_%s_flip) \
				? vec2(floor((isf_fragCoord.x/(_%s_imgSize.x)*_%s_imgRect.z)+_%s_imgRect.x+0.5), floor(_%s_imgRect.w-(isf_fragCoord.y/(_%s_imgSize.y)*_%s_imgRect.w)+_%s_imgRect.y+0.5)) \
				: vec2(floor((isf_fragCoord.x/(_%s_imgSize.x)*_%s_imgRect.z)+_%s_imgRect.x+0.5), floor((isf_fragCoord.y/(_%s_imgSize.y)*_%s_imgRect.w)+_%s_imgRect.y+0.5));\n",
				samplerName,samplerName,samplerName,samplerName,samplerName,samplerName,samplerName,samplerName,samplerName,samplerName,samplerName,samplerName,samplerName,samplerName,samplerName));
		}
		//	run through the IMG_THIS_NORM_PIXEL sampler names, populating the varying vec2 variables i declared
		for (const auto & it : imgThisNormPixelSamplerNames)	{
			const char *	samplerName = it.c_str();
			imgBuffer = getBufferForKey(it);
			if (imgBuffer==nullptr || imgBuffer->desc.target==GLBuffer::Target_2D)	{
				newVertShaderSrc.append(FmtString("\t_%s_normTexCoord = (_%s_flip) \
					? vec2((((isf_FragNormCoord.x*_%s_imgSize.x)/_%s_imgSize.x*_%s_imgRect.z)+_%s_imgRect.x), (_%s_imgRect.w-((isf_FragNormCoord.y*_%s_imgSize.y)/_%s_imgSize.y*_%s_imgRect.w)+_%s_imgRect.y)) \
					: vec2((((isf_FragNormCoord.x*_%s_imgSize.x)/_%s_imgSize.x*_%s_imgRect.z)+_%s_imgRect.x), ((isf_FragNormCoord.y*_%s_imgSize.y)/_%s_imgSize.y*_%s_imgRect.w)+_%s_imgRect.y);\n",
					samplerName,samplerName,samplerName,samplerName,samplerName,samplerName,samplerName,samplerName,samplerName,samplerName,samplerName,samplerName,samplerName,samplerName,samplerName,samplerName,samplerName,samplerName,samplerName));
			}
			else	{
				newVertShaderSrc.append(FmtString("\t_%s_normTexCoord = (_%s_flip) \
					? vec2((((isf_FragNormCoord.x*_%s_imgRect.z)/_%s_imgSize.x*_%s_imgRect.z)+_%s_imgRect.x), (_%s_imgRect.w-((isf_FragNormCoord.y*_%s_imgRect.w)/_%s_imgSize.y*_%s_imgRect.w)+_%s_imgRect.y)) \
					: vec2((((isf_FragNormCoord.x*_%s_imgRect.z)/_%s_imgSize.x*_%s_imgRect.z)+_%s_imgRect.x), ((isf_FragNormCoord.y*_%s_imgRect.w)/_%s_imgSize.y*_%s_imgRect.w)+_%s_imgRect.y);\n",
					samplerName,samplerName,samplerName,samplerName,samplerName,samplerName,samplerName,samplerName,samplerName,samplerName,samplerName,samplerName,samplerName,samplerName,samplerName,samplerName,samplerName,samplerName,samplerName));
			}
		}
		//	...this finishes adding the isf_vertShaderInit() method!
		newVertShaderSrc.append(string("}\n"));
		//cout << "newVertShaderSrc is:\n******************\n" << newVertShaderSrc << "\n******************\n";
	}
	/*
	//	if there are any "#version" tags in the shaders, see that they are preserved and moved to the beginning!
	string			fragVersionString("");
	string			vertVersionString("");
	searchString = string("#version ");
	tmpRange = Range(0, searchString.size());
	tmpRange.loc = newFragShaderSrc.find(searchString);
	if (tmpRange.loc != string::npos)	{
		tmpRange.len = newFragShaderSrc.find_first_of("\n\r\f", tmpRange.max()) - tmpRange.loc;
		fragVersionString = newFragShaderSrc.substr(tmpRange.loc, tmpRange.len);
		newFragShaderSrc.erase(tmpRange.loc, tmpRange.len);
		newFragShaderSrc.insert(0, fragVersionString);
	}
	
	tmpRange = Range(0, searchString.size());
	tmpRange.loc = newVertShaderSrc.find(searchString);
	if (tmpRange.loc != string::npos)	{
		tmpRange.len = newVertShaderSrc.find_first_of("\n\r\f", tmpRange.max()) - tmpRange.loc;
		vertVersionString = newVertShaderSrc.substr(tmpRange.loc, tmpRange.len);
		newVertShaderSrc.erase(tmpRange.loc, tmpRange.len);
		newVertShaderSrc.insert(0, vertVersionString);
	}
	else if (fragVersionString.size()>0)
		newVertShaderSrc.insert(0, fragVersionString);
	*/
	
	//	at this point i've created frag and vertex shaders, and i just need to copy them to the provided strings
	outFragSrc->reserve(newFragShaderSrc.size()+1);
	outFragSrc->append(newFragShaderSrc);
	outVertSrc->reserve(newVertShaderSrc.size()+1);
	outVertSrc->append(newVertShaderSrc);
	return true;
}


void ISFDoc::evalBufferDimensionsWithRenderSize(const VVGL::Size & inSize)	{
	//cout << __FUNCTION__ << "- " << inSize << endl;
	lock_guard<recursive_mutex>		lock(_propLock);
	
	//	assemble the substitution map
	map<string,double*>		subDict;
	_assembleSubstitutionMap(&subDict);
	//subDict[string("WIDTH")] = inSize.width;
	//subDict[string("HEIGHT")] = inSize.height;
	//for (const auto & it : subDict)
		//cout << "\tkey is " << it.first << ", val is " << it.second << endl;
	
	/*
	//	if i don't have anything to evaluate, bail now
	if (!bufferRequiresEval)
		return;
	*/
	
	//	make sure that the persistent buffers are sized appropriately
	for (const auto & targetBufIt : _persistentPassTargets)	{
		targetBufIt->evalTargetSize(inSize, subDict, true, true);
	}
	//	make sure that the temp buffers are sized appropriately
	for (const auto & targetBufIt : _tempPassTargets)	{
		targetBufIt->evalTargetSize(inSize, subDict, true, true);
	}
	
	//cout << "\t" << __FUNCTION__ << "- FINISHED\n";
}


/*	========================================	*/
#pragma mark --------------------- protected methods


void ISFDoc::_initWithRawFragShaderString(const string & inRawFile)	{
	//	isolate the JSON blob that should be at the beginning of the file in a comment, save it as one string- save everything else as the raw shader source string
	auto			openCommentIndex = inRawFile.find("/*");
	auto			closeCommentIndex = inRawFile.find("*/");
	if (openCommentIndex == string::npos || closeCommentIndex == string::npos)	{
		if (_throwExcept)	{
			if (_path != nullptr)
				throw ISFErr(ISFErrType_MalformedJSON, "ISFDoc missing comment blob", *_path);
			else
				throw ISFErr(ISFErrType_MalformedJSON, "ISFDoc missing comment blob", "");
		}
		else
			return;
	}
	//	we need to advance 'closeCommentLineEnd' to include both the full "close comment" as well as the next "line break"
	auto			tmpIt = inRawFile.begin();
	tmpIt += closeCommentIndex;
	while (*tmpIt!='\n' && *tmpIt!='\r')
		++tmpIt;
	++tmpIt;
	auto			closeCommentLineEnd = tmpIt - inRawFile.begin();
	
	_jsonSourceString = new string(inRawFile, 0, closeCommentLineEnd);
	_jsonString = new string(inRawFile, openCommentIndex+2, closeCommentIndex - (openCommentIndex+2) );
	_fragShaderSource = new string(inRawFile, closeCommentLineEnd);
	
	//	parse the JSON blob, turning it into objects we can parse programmatically
	json			jblob;
	bool			caughtJSONException = false;
	try	{
		jblob = json::parse(*_jsonString);
	}
	catch (const json::parse_error& ex)	{
		caughtJSONException = true;
		map<string,string>		tmpErrDict;
		tmpErrDict.insert( pair<string,string>(string("jsonErrLog"), ex.what()) );
		if (_throwExcept)	{
			if (_path != nullptr)
				throw ISFErr(ISFErrType_MalformedJSON, "the JSON blob in this file is malformed.", *_path, tmpErrDict);
			else
				throw ISFErr(ISFErrType_MalformedJSON, "the JSON blob in this file is malformed.", "", tmpErrDict);
		}
	}
	/*
	catch (std::invalid_argument&)	{
		caughtJSONException = true;
		if (_throwExcept)	{
			if (_path != nullptr)
				throw ISFErr(ISFErrType_MalformedJSON, "the JSON blob in this file is malformed.", *_path);
			else
				throw ISFErr(ISFErrType_MalformedJSON, "the JSON blob in this file is malformed.", "");
		}
	}
	catch (const std::exception&) {
		caughtJSONException = true;
		if (_throwExcept)	{
			if (_path != nullptr)
				throw ISFErr(ISFErrType_MalformedJSON, "the JSON blob in this file is malformed.", *_path);
			else
				throw ISFErr(ISFErrType_MalformedJSON, "the JSON blob in this file is malformed.", "");
		}
	}
	catch (const std::string&) {
		caughtJSONException = true;
		if (_throwExcept)	{
			if (_path != nullptr)
				throw ISFErr(ISFErrType_MalformedJSON, "the JSON blob in this file is malformed.", *_path);
			else
				throw ISFErr(ISFErrType_MalformedJSON, "the JSON blob in this file is malformed.", "");
		}
	}
	*/
	catch (...) {
		caughtJSONException = true;
		if (_throwExcept)	{
			if (_path != nullptr)
				throw ISFErr(ISFErrType_MalformedJSON, "the JSON blob in this file is malformed.", *_path);
			else
				throw ISFErr(ISFErrType_MalformedJSON, "the JSON blob in this file is malformed.", "");
		}
	}
	//	if i caught an exception parsing the basic JSON, we can't do anything further
	/*
	if (caughtJSONException)	{
		cout << "\terr: caught JSON exception, bailing " << __PRETTY_FUNCTION__ << endl;
		return;
	}
	*/
	
	//	parse the description
	json			anObj = (caughtJSONException) ? json() : jblob.value("DESCRIPTION",json());
	if (anObj.is_string())	{
		_description = new string(anObj.get<string>());
	}
	
	//	parse the credit
	anObj = (caughtJSONException) ? json() : jblob.value("CREDIT",json());
	if (anObj.is_string())	{
		_credit = new string(anObj.get<string>());
	}
	
	//	parse the vsn
	anObj = (caughtJSONException) ? json() : jblob.value("VSN",json());
	if (anObj.is_string())	{
		_vsn = new string(anObj.get<string>());
	}
	
	//	parse the categories
	anObj = (caughtJSONException) ? json() : jblob.value("CATEGORIES",json());
	if (!anObj.is_null() && anObj.is_array())	{
		for (auto catIt = anObj.begin(); catIt != anObj.end(); ++catIt)	{
			json		catValue = catIt.value();
			if (catValue.is_string())
				_categories.push_back(catValue.get<string>());
		}
	}
	
	//	parse the persistent buffers from the JSON dict (ISF v1, deprecated and no longer in use now)
	anObj = (caughtJSONException) ? json() : jblob.value("PERSISTENT_BUFFERS",json());
	if (!anObj.is_null())	{
		//	if the persistent buffers object is an array, check that they're strings and add accordingly
		if (anObj.type() == json::value_t::array)	{
			//for (auto const & it : anObj)	{
			for (auto & it : anObj)	{
				if (it.type() == json::value_t::string)	{
					_persistentPassTargets.emplace_back(ISFPassTarget::Create(it.get<string>(), this));
				}
			}
		}
		//	else if the persistent buffers object is a dict, add and populate the dict accordingly
		else if (anObj.type() == json::value_t::object)	{
			for (json::iterator		it = anObj.begin(); it != anObj.end(); ++it)	{
				string				bufferName = it.key();
				json				bufferDescription = it.value();
				if (bufferDescription.type() == json::value_t::object)	{
					ISFPassTargetRef		newTargetBuffer = ISFPassTarget::Create(bufferName, this);
					json				tmpObj = bufferDescription.value("WIDTH",json());
					if (tmpObj != nullptr)	{
						switch (tmpObj.type())	{
						case json::value_t::null:
						case json::value_t::object:
						case json::value_t::array:
						case json::value_t::boolean:
						case json::value_t::discarded:
						case json::value_t::string:
							{
							string			tmpString = tmpObj.get<string>();
							FindAndReplaceInPlace("$", "", tmpString);
							newTargetBuffer->setTargetWidthString(tmpString);
							}
							break;
						case json::value_t::number_integer:
							{
							int				tmpVal = tmpObj.get<int>();
							newTargetBuffer->setTargetWidthString(FmtString("%d",tmpVal));
							}
							break;
						case json::value_t::number_unsigned:
							{
							unsigned long	tmpVal = tmpObj.get<unsigned long>();
							newTargetBuffer->setTargetWidthString(FmtString("%ld",tmpVal));
							}
							break;
						case json::value_t::number_float:
							{
							double			tmpVal = tmpObj.get<float>();
							newTargetBuffer->setTargetWidthString(FmtString("%f",tmpVal));
							}
							break;
						}
						
					}
					tmpObj = bufferDescription.value("HEIGHT",json());
					if (tmpObj != nullptr)	{
						switch (tmpObj.type())	{
						case json::value_t::null:
						case json::value_t::object:
						case json::value_t::array:
						case json::value_t::boolean:
						case json::value_t::discarded:
						case json::value_t::string:
							{
							string			tmpString = tmpObj.get<string>();
							FindAndReplaceInPlace("$", "", tmpString);
							newTargetBuffer->setTargetHeightString(tmpString);
							}
							break;
						case json::value_t::number_integer:
							{
							int				tmpVal = tmpObj.get<int>();
							newTargetBuffer->setTargetHeightString(FmtString("%d",tmpVal));
							}
							break;
						case json::value_t::number_unsigned:
							{
							unsigned long	tmpVal = tmpObj.get<unsigned long>();
							newTargetBuffer->setTargetHeightString(FmtString("%ld",tmpVal));
							}
							break;
						case json::value_t::number_float:
							{
							double			tmpVal = tmpObj.get<float>();
							newTargetBuffer->setTargetHeightString(FmtString("%f",tmpVal));
							}
							break;
						}
					}
					tmpObj = bufferDescription.value("FLOAT",json());
					if (tmpObj != nullptr && tmpObj.is_number() && tmpObj.get<bool>())	{
						newTargetBuffer->setFloatFlag(true);
					}
					//	don't forget to flag it as a persistent buffer!
					newTargetBuffer->setPersistentFlag(true);
					//	add the new persistent buffer (as a render pass) to the array of render passes
					_persistentPassTargets.push_back(newTargetBuffer);
				}
			}
		}
		
	}
	
	
	//	parse the array of imported images
	anObj = (caughtJSONException) ? json() : jblob.value("IMPORTED",json());
	if (anObj != nullptr)	{
		string			parentDirectory = (_path==nullptr) ? std::string("") : StringByDeletingLastPathComponent(*_path);
		
		//	this is the block that we're going to use to parse an import dict and import its contents.
		function<void(const json &)>		parseImportedImageDict = [&](const json & importDict)	{
			//cout << __PRETTY_FUNCTION__ << endl;
			
			json			samplerNameJ = importDict.value("NAME",json());
			if (!samplerNameJ.is_null())	{
				json			cubeFlagJ = importDict.value("TYPE",json());
				if (!cubeFlagJ.is_null() && cubeFlagJ.get<string>() != "cube")
					cubeFlagJ = json();
				
				GLBufferRef		importedBuffer = nullptr;
				
				//	are we a cube map?
				if (!cubeFlagJ.is_null())	{
					//	the PATH var should have an array of strings with the paths to the six files...
					json			partialPathsJ = importDict.value("PATH",json());
					if (partialPathsJ.is_null() || !partialPathsJ.is_array())	{
						if (_throwExcept)	{
							if (_path != nullptr)
								throw ISFErr(ISFErrType_MalformedJSON, "PATH for IMPORTED is missing or is not an array", *_path);
							else
								throw ISFErr(ISFErrType_MalformedJSON, "PATH for IMPORTED is missing or is not an array", "");
						}
					}
					else	{
						//	assemble an array with the full paths to the files
						vector<string>		fullPaths(0);
						for (auto it=partialPathsJ.begin(); it!=partialPathsJ.end(); ++it)	{
							json		tmpPath = it.value();
							if (!tmpPath.is_string())	{
								if (_throwExcept)	{
									if (_path != nullptr)
										throw ISFErr(ISFErrType_MalformedJSON, "PATH in array for IMPORTED is not a string", *_path);
									else
										throw ISFErr(ISFErrType_MalformedJSON, "PATH in array for IMPORTED is not a string", "");
								}
							}
							else	{
								fullPaths.emplace_back(FmtString("%s/%s",parentDirectory.c_str(),tmpPath.get<string>().c_str()));
							}
						}
						//	make a cube texture from the array of paths
						importedBuffer = CreateCubeTexFromImagePaths(fullPaths);
						if (importedBuffer == nullptr)	{
							if (_throwExcept)	{
								if (_path != nullptr)
									throw ISFErr(ISFErrType_ErrorLoading, "unable to make texture from cube map files", *_path);
								else
									throw ISFErr(ISFErrType_ErrorLoading, "unable to make texture from cube map files", "");
							}
						}
						else	{
							//	make an attrib from the import and store it
							//ISFAttr		newAttrib(samplerNameJ.get<string>(), string(""), string(""), ISFValType_Image, ISFNullVal(), ISFNullVal(), ISFImageVal(importedBuffer), ISFNullVal(), nullptr, nullptr);
							//ISFAttrRef	newAttribRef = make_shared<ISFAttr>(newAttrib);
							ISFAttrRef	newAttribRef = make_shared<ISFAttr>(samplerNameJ.get<string>(), string(""), string(""), ISFValType_Cube, ISFNullVal(), ISFNullVal(), ISFImageVal(importedBuffer), ISFNullVal(), nullptr, nullptr);
							_imageImports.emplace_back(newAttribRef);
						}
					}
				}
				//	else it's just a normal image
				else	{
					//	if the PATH entry isn't a string, throw an error
					json			partialPathJ = importDict.value("PATH",json());
					if (partialPathJ.is_null() || !partialPathJ.is_string())	{
						if (_throwExcept)	{
							if (_path != nullptr)
								throw ISFErr(ISFErrType_MalformedJSON, "PATH for IMPORTED is missing or of wrong type", *_path);
							else
								throw ISFErr(ISFErrType_MalformedJSON, "PATH for IMPORTED is missing or of wrong type", "");
						}
					}
					else	{
						//	get the full path to the image we need to import
						string			fullPath = FmtString("%s/%s", parentDirectory.c_str(), partialPathJ.get<string>().c_str());
						//	import the image to an GLBufferRef
						GLBufferPoolRef		bp = nullptr;
						if (_parentScene == nullptr)
							bp = GetGlobalBufferPool();
						else	{
							bp = _parentScene->privatePool();
							if (bp == nullptr)
								bp = GetGlobalBufferPool();
						}
						importedBuffer = CreateTexFromImage(fullPath, false, bp);
						if (importedBuffer == nullptr)	{
							if (_throwExcept)
								throw ISFErr(ISFErrType_ErrorLoading, "IMPORTED file cannot be loaded: ", fullPath);
						}
						else	{
							//	make an attrib for the import and store it
							//ISFAttr		newAttrib(samplerNameJ.get<string>(), fullPath, string(""), ISFValType_Image, ISFNullVal(), ISFNullVal(), ISFImageVal(importedBuffer), ISFNullVal(), nullptr, nullptr);
							//ISFAttrRef	newAttribRef = make_shared<ISFAttr>(newAttrib);
							ISFAttrRef	newAttribRef = make_shared<ISFAttr>(samplerNameJ.get<string>(), fullPath, string(""), ISFValType_Image, ISFNullVal(), ISFNullVal(), ISFImageVal(importedBuffer), ISFNullVal(), nullptr, nullptr);
							_imageImports.emplace_back(newAttribRef);
						}
					}
				}
			}
			
		};
		
		//	if i'm importing files from a dictionary, execute the block on all the elements in the dict (each element is another dict describing the thing to import)
		if (anObj.type() == json::value_t::object)	{
			for (auto it = anObj.begin(); it != anObj.end(); ++it)	{
				string		itKey = it.key();
				json			itVal = it.value();
				//	if the value doesn't have a "NAME" key, add it
				//if (itVal["NAME"] == nullptr)
				//	itVal["NAME"] = itKey;
				json			subVal = itVal.value("NAME",json());
				if (subVal.is_null())
					itVal["NAME"] = itKey;
				parseImportedImageDict(itVal);
			}
		}
		//	else it's an array- an array full of dictionaries, each of which describes a file to import
		else if (anObj.type() == json::value_t::array)	{
			for (auto const & subObj : anObj)	{
				if (subObj.type() == json::value_t::object)	{
					parseImportedImageDict(subObj);
				}
			}
		}
	}
	
	//	parse the PASSES array of dictionaries describing the various passes (which may need temp buffers)
	anObj = (caughtJSONException) ? json() : jblob.value("PASSES",json());
	if (!anObj.is_null())	{
		if (!anObj.is_array())	{
			if (_throwExcept)	{
				if (_path != nullptr)
					throw ISFErr(ISFErrType_MalformedJSON, "PASSES entry not an array", *_path);
				else
					throw ISFErr(ISFErrType_MalformedJSON, "PASSES entry not an array", "");
			}
		}
		else	{
			for (auto passIt = anObj.begin(); passIt != anObj.end(); ++passIt)	{
				json		rawPassDict = passIt.value();
				//	make a new render pass and populate it from the raw pass dict
				//ISFRenderPass			newPass = ISFRenderPass();
				json				passTarget = rawPassDict.value("TARGET",json());
				if (passTarget.is_string())	{
					string				tmpBufferName = passTarget.get<string>();
					//newPass.targetName = tmpBufferName;
					//newPass.setTargetName(tmpBufferName);
					//	find the target buffer for this pass- first check the persistent buffers
					ISFPassTargetRef		targetBuffer = persistentPassTargetForKey(tmpBufferName);
					//	if i couldn't find a persistent buffer...
					if (targetBuffer == nullptr)	{
						//	create a new target buffer, set its name
						targetBuffer = ISFPassTarget::Create(tmpBufferName, this);
						//	check for PERSISTENT flag as per the ISF 2.0 spec
						json				persistentObj = rawPassDict.value("PERSISTENT",json());
						ISFVal				persistentVal = ISFNullVal();
						if (persistentObj.is_string())	{
							persistentVal = ParseStringAsBool(persistentObj);
							//if (persistentVal.type() == ISFValType_None)
								//persistentVal = ISFValByEvaluatingString(persistentObj);
						}
						else if (persistentObj.is_boolean())
							persistentVal = ISFBoolVal(persistentObj.get<bool>());
						else if (persistentObj.is_number())
							persistentVal = ISFFloatVal(persistentObj.get<double>());
						//	if there's a valid PERSISTENT flag and it's indicating positive, add the new target buffer as a persistent buffer
						if (persistentVal.getDoubleVal() > 0.)	{
							_persistentPassTargets.push_back(targetBuffer);
							targetBuffer->setPersistentFlag(true);
						}
						//	else there's no PERSISTENT flag (or a negative flag) - add the new target buffer as a temporary buffer
						else	{
							_tempPassTargets.push_back(targetBuffer);
							targetBuffer->setPersistentFlag(false);
						}
					}
				
					//	update the width/height stuff for the target buffer
					json		tmpObj;
					tmpObj = rawPassDict.value("WIDTH",json());
					if (tmpObj != nullptr)	{
						switch (tmpObj.type())	{
						case json::value_t::null:
						case json::value_t::object:
						case json::value_t::array:
						case json::value_t::boolean:
						case json::value_t::discarded:
						case json::value_t::string:
							{
							string			tmpString = tmpObj.get<string>();
							FindAndReplaceInPlace("$", "", tmpString);
							targetBuffer->setTargetWidthString(tmpString);
							}
							break;
						case json::value_t::number_integer:
							{
							int				tmpVal = tmpObj.get<int>();
							targetBuffer->setTargetWidthString(FmtString("%d",tmpVal));
							}
							break;
						case json::value_t::number_unsigned:
							{
							unsigned long	tmpVal = tmpObj.get<unsigned long>();
							targetBuffer->setTargetWidthString(FmtString("%ld",tmpVal));
							}
							break;
						case json::value_t::number_float:
							{
							double			tmpVal = tmpObj.get<float>();
							targetBuffer->setTargetWidthString(FmtString("%f",tmpVal));
							}
							break;
						}
					}
					tmpObj = rawPassDict.value("HEIGHT",json());
					if (tmpObj != nullptr)	{
						switch (tmpObj.type())	{
						case json::value_t::null:
						case json::value_t::object:
						case json::value_t::array:
						case json::value_t::boolean:
						case json::value_t::discarded:
						case json::value_t::string:
							{
							string			tmpString = tmpObj.get<string>();
							FindAndReplaceInPlace("$", "", tmpString);
							targetBuffer->setTargetHeightString(tmpString);
							}
							break;
						case json::value_t::number_integer:
							{
							int				tmpVal = tmpObj.get<int>();
							targetBuffer->setTargetHeightString(FmtString("%d",tmpVal));
							}
							break;
						case json::value_t::number_unsigned:
							{
							unsigned long	tmpVal = tmpObj.get<unsigned long>();
							targetBuffer->setTargetHeightString(FmtString("%ld",tmpVal));
							}
							break;
						case json::value_t::number_float:
							{
							double			tmpVal = tmpObj.get<float>();
							targetBuffer->setTargetHeightString(FmtString("%f",tmpVal));
							}
							break;
						}
					}
				
					//	update the float flag for the target buffer
					json		tmpFloatFlag = rawPassDict.value("FLOAT",json());
					ISFVal		tmpFloatVal = ISFNullVal();
					if (tmpFloatFlag.is_string())	{
						tmpFloatVal = ParseStringAsBool(tmpFloatFlag);
						//if (tmpFloatVal.type() == ISFValType_None)
							//tmpFloatVal = ISFValByEvaluatingString(tmpFloatFlag);
					}
					else if (tmpFloatFlag.is_boolean())
						tmpFloatVal = ISFBoolVal(tmpFloatFlag.get<bool>());
					else if (tmpFloatFlag.is_number())
						tmpFloatVal = ISFBoolVal( (tmpFloatFlag.get<double>()>0.) ? true : false );
					targetBuffer->setFloatFlag( (tmpFloatVal.getDoubleVal()>0.) ? true : false );
					//	add the new render pass to the array of render passes
					_renderPasses.emplace_back(tmpBufferName);
				}
				else	{
					//	add an empty render pass to the array of render passes
					_renderPasses.emplace_back("");
				}
			}
		}
	}
	//	if at this point there aren't any passes, add an empty pass
	if (_renderPasses.size() < 1)
		_renderPasses.emplace_back("");
	
	//	parse the INPUTS from the JSON dict (these form the basis of user interaction)
	auto			inputsArray = (caughtJSONException) ? json() : jblob.value("INPUTS",json());
	if (inputsArray != nullptr && inputsArray.type()==json::value_t::array)	{
		ISFValType			newAttribType = ISFValType_None;
		ISFVal				minVal = ISFNullVal();
		ISFVal				maxVal = ISFNullVal();
		ISFVal				defVal = ISFNullVal();
		ISFVal				idenVal = ISFNullVal();
		vector<string>		labelArray;
		vector<int32_t>		valArray;
		bool				isImageInput = false;
		bool				isAudioInput = false;
		bool				isFilterImageInput = false;
		bool				isTransStartImageInput = false;
		bool				isTransEndImageInput = false;
		bool				isTransProgressFloatInput = false;
		
		//	run through the array of inputs
		for (auto it=inputsArray.begin(); it!=inputsArray.end(); ++it)	{
			json		inputDict = it.value();
			//	skip this input if the input isn't a dict
			if (!inputDict.is_object())
				continue;
			//	skip this input if there isn't a name or type string
			json		inputKeyJ = inputDict.value("NAME",json());
			json		typeStringJ = inputDict.value("TYPE",json());
			if (!inputKeyJ.is_string() || !typeStringJ.is_string())
				continue;
			//	we'll need the description and label too
			json		descStringJ = inputDict.value("DESCRIPTION",json());
			string		descString = (descStringJ.is_string()) ? descStringJ.get<string>() : string("");
			json		labelStringJ = inputDict.value("LABEL",json());
			string		labelString = (labelStringJ.is_string()) ? labelStringJ.get<string>() : string("");
			
			//	clear some state vars
			newAttribType = ISFValType_None;
			minVal = ISFNullVal();
			maxVal = ISFNullVal();
			defVal = ISFNullVal();
			idenVal = ISFNullVal();
			labelArray.clear();
			valArray.clear();
			isImageInput = false;
			isAudioInput = false;
			isFilterImageInput = false;
			isTransStartImageInput = false;
			isTransEndImageInput = false;
			isTransProgressFloatInput = false;
			
			//	update state vars based on the type and further parsing of the input dict
			if (typeStringJ == "image")	{
				newAttribType = ISFValType_Image;
				isImageInput = true;
				if (inputKeyJ == "inputImage")	{
					isFilterImageInput = true;
					_type = ISFFileType_Filter;
				}
				else if (inputKeyJ == "startImage")	{
					isTransStartImageInput = true;
				}
				else if (inputKeyJ == "endImage")	{
					isTransEndImageInput = true;
				}
			}
			else if (typeStringJ == "audio")	{
				newAttribType = ISFValType_Audio;
				isAudioInput = true;
				json		tmpMaxJ = inputDict.value("MAX",json());
				if (tmpMaxJ.is_number())
					maxVal = ISFLongVal(tmpMaxJ.get<int32_t>());
			}
			else if (typeStringJ == "audioFFT")	{
				newAttribType = ISFValType_AudioFFT;
				isAudioInput = true;
				json		tmpMaxJ = inputDict.value("MAX",json());
				if (tmpMaxJ.is_number())
					maxVal = ISFLongVal(tmpMaxJ.get<int32_t>());
			}
			else if (typeStringJ == "cube")	{
				newAttribType = ISFValType_Cube;
			}
			else if (typeStringJ == "float")	{
				newAttribType = ISFValType_Float;
				json		tmpObj;
				json		tmpMinJ;
				json		tmpMaxJ;
				tmpMinJ = inputDict.value("MIN",json());
				if (tmpMinJ.is_number())
					minVal = ISFFloatVal(tmpMinJ.get<double>());
				tmpMaxJ = inputDict.value("MAX",json());
				if (tmpMaxJ.is_number())
					maxVal = ISFFloatVal(tmpMaxJ.get<double>());
				
				tmpObj = inputDict.value("DEFAULT",json());
				if (tmpObj.is_number())
					defVal = ISFFloatVal(tmpObj.get<double>());
				tmpObj = inputDict.value("IDENTITY",json());
				if (tmpObj.is_number())
					idenVal = ISFFloatVal(tmpObj.get<double>());
				
				//	if i'm missing a min or a max val, reset both
				if ((minVal.isNullVal() && !maxVal.isNullVal())	||
				(!minVal.isNullVal() && maxVal.isNullVal()))	{
					minVal = ISFNullVal();
					maxVal = ISFNullVal();
				}
				
				//	if i don't have a min/max val, default to a normalized range
				if (minVal.isNullVal() && maxVal.isNullVal())	{
					minVal = ISFFloatVal(0.);
					maxVal = ISFFloatVal(1.);
				}
				if (defVal.isNullVal())
					defVal = ISFFloatVal((maxVal.getDoubleVal()-minVal.getDoubleVal())/2. + minVal.getDoubleVal());
				else	{
					if (defVal.getDoubleVal()<minVal.getDoubleVal())
						defVal = minVal;
					else if (defVal.getDoubleVal()>maxVal.getDoubleVal())
						defVal = maxVal;
				}
				
				//	if this is 'progress' then it's probably the progress input for the transition
				if (inputKeyJ == "progress")	{
					isTransProgressFloatInput = true;
				}
			}
			else if (typeStringJ == "bool")	{
				newAttribType = ISFValType_Bool;
				json		tmpObj;
				tmpObj = inputDict.value("DEFAULT",json());
				if (tmpObj.is_boolean())
					defVal = ISFBoolVal(tmpObj.get<bool>());
				else if (tmpObj.is_number())
					defVal = ISFBoolVal( (tmpObj.get<double>() > 0.) ? true : false );
				else
					defVal = ISFBoolVal(true);
				tmpObj = inputDict.value("IDENTITY",json());
				if (tmpObj.is_boolean())
					idenVal = ISFBoolVal(tmpObj.get<bool>());
				else if (tmpObj.is_number())
					idenVal = ISFBoolVal( (tmpObj.get<double>() > 0.) ?  true : false );
				
				minVal = ISFBoolVal(false);
				maxVal = ISFBoolVal(true);
			}
			else if (typeStringJ == "long")	{
				newAttribType = ISFValType_Long;
				json		tmpObj;
				json		valArrayJ = inputDict.value("VALUES",json());
				json		labelArrayJ = inputDict.value("LABELS",json());
				//	look for VALUES and LABELS arrays (# of elements must match in both)
				if (!valArrayJ.is_null() && !labelArrayJ.is_null() && valArrayJ.size()==labelArrayJ.size())	{
					for (auto it=valArrayJ.begin(); it!=valArrayJ.end(); ++it)	{
						json &		val = it.value();
						if (!val.is_number())	{
							if (_throwExcept)	{
								if (_path != nullptr)
									throw ISFErr(ISFErrType_MalformedJSON, "item in VALUES attrib for a LONG was not a number", *_path);
								else
									throw ISFErr(ISFErrType_MalformedJSON, "item in VALUES attrib for a LONG was not a number", "");
							}
						}
						else	{
							valArray.push_back(val.get<int32_t>());
						}
					}
					for (auto it=labelArrayJ.begin(); it!=labelArrayJ.end(); ++it)	{
						json &		label = it.value();
						if (!label.is_string())	{
							if (_throwExcept)	{
								if (_path != nullptr)
									throw ISFErr(ISFErrType_MalformedJSON, "item in LABELS attrib for a LONG was not a string", *_path);
								else
									throw ISFErr(ISFErrType_MalformedJSON, "item in LABELS attrib for a LONG was not a string", "");
							}
						}
						else	{
							labelArray.push_back(label.get<string>());
						}
					}
				}
				//	else i couldn't find the values/labels arrays- look for MIN/MAX keys
				else	{
					tmpObj = inputDict.value("MIN",json());
					if (tmpObj.is_number())
						minVal = ISFLongVal(tmpObj.get<int32_t>());
					tmpObj = inputDict.value("MAX",json());
					if (tmpObj.is_number())
						maxVal = ISFLongVal(tmpObj.get<int32_t>());
					
					//	if i'm missing a min or a max val, reset both
					if ((minVal.type()==ISFValType_None && maxVal.type()!=ISFValType_None)	||
					(minVal.type()!=ISFValType_None && maxVal.type()==ISFValType_None))	{
						minVal = ISFNullVal();
						maxVal = ISFNullVal();
					}
				}
				
				tmpObj = inputDict.value("DEFAULT",json());
				if (tmpObj.is_number())
					defVal = ISFLongVal(tmpObj.get<int32_t>());
				tmpObj = inputDict.value("IDENTITY",json());
				if (tmpObj.is_number())
					idenVal = ISFLongVal(tmpObj.get<int32_t>());
			}
			else if (typeStringJ == "event")	{
				newAttribType = ISFValType_Event;
			}
			else if (typeStringJ == "color")	{
				newAttribType = ISFValType_Color;
				
				minVal = ISFColorVal(0., 0., 0., 0.);
				maxVal = ISFColorVal(1., 1., 1., 1.);
				
				json		tmpObj;
				tmpObj = inputDict.value("DEFAULT",json());
				if (tmpObj.is_array() && tmpObj.size()==4)	{
					defVal = ISFColorVal(0., 0., 0., 0.);
					json::iterator		it;
					int			i;
					for (it=tmpObj.begin(), i=0; it!=tmpObj.end(); ++it, ++i)	{
						json &		val = it.value();
						if (!val.is_number())	{
							if (_throwExcept)	{
								if (_path != nullptr)
									throw ISFErr(ISFErrType_MalformedJSON, "val in DEFAULT array in COLOR input was not a number", *_path);
								else
									throw ISFErr(ISFErrType_MalformedJSON, "val in DEFAULT array in COLOR input was not a number", "");
							}
						}
						else	{
							//defVal.val.colorVal[i] = val.get<double>();
							defVal.setColorValByChannel(i, val.get<double>());
						}
					}
				}
				tmpObj = inputDict.value("IDENTITY",json());
				if (tmpObj.is_array() && tmpObj.size()==4)	{
					idenVal = ISFColorVal(0., 0., 0., 0.);
					json::iterator		it;
					int					i;
					for (it=tmpObj.begin(), i=0; it!=tmpObj.end(); ++it, ++i)	{
						json &		val = it.value();
						if (!val.is_number())	{
							if (_throwExcept)	{
								if (_path != nullptr)
									throw ISFErr(ISFErrType_MalformedJSON, "val in IDENTITY array in COLOR input was not a number", *_path);
								else
									throw ISFErr(ISFErrType_MalformedJSON, "val in IDENTITY array in COLOR input was not a number", "");
							}
						}
						else	{
							//idenVal.val.colorVal[i] = val.get<double>();
							idenVal.setColorValByChannel(i, val.get<double>());
						}
					}
				}
				//	if i'm missing a min or a max val, reset both
				if ((minVal.type()==ISFValType_None && maxVal.type()!=ISFValType_None)	||
				(minVal.type()!=ISFValType_None && maxVal.type()==ISFValType_None))	{
					minVal = ISFNullVal();
					maxVal = ISFNullVal();
				}
			}
			else if (typeStringJ == "point2D")	{
				newAttribType = ISFValType_Point2D;
				
				json		tmpObj;
				tmpObj = inputDict.value("DEFAULT",json());
				if (tmpObj.is_array() && tmpObj.size()==2)	{
					for_each(tmpObj.begin(), tmpObj.end(), [&](json &n)	{
						if (!n.is_number())	{
							if (_throwExcept)	{
								if (_path != nullptr)
									throw ISFErr(ISFErrType_MalformedJSON, "DEFAULT for point2D input is not a number", *_path);
								else
									throw ISFErr(ISFErrType_MalformedJSON, "DEFAULT for point2D input is not a number", "");
							}
						}
					});
					defVal = ISFPoint2DVal(tmpObj[0].get<double>(), tmpObj[1].get<double>());
				}
				tmpObj = inputDict.value("IDENTITY",json());
				if (tmpObj.is_array() && tmpObj.size()==2)	{
					for_each(tmpObj.begin(), tmpObj.end(), [&](json &n)	{
						if (!n.is_number())	{
							if (_throwExcept)	{
								if (_path != nullptr)
									throw ISFErr(ISFErrType_MalformedJSON, "IDENTITY for point2D input is not a number", *_path);
								else
									throw ISFErr(ISFErrType_MalformedJSON, "IDENTITY for point2D input is not a number", "");
							}
						}
					});
					idenVal = ISFPoint2DVal(tmpObj[0].get<double>(), tmpObj[1].get<double>());
				}
				tmpObj = inputDict.value("MIN",json());
				if (tmpObj.is_array() && tmpObj.size()==2)	{
					for_each(tmpObj.begin(), tmpObj.end(), [&](json &n)	{
						if (!n.is_number())	{
							if (_throwExcept)	{
								if (_path != nullptr)
									throw ISFErr(ISFErrType_MalformedJSON, "MIN for point2D input is not a number", *_path);
								else
									throw ISFErr(ISFErrType_MalformedJSON, "MIN for point2D input is not a number", "");
							}
						}
					});
					minVal = ISFPoint2DVal(tmpObj[0].get<double>(), tmpObj[1].get<double>());
				}
				tmpObj = inputDict.value("MAX",json());
				if (tmpObj.is_array() && tmpObj.size()==2)	{
					for_each(tmpObj.begin(), tmpObj.end(), [&](json &n)	{
						if (!n.is_number())	{
							if (_throwExcept)	{
								if (_path != nullptr)
									throw ISFErr(ISFErrType_MalformedJSON, "MAX for point2D input is not a number", *_path);
								else
									throw ISFErr(ISFErrType_MalformedJSON, "MAX for point2D input is not a number", "");
							}
						}
					});
					maxVal = ISFPoint2DVal(tmpObj[0].get<double>(), tmpObj[1].get<double>());
				}
				//	if i'm missing a min or a max val, reset both
				if ((minVal.type()==ISFValType_None && maxVal.type()!=ISFValType_None)	||
				(minVal.type()!=ISFValType_None && maxVal.type()==ISFValType_None))	{
					minVal = ISFNullVal();
					maxVal = ISFNullVal();
				}
			}
			//	else the attribute wasn't recognized- skip it
			else
				continue;
			
			//	if i'm here, i've got all the data necessary to create an input/attribute and need to do so
			ISFAttrRef		newAttribRef = make_shared<ISFAttr>(inputKeyJ,
				descString,
				labelString,
				newAttribType,
				minVal,
				maxVal,
				defVal,
				idenVal,
				&labelArray,
				&valArray);
			newAttribRef->setIsFilterInputImage(isFilterImageInput);
			newAttribRef->setIsTransStartImage(isTransStartImageInput);
			newAttribRef->setIsTransEndImage(isTransEndImageInput);
			newAttribRef->setIsTransProgressFloat(isTransProgressFloatInput);
			_inputs.push_back(newAttribRef);
			if (isImageInput)
				_imageInputs.push_back(newAttribRef);
			if (isAudioInput)
				_audioInputs.push_back(newAttribRef);
			
		}
		
		//	check to see if this is a transition
		bool		hasStartImage = false;
		bool		hasEndImage = false;
		bool		hasProgress = false;
		for (const auto & input : _inputs)	{
			if (input!=nullptr)	{
				if (input->type()==ISFValType_Image)	{
					if (input->name() == "startImage")
						hasStartImage = true;
					if (input->name() == "endImage")
						hasEndImage = true;
				}
				else if (input->type() == ISFValType_Float)	{
					if (input->name() == "progress")
						hasProgress = true;
				}
			}
			if (hasStartImage && hasEndImage && hasProgress)
				break;
		}
		if (hasStartImage && hasEndImage && hasProgress)
			_type = ISFFileType_Transition;
	}
}
bool ISFDoc::_assembleShaderSource_VarDeclarations(string * outVSString, string * outFSString, GLVersion & inGLVers, const bool & inVarsAsUBO)	{
	lock_guard<recursive_mutex>		lock(_propLock);
	
	if (outVSString==nullptr || outFSString==nullptr)
		return false;
	
	//	we're going to work this by assembling an array of strings, one for each line- have the vector reserve space for enough strings
	vector<string>		vsDeclarations;
	vector<string>		fsDeclarations;
	vector<string>		uboDeclarations;
	
	vsDeclarations.reserve(_inputs.size()+_imageImports.size()+_persistentPassTargets.size()+_tempPassTargets.size()+9);
	fsDeclarations.reserve(vsDeclarations.capacity());
	uboDeclarations.reserve(vsDeclarations.capacity());
	
	//	frag shader always needs an output, which we're naming gl_FragColor so shaders written against GL 2.1 will work.  we'll use a #define in the shader source to make the shader precompiler change gl_FragColor to isf_FragColor
	switch (inGLVers)	{
	case GLVersion_Unknown:
	case GLVersion_2:
	case GLVersion_ES:
	case GLVersion_ES2:
		break;
	case GLVersion_ES3:
	case GLVersion_33:
	case GLVersion_4:
		fsDeclarations.emplace_back("#define gl_FragColor isf_FragColor\n");
		fsDeclarations.emplace_back("out vec4 gl_FragColor;\n");
		break;
	}
	
	//	these are the 9 standard entries
	if (!inVarsAsUBO)	{
		vsDeclarations.emplace_back("uniform int\t\tPASSINDEX;\n");
		fsDeclarations.emplace_back("uniform int\t\tPASSINDEX;\n");
		vsDeclarations.emplace_back("uniform vec2\t\tRENDERSIZE;\n");
		fsDeclarations.emplace_back("uniform vec2\t\tRENDERSIZE;\n");
	}
	else	{
		uboDeclarations.emplace_back("\tint\t\tPASSINDEX;\n");
		uboDeclarations.emplace_back("\tvec2\t\tRENDERSIZE;\n");
	}
	switch (inGLVers)	{
	case GLVersion_Unknown:
	case GLVersion_2:
	case GLVersion_ES:
	case GLVersion_ES2:
		vsDeclarations.emplace_back("varying vec2\t\tisf_FragNormCoord;\n");
		fsDeclarations.emplace_back("varying vec2\t\tisf_FragNormCoord;\n");
		//vsDeclarations.emplace_back("varying vec3\t\tisf_VertNorm;\n");
		//fsDeclarations.emplace_back("varying vec3\t\tisf_VertNorm;\n");
		//vsDeclarations.emplace_back("varying vec3\t\tisf_VertPos;\n");
		//fsDeclarations.emplace_back("varying vec3\t\tisf_VertPos;\n");
		break;
	case GLVersion_ES3:
	case GLVersion_33:
	case GLVersion_4:
		vsDeclarations.emplace_back("out vec2\t\tisf_FragNormCoord;\n");
		fsDeclarations.emplace_back("in vec2\t\tisf_FragNormCoord;\n");
		//vsDeclarations.emplace_back("out vec3\t\tisf_VertNorm;\n");
		//fsDeclarations.emplace_back("in vec3\t\tisf_VertNorm;\n");
		//vsDeclarations.emplace_back("out vec3\t\tisf_VertPos;\n");
		//fsDeclarations.emplace_back("in vec3\t\tisf_VertPos;\n");
		break;
	}
	if (!inVarsAsUBO)	{
		vsDeclarations.emplace_back("uniform float\t\tTIME;\n");
		fsDeclarations.emplace_back("uniform float\t\tTIME;\n");
		vsDeclarations.emplace_back("uniform float\t\tTIMEDELTA;\n");
		fsDeclarations.emplace_back("uniform float\t\tTIMEDELTA;\n");
		vsDeclarations.emplace_back("uniform vec4\t\tDATE;\n");
		fsDeclarations.emplace_back("uniform vec4\t\tDATE;\n");
		vsDeclarations.emplace_back("uniform int\t\tFRAMEINDEX;\n");
		fsDeclarations.emplace_back("uniform int\t\tFRAMEINDEX;\n");
	}
	else	{
		uboDeclarations.emplace_back("\tfloat\t\tTIME;\n");
		uboDeclarations.emplace_back("\tfloat\t\tTIMEDELTA;\n");
		uboDeclarations.emplace_back("\tvec4\t\tDATE;\n");
		uboDeclarations.emplace_back("\tint\t\tFRAMEINDEX;\n");
	}
	
	//	this block will be used to add declarations for a provided ISFAttr
	auto	attribDecBlock = [&](const ISFAttrRef & inRef)	{
		const string &		tmpName = inRef->name();
		const char *		nameCStr = tmpName.c_str();
		switch (inRef->type())	{
		case ISFValType_None:
			break;
		case ISFValType_Event:
		case ISFValType_Bool:
			if (!inVarsAsUBO)	{
				vsDeclarations.emplace_back(FmtString("uniform bool\t\t%s;\n", nameCStr));
				fsDeclarations.emplace_back(FmtString("uniform bool\t\t%s;\n", nameCStr));
			}
			else	{
				uboDeclarations.emplace_back(FmtString("\tbool\t\t%s;\n", nameCStr));
			}
			break;
		case ISFValType_Long:
			if (!inVarsAsUBO)	{
				vsDeclarations.emplace_back(FmtString("uniform int\t\t%s;\n", nameCStr));
				fsDeclarations.emplace_back(FmtString("uniform int\t\t%s;\n", nameCStr));
			}
			else	{
				uboDeclarations.emplace_back(FmtString("\tint\t\t%s;\n", nameCStr));
			}
			break;
		case ISFValType_Float:
			if (!inVarsAsUBO)	{
				vsDeclarations.emplace_back(FmtString("uniform float\t\t%s;\n", nameCStr));
				fsDeclarations.emplace_back(FmtString("uniform float\t\t%s;\n", nameCStr));
			}
			else	{
				uboDeclarations.emplace_back(FmtString("\tfloat\t\t%s;\n", nameCStr));
			}
			break;
		case ISFValType_Point2D:
			if (!inVarsAsUBO)	{
				vsDeclarations.emplace_back(FmtString("uniform vec2\t\t%s;\n", nameCStr));
				fsDeclarations.emplace_back(FmtString("uniform vec2\t\t%s;\n", nameCStr));
			}
			else	{
				uboDeclarations.emplace_back(FmtString("\tvec2\t\t%s;\n", nameCStr));
			}
			break;
		case ISFValType_Color:
			if (!inVarsAsUBO)	{
				vsDeclarations.emplace_back(FmtString("uniform vec4\t\t%s;\n", nameCStr));
				fsDeclarations.emplace_back(FmtString("uniform vec4\t\t%s;\n", nameCStr));
			}
			else	{
				uboDeclarations.emplace_back(FmtString("\tvec4\t\t%s;\n", nameCStr));
			}
			break;
		case ISFValType_Cube:
			//	make a sampler for the cubemap texture
			vsDeclarations.emplace_back(FmtString("uniform samplerCube\t\t%s;\n", nameCStr));
			fsDeclarations.emplace_back(FmtString("uniform samplerCube\t\t%s;\n", nameCStr));
			//	just pass in the imgSize
			if (!inVarsAsUBO)	{
				vsDeclarations.emplace_back(FmtString("uniform vec2\t\t_%s_imgSize;\n", nameCStr));
				fsDeclarations.emplace_back(FmtString("uniform vec2\t\t_%s_imgSize;\n", nameCStr));
			}
			else	{
				uboDeclarations.emplace_back(FmtString("\tvec2\t\t_%s_imgSize;\n", nameCStr));
			}
			break;
		case ISFValType_Image:
		case ISFValType_Audio:
		case ISFValType_AudioFFT:
			{
				ISFVal			attribVal = inRef->currentVal();
				GLBufferRef	attribBuffer = attribVal.imageBuffer();
				if (attribBuffer==nullptr || attribBuffer->desc.target==GLBuffer::Target_2D)	{
					vsDeclarations.emplace_back(FmtString("uniform sampler2D\t\t%s;\n", nameCStr));
					fsDeclarations.emplace_back(FmtString("uniform sampler2D\t\t%s;\n", nameCStr));
				}
				else	{
					vsDeclarations.emplace_back(FmtString("uniform sampler2DRect\t\t%s;\n", nameCStr));
					fsDeclarations.emplace_back(FmtString("uniform sampler2DRect\t\t%s;\n", nameCStr));
				}
				//	a vec4 describing the image rect IN NATIVE GL TEXTURE COORDS (2D is normalized, RECT is not)
				if (!inVarsAsUBO)	{
					vsDeclarations.emplace_back(FmtString("uniform vec4\t\t_%s_imgRect;\n", nameCStr));
					fsDeclarations.emplace_back(FmtString("uniform vec4\t\t_%s_imgRect;\n", nameCStr));
				}
				else	{
					uboDeclarations.emplace_back(FmtString("\tvec4\t\t_%s_imgRect;\n", nameCStr));
				}
				//	a vec2 describing the size in pixels of the image
				if (!inVarsAsUBO)	{
					vsDeclarations.emplace_back(FmtString("uniform vec2\t\t_%s_imgSize;\n", nameCStr));
					fsDeclarations.emplace_back(FmtString("uniform vec2\t\t_%s_imgSize;\n", nameCStr));
				}
				else	{
					uboDeclarations.emplace_back(FmtString("\tvec2\t\t_%s_imgSize;\n", nameCStr));
				}
				//	a bool describing whether the image in the texture should be flipped vertically
				if (!inVarsAsUBO)	{
					vsDeclarations.emplace_back(FmtString("uniform bool\t\t_%s_flip;\n", nameCStr));
					fsDeclarations.emplace_back(FmtString("uniform bool\t\t_%s_flip;\n", nameCStr));
				}
				else	{
					uboDeclarations.emplace_back(FmtString("\tbool\t\t_%s_flip;\n", nameCStr));
				}
				break;
			}
		}
	};
	
	//	this block will be used to add declarations for a provided ISFPassTarget
	auto		targetBufferBlock = [&](const ISFPassTargetRef & inRef)	{
		const string &		tmpName = inRef->name();
		const char *		nameCStr = tmpName.c_str();
		GLBufferRef		bufferRef = inRef->buffer();
		if (bufferRef==nullptr || bufferRef->desc.target==GLBuffer::Target_2D)	{
			vsDeclarations.emplace_back(FmtString("uniform sampler2D\t\t%s;\n", nameCStr));
			fsDeclarations.emplace_back(FmtString("uniform sampler2D\t\t%s;\n", nameCStr));
		}
		else	{
			vsDeclarations.emplace_back(FmtString("uniform sampler2DRect\t\t%s;\n", nameCStr));
			fsDeclarations.emplace_back(FmtString("uniform sampler2DRect\t\t%s;\n", nameCStr));
		}
		if (!inVarsAsUBO)	{
			vsDeclarations.emplace_back(FmtString("uniform vec4\t\t_%s_imgRect;\n", nameCStr));
			fsDeclarations.emplace_back(FmtString("uniform vec4\t\t_%s_imgRect;\n", nameCStr));
			vsDeclarations.emplace_back(FmtString("uniform vec2\t\t_%s_imgSize;\n", nameCStr));
			fsDeclarations.emplace_back(FmtString("uniform vec2\t\t_%s_imgSize;\n", nameCStr));
			vsDeclarations.emplace_back(FmtString("uniform bool\t\t_%s_flip;\n", nameCStr));
			fsDeclarations.emplace_back(FmtString("uniform bool\t\t_%s_flip;\n", nameCStr));
		}
		else	{
			uboDeclarations.emplace_back(FmtString("\tvec4\t\t_%s_imgRect;\n", nameCStr));
			uboDeclarations.emplace_back(FmtString("\tvec2\t\t_%s_imgSize;\n", nameCStr));
			uboDeclarations.emplace_back(FmtString("\tbool\t\t_%s_flip;\n", nameCStr));
		}
	};
	
	//	add the variables for the various inputs
	for (const auto & attrIt : _inputs)
		attribDecBlock(attrIt);
	//	add the variables for the imported buffers
	for (const auto & attrIt : _imageImports)
		attribDecBlock(attrIt);
	//	add the variables for the persistent buffers
	for (const auto & tgBufIt : _persistentPassTargets)
		targetBufferBlock(tgBufIt);
	//	add the variables for the temp buffers
	for (const auto & tgBufIt : _tempPassTargets)
		targetBufferBlock(tgBufIt);
	/*
	cout << "VS declarations are:\n";
	for (const auto & stringIt : vsDeclarations)	{
		cout << "\t" << stringIt << endl;
	}
	cout << "FS declarations are:\n";
	for (const auto & stringIt : fsDeclarations)	{
		cout << "\t" << stringIt << endl;
	}
	*/
	//	now calculate the total length of the output string and reserve space for it
	size_t			reserveSize = 0;
	for (const auto & stringIt : fsDeclarations)	{
		reserveSize += stringIt.size();
	}
	//	reserve a bit extra if we're declaring the vars as a UBO
	const string		varsAsUBOHeader = string("layout (std140) uniform VVISF_UNIFORMS\t{\n");
	const string		varsAsUBOFooter = string("};\n");
	if (inVarsAsUBO)	{
		reserveSize += varsAsUBOHeader.size();
		reserveSize += varsAsUBOFooter.size();
	}
	outVSString->reserve(reserveSize);
	outFSString->reserve(reserveSize);
	//	now copy the individual declarations to the output string
	for (const auto & stringIt : vsDeclarations)	{
		outVSString->append(stringIt);
	}
	for (const auto & stringIt : fsDeclarations)	{
		outFSString->append(stringIt);
	}
	//	dump the ubo declarations (if there are any)
	if (uboDeclarations.size() > 0)	{
		outVSString->append(varsAsUBOHeader);
		outFSString->append(varsAsUBOHeader);
		for (const auto & stringIt : uboDeclarations)	{
			outVSString->append(stringIt);
			outFSString->append(stringIt);
		}
		outVSString->append(varsAsUBOFooter);
		outFSString->append(varsAsUBOFooter);
	}
	
	return true;
}
bool ISFDoc::_assembleSubstitutionMap(map<string,double*> * outMap)	{
	//cout << __PRETTY_FUNCTION__ << endl;
	lock_guard<recursive_mutex>		lock(_propLock);
	
	if (outMap == nullptr)
		return false;
	
	map<string,double*> &	outMapRef = *outMap;
	
	for (const auto & attribRefIt : _inputs)	{
		
		const string &		tmpName = attribRefIt->name();
		
		switch (attribRefIt->type())	{
		case ISFValType_None:
			break;
		case ISFValType_Event:
		case ISFValType_Bool:
		case ISFValType_Long:
		case ISFValType_Float:
			//outMapRef[tmpName] = attribRefIt->currentVal().getDoubleVal();
			outMapRef[tmpName] = attribRefIt->updateAndGetEvalVariable();
			break;
		case ISFValType_Point2D:
		case ISFValType_Color:
		case ISFValType_Cube:
		case ISFValType_Image:
		case ISFValType_Audio:
		case ISFValType_AudioFFT:
			break;
		}
		
	}
	
	return true;
}


/*	========================================	*/
#pragma mark --------------------- ostream


ostream & operator<<(ostream & os, const ISFDoc & n)	{
	os << "doc is \"" << *(n._name) << "\"" << endl;
	os << "\tdoc is a " << ISFFileTypeString(n.type()) << endl;
	
	string				tmpDescription = n.description();
	os << "\tdescription: " << tmpDescription << endl;
	
	string				tmpCredit = n.credit();
	os << "\tcredit: " << tmpCredit << endl;
	
	vector<ISFAttrRef> &		tmpImports = const_cast<ISFDoc*>(&n)->imageImports();
	if (tmpImports.size() > 0)	{
		os << "\tdoc has " << tmpImports.size() << " imported images\n";
		for (auto it=tmpImports.begin(); it!=tmpImports.end(); ++it)	{
			os << "\t\t" << (*it)->name() << ": " << StringFromISFValType((*it)->type());
			if ((*it)->isFilterInputImage())
				os << " <- is filter input image";
			os << endl;
		}
	}

	const vector<ISFPassTargetRef> 	tmpPersistent = const_cast<ISFDoc*>(&n)->persistentPassTargets();
	if (tmpPersistent.size() > 0)	{
		os << "\tdoc has " << tmpPersistent.size() << " persistent buffers\n";
		for (const auto & it : tmpPersistent)	{
			const std::string		tmpWidth = it->targetWidthString();
			const std::string		tmpHeight = it->targetHeightString();
			os << "\t\t" << it->name();
			if (tmpWidth.size() > 0)
				os << ", " << tmpWidth;
			if (tmpHeight.size() > 0)
				os << ", " << tmpHeight;
			if (it->floatFlag())
				os << ", is a FLOAT tex";
			os << endl;
		}
	}
	
	const vector<ISFPassTargetRef> 	tmpTemp = const_cast<ISFDoc*>(&n)->tempPassTargets();
	if (tmpTemp.size() > 0)	{
		os << "\tdoc has " << tmpTemp.size() << " temp buffers\n";
		for (const auto & it : tmpTemp)	{
			const std::string		tmpWidth = it->targetWidthString();
			const std::string		tmpHeight = it->targetHeightString();
			os << "\t\t" << it->name();
			if (tmpWidth.size() > 0)
				os << ", " << tmpWidth;
			if (tmpHeight.size() > 0)
				os << ", " << tmpHeight;
			if (it->floatFlag())
				os << ", is a FLOAT tex";
			os << endl;
		}
	}
	
	vector<string> &		tmpPasses = const_cast<ISFDoc*>(&n)->renderPasses();
	if (tmpPasses.size() > 0)	{
		os << "\tdoc has " << tmpPasses.size() << " render passes\n";
		for (auto it=tmpPasses.begin(); it!=tmpPasses.end(); ++it)
			os << "\t\tpass name: " << *it << endl;
	}

	vector<ISFAttrRef> &		tmpInputs = const_cast<ISFDoc*>(&n)->inputs();
	if (tmpInputs.size() > 0)	{
		os << "\tdoc has " << tmpInputs.size() << " inputs\n";
		for (const auto & it : tmpInputs)	{
			os << "\t\tinput \"" << it->name() << "\" is a " << StringFromISFValType(it->type()) << endl;
			ISFVal&		currentVal = it->currentVal();
			ISFVal&		minVal = it->minVal();
			ISFVal&		maxVal = it->maxVal();
			ISFVal&		defaultVal = it->defaultVal();
			ISFVal&		identityVal = it->identityVal();
			vector<string>&		labels = it->labelArray();
			vector<int32_t>&	vals = it->valArray();
			bool			needsAComma = false;
			bool			needsANewline = false;
			if (currentVal.type()!=ISFValType_None || minVal.type()!=ISFValType_None || maxVal.type()!=ISFValType_None || defaultVal.type()!=ISFValType_None || identityVal.type()!=ISFValType_None || labels.size()>0 || vals.size()>0)	{
				os << "\t\t\t";
				needsANewline = true;
			}
			if (currentVal.type() != ISFValType_None)	{
				if (needsAComma) os << ", ";
				else needsAComma = true;
				os << "current val is " <<  currentVal.getValString();
			}
			if (minVal.type() != ISFValType_None)	{
				if (needsAComma) os << ", ";
				else needsAComma = true;
				os << "min val is " <<  minVal.getValString();
			}
			if (maxVal.type() != ISFValType_None)	{
				if (needsAComma) os << ", ";
				else needsAComma = true;
				os << "max val is " <<  maxVal.getValString();
			}
			if (defaultVal.type() != ISFValType_None)	{
				if (needsAComma) os << ", ";
				else needsAComma = true;
				os << "default val is " <<  defaultVal.getValString();
			}
			if (identityVal.type() != ISFValType_None)	{
				if (needsAComma) os << ", ";
				else needsAComma = true;
				os << "identity val is " <<  identityVal.getValString();
			}
			if (labels.size() > 0)	{
				if (needsAComma) os << ", ";
				else needsAComma = true;
				os << "labels are: {";
				needsAComma = false;
				for (const auto & it : labels)	{
					if (needsAComma) os << ", ";
					else needsAComma = true;
					os << it;
				}
				os << "}";
			}
			if (vals.size() > 0)	{
				if (needsAComma) os << ", ";
				else needsAComma = true;
				os << "vals are: {";
				needsAComma = false;
				for (const auto & it : vals)	{
					if (needsAComma) os << ", ";
					else needsAComma = true;
					os << it;
				}
				os << "}";
			}
			if (needsANewline)
				os << endl;
		}
	}
	
	return os;
}




}
