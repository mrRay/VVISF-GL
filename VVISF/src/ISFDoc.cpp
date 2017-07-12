#include "ISFDoc.hpp"
#include "ISFAttr.hpp"

#include <iostream>
#include <fstream>

#include "ISFPassTarget.hpp"

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


ISFDoc::ISFDoc(const string & inPath, ISFScene * inParentScene) throw(ISFErr)	{
	//cout << __PRETTY_FUNCTION__ << endl;
	
	parentScene = inParentScene;
	
	//	set the local path and name variables
	path = new string(inPath);
	name = new string(LastPathComponent(inPath));
	//cout << "\tpath is " << *path << endl;
	//cout << "\tname is " << *name << endl;
	
	
	//	read the passed file into a string
	ifstream		fin;
	fin.open(inPath);
	if (!fin.is_open())
		throw ISFErr(ISFErrType_MissingResource, "cannot create ISFDoc from passed path", inPath);
	string			rawFile( static_cast<stringstream const &>(stringstream() << fin.rdbuf()).str() );
	fin.close();
	/*
	cout << "\trawFile is:\n";
	cout << "**************************" << endl;
	cout << rawFile << endl;
	cout << "**************************" << endl;
	*/
	
	//	isolate the JSON blob that should be at the beginning of the file in a comment, save it as one string- save everything else as the raw shader source string
	auto			openCommentIndex = rawFile.find("/*");	//	the "+2" is to move the index up past the string we're searching for
	auto			closeCommentIndex = rawFile.find("*/");
	if (openCommentIndex == string::npos || closeCommentIndex == string::npos)	{
		throw ISFErr(ISFErrType_MalformedJSON, "ISFDoc missing comment blob", inPath);
	}
	jsonSourceString = new string(rawFile, 0, closeCommentIndex+2);
	jsonString = new string(rawFile, openCommentIndex+2, closeCommentIndex - (openCommentIndex+2));
	fragShaderSource = new string(rawFile, closeCommentIndex + 2);
	
	//	parse the JSON blob, turning it into objects we can parse programmatically
	json			jblob;
	try	{
		jblob = json::parse(*jsonString);
	}
	catch (std::invalid_argument&)	{
		throw ISFErr(ISFErrType_MalformedJSON, "the JSON blob in this file is malformed.", inPath);
	}
	
	//	parse the description
	auto			anObj = jblob.value("DESCRIPTION",json());
	if (anObj.is_string())	{
		description = new string(anObj.get<string>());
	}
	
	//	parse the credit
	anObj = jblob.value("CREDIT",json());
	if (anObj.is_string())	{
		credit = new string(anObj.get<string>());
	}
	
	//	parse the categories
	anObj = jblob.value("CATEGORIES",json());
	if (!anObj.is_null() && anObj.is_array())	{
		for (auto catIt = anObj.begin(); catIt != anObj.end(); ++catIt)	{
			json		catValue = catIt.value();
			if (catValue.is_string())
				categories.push_back(catValue.get<string>());
		}
	}
	
	//	parse the persistent buffers from the JSON dict (ISF v1, deprecated and no longer in use now)
	anObj = jblob.value("PERSISTENT_BUFFERS",json());
	if (!anObj.is_null())	{
		//	if the persistent buffers object is an array, check that they're strings and add accordingly
		if (anObj.type() == json::value_t::array)	{
			//for (auto const & it : anObj)	{
			for (auto & it : anObj)	{
				if (it.type() == json::value_t::string)	{
					persistentBuffers.emplace_back(ISFPassTarget::Create(it.get<string>(), this));
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
						if (tmpObj.type() == json::value_t::string)	{
							//newTargetBuffer->setTargetWidthString(tmpObj.get<string>());
							
							//string			tmpString = tmpObj.get<string>();
							//replace(tmpString.begin(), tmpString.end(), '$', ' ');
							//newTargetBuffer->setTargetWidthString(tmpString);
							
							string			tmpString = tmpObj.get<string>();
							FindAndReplaceInPlace("$", "", tmpString);
							newTargetBuffer->setTargetWidthString(tmpString);
						}
						else if (tmpObj.is_number())	{
							double			tmpVal = tmpObj.get<float>();
							newTargetBuffer->setTargetWidthString(FmtString("%f", tmpVal));
						}
					}
					tmpObj = bufferDescription.value("HEIGHT",json());
					if (tmpObj != nullptr)	{
						if (tmpObj.type() == json::value_t::string)	{
							//newTargetBuffer->setTargetHeightString(tmpObj.get<string>());
							
							//string			tmpString = tmpObj.get<string>();
							//replace(tmpString.begin(), tmpString.end(), '$', ' ');
							//newTargetBuffer->setTargetHeightString(tmpString);
							
							string			tmpString = tmpObj.get<string>();
							FindAndReplaceInPlace("$", "", tmpString);
							newTargetBuffer->setTargetHeightString(tmpString);
						}
						else if (tmpObj.is_number())	{
							double			tmpVal = tmpObj.get<float>();
							newTargetBuffer->setTargetHeightString(FmtString("%f", tmpVal));
						}
					}
					tmpObj = bufferDescription.value("FLOAT",json());
					if (tmpObj != nullptr && tmpObj.is_number() && tmpObj.get<bool>())	{
						newTargetBuffer->setFloatFlag(true);
					}
					//	add the new persistent buffer (as a render pass) to the array of render passes
					persistentBuffers.push_back(newTargetBuffer);
				}
			}
		}
		
	}
	
	
	//	parse the array of imported images
	anObj = jblob.value("IMPORTED",json());
	if (anObj != nullptr)	{
		string			parentDirectory = StringByDeletingLastPathComponent(inPath);
		
		//	this is the block that we're going to use to parse an import dict and import its contents.
		function<void(const json &)>		parseImportedImageDict = [&](const json & importDict)	{
			//cout << __PRETTY_FUNCTION__ << endl;
			
			json			samplerNameJ = importDict.value("NAME",json());
			if (!samplerNameJ.is_null())	{
				json			cubeFlagJ = importDict.value("TYPE",json());
				if (!cubeFlagJ.is_null() && cubeFlagJ.get<string>() != "cube")
					cubeFlagJ = json();
				
				VVGLBufferRef		importedBuffer = nullptr;
				
				//	are we a cube map?
				if (!cubeFlagJ.is_null())	{
					//	the PATH var should have an array of strings with the paths to the six files...
					json			partialPathsJ = importDict.value("PATH",json());
					if (!partialPathsJ.is_array())
						throw ISFErr(ISFErrType_MalformedJSON, "PATH for IMPORTED is not an array", inPath);
					//	assemble an array with the full paths to the files
					vector<string>		fullPaths(0);
					for (auto it=partialPathsJ.begin(); it!=partialPathsJ.end(); ++it)	{
						json		tmpPath = it.value();
						if (!tmpPath.is_string())
							throw ISFErr(ISFErrType_MalformedJSON, "PATH in array for IMPORTED is not a string", inPath);
						fullPaths.emplace_back(FmtString("%s/%s",parentDirectory.c_str(),tmpPath.get<string>().c_str()));
					}
					//	make a cube texture from the array of paths
					importedBuffer = CreateCubeTexFromImagePaths(fullPaths);
					if (importedBuffer == nullptr)
						throw ISFErr(ISFErrType_ErrorLoading, "unable to make texture from cube map files", inPath);
					//	make an attrib from the import and store it
					//ISFAttr		newAttrib(samplerNameJ.get<string>(), string(""), string(""), ISFValType_Image, ISFNullVal(), ISFNullVal(), ISFImageVal(importedBuffer), ISFNullVal(), nullptr, nullptr);
					//ISFAttrRef	newAttribRef = make_shared<ISFAttr>(newAttrib);
					ISFAttrRef	newAttribRef = make_shared<ISFAttr>(samplerNameJ.get<string>(), string(""), string(""), ISFValType_Cube, ISFNullVal(), ISFNullVal(), ISFImageVal(importedBuffer), ISFNullVal(), nullptr, nullptr);
					imageImports.emplace_back(newAttribRef);
				}
				//	else it's just a normal image
				else	{
					//	if the PATH entry isn't a string, throw an error
					json			partialPathJ = importDict.value("PATH",json());
					if (!partialPathJ.is_string())
						throw ISFErr(ISFErrType_MalformedJSON, "PATH for IMPORTED is missing or of wrong type", inPath);
					//	get the full path to the image we need to import
					string			fullPath = FmtString("%s/%s", parentDirectory.c_str(), partialPathJ.get<string>().c_str());
					//	import the image to an VVGLBufferRef
					importedBuffer = CreateTexFromImage(fullPath);
					if (importedBuffer == nullptr)
						throw ISFErr(ISFErrType_ErrorLoading, "IMPORTED file cannot be loaded", fullPath);
					//	make an attrib for the import and store it
					//ISFAttr		newAttrib(samplerNameJ.get<string>(), fullPath, string(""), ISFValType_Image, ISFNullVal(), ISFNullVal(), ISFImageVal(importedBuffer), ISFNullVal(), nullptr, nullptr);
					//ISFAttrRef	newAttribRef = make_shared<ISFAttr>(newAttrib);
					ISFAttrRef	newAttribRef = make_shared<ISFAttr>(samplerNameJ.get<string>(), fullPath, string(""), ISFValType_Image, ISFNullVal(), ISFNullVal(), ISFImageVal(importedBuffer), ISFNullVal(), nullptr, nullptr);
					imageImports.emplace_back(newAttribRef);
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
	anObj = jblob.value("PASSES",json());
	if (!anObj.is_null())	{
		if (!anObj.is_array())
			throw ISFErr(ISFErrType_MalformedJSON, "PASSES entry not an array", inPath);
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
				ISFPassTargetRef		targetBuffer = getPersistentTargetBuffer(tmpBufferName);
				//	if i couldn't find a persistent buffer...
				if (targetBuffer == nullptr)	{
					//	create a new target buffer, set its name
					targetBuffer = ISFPassTarget::Create(tmpBufferName, this);
					//	check for PERSISTENT flag as per the ISF 2.0 spec
					json				persistentObj = rawPassDict.value("PERSISTENT",json());
					ISFVal				persistentVal = ISFNullVal();
					if (persistentObj.is_string())	{
						persistentVal = ParseStringAsBool(persistentObj);
						//if (persistentVal.getType() == ISFValType_None)
							//persistentVal = ValByEvaluatingString(persistentObj);
					}
					else if (persistentObj.is_boolean())
						persistentVal = ISFBoolVal(persistentObj.get<bool>());
					else if (persistentObj.is_number())
						persistentVal = ISFFloatVal(persistentObj.get<double>());
					//	if there's a valid PERSISTENT flag and it's indicating positive, add the new target buffer as a persistent buffer
					if (persistentVal.getDoubleVal() > 0.)
						persistentBuffers.push_back(targetBuffer);
					//	else there's no PERSISTENT flag (or a negative flag) - add the new target buffer as a temporary buffer
					else
						tempBuffers.push_back(targetBuffer);
				}
				//	update the width/height stuff for the target buffer
				json		tmpString;
				tmpString = rawPassDict.value("WIDTH",json());
				if (tmpString.is_string())	{
					//targetBuffer->setTargetWidthString(tmpString);
					//bufferRequiresEval = true;
					
					//string		tmpString2 = tmpString;
					//replace(tmpString2.begin(), tmpString2.end(), '$', ' ');
					//targetBuffer->setTargetWidthString(tmpString2);
					
					string			tmpString2 = tmpString;
					FindAndReplaceInPlace("$", "", tmpString2);
					targetBuffer->setTargetWidthString(tmpString2);
				}
				tmpString = rawPassDict.value("HEIGHT",json());
				if (tmpString.is_string())	{
					//targetBuffer->setTargetHeightString(tmpString);
					//bufferRequiresEval = true;
					
					//string		tmpString2 = tmpString;
					//replace(tmpString2.begin(), tmpString2.end(), '$', ' ');
					//targetBuffer->setTargetHeightString(tmpString2);
					
					string			tmpString2 = tmpString;
					FindAndReplaceInPlace("$", "", tmpString2);
					targetBuffer->setTargetHeightString(tmpString2);
				}
				//	update the float flag for the target buffer
				json		tmpFloatFlag = rawPassDict.value("FLOAT",json());
				ISFVal		tmpFloatVal = ISFNullVal();
				if (tmpFloatFlag.is_string())	{
					tmpFloatVal = ParseStringAsBool(tmpFloatFlag);
					//if (tmpFloatVal.getType() == ISFValType_None)
						//tmpFloatVal = ValByEvaluatingString(tmpFloatFlag);
				}
				else if (tmpFloatFlag.is_boolean())
					tmpFloatVal = ISFBoolVal(tmpFloatFlag.get<bool>());
				else if (tmpFloatFlag.is_number())
					tmpFloatVal = ISFBoolVal( (tmpFloatFlag.get<double>()>0.) ? true : false );
				targetBuffer->setFloatFlag( (tmpFloatVal.getDoubleVal()>0.) ? true : false );
				//	add the new render pass to the array of render passes
				renderPasses.emplace_back(tmpBufferName);
			}
			else	{
				//	add an empty render pass to the array of render passes
				renderPasses.emplace_back("");
			}
		}
	}
	//	if at this point there aren't any passes, add an empty pass
	if (renderPasses.size() < 1)
		renderPasses.emplace_back("");
	
	//	parse the INPUTS from the JSON dict (these form the basis of user interaction)
	auto			inputsArray = jblob.value("INPUTS",json());
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
			
			//	update state vars based on the type and further parsing of the input dict
			if (typeStringJ == "image")	{
				newAttribType = ISFValType_Image;
				isImageInput = true;
				if (inputKeyJ == "inputImage")	{
					isFilterImageInput = true;
					type = ISFFileType_Filter;
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
						if (!val.is_number())
							throw ISFErr(ISFErrType_MalformedJSON, "item in VALUES attrib for a LONG was not a number", inPath);
						valArray.push_back(val.get<int32_t>());
					}
					for (auto it=labelArrayJ.begin(); it!=labelArrayJ.end(); ++it)	{
						json &		label = it.value();
						if (!label.is_string())
							throw ISFErr(ISFErrType_MalformedJSON, "item in LABELS attrib for a LONG was not a string", inPath);
						labelArray.push_back(label.get<string>());
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
					if ((minVal.getType()==ISFValType_None && maxVal.getType()!=ISFValType_None)	||
					(minVal.getType()!=ISFValType_None && maxVal.getType()==ISFValType_None))	{
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
						if (!val.is_number())
							throw ISFErr(ISFErrType_MalformedJSON, "val in DEFAULT array in COLOR input was not a number", inPath);
						//defVal.val.colorVal[i] = val.get<double>();
						defVal.setColorValByChannel(i, val.get<double>());
					}
				}
				tmpObj = inputDict.value("IDENTITY",json());
				if (tmpObj.is_array() && tmpObj.size()==4)	{
					idenVal = ISFColorVal(0., 0., 0., 0.);
					json::iterator		it;
					int					i;
					for (it=tmpObj.begin(), i=0; it!=tmpObj.end(); ++it, ++i)	{
						json &		val = it.value();
						if (!val.is_number())
							throw ISFErr(ISFErrType_MalformedJSON, "val in IDENTITY array in COLOR input was not a number", inPath);
						//idenVal.val.colorVal[i] = val.get<double>();
						idenVal.setColorValByChannel(i, val.get<double>());
					}
				}
				//	if i'm missing a min or a max val, reset both
				if ((minVal.getType()==ISFValType_None && maxVal.getType()!=ISFValType_None)	||
				(minVal.getType()!=ISFValType_None && maxVal.getType()==ISFValType_None))	{
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
						if (!n.is_number()) throw ISFErr(ISFErrType_MalformedJSON, "DEFAULT for point2D input is not a number", inPath);
					});
					defVal = ISFPoint2DVal(tmpObj[0].get<double>(), tmpObj[1].get<double>());
				}
				tmpObj = inputDict.value("IDENTITY",json());
				if (tmpObj.is_array() && tmpObj.size()==2)	{
					for_each(tmpObj.begin(), tmpObj.end(), [&](json &n)	{
						if (!n.is_number()) throw ISFErr(ISFErrType_MalformedJSON, "IDENTITY for point2D input is not a number", inPath);
					});
					idenVal = ISFPoint2DVal(tmpObj[0].get<double>(), tmpObj[1].get<double>());
				}
				tmpObj = inputDict.value("MIN",json());
				if (tmpObj.is_array() && tmpObj.size()==2)	{
					for_each(tmpObj.begin(), tmpObj.end(), [&](json &n)	{
						if (!n.is_number()) throw ISFErr(ISFErrType_MalformedJSON, "MIN for point2D input is not a number", inPath);
					});
					minVal = ISFPoint2DVal(tmpObj[0].get<double>(), tmpObj[1].get<double>());
				}
				tmpObj = inputDict.value("MAX",json());
				if (tmpObj.is_array() && tmpObj.size()==2)	{
					for_each(tmpObj.begin(), tmpObj.end(), [&](json &n)	{
						if (!n.is_number()) throw ISFErr(ISFErrType_MalformedJSON, "MAX for point2D input is not a number", inPath);
					});
					maxVal = ISFPoint2DVal(tmpObj[0].get<double>(), tmpObj[1].get<double>());
				}
				//	if i'm missing a min or a max val, reset both
				if ((minVal.getType()==ISFValType_None && maxVal.getType()!=ISFValType_None)	||
				(minVal.getType()!=ISFValType_None && maxVal.getType()==ISFValType_None))	{
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
			inputs.push_back(newAttribRef);
			if (isImageInput)
				imageInputs.push_back(newAttribRef);
			if (isAudioInput)
				audioInputs.push_back(newAttribRef);
			
		}
	}
	
	
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
		vertShaderSource = new string(ISFVertPassthru_GL2);
	}
	//	else the file's open- read it into a string
	else	{
		vertShaderSource = new string( static_cast<stringstream const &>(stringstream() << fin.rdbuf()).str() );
		fin.close();
	}
}
ISFDoc::~ISFDoc()	{
	//cout << __PRETTY_FUNCTION__ << endl;
	
	lock_guard<recursive_mutex>		lock(propLock);
	
	if (path != nullptr)	{
		delete path;
		path = nullptr;
	}
	if (name != nullptr)	{
		delete name;
		name = nullptr;
	}
	if (description != nullptr)	{
		delete description;
		description = nullptr;
	}
	if (credit != nullptr)	{
		delete credit;
		credit = nullptr;
	}
	
	
	if (jsonSourceString != nullptr)	{
		delete jsonSourceString;
		jsonSourceString = nullptr;
	}
	if (jsonString != nullptr)	{
		delete jsonString;
		jsonString = nullptr;
	}
	if (vertShaderSource != nullptr)	{
		delete vertShaderSource;
		vertShaderSource = nullptr;
	}
	if (fragShaderSource != nullptr)	{
		delete fragShaderSource;
		fragShaderSource = nullptr;
	}
}


