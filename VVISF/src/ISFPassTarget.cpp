#include "ISFPassTarget.hpp"
#include "ISFScene.hpp"




namespace VVISF
{


using namespace std;
using namespace exprtk;




//	this modifies the passed vars, so you shoud probably make sure the owning doc is locked before running it
void ExpressionUpdater(string ** exprString, expression<double> ** expr, const map<string,double*> & inSubDict, double * outVal);




/*	========================================	*/
#pragma mark *************** ISFPassTarget class ***************

/*	========================================	*/
#pragma mark --------------------- factory/creation

ISFPassTargetRef ISFPassTarget::Create(const string & inName, const ISFDoc * inParentDoc)	{
	return make_shared<ISFPassTarget>(inName, inParentDoc);
}

/*	========================================	*/
#pragma mark --------------------- constructor/destructor

ISFPassTarget::ISFPassTarget(const string & inName, const ISFDoc * inParentDoc)	{
	//cout << __PRETTY_FUNCTION__ << "->" << this << endl;
	name = string(inName);
	parentDoc = (ISFDoc *)inParentDoc;
}
ISFPassTarget::~ISFPassTarget()	{
	//cout << __PRETTY_FUNCTION__ << "->" << this << endl;
	
	lock_guard<mutex>		lock(targetLock);
	if (targetWidthString != nullptr)	{
		delete targetWidthString;
		targetWidthString = nullptr;
	}
	if (targetWidthExpression != nullptr)	{
		delete targetWidthExpression;
		targetWidthExpression = nullptr;
	}
	if (targetHeightString != nullptr)	{
		delete targetHeightString;
		targetHeightString = nullptr;
	}
	if (targetHeightExpression != nullptr)	{
		delete targetHeightExpression;
		targetHeightExpression = nullptr;
	}
}

/*	========================================	*/
#pragma mark --------------------- getters/setters

void ISFPassTarget::setTargetSize(const VVGL::Size & inSize, const bool & inResize, const bool & inCreateNewBuffer)	{
	//using namespace VVISF;
	
	targetWidth = inSize.width;
	targetHeight = inSize.height;
	
	
	//	figure out what pool/copier to use to do stuff- try to use the resources associated with my parent doc's parent scene (if there is one)
	VVGLBufferPoolRef		bp = nullptr;
	VVGLBufferCopierRef		copier = nullptr;
	if (parentDoc != nullptr)	{
		ISFScene		*parentScene = parentDoc->getParentScene();
		if (parentScene != nullptr)	{
			bp = parentScene->getPrivatePool();
			copier = parentScene->getPrivateCopier();
		}
	}
	//	if that didn't work, use the globals...
	if (bp == nullptr)
		bp = GetGlobalBufferPool();
	if (copier == nullptr)
		copier = GetGlobalBufferCopier();
	
	
	//	if the buffer's currently nil
	if (buffer == nullptr)	{
		if (inCreateNewBuffer)	{
			buffer = (floatFlag) ? CreateRGBAFloatTex(inSize, bp) : CreateRGBATex(inSize, bp);
			//buffer = (floatFlag) ? CreateBGRAFloatTex(inSize, bp) : CreateBGRATex(inSize, bp);
			copier->copyBlackFrameTo(buffer);
		}
	}
	//	else there's a buffer...
	else	{
		//	if the buffer size is wrong...
		if (inSize != buffer->srcRect.size)	{
			//	if i'm supposed to resize, do so
			if (inResize)	{
				VVGLBufferRef		newBuffer = (floatFlag) ? CreateRGBAFloatTex(inSize, bp) : CreateRGBATex(inSize, bp);
				//VVGLBufferRef		newBuffer = (floatFlag) ? CreateBGRAFloatTex(inSize, bp) : CreateBGRATex(inSize, bp);
				copier->sizeVariantCopy(buffer, newBuffer);
				buffer = newBuffer;
			}
			//	else i'm not supposed to resize
			else	{
				//	if i'm supposed to create a new buffer
				if (inCreateNewBuffer)	{
					buffer = (floatFlag) ? CreateRGBAFloatTex(inSize, bp) : CreateRGBATex(inSize, bp);
					//buffer = (floatFlag) ? CreateBGRAFloatTex(inSize, bp) : CreateBGRATex(inSize, bp);
				}
				//	else i'm not supposed to create a new buffer
				else	{
					buffer = nullptr;
				}
			}
		}
		//	else the buffer size is fine- do nothing...
		
	}
}
void ISFPassTarget::setTargetWidthString(const string & n)	{
	//cout << __PRETTY_FUNCTION__ << endl;
	
	lock_guard<mutex>		lock(targetLock);
	
	if (targetWidthString != nullptr)	{
		delete targetWidthString;
		targetWidthString = nullptr;
	}
	if (targetWidthExpression != nullptr)	{
		delete targetWidthExpression;
		targetWidthExpression = nullptr;
	}
	
	if (n.length() < 1)
		return;
	
	targetWidthString = new string(n);
	//	leave the expression nil- it'll be instantiated (and compiled) when we evaluate (we need to provide a symbol table with variables)
}
const string ISFPassTarget::getTargetWidthString()	{
	lock_guard<mutex>		lock(targetLock);
	return (targetWidthString==nullptr) ? string("") : string(*targetWidthString);
}
void ISFPassTarget::setTargetHeightString(const string & n)	{
	//cout << __PRETTY_FUNCTION__ << endl;
	
	lock_guard<mutex>		lock(targetLock);
	
	if (targetHeightString != nullptr)	{
		delete targetHeightString;
		targetHeightString = nullptr;
	}
	if (targetHeightExpression != nullptr)	{
		delete targetHeightExpression;
		targetHeightExpression = nullptr;
	}
	
	if (n.length() < 1)
		return;
	
	targetHeightString = new string(n);
	//	leave the expression nil- it'll be instantiated (and compiled) when we evaluate (we need to provide a symbol table with variables)
}
const string ISFPassTarget::getTargetHeightString()	{
	lock_guard<mutex>		lock(targetLock);
	return (targetHeightString==nullptr) ? string("") : string(*targetHeightString);
}
void ISFPassTarget::setFloatFlag(const bool & n)	{
	bool		changed = (floatFlag==n) ? false : true;
	if (!changed)
		return;
	floatFlag = n;
	if (buffer != nullptr)	{
		//	figure out what pool/copier to use to do stuff- try to use the resources associated with my parent doc's parent scene (if there is one)
		VVGLBufferPoolRef		bp = nullptr;
		VVGLBufferCopierRef		copier = nullptr;
		if (parentDoc != nullptr)	{
			ISFScene		*parentScene = parentDoc->getParentScene();
			if (parentScene != nullptr)	{
				bp = parentScene->getPrivatePool();
				copier = parentScene->getPrivateCopier();
			}
		}
		//	if that didn't work, use the globals...
		if (bp == nullptr)
			bp = GetGlobalBufferPool();
		if (copier == nullptr)
			copier = GetGlobalBufferCopier();
		
		VVGLBufferRef		newBuffer = (floatFlag) ? CreateRGBAFloatTex(targetSize(), bp) : CreateRGBATex(targetSize(), bp);
		//VVGLBufferRef		newBuffer = (floatFlag) ? CreateBGRAFloatTex(targetSize(), bp) : CreateBGRATex(targetSize(), bp);
		if (newBuffer != nullptr)	{
			if (copier != nullptr)
				copier->ignoreSizeCopy(buffer, newBuffer);
			buffer = newBuffer;
		}
	}
}

/*	========================================	*/
#pragma mark --------------------- methods

void ISFPassTarget::clearBuffer()	{
	buffer = nullptr;
}
void ISFPassTarget::evalTargetSize(const VVGL::Size & inSize, map<string, double*> & inSymbols, const bool & inResize, const bool & inCreateNewBuffer)	{
	using namespace exprtk;
	
	//cout << __FUNCTION__ << endl;
	//cout << "\tbefore, target size was " << targetWidth << " x " << targetHeight << endl;
	VVGL::Size			newSize(1., 1.);
	{
		lock_guard<mutex>		lock(targetLock);
		
		//	update my local size vars
		widthExpressionVar = inSize.width;
		heightExpressionVar = inSize.height;
		//	update the passed dict with the address of my local size vars
		inSymbols.erase(string("WIDTH"));
		inSymbols.insert(make_pair(string("WIDTH"), &widthExpressionVar));
		inSymbols.erase(string("HEIGHT"));
		inSymbols.insert(make_pair(string("HEIGHT"), &heightExpressionVar));
		
		//	evaluate the width/height expressions, outputting to a new size struct
		if (targetWidthString == nullptr)
			newSize.width = *(inSymbols[string("WIDTH")]);
		else
			ExpressionUpdater(&targetWidthString, &targetWidthExpression, inSymbols, &(newSize.width));
	
		if (targetHeightString == nullptr)
			newSize.height = *(inSymbols[string("HEIGHT")]);
		else
			ExpressionUpdater(&targetHeightString, &targetHeightExpression, inSymbols, &(newSize.height));
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
