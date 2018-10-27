#include "ISFPassTarget.hpp"
#include "ISFScene.hpp"
#include "ISFDoc.hpp"




namespace VVISF
{


using namespace std;
using namespace exprtk;




//	this function modifies the passed vars, so you shoud probably make sure the owning doc is locked before running it
void ExpressionUpdater(string ** exprString, expression<double> ** expr, const map<string,double*> & inSubDict, double * outVal);
//	this is the global copier shared amongst all pass targets as a backup.
static GLTexToTexCopierRef _isfPassTargetCopier = nullptr;




/*	========================================	*/
#pragma mark *************** ISFPassTarget class ***************

/*	========================================	*/
#pragma mark --------------------- factory/creation

ISFPassTargetRef ISFPassTarget::Create(const string & inName, const ISFDoc * inParentDoc)	{
	return make_shared<ISFPassTarget>(inName, inParentDoc);
}
void ISFPassTarget::cleanup()	{
	_isfPassTargetCopier = nullptr;
}

/*	========================================	*/
#pragma mark --------------------- constructor/destructor

ISFPassTarget::ISFPassTarget(const string & inName, const ISFDoc * inParentDoc)	{
	//cout << __PRETTY_FUNCTION__ << "->" << this << endl;
	_name = string(inName);
	_parentDoc = (ISFDoc *)inParentDoc;
	_cachedUnis[0] = make_shared<GLCachedUni>(_name);
	_cachedUnis[1] = make_shared<GLCachedUni>(FmtString("_%s_imgRect",_name.c_str()));
	_cachedUnis[2] = make_shared<GLCachedUni>(FmtString("_%s_imgSize",_name.c_str()));
	_cachedUnis[3] = make_shared<GLCachedUni>(FmtString("_%s_flip",_name.c_str()));
}
ISFPassTarget::~ISFPassTarget()	{
	//cout << __PRETTY_FUNCTION__ << "->" << this << endl;
	
	lock_guard<mutex>		lock(_targetLock);
	if (_targetWidthString != nullptr)	{
		delete _targetWidthString;
		_targetWidthString = nullptr;
	}
	if (_targetWidthExpression != nullptr)	{
		delete _targetWidthExpression;
		_targetWidthExpression = nullptr;
	}
	if (_targetHeightString != nullptr)	{
		delete _targetHeightString;
		_targetHeightString = nullptr;
	}
	if (_targetHeightExpression != nullptr)	{
		delete _targetHeightExpression;
		_targetHeightExpression = nullptr;
	}
}

/*	========================================	*/
#pragma mark --------------------- getters/setters

void ISFPassTarget::setTargetSize(const VVGL::Size & inSize, const bool & inResize, const bool & inCreateNewBuffer)	{
	//using namespace VVISF;
	using namespace std;
	
	_targetWidth = inSize.width;
	_targetHeight = inSize.height;
	
	
	//	figure out what pool/copier to use to do stuff- try to use the resources associated with my parent doc's parent scene (if there is one)
	GLBufferPoolRef		bp = nullptr;
	GLTexToTexCopierRef		copier = nullptr;
	bool					shouldBeFloat = false;
	bool					shouldBeIOSurface = false;
	if (_parentDoc != nullptr)	{
		ISFScene		*parentScene = _parentDoc->getParentScene();
		if (parentScene != nullptr)	{
			bp = parentScene->getPrivatePool();
			copier = parentScene->getPrivateCopier();
			shouldBeFloat = parentScene->getAlwaysRenderToFloat();
			shouldBeIOSurface = parentScene->getPersistentToIOSurface();
		}
	}
	//	if that didn't work, use the globals...
	if (bp == nullptr)
		bp = GetGlobalBufferPool();
	if (copier == nullptr)	{
		if (_isfPassTargetCopier == nullptr)	{
			GLContextRef		bufferPoolCtx = (bp==nullptr) ? nullptr : bp->getContext();
			if (bufferPoolCtx != nullptr)	{
				_isfPassTargetCopier = make_shared<GLTexToTexCopier>(bufferPoolCtx);
				copier = _isfPassTargetCopier;
			}
		}
		else
			copier = _isfPassTargetCopier;
	}
	
	if (bp==nullptr || copier==nullptr)	{
		cout << "\tERR: bailing, pool/copier null, " << __PRETTY_FUNCTION__ << endl;
		return;
	}
	
	//	if the buffer's currently nil
	if (_buffer == nullptr)	{
		if (inCreateNewBuffer)	{
			//_buffer = (_floatFlag) ? CreateBGRAFloatTex(inSize, bp) : CreateBGRATex(inSize, bp);
			//_buffer = (_floatFlag) ? CreateRGBAFloatTex(inSize, bp) : CreateRGBATex(inSize, bp);
			
#if defined(VVGL_SDK_MAC)
			if (shouldBeIOSurface)
				_buffer = (shouldBeFloat) ? CreateRGBAFloatTexIOSurface(inSize, true, bp) : CreateRGBATexIOSurface(inSize, true, bp);
			else
#endif
				_buffer = (shouldBeFloat) ? CreateRGBAFloatTex(inSize, true, bp) : CreateRGBATex(inSize, true, bp);
			
			copier->copyBlackFrameTo(_buffer);
		}
	}
	//	else there's a buffer...
	else	{
		//	if the buffer size is wrong...
		if (inSize != _buffer->srcRect.size)	{
			//	if i'm supposed to resize, do so
			if (inResize)	{
				//GLBufferRef		newBuffer = (_floatFlag) ? CreateBGRAFloatTex(inSize, bp) : CreateBGRATex(inSize, bp);
				//GLBufferRef		newBuffer = (_floatFlag) ? CreateRGBAFloatTex(inSize, bp) : CreateRGBATex(inSize, bp);
				GLBufferRef		newBuffer;
				
#if defined(VVGL_SDK_MAC)
				if (shouldBeIOSurface)
					newBuffer = (shouldBeFloat) ? CreateRGBAFloatTexIOSurface(inSize, true, bp) : CreateRGBATexIOSurface(inSize, true, bp);
				else
#endif
					newBuffer = (shouldBeFloat) ? CreateRGBAFloatTex(inSize, true, bp) : CreateRGBATex(inSize, true, bp);
				
				copier->sizeVariantCopy(_buffer, newBuffer);
				_buffer = newBuffer;
			}
			//	else i'm not supposed to resize
			else	{
				//	if i'm supposed to create a new buffer
				if (inCreateNewBuffer)	{
					//_buffer = (_floatFlag) ? CreateBGRAFloatTex(inSize, bp) : CreateBGRATex(inSize, bp);
					//_buffer = (_floatFlag) ? CreateRGBAFloatTex(inSize, bp) : CreateRGBATex(inSize, bp);
					
#if defined(VVGL_SDK_MAC)
					if (shouldBeIOSurface)
						_buffer = (shouldBeFloat) ? CreateRGBAFloatTexIOSurface(inSize, true, bp) : CreateRGBATexIOSurface(inSize, true, bp);
					else
#endif
						_buffer = (shouldBeFloat) ? CreateRGBAFloatTex(inSize, true, bp) : CreateRGBATex(inSize, true, bp);
				}
				//	else i'm not supposed to create a new buffer
				else	{
					_buffer = nullptr;
				}
			}
		}
		//	else the buffer size is fine- do nothing...
		
	}
}
void ISFPassTarget::setTargetWidthString(const string & n)	{
	//cout << __PRETTY_FUNCTION__ << endl;
	
	lock_guard<mutex>		lock(_targetLock);
	
	if (_targetWidthString != nullptr)	{
		delete _targetWidthString;
		_targetWidthString = nullptr;
	}
	if (_targetWidthExpression != nullptr)	{
		delete _targetWidthExpression;
		_targetWidthExpression = nullptr;
	}
	
	if (n.length() < 1)
		return;
	
	_targetWidthString = new string(n);
	//	leave the expression nil- it'll be instantiated (and compiled) when we evaluate (we need to provide a symbol table with variables)
}
const string ISFPassTarget::getTargetWidthString()	{
	lock_guard<mutex>		lock(_targetLock);
	return (_targetWidthString==nullptr) ? string("") : string(*_targetWidthString);
}
void ISFPassTarget::setTargetHeightString(const string & n)	{
	//cout << __PRETTY_FUNCTION__ << endl;
	
	lock_guard<mutex>		lock(_targetLock);
	
	if (_targetHeightString != nullptr)	{
		delete _targetHeightString;
		_targetHeightString = nullptr;
	}
	if (_targetHeightExpression != nullptr)	{
		delete _targetHeightExpression;
		_targetHeightExpression = nullptr;
	}
	
	if (n.length() < 1)
		return;
	
	_targetHeightString = new string(n);
	//	leave the expression nil- it'll be instantiated (and compiled) when we evaluate (we need to provide a symbol table with variables)
}
const string ISFPassTarget::getTargetHeightString()	{
	lock_guard<mutex>		lock(_targetLock);
	return (_targetHeightString==nullptr) ? string("") : string(*_targetHeightString);
}
void ISFPassTarget::setFloatFlag(const bool & n)	{
	using namespace std;
	
	bool		changed = (_floatFlag==n) ? false : true;
	if (!changed)
		return;
	_floatFlag = n;
	if (_buffer != nullptr)	{
		//	figure out what pool/copier to use to do stuff- try to use the resources associated with my parent doc's parent scene (if there is one)
		GLBufferPoolRef		bp = nullptr;
		GLTexToTexCopierRef		copier = nullptr;
		bool					shouldBeFloat = false;
		bool					shouldBeIOSurface = false;
		if (_parentDoc != nullptr)	{
			ISFScene		*parentScene = _parentDoc->getParentScene();
			if (parentScene != nullptr)	{
				bp = parentScene->getPrivatePool();
				copier = parentScene->getPrivateCopier();
				shouldBeFloat = parentScene->getAlwaysRenderToFloat();
				shouldBeIOSurface = parentScene->getPersistentToIOSurface();
			}
		}
		//	if that didn't work, use the globals...
		if (bp == nullptr)
			bp = GetGlobalBufferPool();
		if (copier == nullptr)	{
			if (_isfPassTargetCopier == nullptr)	{
				GLContextRef		bufferPoolCtx = (bp==nullptr) ? nullptr : bp->getContext();
				if (bufferPoolCtx != nullptr)	{
					_isfPassTargetCopier = make_shared<GLTexToTexCopier>(bufferPoolCtx);
					copier = _isfPassTargetCopier;
				}
			}
			else
				copier = _isfPassTargetCopier;
		}
		
		if (bp==nullptr || copier==nullptr)	{
			cout << "\tERR: bailing, pool/copier null, " << __PRETTY_FUNCTION__ << endl;
			return;
		}
		
		//GLBufferRef		newBuffer = (_floatFlag) ? CreateBGRAFloatTex(targetSize(), bp) : CreateBGRATex(targetSize(), bp);
		//GLBufferRef		newBuffer = (_floatFlag) ? CreateRGBAFloatTex(targetSize(), bp) : CreateRGBATex(targetSize(), bp);
		GLBufferRef		newBuffer;
		
#if defined(VVGL_SDK_MAC)
		if (shouldBeIOSurface)
			newBuffer = (shouldBeFloat) ? CreateRGBAFloatTexIOSurface(targetSize(), true, bp) : CreateRGBATexIOSurface(targetSize(), true, bp);
		else
#endif
			newBuffer = (shouldBeFloat) ? CreateRGBAFloatTex(targetSize(), true, bp) : CreateRGBATex(targetSize(), true, bp);
		
		if (newBuffer != nullptr)	{
			if (copier != nullptr)
				copier->ignoreSizeCopy(_buffer, newBuffer);
			_buffer = newBuffer;
		}
	}
}

/*	========================================	*/
#pragma mark --------------------- methods

void ISFPassTarget::clearBuffer()	{
	_buffer = nullptr;
}
void ISFPassTarget::evalTargetSize(const VVGL::Size & inSize, map<string, double*> & inSymbols, const bool & inResize, const bool & inCreateNewBuffer)	{
	using namespace exprtk;
	
	//cout << __FUNCTION__ << endl;
	//cout << "\tbefore, target size was " << _targetWidth << " x " << _targetHeight << endl;
	VVGL::Size			newSize(1., 1.);
	{
		lock_guard<mutex>		lock(_targetLock);
		
		//	update my local size vars
		_widthExpressionVar = inSize.width;
		_heightExpressionVar = inSize.height;
		//	update the passed dict with the address of my local size vars
		inSymbols.erase(string("WIDTH"));
		inSymbols.insert(make_pair(string("WIDTH"), &_widthExpressionVar));
		inSymbols.erase(string("HEIGHT"));
		inSymbols.insert(make_pair(string("HEIGHT"), &_heightExpressionVar));
		
		//	evaluate the width/height expressions, outputting to a new size struct
		if (_targetWidthString == nullptr)
			newSize.width = *(inSymbols[string("WIDTH")]);
		else	{
			//cout << "\tevaluating width string " << *_targetWidthString << endl;
			ExpressionUpdater(&_targetWidthString, &_targetWidthExpression, inSymbols, &(newSize.width));
		}
	
		if (_targetHeightString == nullptr)
			newSize.height = *(inSymbols[string("HEIGHT")]);
		else	{
			//cout << "\tevaluating height string " << *_targetHeightString << endl;
			ExpressionUpdater(&_targetHeightString, &_targetHeightExpression, inSymbols, &(newSize.height));
		}
	}
	if (std::isnan(newSize.width))
		newSize.width = 1.;
	if (std::isnan(newSize.height))
		newSize.height = 1.;
	
	//	set the target size based on the new size i just calculated
	//cout << "\tsetting target size to " << newSize << endl;
	setTargetSize(newSize, inResize, inCreateNewBuffer);
	
	//cout << "\t" << __FUNCTION__ << "- FINISHED" << endl;
}




void ExpressionUpdater(string ** exprString, expression<double> ** expr, const map<string,double*> & inSubDict, double * outVal)	{
	//cout << __PRETTY_FUNCTION__ << endl;
	
	//	if there's an expression
	if (*expr != nullptr)	{
		//cout << "\t\tthere's already an expression- updating values...\n";
		bool		deleteFlag = false;
		symbol_table<double> &		exprTable = (*expr)->get_symbol_table();
		
		//	run through the substitution dict
		for (auto const & it : inSubDict)	{
			//	if this key isn't in the symbol table, bail and flag for deletion
			if (!exprTable.is_variable(it.first))	{
				deleteFlag = true;
				break;
			}
			
			//	else the key is in the symbol table- update the value
			//else	{
			//	double &		tmpVar = exprTable.get_variable(it.first)->ref();
			//	tmpVar = it.second;
			//	//cout << "\t\t\tupdated " << it.first << " to " << tmpVar << endl;
			//}
			
		}
		
		//exprTable.clear();
		//for (auto const & it : inSubDict)	{
		//	exprTable.add_variable(it.first, const_cast<double&>(it.second));
		//}
		
		//	if i'm flagged for deletion- delete the expression
		if (deleteFlag)	{
			delete *expr;
			*expr = nullptr;
		}
	}
	//	if there's an expression string but no expression
	if (*exprString != nullptr && *expr == nullptr)	{
		//cout << "\t\tmaking an expression...\n";
		//	make a new expression, set it up with a symbol table, and compile it
		*expr = new expression<double>;
		symbol_table<double>		tmpTable;
		parser<double>				tmpParser;
		for (auto const & it : inSubDict)	{
			//tmpTable.add_variable(it.first, const_cast<double&>(it.second));
			tmpTable.add_variable(it.first, const_cast<double&>(*(it.second)));
			//double			tmpDouble = it.second;
			//tmpTable.add_variable(it.first, tmpDouble);
		}
		(*(*expr)).register_symbol_table(tmpTable);
		tmpParser.compile(**exprString, **expr);
	}
	//	if there's an expression, evaluate it and dump it to the output value
	if (*expr != nullptr)	{
		*outVal = (*(*expr)).value();
		
		//delete *expr;
		//*expr = nullptr;
	}
	
}



}