/*	========================================	*/
#pragma mark --------------------- getters


const VVGLBufferRef ISFDoc::getBufferForKey(const string & n)	{
	lock_guard<recursive_mutex>		lock(propLock);
	
	for (const auto & attribRefIt : imageInputs)	{
		if (attribRefIt->getName() == n)
			return attribRefIt->getCurrentVal().getImageBuffer();
	}
	for (const auto & attribRefIt : imageImports)	{
		if (attribRefIt->getName() == n)
			return attribRefIt->getCurrentVal().getImageBuffer();
	}
	for (const auto & attribRefIt : audioInputs)	{
		if (attribRefIt->getName() == n)
			return attribRefIt->getCurrentVal().getImageBuffer();
	}
	
	for (const auto & targetBufIt : persistentBuffers)	{
		if (targetBufIt->getName() == n)
			return targetBufIt->getBuffer();
	}
	for (const auto & targetBufIt : tempBuffers)	{
		if (targetBufIt->getName() == n)
			return targetBufIt->getBuffer();
	}
	return nullptr;
}
const VVGLBufferRef ISFDoc::getPersistentBufferForKey(const string & n)	{
	lock_guard<recursive_mutex>		lock(propLock);
	
	for (const auto & targetBufIt : persistentBuffers)	{
		if (targetBufIt->getName() == n)
			return targetBufIt->getBuffer();
	}
	return nullptr;
}
const VVGLBufferRef ISFDoc::getTempBufferForKey(const string & n)	{
	lock_guard<recursive_mutex>		lock(propLock);
	
	for (const auto & targetBufIt : tempBuffers)	{
		if (targetBufIt->getName() == n)
			return targetBufIt->getBuffer();
	}
	return nullptr;
}
const ISFPassTargetRef ISFDoc::getPersistentTargetForKey(const string & n)	{
	//cout << __PRETTY_FUNCTION__ << ", key is \"" << n << "\"" << endl;
	lock_guard<recursive_mutex>		lock(propLock);
	
	for (const auto & targetBufIt : persistentBuffers)	{
		//cout << "\tchecking persistent buffer named \"" << targetBufIt->getName() << "\"" << endl;
		if (targetBufIt->getName() == n)	{
			//cout << "\tmatch found! returning " << targetBufIt << endl;
			return targetBufIt;
		}
	}
	return nullptr;
}
const ISFPassTargetRef ISFDoc::getTempTargetForKey(const string & n)	{
	//cout << __PRETTY_FUNCTION__ << ", key is \"" << n << "\"" << endl;
	lock_guard<recursive_mutex>		lock(propLock);
	
	for (const auto & targetBufIt : tempBuffers)	{
		//cout << "\tchecking temp buffer named \"" << targetBufIt->getName() << "\"" << endl;
		if (targetBufIt->getName() == n)	{
			//cout << "\tmatch found! returning " << targetBufIt << endl;
			return targetBufIt;
		}
	}
	return nullptr;
}


void ISFDoc::getJSONSourceString(string & outStr)	{
	lock_guard<recursive_mutex>		lock(propLock);
	if (jsonSourceString==nullptr)	{
		outStr = "";
		return;
	}
	outStr.reserve(jsonSourceString->length());
	outStr.clear();
	outStr.append(*jsonSourceString);
}
void ISFDoc::getJSONString(string & outStr)	{
	lock_guard<recursive_mutex>		lock(propLock);
	if (jsonString==nullptr)	{
		outStr = "";
		return;
	}
	outStr.reserve(jsonString->length());
	outStr.clear();
	outStr.append(*jsonString);
}
void ISFDoc::getVertShaderSource(string & outStr)	{
	lock_guard<recursive_mutex>		lock(propLock);
	if (vertShaderSource==nullptr)	{
		outStr = "";
		return;
	}
	outStr.reserve(vertShaderSource->length());
	outStr.clear();
	outStr.append(*vertShaderSource);
}
void ISFDoc::getFragShaderSource(string & outStr)	{
	lock_guard<recursive_mutex>		lock(propLock);
	if (fragShaderSource==nullptr)	{
		outStr = "";
		return;
	}
	outStr.reserve(fragShaderSource->length());
	outStr.clear();
	outStr.append(*fragShaderSource);
}


ISFAttrRef ISFDoc::getInput(const string & inAttrName)	{
	lock_guard<recursive_mutex>		lock(propLock);
	
	for (auto it=inputs.begin(); it!=inputs.end(); ++it)	{
		ISFAttrRef		&tmpAttrib = *it;
		if (tmpAttrib->getName() == inAttrName)
			return tmpAttrib;
	}
	return nullptr;
}
vector<ISFAttrRef> ISFDoc::getInputs(const ISFValType & n)	{
	auto		returnMe = vector<ISFAttrRef>(0);
	lock_guard<recursive_mutex>		lock(propLock);
	
	for (auto it=inputs.begin(); it!=inputs.end(); ++it)	{
		ISFAttrRef		&tmpAttrib = *it;
		if ((tmpAttrib->getType() & n) == n)
			returnMe.push_back(tmpAttrib);
	}
	
	return returnMe;
}
ISFPassTargetRef ISFDoc::getPersistentTargetBuffer(const string & n)	{
	lock_guard<recursive_mutex>		lock(propLock);
	
	for (auto it=persistentBuffers.begin(); it!=persistentBuffers.end(); ++it)	{
		ISFPassTargetRef		&tmpBuffer = *it;
		if (tmpBuffer->getName() == n)
			return tmpBuffer;
	}
	return nullptr;
}
ISFPassTargetRef ISFDoc::getTempTargetBuffer(const string & n)	{
	lock_guard<recursive_mutex>		lock(propLock);
	
	for (auto it=tempBuffers.begin(); it!=tempBuffers.end(); ++it)	{
		ISFPassTargetRef		&tmpBuffer = *it;
		if (tmpBuffer->getName() == n)
			return tmpBuffer;
	}
	return nullptr;
}
string ISFDoc::generateTextureTypeString()	{
	string		returnMe("");
	lock_guard<recursive_mutex>		lock(propLock);
	
	for (const auto & attribRefIt : imageInputs)	{
		if (attribRefIt->shouldHaveImageBuffer())	{
			VVGLBufferRef		tmpBuffer = attribRefIt->getCurrentImageBuffer();
			if (tmpBuffer==nullptr || tmpBuffer->desc.target==VVGLBuffer::Target_2D)
				returnMe.append("2");
#if ISF_TARGET_MAC
			else if (tmpBuffer->desc.target==VVGLBuffer::Target_Rect)
				returnMe.append("R");
#endif
#if !ISF_TARGET_RPI
			else if (tmpBuffer->desc.target==VVGLBuffer::Target_Cube)
				returnMe.append("C");
#endif
		}
	}
	for (const auto & attribRefIt : audioInputs)	{
		if (attribRefIt->shouldHaveImageBuffer())	{
			VVGLBufferRef		tmpBuffer = attribRefIt->getCurrentImageBuffer();
			if (tmpBuffer==nullptr || tmpBuffer->desc.target==VVGLBuffer::Target_2D)
				returnMe.append("2");
#if ISF_TARGET_MAC
			else if (tmpBuffer->desc.target==VVGLBuffer::Target_Rect)
				returnMe.append("R");
#endif
#if !ISF_TARGET_RPI
			else if (tmpBuffer->desc.target==VVGLBuffer::Target_Cube)
				returnMe.append("C");
#endif
		}
	}
	for (const auto & attribRefIt : imageImports)	{
		if (attribRefIt->shouldHaveImageBuffer())	{
			VVGLBufferRef		tmpBuffer = attribRefIt->getCurrentImageBuffer();
			if (tmpBuffer==nullptr || tmpBuffer->desc.target==VVGLBuffer::Target_2D)
				returnMe.append("2");
#if ISF_TARGET_MAC
			else if (tmpBuffer->desc.target==VVGLBuffer::Target_Rect)
				returnMe.append("R");
#endif
#if !ISF_TARGET_RPI
			else if (tmpBuffer->desc.target==VVGLBuffer::Target_Cube)
				returnMe.append("C");
#endif
		}
	}
	for (const auto & targetBufIt : persistentBuffers)	{
		VVGLBufferRef		tmpBuffer = targetBufIt->getBuffer();
		if (tmpBuffer==nullptr || tmpBuffer->desc.target==VVGLBuffer::Target_2D)
			returnMe.append("2");
#if ISF_TARGET_MAC
		else if (tmpBuffer->desc.target==VVGLBuffer::Target_Rect)
			returnMe.append("R");
#endif
#if !ISF_TARGET_RPI
		else if (tmpBuffer->desc.target==VVGLBuffer::Target_Cube)
			returnMe.append("C");
#endif
	}
	for (const auto & targetBufIt : tempBuffers)	{
		VVGLBufferRef		tmpBuffer = targetBufIt->getBuffer();
		if (tmpBuffer==nullptr || tmpBuffer->desc.target==VVGLBuffer::Target_2D)
			returnMe.append("2");
#if ISF_TARGET_MAC
		else if (tmpBuffer->desc.target==VVGLBuffer::Target_Rect)
			returnMe.append("R");
#endif
#if !ISF_TARGET_RPI
		else if (tmpBuffer->desc.target==VVGLBuffer::Target_Cube)
			returnMe.append("C");
#endif
	}
	return returnMe;
}
bool ISFDoc::generateShaderSource(string * outFragSrc, string * outVertSrc, GLVersion & inGLVers)	{
	//cout << __PRETTY_FUNCTION__ << endl;
	lock_guard<recursive_mutex>		lock(propLock);
	
	if (outFragSrc==nullptr || outVertSrc==nullptr || vertShaderSource==nullptr || fragShaderSource==nullptr)
		throw ISFErr(ISFErrType_ErrorParsingFS, "Preflight failed", __PRETTY_FUNCTION__);
	//	assemble the variable declarations
	string		vsVarDeclarations = string("");
	string		fsVarDeclarations = string("");
	if (!_assembleShaderSource_VarDeclarations(&vsVarDeclarations, &fsVarDeclarations, inGLVers))
		throw ISFErr(ISFErrType_ErrorParsingFS, "Var Dec failed", __PRETTY_FUNCTION__);
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
	VVRange		tmpRange(0,0);
	string			searchString("");
	VVGLBufferRef	imgBuffer = nullptr;
	string			modSrcString("");
	string			newString("");
	size_t			tmpIndex;
	
	//	put together a new frag shader string from the raw shader source
	string			newFragShaderSrc = string("");
	newFragShaderSrc.reserve(1.5 * (fsVarDeclarations.size()+fragShaderSource->size()));
	{
		//	remove any lines containing #version tags
		searchString = string("#version");
		tmpRange = VVRange(newFragShaderSrc.find(searchString), searchString.size());
		do	{
			if (tmpRange.loc != string::npos)	{
				tmpIndex = modSrcString.find_first_of("\n\r\f", tmpRange.max());
				if (tmpIndex != string::npos)	{
					tmpRange.len = tmpIndex - tmpRange.loc;
					newFragShaderSrc.erase(tmpRange.loc, tmpRange.len);
					
					tmpRange = VVRange(newFragShaderSrc.find(searchString), searchString.size());
				}
			}
		} while (tmpRange.loc != string::npos);
		//	add the #version tag for the min version of GLSL supported by this major vsn of openGL
		switch (inGLVers)	{
		case GLVersion_Unknown:
		case GLVersion_2:
			newFragShaderSrc.insert(0, string("#version 110\n"));
			break;
		case GLVersion_ES2:
			newFragShaderSrc.insert(0, string("#version 100\n"));
			break;
		case GLVersion_33:
			newFragShaderSrc.insert(0, string("#version 130\n"));
			break;
		case GLVersion_4:
			newFragShaderSrc.insert(0, string("#version 400\n"));
			break;
		}
		
		//	add the compatibility define
		newFragShaderSrc.append(ISF_ES_Compatibility);
		//	copy the variable declarations to the frag shader src
		newFragShaderSrc.append(fsVarDeclarations);
		
		//	now i have to find-and-replace the shader source for various things- make a copy of the raw source and work from that.
		modSrcString.reserve(newFragShaderSrc.capacity());
		modSrcString.append(*fragShaderSource);
		
		//	find-and-replace vv_FragNormCoord (v1 of the ISF spec) with isf_FragNormCoord (v2 of the ISF spec)
		searchString = string("vv_FragNormCoord");
		newString = string("isf_FragNormCoord");
		tmpRange = VVRange(0, searchString.size());
		do	{
			tmpRange.loc = modSrcString.find(searchString);
			if (tmpRange.loc != string::npos)
				modSrcString.replace(tmpRange.loc, tmpRange.len, newString, 0, newString.size());
		} while (tmpRange.loc != string::npos);
		
		
		//	now find-and-replace IMG_PIXEL
		searchString = string("IMG_PIXEL");
		imgBuffer = nullptr;
		tmpRange = VVRange(0, searchString.size());
		do	{
			tmpRange = VVRange(modSrcString.find(searchString), searchString.size());
			if (tmpRange.loc != string::npos)	{
				vector<string>		varArray(0);
				varArray.reserve(5);
				VVRange			fullFuncRangeToReplace = LexFunctionCall(modSrcString, tmpRange, varArray);
				size_t				varArrayCount = varArray.size();
				if (varArrayCount!=2 && varArrayCount!=3)	{
					throw ISFErr(ISFErrType_ErrorParsingFS, "IMG_PIXEL has wrong number of arguments", *path);
				}
				else	{
					string			newFuncString("");
					string &		samplerName = varArray[0];
					const char *	samplerNameC = samplerName.c_str();
					string &		samplerCoord = varArray[1];
					const char *	samplerCoordC = samplerCoord.c_str();
					imgBuffer = getBufferForKey(samplerName);
					if (imgBuffer==nullptr || imgBuffer->desc.target==VVGLBuffer::Target_2D)	{
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
		tmpRange = VVRange(0, searchString.size());
		do	{
			tmpRange = VVRange(modSrcString.find(searchString), searchString.size());
			if (tmpRange.loc != string::npos)	{
				vector<string>		varArray(0);
				varArray.reserve(5);
				VVRange			fullFuncRangeToReplace = LexFunctionCall(modSrcString, tmpRange, varArray);
				size_t				varArrayCount = varArray.size();
				if (varArrayCount!=2 && varArrayCount!=3)	{
					throw ISFErr(ISFErrType_ErrorParsingFS, "IMG_NORM_PIXEL has wrong number of arguments", *path);
				}
				else	{
					string			newFuncString("");
					string &		samplerName = varArray[0];
					const char *	samplerNameC = samplerName.c_str();
					string &		samplerCoord = varArray[1];
					const char *	samplerCoordC = samplerCoord.c_str();
					imgBuffer = getBufferForKey(samplerName);
					if (imgBuffer==nullptr || imgBuffer->desc.target==VVGLBuffer::Target_2D)	{
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
		tmpRange = VVRange(0, searchString.size());
		do	{
			tmpRange = VVRange(modSrcString.find(searchString), searchString.size());
			if (tmpRange.loc != string::npos)	{
				vector<string>		varArray(0);
				varArray.reserve(5);
				VVRange			fullFuncRangeToReplace = LexFunctionCall(modSrcString, tmpRange, varArray);
				size_t				varArrayCount = varArray.size();
				if (varArrayCount!=1)	{
					throw ISFErr(ISFErrType_ErrorParsingFS, "IMG_THIS_PIXEL has wrong number of arguments", *path);
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
					case GLVersion_ES2:
						if (imgBuffer==nullptr || imgBuffer->desc.target==VVGLBuffer::Target_2D)	{
							newFuncString = FmtString("texture2D(%s, _%s_texCoord)",samplerNameC,samplerNameC);
						}
						else	{
							newFuncString = FmtString("texture2DRect(%s, _%s_texCoord)",samplerNameC,samplerNameC);
						}
						break;
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
				case GLVersion_ES2:
					newFragShaderSrc.append(FmtString("varying vec2\t\t_%s_texCoord;\n",it.c_str()));
					break;
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
		tmpRange = VVRange(0, searchString.size());
		do	{
			tmpRange = VVRange(modSrcString.find(searchString), searchString.size());
			if (tmpRange.loc != string::npos)	{
				vector<string>		varArray(0);
				varArray.reserve(5);
				VVRange			fullFuncRangeToReplace = LexFunctionCall(modSrcString, tmpRange, varArray);
				size_t				varArrayCount = varArray.size();
				if (varArrayCount!=1)	{
					throw ISFErr(ISFErrType_ErrorParsingFS, "IMG_THIS_NORM_PIXEL has wrong number of arguments", *path);
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
					case GLVersion_ES2:
						if (imgBuffer==nullptr || imgBuffer->desc.target==VVGLBuffer::Target_2D)	{
							newFuncString = FmtString("texture2D(%s, _%s_normTexCoord)",samplerNameC,samplerNameC);
						}
						else	{
							newFuncString = FmtString("texture2DRect(%s, _%s_normTexCoord)",samplerNameC,samplerNameC);
						}
						break;
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
				case GLVersion_ES2:
					newFragShaderSrc.append(FmtString("varying vec2\t\t_%s_normTexCoord;\n",it.c_str()));
					break;
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
		tmpRange = VVRange(0, searchString.size());
		do	{
			tmpRange = VVRange(modSrcString.find(searchString), searchString.size());
			if (tmpRange.loc != string::npos)	{
				vector<string>		varArray(0);
				varArray.reserve(5);
				VVRange		fullFuncRangeToReplace = LexFunctionCall(modSrcString, tmpRange, varArray);
				size_t			varArrayCount = varArray.size();
				if (varArrayCount != 1)	{
					throw ISFErr(ISFErrType_ErrorParsingFS, "IMG_SIZE has wrong number of arguments", *path);
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
	newVertShaderSrc.reserve(2.5 * (vsVarDeclarations.size()+vertShaderSource->size()));
	{
		//	remove any lines containing #version tags
		searchString = string("#version");
		tmpRange = VVRange(newVertShaderSrc.find(searchString), searchString.size());
		do	{
			if (tmpRange.loc != string::npos)	{
				tmpIndex = modSrcString.find_first_of("\n\r\f", tmpRange.max());
				if (tmpIndex != string::npos)	{
					tmpRange.len = tmpIndex - tmpRange.loc;
					newVertShaderSrc.erase(tmpRange.loc, tmpRange.len);
					
					tmpRange = VVRange(newVertShaderSrc.find(searchString), searchString.size());
				}
			}
		} while (tmpRange.loc != string::npos);
		//	add the #version tag for the min version of GLSL supported by this major vsn of openGL
		switch (inGLVers)	{
		case GLVersion_Unknown:
		case GLVersion_2:
			newVertShaderSrc.insert(0, string("#version 110\n"));
			break;
		case GLVersion_ES2:
			newVertShaderSrc.insert(0, string("#version 100\n"));
			break;
		case GLVersion_33:
			newVertShaderSrc.insert(0, string("#version 130\n"));
			break;
		case GLVersion_4:
			newVertShaderSrc.insert(0, string("#version 400\n"));
			break;
		}
		
		//	add the compatibility define
		newVertShaderSrc.append(ISF_ES_Compatibility);
		//	load any specific vars or function declarations for the vertex shader from an included file
		switch (inGLVers)	{
		case GLVersion_Unknown:
		case GLVersion_2:
		case GLVersion_ES2:
			newVertShaderSrc.append(ISFVertVarDec_GLES2);
			break;
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
					case GLVersion_ES2:
						newVertShaderSrc.append(FmtString("varying vec2\t\t_%s_texCoord;\n",it.c_str()));
						break;
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
					case GLVersion_ES2:
						newVertShaderSrc.append(FmtString("varying vec2\t\t_%s_normTexCoord;\n",it.c_str()));
						break;
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
		modSrcString.append(*vertShaderSource);
		
		//	find-and-replace vv_FragNormCoord (v1 of the ISF spec) with isf_FragNormCoord (v2 of the ISF spec)
		searchString = string("vv_FragNormCoord");
		newString = string("isf_FragNormCoord");
		tmpRange = VVRange(0, searchString.size());
		do	{
			tmpRange.loc = modSrcString.find(searchString);
			if (tmpRange.loc != string::npos)
				modSrcString.replace(tmpRange.loc, tmpRange.len, newString, 0, newString.size());
		} while (tmpRange.loc != string::npos);
		
		//	find-and-replace vv_vertShaderInit (v1 of the ISF spec) with isf_vertShaderInit (v2 of the ISF spec)
		searchString = string("vv_vertShaderInit");
		newString = string("isf_vertShaderInit");
		tmpRange = VVRange(0, searchString.size());
		do	{
			tmpRange.loc = modSrcString.find(searchString);
			if (tmpRange.loc != string::npos)
				modSrcString.replace(tmpRange.loc, tmpRange.len, newString, 0, newString.size());
		} while (tmpRange.loc != string::npos);
		
		//	now find-and-replace IMG_PIXEL
		searchString = string("IMG_PIXEL");
		imgBuffer = nullptr;
		tmpRange = VVRange(0, searchString.size());
		do	{
			tmpRange = VVRange(modSrcString.find(searchString), searchString.size());
			if (tmpRange.loc != string::npos)	{
				vector<string>		varArray(0);
				varArray.reserve(5);
				VVRange			fullFuncRangeToReplace = LexFunctionCall(modSrcString, tmpRange, varArray);
				size_t				varArrayCount = varArray.size();
				if (varArrayCount!=2 && varArrayCount!=3)	{
					throw ISFErr(ISFErrType_ErrorParsingFS, "IMG_PIXEL has wrong number of arguments", *path);
				}
				else	{
					string			newFuncString("");
					string &		samplerName = varArray[0];
					const char *	samplerNameC = samplerName.c_str();
					string &		samplerCoord = varArray[1];
					const char *	samplerCoordC = samplerCoord.c_str();
					imgBuffer = getBufferForKey(samplerName);
					if (imgBuffer==nullptr || imgBuffer->desc.target==VVGLBuffer::Target_2D)	{
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
		tmpRange = VVRange(0, searchString.size());
		do	{
			tmpRange = VVRange(modSrcString.find(searchString), searchString.size());
			if (tmpRange.loc != string::npos)	{
				vector<string>		varArray(0);
				varArray.reserve(5);
				VVRange			fullFuncRangeToReplace = LexFunctionCall(modSrcString, tmpRange, varArray);
				size_t				varArrayCount = varArray.size();
				if (varArrayCount!=2 && varArrayCount!=3)	{
					throw ISFErr(ISFErrType_ErrorParsingFS, "IMG_NORM_PIXEL has wrong number of arguments", *path);
				}
				else	{
					string			newFuncString("");
					string &		samplerName = varArray[0];
					const char *	samplerNameC = samplerName.c_str();
					string &		samplerCoord = varArray[1];
					const char *	samplerCoordC = samplerCoord.c_str();
					imgBuffer = getBufferForKey(samplerName);
					if (imgBuffer==nullptr || imgBuffer->desc.target==VVGLBuffer::Target_2D)	{
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
		tmpRange = VVRange(0, searchString.size());
		do	{
			tmpRange = VVRange(modSrcString.find(searchString), searchString.size());
			if (tmpRange.loc != string::npos)	{
				vector<string>		varArray(0);
				varArray.reserve(5);
				VVRange		fullFuncRangeToReplace = LexFunctionCall(modSrcString, tmpRange, varArray);
				size_t			varArrayCount = varArray.size();
				if (varArrayCount != 1)	{
					throw ISFErr(ISFErrType_ErrorParsingFS, "IMG_SIZE has wrong number of arguments", *path);
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
			newVertShaderSrc.append(FmtString("\t_%s_texCoord = (_%s_flip) ? vec2(((isf_fragCoord.x/_%s_imgSize.x*_%s_imgRect.z)+_%s_imgRect.x), (_%s_imgRect.w-(isf_fragCoord.y/_%s_imgSize.y*_%s_imgRect.w)+_%s_imgRect.y)) : vec2(((isf_fragCoord.x/_%s_imgSize.x*_%s_imgRect.z)+_%s_imgRect.x), (isf_fragCoord.y/_%s_imgSize.y*_%s_imgRect.w)+_%s_imgRect.y);\n",samplerName,samplerName,samplerName,samplerName,samplerName,samplerName,samplerName,samplerName,samplerName,samplerName,samplerName,samplerName,samplerName,samplerName,samplerName));
		}
		//	run through the IMG_THIS_NORM_PIXEL sampler names, populating the varying vec2 variables i declared
		for (const auto & it : imgThisNormPixelSamplerNames)	{
			const char *	samplerName = it.c_str();
			imgBuffer = getBufferForKey(it);
			if (imgBuffer==nullptr || imgBuffer->desc.target==VVGLBuffer::Target_2D)
				newVertShaderSrc.append(FmtString("\t_%s_normTexCoord = (_%s_flip) ? vec2((((isf_FragNormCoord.x*_%s_imgSize.x)/_%s_imgSize.x*_%s_imgRect.z)+_%s_imgRect.x), (_%s_imgRect.w-((isf_FragNormCoord.y*_%s_imgSize.y)/_%s_imgSize.y*_%s_imgRect.w)+_%s_imgRect.y)) : vec2((((isf_FragNormCoord.x*_%s_imgSize.x)/_%s_imgSize.x*_%s_imgRect.z)+_%s_imgRect.x), ((isf_FragNormCoord.y*_%s_imgSize.y)/_%s_imgSize.y*_%s_imgRect.w)+_%s_imgRect.y);\n",samplerName,samplerName,samplerName,samplerName,samplerName,samplerName,samplerName,samplerName,samplerName,samplerName,samplerName,samplerName,samplerName,samplerName,samplerName,samplerName,samplerName,samplerName,samplerName));
			else
				newVertShaderSrc.append(FmtString("\t_%s_normTexCoord = (_%s_flip) ? vec2((((isf_FragNormCoord.x*_%s_imgRect.z)/_%s_imgSize.x*_%s_imgRect.z)+_%s_imgRect.x), (_%s_imgRect.w-((isf_FragNormCoord.y*_%s_imgRect.w)/_%s_imgSize.y*_%s_imgRect.w)+_%s_imgRect.y)) : vec2((((isf_FragNormCoord.x*_%s_imgRect.z)/_%s_imgSize.x*_%s_imgRect.z)+_%s_imgRect.x), ((isf_FragNormCoord.y*_%s_imgRect.w)/_%s_imgSize.y*_%s_imgRect.w)+_%s_imgRect.y);\n",samplerName,samplerName,samplerName,samplerName,samplerName,samplerName,samplerName,samplerName,samplerName,samplerName,samplerName,samplerName,samplerName,samplerName,samplerName,samplerName,samplerName,samplerName,samplerName));
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
	tmpRange = VVRange(0, searchString.size());
	tmpRange.loc = newFragShaderSrc.find(searchString);
	if (tmpRange.loc != string::npos)	{
		tmpRange.len = newFragShaderSrc.find_first_of("\n\r\f", tmpRange.max()) - tmpRange.loc;
		fragVersionString = newFragShaderSrc.substr(tmpRange.loc, tmpRange.len);
		newFragShaderSrc.erase(tmpRange.loc, tmpRange.len);
		newFragShaderSrc.insert(0, fragVersionString);
	}
	
	tmpRange = VVRange(0, searchString.size());
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
	lock_guard<recursive_mutex>		lock(propLock);
	
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
	for (const auto & targetBufIt : persistentBuffers)	{
		targetBufIt->evalTargetSize(inSize, subDict, true, true);
	}
	//	make sure that the temp buffers are sized appropriately
	for (const auto & targetBufIt : tempBuffers)	{
		targetBufIt->evalTargetSize(inSize, subDict, true, true);
	}
	
	//cout << "\t" << __FUNCTION__ << "- FINISHED\n";
}


/*	========================================	*/
#pragma mark --------------------- protected methods


bool ISFDoc::_assembleShaderSource_VarDeclarations(string * outVSString, string * outFSString, GLVersion & inGLVers)	{
	lock_guard<recursive_mutex>		lock(propLock);
	
	if (outVSString==nullptr || outFSString==nullptr)
		return false;
	
	//	we're going to work this by assembling an array of strings, one for each line- have the vector reserve space for enough strings
	vector<string>		vsDeclarations;
	vector<string>		fsDeclarations;
	vsDeclarations.reserve(inputs.size()+imageImports.size()+persistentBuffers.size()+tempBuffers.size()+9);
	fsDeclarations.reserve(vsDeclarations.capacity());
	
	//	frag shader always needs an output, which we're naming gl_FragColor so shaders written against GL 2.1 will work.  we'll use a #define in the shader source to make the shader precompiler change gl_FragColor to isf_FragColor
	switch (inGLVers)	{
	case GLVersion_Unknown:
	case GLVersion_2:
	case GLVersion_ES2:
		break;
	case GLVersion_33:
	case GLVersion_4:
		fsDeclarations.emplace_back("#define gl_FragColor isf_FragColor\n");
		fsDeclarations.emplace_back("out vec4 gl_FragColor;\n");
		break;
	}
	
	//	these are the 9 standard entries
	vsDeclarations.emplace_back("uniform int\t\tPASSINDEX;\n");
	fsDeclarations.emplace_back("uniform int\t\tPASSINDEX;\n");
	vsDeclarations.emplace_back("uniform vec2\t\tRENDERSIZE;\n");
	fsDeclarations.emplace_back("uniform vec2\t\tRENDERSIZE;\n");
	switch (inGLVers)	{
	case GLVersion_Unknown:
	case GLVersion_2:
	case GLVersion_ES2:
		vsDeclarations.emplace_back("varying vec2\t\tisf_FragNormCoord;\n");
		fsDeclarations.emplace_back("varying vec2\t\tisf_FragNormCoord;\n");
		//vsDeclarations.emplace_back("varying vec3\t\tisf_VertNorm;\n");
		//fsDeclarations.emplace_back("varying vec3\t\tisf_VertNorm;\n");
		//vsDeclarations.emplace_back("varying vec3\t\tisf_VertPos;\n");
		//fsDeclarations.emplace_back("varying vec3\t\tisf_VertPos;\n");
		break;
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
	vsDeclarations.emplace_back("uniform float\t\tTIME;\n");
	fsDeclarations.emplace_back("uniform float\t\tTIME;\n");
	vsDeclarations.emplace_back("uniform float\t\tTIMEDELTA;\n");
	fsDeclarations.emplace_back("uniform float\t\tTIMEDELTA;\n");
	vsDeclarations.emplace_back("uniform vec4\t\tDATE;\n");
	fsDeclarations.emplace_back("uniform vec4\t\tDATE;\n");
	vsDeclarations.emplace_back("uniform int\t\tFRAMEINDEX;\n");
	fsDeclarations.emplace_back("uniform int\t\tFRAMEINDEX;\n");
	
	//	this block will be used to add declarations for a provided ISFAttr
	auto	attribDecBlock = [&](const ISFAttrRef & inRef)	{
		const string &		tmpName = inRef->getName();
		const char *		nameCStr = tmpName.c_str();
		switch (inRef->getType())	{
		case ISFValType_None:
			break;
		case ISFValType_Event:
		case ISFValType_Bool:
			vsDeclarations.emplace_back(FmtString("uniform bool\t\t%s;\n", nameCStr));
			fsDeclarations.emplace_back(FmtString("uniform bool\t\t%s;\n", nameCStr));
			break;
		case ISFValType_Long:
			vsDeclarations.emplace_back(FmtString("uniform int\t\t%s;\n", nameCStr));
			fsDeclarations.emplace_back(FmtString("uniform int\t\t%s;\n", nameCStr));
			break;
		case ISFValType_Float:
			vsDeclarations.emplace_back(FmtString("uniform float\t\t%s;\n", nameCStr));
			fsDeclarations.emplace_back(FmtString("uniform float\t\t%s;\n", nameCStr));
			break;
		case ISFValType_Point2D:
			vsDeclarations.emplace_back(FmtString("uniform vec2\t\t%s;\n", nameCStr));
			fsDeclarations.emplace_back(FmtString("uniform vec2\t\t%s;\n", nameCStr));
			break;
		case ISFValType_Color:
			vsDeclarations.emplace_back(FmtString("uniform vec4\t\t%s;\n", nameCStr));
			fsDeclarations.emplace_back(FmtString("uniform vec4\t\t%s;\n", nameCStr));
			break;
		case ISFValType_Cube:
			//	make a sampler for the cubemap texture
			vsDeclarations.emplace_back(FmtString("uniform samplerCube\t\t%s;\n", nameCStr));
			fsDeclarations.emplace_back(FmtString("uniform samplerCube\t\t%s;\n", nameCStr));
			//	just pass in the imgSize
			vsDeclarations.emplace_back(FmtString("uniform vec2\t\t_%s_imgSize;\n", nameCStr));
			fsDeclarations.emplace_back(FmtString("uniform vec2\t\t_%s_imgSize;\n", nameCStr));
			break;
		case ISFValType_Image:
		case ISFValType_Audio:
		case ISFValType_AudioFFT:
			{
				ISFVal			attribVal = inRef->getCurrentVal();
				VVGLBufferRef	attribBuffer = attribVal.getImageBuffer();
				if (attribBuffer==nullptr || attribBuffer->desc.target==VVGLBuffer::Target_2D)	{
					vsDeclarations.emplace_back(FmtString("uniform sampler2D\t\t%s;\n", nameCStr));
					fsDeclarations.emplace_back(FmtString("uniform sampler2D\t\t%s;\n", nameCStr));
				}
				else	{
					vsDeclarations.emplace_back(FmtString("uniform sampler2DRect\t\t%s;\n", nameCStr));
					fsDeclarations.emplace_back(FmtString("uniform sampler2DRect\t\t%s;\n", nameCStr));
				}
				//	a vec4 describing the image rect IN NATIVE GL TEXTURE COORDS (2D is normalized, RECT is not)
				vsDeclarations.emplace_back(FmtString("uniform vec4\t\t_%s_imgRect;\n", nameCStr));
				fsDeclarations.emplace_back(FmtString("uniform vec4\t\t_%s_imgRect;\n", nameCStr));
				//	a vec2 describing the size in pixels of the image
				vsDeclarations.emplace_back(FmtString("uniform vec2\t\t_%s_imgSize;\n", nameCStr));
				fsDeclarations.emplace_back(FmtString("uniform vec2\t\t_%s_imgSize;\n", nameCStr));
				//	a bool describing whether the image in the texture should be flipped vertically
				vsDeclarations.emplace_back(FmtString("uniform bool\t\t_%s_flip;\n", nameCStr));
				fsDeclarations.emplace_back(FmtString("uniform bool\t\t_%s_flip;\n", nameCStr));
				break;
			}
		}
	};
	
	//	this block will be used to add declarations for a provided ISFPassTarget
	auto		targetBufferBlock = [&](const ISFPassTargetRef & inRef)	{
		const string &		tmpName = inRef->getName();
		const char *		nameCStr = tmpName.c_str();
		VVGLBufferRef		bufferRef = inRef->getBuffer();
		if (bufferRef==nullptr || bufferRef->desc.target==VVGLBuffer::Target_2D)	{
			vsDeclarations.emplace_back(FmtString("uniform sampler2D\t\t%s;\n", nameCStr));
			fsDeclarations.emplace_back(FmtString("uniform sampler2D\t\t%s;\n", nameCStr));
		}
		else	{
			vsDeclarations.emplace_back(FmtString("uniform sampler2DRect\t\t%s;\n", nameCStr));
			fsDeclarations.emplace_back(FmtString("uniform sampler2DRect\t\t%s;\n", nameCStr));
		}
		vsDeclarations.emplace_back(FmtString("uniform vec4\t\t_%s_imgRect;\n", nameCStr));
		fsDeclarations.emplace_back(FmtString("uniform vec4\t\t_%s_imgRect;\n", nameCStr));
		vsDeclarations.emplace_back(FmtString("uniform vec2\t\t_%s_imgSize;\n", nameCStr));
		fsDeclarations.emplace_back(FmtString("uniform vec2\t\t_%s_imgSize;\n", nameCStr));
		vsDeclarations.emplace_back(FmtString("uniform bool\t\t_%s_flip;\n", nameCStr));
		fsDeclarations.emplace_back(FmtString("uniform bool\t\t_%s_flip;\n", nameCStr));
	};
	
	//	add the variables for the various inputs
	for (const auto & attrIt : inputs)
		attribDecBlock(attrIt);
	//	add the variables for the imported buffers
	for (const auto & attrIt : imageImports)
		attribDecBlock(attrIt);
	//	add the variables for the persistent buffers
	for (const auto & tgBufIt : persistentBuffers)
		targetBufferBlock(tgBufIt);
	//	add the variables for the temp buffers
	for (const auto & tgBufIt : tempBuffers)
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
	outVSString->reserve(reserveSize);
	outFSString->reserve(reserveSize);
	//	now copy the individual declarations to the output string
	for (const auto & stringIt : vsDeclarations)	{
		outVSString->append(stringIt);
	}
	for (const auto & stringIt : fsDeclarations)	{
		outFSString->append(stringIt);
	}
	
	return true;
}
bool ISFDoc::_assembleSubstitutionMap(map<string,double*> * outMap)	{
	//cout << __PRETTY_FUNCTION__ << endl;
	lock_guard<recursive_mutex>		lock(propLock);
	
	if (outMap == nullptr)
		return false;
	
	map<string,double*> &	outMapRef = *outMap;
	
	for (const auto & attribRefIt : inputs)	{
		
		const string &		tmpName = attribRefIt->getName();
		
		switch (attribRefIt->getType())	{
		case ISFValType_None:
			break;
		case ISFValType_Event:
		case ISFValType_Bool:
		case ISFValType_Long:
		case ISFValType_Float:
			//outMapRef[tmpName] = attribRefIt->getCurrentVal().getDoubleVal();
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
	os << "doc is \"" << *(n.name) << "\"" << endl;
	os << "\tdoc is a " << ISFFileTypeString(n.getType()) << endl;
	
	string				tmpDescription = n.getDescription();
	os << "\tdesription: " << tmpDescription << endl;
	
	string				tmpCredit = n.getCredit();
	os << "\tcredit: " << tmpCredit << endl;
	
	vector<ISFAttrRef> &		tmpImports = const_cast<ISFDoc*>(&n)->getImageImports();
	if (tmpImports.size() > 0)	{
		os << "\tdoc has " << tmpImports.size() << " imported images\n";
		for (auto it=tmpImports.begin(); it!=tmpImports.end(); ++it)	{
			os << "\t\t" << (*it)->getName() << ": " << ISFValTypeString((*it)->getType());
			if ((*it)->getIsFilterInputImage())
				os << " <- is filter input image";
			os << endl;
		}
	}

	const vector<ISFPassTargetRef> 	tmpPersistent = const_cast<ISFDoc*>(&n)->getPersistentBuffers();
	if (tmpPersistent.size() > 0)	{
		os << "\tdoc has " << tmpPersistent.size() << " persistent buffers\n";
		for (const auto & it : tmpPersistent)	{
			const std::string		tmpWidth = it->getTargetWidthString();
			const std::string		tmpHeight = it->getTargetHeightString();
			os << "\t\t" << it->getName();
			if (tmpWidth.size() > 0)
				os << ", " << tmpWidth;
			if (tmpHeight.size() > 0)
				os << ", " << tmpHeight;
			if (it->getFloatFlag())
				os << ", is a FLOAT tex";
			os << endl;
		}
	}
	
	const vector<ISFPassTargetRef> 	tmpTemp = const_cast<ISFDoc*>(&n)->getTempBuffers();
	if (tmpTemp.size() > 0)	{
		os << "\tdoc has " << tmpTemp.size() << " temp buffers\n";
		for (const auto & it : tmpTemp)	{
			const std::string		tmpWidth = it->getTargetWidthString();
			const std::string		tmpHeight = it->getTargetHeightString();
			os << "\t\t" << it->getName();
			if (tmpWidth.size() > 0)
				os << ", " << tmpWidth;
			if (tmpHeight.size() > 0)
				os << ", " << tmpHeight;
			if (it->getFloatFlag())
				os << ", is a FLOAT tex";
			os << endl;
		}
	}
	
	vector<string> &		tmpPasses = const_cast<ISFDoc*>(&n)->getRenderPasses();
	if (tmpPasses.size() > 0)	{
		os << "\tdoc has " << tmpPasses.size() << " render passes\n";
		for (auto it=tmpPasses.begin(); it!=tmpPasses.end(); ++it)
			os << "\t\tpass name: " << *it << endl;
	}

	vector<ISFAttrRef> &		tmpInputs = const_cast<ISFDoc*>(&n)->getInputs();
	if (tmpInputs.size() > 0)	{
		os << "\tdoc has " << tmpInputs.size() << " inputs\n";
		for (const auto & it : tmpInputs)	{
			os << "\t\tinput \"" << it->getName() << "\" is a " << ISFValTypeString(it->getType()) << endl;
			ISFVal&		currentVal = it->getCurrentVal();
			ISFVal&		minVal = it->getMinVal();
			ISFVal&		maxVal = it->getMaxVal();
			ISFVal&		defaultVal = it->getDefaultVal();
			ISFVal&		identityVal = it->getIdentityVal();
			vector<string>&		labels = it->getLabelArray();
			vector<int32_t>&	vals = it->getValArray();
			bool			needsAComma = false;
			bool			needsANewline = false;
			if (currentVal.getType()!=ISFValType_None || minVal.getType()!=ISFValType_None || maxVal.getType()!=ISFValType_None || defaultVal.getType()!=ISFValType_None || identityVal.getType()!=ISFValType_None || labels.size()>0 || vals.size()>0)	{
				os << "\t\t\t";
				needsANewline = true;
			}
			if (currentVal.getType() != ISFValType_None)	{
				if (needsAComma) os << ", ";
				else needsAComma = true;
				os << "current val is " <<  currentVal.getValString();
			}
			if (minVal.getType() != ISFValType_None)	{
				if (needsAComma) os << ", ";
				else needsAComma = true;
				os << "min val is " <<  minVal.getValString();
			}
			if (maxVal.getType() != ISFValType_None)	{
				if (needsAComma) os << ", ";
				else needsAComma = true;
				os << "max val is " <<  maxVal.getValString();
			}
			if (defaultVal.getType() != ISFValType_None)	{
				if (needsAComma) os << ", ";
				else needsAComma = true;
				os << "default val is " <<  defaultVal.getValString();
			}
			if (identityVal.getType() != ISFValType_None)	{
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
