#include "ShadertoyConverter.h"
#include "ui_ShadertoyConverter.h"

#include <QUrl>
#include <QNetworkRequest>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QMessageBox>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDir>

#include "LoadingWindow.h"
#include "StringUtilities.h"
#include "VVGL.hpp"




ShadertoyConverter::ShadertoyConverter(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::ShadertoyConverter)
{
	qDebug() << __PRETTY_FUNCTION__;
	//setWindowModality(Qt::ApplicationModal);
	setWindowModality(Qt::WindowModal);
	ui->setupUi(this);
}

ShadertoyConverter::~ShadertoyConverter()
{
	qDebug() << __PRETTY_FUNCTION__;
	delete ui;
}




void ShadertoyConverter::cancelClicked()	{
	qDebug() << __PRETTY_FUNCTION__;
	done(1);
}
void ShadertoyConverter::okClicked()	{
	qDebug() << __PRETTY_FUNCTION__;
	
	//QString			rawURLString = ui->lineEdit->text();
	//QString			rawURLString("https://www.shadertoy.com/view/ldtczX");
	//QString			rawURLString("https://www.shadertoy.com/view/4lcfDB");
	//QString			rawURLString("https://www.shadertoy.com/view/lljfRD");
	//QString			rawURLString("https://www.shadertoy.com/view/MlKBWD");
	//QString			rawURLString("https://www.shadertoy.com/view/XtGBWW");
	//QString			rawURLString("https://www.shadertoy.com/view/4stXDX");
	//QString			rawURLString("https://www.shadertoy.com/view/4tdBDS");
	//QString			rawURLString("https://www.shadertoy.com/view/MtyBWh");	//	cube texture upload still not implemented for Qt
	//QString			rawURLString("https://www.shadertoy.com/view/4dfGzs");
	QString			rawURLString("https://www.shadertoy.com/view/4dcGW2");
	
	if (rawURLString.isNull() || rawURLString.length()<1)	{
		QMessageBox::warning(GetLoadingWindow(), "", QString("Error: cannot parse URL \"%1\"").arg(rawURLString), QMessageBox::Ok);
		done(1);
		return;
	}
	
	QString			shaderIDString = QString::fromStdString( VVGL::LastPathComponent(rawURLString.toStdString()) );
	if (shaderIDString.isNull() || shaderIDString.length()<1)	{
		QMessageBox::warning(GetLoadingWindow(), "", QString("Error: cannot parse URL \"%1\"").arg(rawURLString), QMessageBox::Ok);
		done(2);
		return;
	}
	
	//	download the information from shadertoy, parse it
	QString			sourceBlobURL = QString("https://www.shadertoy.com/shadertoy");
	QJsonDocument	parsedDownload;
	QJsonObject		parsedDict;
	{
		QUrl				url(sourceBlobURL);
		QNetworkRequest		req(url);
		req.setRawHeader("Accept-Encoding", "gzip,deflate" );
		req.setRawHeader("Accept-Language", QString("en-US,en;q=0.8").toUtf8());
		//req.setRawHeader("User-Agent", QString("Hello Kitty Internet Thingy").toUtf8());
		req.setRawHeader("Content-Type", QString("application/x-www-form-urlencoded").toUtf8());
		req.setRawHeader("Referer", QString("https://www.shadertoy.com/view/%1").arg(shaderIDString).toUtf8());
		
		QNetworkAccessManager	nm;
		QString				postDataString = QString("s=%1").arg(QString(QUrl::toPercentEncoding( QString("{\"shaders\":[\"%1\"]}").arg(shaderIDString) )));
		QNetworkReply		*reply = nm.post(req, postDataString.toUtf8());
		if (reply == nullptr)	{
			QMessageBox::warning(GetLoadingWindow(), "", QString("Error: can't construct request for URL \"%1\"").arg(rawURLString), QMessageBox::Ok);
			done(3);
			return;
		}
		
		QEventLoop			tmpLoop;
		connect(reply, SIGNAL(finished()), &tmpLoop, SLOT(quit()));
		tmpLoop.exec();
		
		if (reply->size() < 1)	{
			QMessageBox::warning(GetLoadingWindow(), "", QString("Error: download from URL \"%1\" failed").arg(rawURLString), QMessageBox::Ok);
			done(4);
			return;
		}
		
		parsedDownload = QJsonDocument::fromJson( reply->readAll() );
		QJsonArray		tmpArray = parsedDownload.array();
		if (tmpArray.size() < 1)	{
			QMessageBox::warning(GetLoadingWindow(), "", QString("Error: download from URL \"%1\" is of wrong type").arg(rawURLString), QMessageBox::Ok);
			done(5);
			return;
		}
		QJsonValue		tmpValue = tmpArray.at(0);
		if (!tmpValue.isObject())	{
			QMessageBox::warning(GetLoadingWindow(), "", QString("Error: download from URL \"%1\" is of wrong type").arg(rawURLString), QMessageBox::Ok);
			done(6);
			return;
		}
		parsedDict = tmpValue.toObject();
		
		reply->deleteLater();
	}
	//qDebug() << "\tparsedDict is " << parsedDict;
	
	
	//	get the array of objects describing render passes
	QJsonArray		renderpassArray = parsedDict.value("renderpass").toArray();
	if (renderpassArray.count() < 1)	{
		QMessageBox::warning(GetLoadingWindow(), "", QString("Error: download from URL \"%1\" is of wrong type").arg(rawURLString), QMessageBox::Ok);
		done(7);
		return;
	}
	
	//	the renderpass array from shadertoy has a weird order- the first item is the last step.  the second and subsequent items are the first and subsequent steps.  sort this.
	QJsonArray		sortedRenderpassArray;
	if (renderpassArray.count() == 1)
		sortedRenderpassArray.append(renderpassArray.at(0));
	else	{
		for (int i=1; i<renderpassArray.count(); ++i)
			sortedRenderpassArray.append(renderpassArray.at(i));
		sortedRenderpassArray.append(renderpassArray.at(0));
		
		//qDebug() << "\torig sortedRenderpassArray is " << sortedRenderpassArray;
		
		//	run through the sorted renderpass array, removing any passes that render to audio
		auto			renderpassIt = sortedRenderpassArray.begin();
		while (renderpassIt!=sortedRenderpassArray.end())	{
			QJsonObject		renderpassDict = renderpassIt->toObject();
			QJsonArray		outputs = renderpassDict.value(QString("outputs")).toArray();
			if (outputs.count() > 0)	{
				QJsonObject		output = outputs.at(0).toObject();
				QJsonValue		outputIDVal = output.value("id");
				//	if the id of this pass's output is 38, it's an audio output
				if (outputIDVal.isDouble() && outputIDVal.toInt()==38)	{
					renderpassIt = sortedRenderpassArray.erase(renderpassIt);
					continue;
				}
			}
			++renderpassIt;
		}
	}
	//qDebug() << "\tsortedRenderpassArray is " << sortedRenderpassArray;
	
	//	after the conversion is finished, i'll want to show an alert if the shader had either a mouse or a keyboard input informing the user that further conversion may be required
	bool				hasMouseOrKeyboardInput = false;
	//	i'm going to need an array with all the audio inputs so i can find-and-replace them later with more appropriate names
	QStringList			musicInputNames;
	//	the first video-type channel will be renamed to "inputImage" under the assumption that this is a filter, so we need to record that...
	QString				inputImageChannelName;
	//	assemble a dict of supplemental entries
	QJsonObject			suppEntries;
	QJsonObject			infoDict = parsedDict.value("info").toObject();
	QString				shadertoyUsername = infoDict.value("username").toString();
	QString				shadertoyDescription = infoDict.value("description").toString();
	if (shadertoyDescription.isNull())	{
		if (!shadertoyUsername.isNull())
			shadertoyDescription = QString("Automatically converted from %1 by %2").arg(rawURLString).arg(shadertoyUsername);
		else
			shadertoyDescription = QString("Automatically converted from %1").arg(rawURLString);
	}
	else	{
		if (!shadertoyUsername.isNull())
			shadertoyDescription = QString("Automatically converted from %1 by %2.  %3").arg(rawURLString).arg(shadertoyUsername).arg(shadertoyDescription);
		else
			shadertoyDescription = QString("Automatically converted from %1.  %2").arg(rawURLString).arg(shadertoyDescription);
	}
	suppEntries.insert("DESCRIPTION", QJsonValue(shadertoyDescription));
	
	QJsonArray			categories;
	categories.append(QJsonValue(QString("Automatically Converted")));
	categories.append(QJsonValue(QString("Shadertoy")));
	for (const QJsonValue & tmpVal : infoDict.value("tags").toArray())	{
		if (tmpVal.isString())
			categories.append(tmpVal);
	}
	suppEntries.insert("CATEGORIES", QJsonValue(categories));
	
	QString				shadertoyName = infoDict.value("name").toString();
	if (!shadertoyName.isNull() && shadertoyName.length()<1)
		shadertoyName = QString();
	else	{
		shadertoyName = shadertoyName.replace(" ", "_");
		shadertoyName = shadertoyName.replace(",", "_");
		shadertoyName = shadertoyName.replace(":", "-");
		shadertoyName = shadertoyName.replace("\t", "_");
		shadertoyName = shadertoyName.replace("_-_", "-");
		shadertoyName = shadertoyName.replace("_-", "-");
		shadertoyName = shadertoyName.replace("-_", "-");
	}
	
	//	make a dict containing replacement keys- in shadertoy, "iChannel0" may vary from pass to pass, while ISF needs consistent names spanning passes
	QJsonArray			passVarNameSwapDicts;
	//	if there's more than one renderpass, make the 'passes' array (stores dicts describing passes), and add it to the supplemental entries dict
	QJsonArray			passes;
	
	//	create arrays for inputs and imports, add them to the entries dict
	QJsonArray			suppInputs;
	QJsonObject			suppImports;
	//	create a block that checks to see if a given name is being used by an input
	auto IsThisInputNameUnique = [&](QString baseName)	{
		bool				returnMe = true;
		for (const QJsonValue & suppInputValue : suppInputs)	{
			if (suppInputValue.toObject().value("NAME").toString() == baseName)	{
				returnMe = false;
				break;
			}
		}
		return returnMe;
	};
	auto UniqueNameForInput = [&](QString & baseName)	{
		if (IsThisInputNameUnique(baseName))
			return QString(baseName);
		QString			returnMe;
		int				tmpInt = 2;
		do	{
			QString			tmpString = QString("%1_%2").arg(baseName).arg(tmpInt);
			if (IsThisInputNameUnique(tmpString))
				returnMe = tmpString;
			++tmpInt;
		} while (returnMe.isNull());
		return returnMe;
	};
	//	create a block that checks to see if a given name is being used by an import
	auto IsThisImportNameUnique = [&](QString & baseName)	{
		return !suppImports.keys().contains(baseName);
		/*
		bool			returnMe = true;
		for (const QJsonValue & suppImportValue : suppImports)	{
			if (suppImportValue.toObject().value("NAME").toString() == baseName)	{
				returnMe = false;
				break;
			}
		}
		return returnMe;
		*/
	};
	auto UniqueNameForImport = [&](QString & baseName)	{
		if (IsThisImportNameUnique(baseName))
			return QString(baseName);
		QString			returnMe;
		int				tmpInt = 2;
		do	{
			QString			tmpString = QString("%1_%2").arg(baseName).arg(tmpInt);
			if (IsThisImportNameUnique(tmpString))
				returnMe = tmpString;
			++tmpInt;
		} while (returnMe.isNull());
		return returnMe;
	};
	
	//	render passes are identified by 'id', which is now a string.
	QStringList				renderOutputNames;
	//	after we sort the array of render passes, we run through each pass and store the name of the output id, so we can convert from render pass index to id
	for (const QJsonValue & renderpassDictValue : sortedRenderpassArray)	{
		QJsonObject				renderpassDict = renderpassDictValue.toObject();
		//	get the array of OUTPUTS for this pass- if there's more than one, bail with error
		QJsonArray				renderpassOutputs = renderpassDict.value("outputs").toArray();
		if (renderpassOutputs.size()>1)	{
			qDebug() << "\tERR: renderpassOutputs array is of unexpected length, bailing";
			QMessageBox::warning(GetLoadingWindow(), "", QString("Error: couldn't convert URL \"%1\"").arg(rawURLString), QMessageBox::Ok);
			done(8);
			return;
		}
		//	if there aren't any render passes, assume we're fine
		if (renderpassOutputs.size() == 0)
			continue;
		//	get what i'm presently assuming is the main output
		QJsonObject				outputDict = renderpassOutputs.at(0).toObject();
		if (outputDict.size()<1)	{
			qDebug() << "\tERR: renderpass output dict is of unexpected type, bailing";
			QMessageBox::warning(GetLoadingWindow(), "", QString("Error: couldn't convert URL \"%1\"").arg(rawURLString), QMessageBox::Ok);
			done(9);
			return;
		}
		//	get the id of the main output- '4dfGRr' is the last step (draw to screen)
		QString					outputIDString = outputDict.value("id").toString();
		if (outputIDString.isNull() || outputIDString.size()<1)	{
			qDebug() << "\tERR: no output id found, bailing";
			QMessageBox::warning(GetLoadingWindow(), "", QString("Error: couldn't convert URL \"%1\"").arg(rawURLString), QMessageBox::Ok);
			done(10);
			return;
		}
		//	add the output string to the array
		if (outputIDString != QString("4dfGRr"))
			renderOutputNames.append(outputIDString);
	}
	//qDebug() << "\trenderOutputNames are " << renderOutputNames;
	//	create a block that converts a buffer id string to the name of the buffer
	auto NameForBufferIDString = [&](QString & bufferIDString)	{
		QString			returnMe;
		if (bufferIDString.isNull() || bufferIDString.size()<1)
			return returnMe;
		bool			found = false;
		int				tmpInt = 0;
		for (const QString & tmpString : renderOutputNames)	{
			if (tmpString == bufferIDString)	{
				found = true;
				break;
			}
			++tmpInt;
		}
		if (found)	{
			switch (tmpInt)	{
			case 0:
				return QString("BufferA");
			case 1:
				return QString("BufferB");
			case 2:
				return QString("BufferC");
			case 3:
				return QString("BufferD");
			}
		}
		return returnMe;
	};
	/*
	auto NameForRenderPassIndex = [&](int tmpIndex)	{
		QString			returnMe;
		if (tmpIndex<0 || tmpIndex>=renderOutputNames.size())
			return returnMe;
		switch (tmpIndex)	{
		case 0:
			return QString("BufferA");
		case 1:
			return QString("BufferB");
		case 2:
			return QString("BufferC");
		case 3:
			return QString("BufferD");
		}
		return returnMe;
	};
	*/
	
	
	//	figure out if the source code uses the mouse var
	bool			usesMouseVar = false;
	//	create an array with the source code for each of the passes- we'll need this later when we're find-and-replacing source
	QStringList			sortedShaderSourceArray;
	for (const QJsonValue & renderpassDictValue : sortedRenderpassArray)	{
		if (!renderpassDictValue.isObject())
			continue;
		QJsonObject				renderpassDict = renderpassDictValue.toObject();
		QString					rawShaderSource = renderpassDict.value("code").toString();
		if (rawShaderSource.isNull() || rawShaderSource.size()<1)	{
			qDebug() << "\tERR: couldnt locate raw shader source in parsed reply";
			QMessageBox::warning(GetLoadingWindow(), "", QString("Error: couldn't convert URL \"%1\"").arg(rawURLString), QMessageBox::Ok);
			done(11);
			return;
		}
		else	{
			//	check to see if this source code is using a mouse input
			usesMouseVar |= rawShaderSource.contains(QRegularExpression("([^\\w_])(iMouse)([^\\w_])"));
			//	append the raw shader source to the array
			sortedShaderSourceArray.append(rawShaderSource);
		}
	}
	
	
	//	if i'm using the mouse var, make a 2d input for it, add it to the suppInputs array
	if (usesMouseVar)	{
		QJsonObject		tmpInput;
		tmpInput.insert("NAME", QJsonValue(QString("iMouse")));
		tmpInput.insert("TYPE", QJsonValue(QString("point2D")));
		
		suppInputs.append(tmpInput);
	}
	
	
	//	run through the sorted array of render pass dicts- parse the inputs and outputs
	int				passIndex = 0;
	for (const QJsonValue & renderpassDictValue : sortedRenderpassArray)	{
		//qDebug() << "\t******* processing render pass " << passIndex << "...";
		QJsonObject		renderpassDict = renderpassDictValue.toObject();
		
		//	get the array of OUTPUTS for thsi pass- if there's more than one, bail with error
		QJsonArray		renderpassOutputs = renderpassDict.value("outputs").toArray();
		if (renderpassOutputs.size() > 1)	{
			qDebug() << "\tERR: renderpass outputs array B is of unexpected length, bailing";
			QMessageBox::warning(GetLoadingWindow(), "", QString("Error: couldn't convert URL \"%1\"").arg(rawURLString), QMessageBox::Ok);
			done(12);
			return;
		}
		QString			outputIDString;
		//	if there aren't any outputs, assume that this is the final output
		if (renderpassOutputs.size() == 0)
			outputIDString = QString("4dfGRr");
		//	else there are outputs- we have to get the output id string
		else	{
			//	get what i'm presently assuming is the main output
			QJsonValue			outputDictVal = renderpassOutputs.at(0);
			if (outputDictVal.isNull() || !outputDictVal.isObject())	{
				qDebug() << "\tERR: renderpass output dict B is of unexpected type, bailing";
				QMessageBox::warning(GetLoadingWindow(), "", QString("Error: couldn't convert URL \"%1\"").arg(rawURLString), QMessageBox::Ok);
				done(13);
				return;
			}
			QJsonObject			outputDict = outputDictVal.toObject();
			//	get the id of the main output- '4dfGRr' is the last step (draw to screen)
			QJsonValue			outputIDStringVal = outputDict.value("id");
			if (outputIDStringVal.isNull() || !outputIDStringVal.isString())	{
				qDebug() << "\tERR: no output id B found, bailing";
				QMessageBox::warning(GetLoadingWindow(), "", QString("Error: couldn't convert URL \"%1\"").arg(rawURLString), QMessageBox::Ok);
				done(14);
				return;
			}
			outputIDString = outputIDStringVal.toString();
		}
		
		
		//	if there's a 'passes' array, then i need to make a pass dict and add it to the array
		QJsonObject			newPassDict;
		if (sortedRenderpassArray.size() > 1)	{
			
			//	'4dfGRr' is the last step (draw to screen)
			if (outputIDString == QString("4dfGRr"))	{
			}
			//	else the output id string isn't '4dfGRr'- we're outputting to a buffer
			else	{
				QString			targetBufferName = NameForBufferIDString(outputIDString);
				if (!targetBufferName.isNull() && targetBufferName.size()>0)	{
					newPassDict.insert("TARGET", QJsonValue(targetBufferName));
					newPassDict.insert("PERSISTENT", QJsonValue(true));
					newPassDict.insert("FLOAT", QJsonValue(true));
				}
			}
			
			passes.append( QJsonValue(newPassDict) );
		}
		
		
		//	make a dict that we'll use to store the names we need to swap
		QJsonObject			passVarNameSwapDict;
		
		
		//	get the array of INPUTS for this pass
		QJsonArray			renderpassInputs = renderpassDict.value("inputs").toArray();
		//	run through the inputs
		for (const QJsonValue & renderpassInputVal : renderpassInputs)	{
			QJsonObject			renderpassInput = renderpassInputVal.toObject();
			//qDebug() << "\t*******";
			//qDebug() << "\trenderpassInput is " << renderpassInput;
			int					channelNum = renderpassInput.value("channel").toInt();
			QString				channelType = renderpassInput.value("type").toString();
			QString				channelSrc = QString::fromStdString(VVGL::LastPathComponent( renderpassInput.value("filepath").toString().toStdString() ));
			QString				channelName = QString("iChannel%1").arg(channelNum);
			
			//	make sure the channel name is unique (a prior pass may have added an input or something with this name)
			QString				uniqueChannelName;
			//qDebug() << "\tprocessing channelNum " << channelNum << ", channelType is " << channelType << ", channelName is " << channelName << ", channelSrc is " << channelSrc;
			
			if (channelType == QString("texture"))	{
				//	texture are IMPORTs, so check the supplemental imports array for a dict using this name
				uniqueChannelName = UniqueNameForImport(channelName);
				//	if the unique channel name doesn't match the channel name, we're going to have to replace stuff when we convert the shader source
				if (uniqueChannelName != channelName)
					passVarNameSwapDict.insert(channelName, QJsonValue(uniqueChannelName));
				
				QJsonObject			channelDict;
				channelDict.insert("NAME", QJsonValue(uniqueChannelName));
				channelDict.insert("PATH", QJsonValue(channelSrc));
				suppImports.insert(uniqueChannelName, QJsonValue(channelDict));
			}
			else if (channelType==QString("music") || channelType==QString("mic") || channelType==QString("musicstream"))	{
				//	texture are IMPORTs, so check the supplemental imports array for a dict using this name
				uniqueChannelName = UniqueNameForInput(channelName);
				//	if the unique channel name doesn't match the channel name, we're going to have to replace stuff when we convert the shader source
				if (uniqueChannelName != channelName)
					passVarNameSwapDict.insert(channelName, QJsonValue(uniqueChannelName));
				
				musicInputNames.append(channelName);
				QJsonObject			channelDict;
				channelDict.insert("NAME", uniqueChannelName);
				channelDict.insert("TYPE", QJsonValue(QString("audio")));
				suppInputs.append(QJsonValue(channelDict));
			}
			else if (channelType == QString("cubemap"))	{
				//	texture are IMPORTs, so check the supplemental imports array for a dict using this name
				uniqueChannelName = UniqueNameForImport(channelName);
				//	if the unique channel name doesn't match the channel name, we're going to have to replace stuff when we convert the shader source
				if (uniqueChannelName != channelName)
					passVarNameSwapDict.insert(channelName, QJsonValue(uniqueChannelName));
				
				QJsonObject			channelDict;
				channelDict.insert("NAME", uniqueChannelName);
				//	cubemaps only list one path even though there are six.  so we have to parse the string, then synthesize all the path names from that.
				QJsonArray			pathArray;
				QRegularExpression	regex("([\\w]+)(\\.((jpg)|(png)))");
				for (int i=0; i<6; ++i)	{
					QString				modString = channelSrc;
					if (i != 0)
						modString.replace(regex, QString("\\1_%1\\2").arg(i));
					//if (i == 0)
					//	modString = channelSrc;
					//else
					//	modString = channelSrc.replace(regex, QString("\\1_%1\\2").arg(i));
					if (modString.isNull())	{
						qDebug() << "\tERR: couldnt calculate cubemap file name, src string was " << channelSrc;
						break;
					}
					pathArray.append(QJsonValue(modString));
				}
				channelDict.insert("PATH", QJsonValue(pathArray));
				channelDict.insert("TYPE", QJsonValue(QString("cube")));
				suppImports.insert(uniqueChannelName, QJsonValue(channelDict));
			}
			else if (channelType == QString("video"))	{
				//	texture are IMPORTs, so check the supplemental imports array for a dict using this name
				uniqueChannelName = UniqueNameForInput(channelName);
				//	if the unique channel name doesn't match the channel name, we're going to have to replace stuff when we convert the shader source
				if (uniqueChannelName != channelName)
					passVarNameSwapDict.insert(channelName, QJsonValue(uniqueChannelName));
				
				QJsonObject			channelDict;
				channelDict.insert("TYPE", QJsonValue(QString("image")));
				channelDict.insert("NAME", QJsonValue(uniqueChannelName));
				suppInputs.append(QJsonValue(channelDict));
				//	if the inputImageChannelName is still nil, then this video-type channel is going to be turned into the default image input for a video filter...
				if (inputImageChannelName.isNull())
					inputImageChannelName = channelName;
			}
			//	buffers are the results of prior rendering passes
			else if (channelType == QString("buffer"))	{
				//	results of prior rendering passes have unique names (A, B, C, or D)
				QString			tmpString = renderpassInput.value("id").toString();
				QString			bufferName = NameForBufferIDString(tmpString);
				//	since we have a static name, we know we need to replace stuff
				passVarNameSwapDict.insert(channelName, QJsonValue(bufferName));
			}
			//	if i found a keyboard-type input, i want to present the user with an alert informing them that the conversion hasn't been complete
			else if (channelType == QString("keyboard"))	{
				hasMouseOrKeyboardInput = true;
			}
		}
		
		//	append the pass var name swap dict we made earlier to the array of pass var name swap dicts
		passVarNameSwapDicts.append(QJsonValue(passVarNameSwapDict));
		
		
		++passIndex;
	}
	
	
	//	now that we're done populating the 'passes' array, add it to the supplemental entries object
	if (sortedRenderpassArray.count() > 1)	{
		suppEntries.insert("PASSES", QJsonValue(passes));
	}
	suppEntries.insert("INPUTS", QJsonValue(suppInputs));
	suppEntries.insert("IMPORTED", QJsonValue(suppImports));
	
	
	/*
	qDebug() << "****************";
	qDebug() << "\tsortedShaderSourceArray is " << sortedShaderSourceArray;
	qDebug() << "****************";
	qDebug() << "\tsuppEntries is " << suppEntries;
	qDebug() << "****************";
	qDebug() << "\tpassVarNameSwapDicts is " << passVarNameSwapDicts;
	qDebug() << "****************";
	*/
	QString			convertedShaderSource = convertShaderSource(sortedShaderSourceArray, suppEntries, passVarNameSwapDicts);
	//qDebug() << "\tconvertedShaderSource is " << convertedShaderSource;
	
	//	if there's a "mouse" input, then there's a mouse and i need to show an alert (we check now because the conversion method will add an "iMouse" input if appropriate)
	if (!IsThisInputNameUnique(QString("iMouse")))	{
		//qDebug() << "\tiMouse input name is NOT unique- mouse detected";
		hasMouseOrKeyboardInput = true;
	}
	else	{
		//qDebug() << "\tiMouse input name was unique- no mouse detected!";
	}
	
	
	//	convert the shader source string, export to the user-library ISF folder
	//	okay, so i converted the shader source- now i want to rename any music-based image inputs to something better than "iChannel"
	//	if i have an inputImageChannelName, i want to find-and-replace that as well
	if (!inputImageChannelName.isNull())	{
		convertedShaderSource.replace(inputImageChannelName,"inputImage");
	}
	
	
	
	
	
	//	now i need to figure out a write location/file name.  try to base this off the "name" from shadertoy, appended by the shadertoy username, and then shadertoy ID
	QString			writeFolder = QString("%1/Library/Graphics/ISF").arg(QDir::homePath());
	QString			writeLocation;
	if (!shadertoyName.isNull())	{
		writeLocation = QString("%1/%2_%3.fs").arg(writeFolder).arg(shadertoyName.replace("/","_")).arg(shaderIDString);
	}
	else	{
		writeLocation = QString("%1/shadertoy_%2.fs").arg(writeFolder).arg(shaderIDString);
	}
	qDebug() << "\twriteLocation is " << writeLocation;
	
	QFile		wFile(writeLocation);
	if (wFile.open(QIODevice::WriteOnly))	{
		_exportedISFPath = writeLocation;
		
		QTextStream		wStream(&wFile);
		wStream << convertedShaderSource;
		wFile.close();
	}
	else	{
		qDebug() << "\tERR: couldnt open file for writing, bailing";
		QMessageBox::warning(GetLoadingWindow(), "", QString("Error: couldn't write file to disk \"%1\"").arg(writeLocation), QMessageBox::Ok);
		done(15);
		return;
	}
	
	
	
	
	//	if the shader requires any imported assets, make sure they've been copied to the write folder as well
	for (const QString & tmpKey : suppImports.keys())	{
		QJsonValue		importDictVal = suppImports.value(tmpKey);
		QJsonObject		importDict = importDictVal.toObject();
		QString			cubeFlag = importDict.value("TYPE").toString();
		if (!cubeFlag.isNull() && cubeFlag!=QString("cube"))
			cubeFlag = QString();
		
		//	if this import describes a cubemap, the PATH is an array
		if (!cubeFlag.isNull())	{
			QJsonArray		importFileNames = importDict.value("PATH").toArray();
			for (const QJsonValue & importFileNameVal : importFileNames)	{
				QString			importFileName = importFileNameVal.toString();
#if defined(Q_OS_MAC)
				QString			importSrcImgPath = QString("%1/Resources/%2").arg(QString::fromStdString(VVGL::StringByDeletingLastPathComponent(QDir::currentPath().toStdString()))).arg(importFileName);
#else
#endif
				QString			importDstImgPath = QString("%1/%2").arg(writeFolder).arg(importFileName);
				
				QFileInfo		fileInfo(importDstImgPath);
				if (!fileInfo.exists())	{
					QFile::copy(importSrcImgPath, importDstImgPath);
				}
			}
		}
		//	else this import doesn't describe a cubemap- just a plain ol' image file
		else	{
			QString			importFileName = importDict.value("PATH").toString();
			QString			importSrcImgPath = QString(":/shadertoy_textures/%1").arg(importFileName);
			QString			importDstImgPath = QString("%1/%2").arg(writeFolder).arg(importFileName);
			
			QFileInfo		fileInfo(importDstImgPath);
			if (!fileInfo.exists())	{
				QFile::copy(importSrcImgPath, importDstImgPath);
			}
		}
	}
	
	
	
	
	//	if there was a mouse of keyboard input, i need to show an alert
	if (hasMouseOrKeyboardInput)	{
		//qDebug() << "\thas mouse or keyboard input, should be showing alert";
		QMessageBox::warning(GetLoadingWindow(), "", QString("The Shadertoy you're converting has a mouse and/or keyboard input, and may require further conversion to function correctly"), QMessageBox::Ok);
	}
	
	
	
	
	//	close the sheet
	done(0);
}




QString ShadertoyConverter::convertShaderSource(const QStringList & rawFragStrings, QJsonObject & suppEntries, const QJsonArray & varSwapNameDicts)	{
	//qDebug() << __PRETTY_FUNCTION__;
	//qDebug() << "\tsuppEntries are " << suppEntries;
	//qDebug() << "\tvarSwapNameDicts are " << varSwapNameDicts;
	
	QStringList			environmentProvidedSamplers;
	QJsonArray			tmpSuppArray;
	QJsonObject			tmpSuppObject;
	
	tmpSuppObject = suppEntries.value("IMPORTED").toObject();
	for (const QString & tmpKey : tmpSuppObject.keys())	{
		QJsonObject			importDict = tmpSuppObject.value(tmpKey).toObject();
		//QString				tmpString = importDict.value("TYPE").toString();
		//if (!tmpString.isNull() && (tmpString==QString("cube") || tmpString==QString("image")))
			environmentProvidedSamplers.append(importDict.value("NAME").toString());
	}
	tmpSuppArray = suppEntries.value("INPUTS").toArray();
	for (const QJsonValue & inputDictVal : tmpSuppArray)	{
		QJsonObject		inputDict = inputDictVal.toObject();
		QString			tmpString = inputDict.value("TYPE").toString();
		if (tmpString==QString("image") || tmpString==QString("cube") || tmpString==QString("audio") || tmpString==QString("audioFFT"))
			environmentProvidedSamplers.append(inputDict.value("NAME").toString());
	}
	tmpSuppArray = suppEntries.value("PASSES").toArray();
	for (const QJsonValue & passDictVal : tmpSuppArray)	{
		QJsonObject		passDict = passDictVal.toObject();
		QString			tmpString = passDict.value("TARGET").toString();
		if (!tmpString.isNull() && tmpString.length()>0)
			environmentProvidedSamplers.append(tmpString);
	}
	//qDebug() << "\tenvironmentProvidedSamplers are " << environmentProvidedSamplers;
	//qDebug() << "\tvarSwapNameDicts are " << varSwapNameDicts;
	
	auto ImportedDictForBufferName = [&](const QString & bufferName)	{
		QJsonObject			returnMe;
		QJsonObject			importedDict = suppEntries.value("IMPORTED").toObject();
		for (const QString & tmpName : importedDict.keys())	{
			if (tmpName == bufferName)	{
				returnMe = importedDict.value(tmpName).toObject();
				break;
			}
		}
		return returnMe;
	};
	
	
	/*	this is a little complicated.
		- i have an array of dictionaries (one dict per pass)- these dicts describe variables that 
		need to be replaced on a pass-by-pass basis (the key in the dict is the string we have to 
		replace, the object is the string to replace it with).
		- there are a number of standard strings i have to find-and-replace
		- the variables passed to shadertoy's "mainImage()" function have to be find-and-replaced 
		with variable names standard to GLSL 1.2.
		- each "renderpass" in shadertoy is a separate GLSL program, possibly with a bunch of 
		external functions (outside the mainImage() function).  because of this, i have to run 
		through all the shadertoy renderpasses, copying everything *before* mainImage into my ISF 
		program.  then i have to run through the passes a second time, taking the contents of each 
		pass's mainImage function and putting them in if/else PASSINDEX statements.  then i have to 
		run through the passes a third time, copying everything *after* mainImage from each pass 
		into my ISF program.
		- ...while i'm doing all of the above, i have to do a lot of find-and-replacing.  the first 
		and third passses, i have to find-and-replace the var swap dict and the standard strings.  
		the second pass i have to find-and-replace the var swap dict, the standard strings, and the 
		variables passed to the "mainImage()" function.
	
	*/
	
	//	make a block that accepts a mutable string and a var swap name dict and find-and-replaces 
	//	the string with the contents of the var swap name dict and also the standard strings.
	auto LineFindAndReplaceBlock = [&](QString & targetLine, const QJsonObject & varSwapNameDict, const QString & rawString, const StringRange & targetRangeInRaw)	{
		//qDebug() << "LineFindAndReplaceBlock() ... " << targetLine;
		
		Q_UNUSED(rawString);
		Q_UNUSED(targetRangeInRaw);
		
		//	we have a dictionary of names that need to be replaced- iterate through it, checking every entry against this line
		for (const QString & key : varSwapNameDict.keys())	{
			QString			newKey = varSwapNameDict.value(key).toString();
			//	the key is the string we want to replace, the value is the new string...
			QRegularExpression		tmpRegex;
			QString					tmpReplaceString;
			//	first replace all instances where the string being replaced is surrounded by non-words
			tmpRegex = QRegularExpression(QString("([^\\w])(%1)([^\\w])").arg(key));
			tmpReplaceString = QString("\\1%1\\3").arg(newKey);
			if (targetLine.contains(tmpRegex))
				targetLine.replace(tmpRegex, tmpReplaceString);
			//	replace all instances where the string being replaced occurs at the beginning of the line
			tmpRegex = QRegularExpression(QString("(^%1)([^\\w])").arg(key));
			tmpReplaceString = QString("%1\\2").arg(newKey);
			if (targetLine.contains(tmpRegex))
				targetLine.replace(tmpRegex, tmpReplaceString);
		}
		
		
		//	now do all the standard find-and-replacing
		{
			//	first we replace the global uniforms passed by the ISF spec...
			
			//	we supply a dict- the key is the string to replace, the object is the string we want to replace it with
			QHash<QString,QString>		srcStrings;
			srcStrings.insert("iDate", "DATE");
			srcStrings.insert("iGlobalTime", "TIME");
			srcStrings.insert("iTime", "TIME");
			srcStrings.insert("iChannelTime\\[[0-9]\\]", "TIME");
			srcStrings.insert("iTimeDelta", "TIMEDELTA");
			srcStrings.insert("iResolution", "RENDERSIZE");
			srcStrings.insert("iFrame", "FRAMEINDEX");
			//	the key is the regex string, the value is the replace string.  using a dict simply to keep the association.
			QHash<QString,QString>		regexReplaceStrings;
			regexReplaceStrings.insert("([^\\w_])(%1)([^\\w_])", "\\1%1\\3");
			regexReplaceStrings.insert("(^%1)([^\\w_])", "%1\\2");
			regexReplaceStrings.insert("([^\\w_])(%1$)", "\\1%1");
			//	enumerate the keys and values in the dict of strings describing what i want to replace and waht i want to replace it with
			for (const QString & replaceMe : srcStrings.keys())	{
				const QString		&replacement = srcStrings.value(replaceMe);
				//	...for each pair of strings (replace me/replacement), construct a number of different regex/replace string pairs and apply them.  do this by enumerating the dict of regex/replacement strings.
				
				for (const QString & regexFmtString : regexReplaceStrings.keys())	{
					const QString		&replaceFmtString = regexReplaceStrings.value(regexFmtString);
					
					QString				regexString = regexFmtString.arg(replaceMe);
					QString				replaceString = replaceFmtString.arg(replacement);
					QRegularExpression	regex(regexString);
					while (targetLine.contains(regex))
						targetLine.replace(regex, replaceString);
				}
			}
			
			
			QString			regexString;
			QString			replaceString;
			//	replace the "channel resolution" var instances with calls to IMG_SIZE
			regexString = "iChannelResolution\\[([0-9]+)\\]";
			QRegularExpression	regex(regexString);
			while (targetLine.contains(regex))	{
				QRegularExpressionMatch		regexMatch = regex.match(targetLine);
				if (regexMatch.capturedTexts().count() != 2)	{
					qDebug() << "\tERR: potential problem, iChannelResolution group capture count wrong";
					break;
				}
				QString				channelNumberString = regexMatch.captured(1);
				QString				channelStringToCheck = QString("iChannel%1").arg(channelNumberString.toInt());
				QString				uniqueChannelName = varSwapNameDict.value(channelStringToCheck).toString();
				if (uniqueChannelName.isNull())
					uniqueChannelName = channelStringToCheck;
				replaceString = QString("IMG_SIZE(%1)").arg(uniqueChannelName);
				targetLine.replace(regexMatch.capturedStart(), regexMatch.capturedLength(), replaceString);
			}
			
			
			//	now remove any and all texture2D or texture2DRect function calls (there may be more than one!) and replace with the appropriate macro for accessing the texture
			StringRange			tmpRange;
			do	{
				bool				textureLookupWas2D = false;
				bool				textureLookupWasCube = false;
				tmpRange = QRegularExpression("texture2DRect[\\s]*\\(").match(targetLine);
				if (tmpRange.length != 0)	{
					--tmpRange.length;	//	i searched for the string + left parenthesis
				}
				else	{
					tmpRange = QRegularExpression("texture2D[\\s]*\\(").match(targetLine);
					if (tmpRange.length != 0)	{
						--tmpRange.length;	//	i searched for the string + left parenthesis
						textureLookupWas2D = true;
					}
					else	{
						tmpRange = QRegularExpression("texture[\\s]*\\(").match(targetLine);
						if (tmpRange.length != 0)	{
							--tmpRange.length;	//	i searched for the string + left parenthesis
							//	'texture()' implies a newer GL environment, and may be referring to a cube sampler- so we can't just assume it's 2D and replace it...
							QStringList			tmpVarArray;
							StringRange			fullFuncRangeToReplace = LexFunctionCall(targetLine, tmpRange, tmpVarArray);
							//qDebug() << "\ttexture() call's vars are " << tmpVarArray;
							if (tmpVarArray.count()>0 && environmentProvidedSamplers.contains(tmpVarArray.at(0)))	{
								//qDebug() << "\ttexture() call phase A complete";
								QJsonObject			importDict = ImportedDictForBufferName(tmpVarArray.at(0));
								if (importDict.contains("TYPE"))	{
									textureLookupWasCube = true;
								}
								else
									textureLookupWas2D = true;
							}
							else
								textureLookupWas2D = true;
						}
						else	{
							//qDebug() << "\tdidn't find any texture-related calls";
						}
					}
				}
				if (tmpRange.length != 0)	{
					//qDebug() << "\tline matches a texture lookup:\n\t\"" << targetLine << "\"";
					StringRange			funcNameRange = tmpRange;
					QStringList			tmpVarArray;
					StringRange			fullFuncRangeToReplace = LexFunctionCall(targetLine, funcNameRange, tmpVarArray);
					//	i only want to replace this function if the sampler is one of the samplers i'm converting/replacing
					if (tmpVarArray.count()>0 && environmentProvidedSamplers.contains(tmpVarArray.at(0)))	{
						if (tmpVarArray.count() == 2)	{
							QString			newFuncString;
							QString			samplerName = tmpVarArray.at(0);
							QString			samplerCoord = tmpVarArray.at(1);
							if (textureLookupWas2D)	{
								newFuncString = QString("IMG_NORM_PIXEL(%1,mod(%2,1.0))").arg(samplerName).arg(samplerCoord);
							}
							else if (textureLookupWasCube)	{
								newFuncString = QString("textureCube(%1,%2)").arg(samplerName).arg(samplerCoord);
							}
							else	{
								newFuncString = QString("IMG_PIXEL(%1,%2)").arg(samplerName).arg(samplerCoord);
							}
							//qDebug() << "\tnewFuncString is " << newFuncString << ", range to replace is " << fullFuncRangeToReplace.location << "/" << fullFuncRangeToReplace.length;
							targetLine.replace(fullFuncRangeToReplace.location, fullFuncRangeToReplace.length, newFuncString);
						}
						else if (tmpVarArray.count() == 3)	{
							QString			newFuncString;
							QString			samplerName = tmpVarArray.at(0);
							QString			samplerCoord = tmpVarArray.at(1);
							QString			samplerBias = tmpVarArray.at(2);
							if (textureLookupWas2D)	{
								newFuncString = QString("IMG_NORM_PIXEL(%1,mod(%2,1.0),%3)").arg(samplerName).arg(samplerCoord).arg(samplerBias);
							}
							else if (textureLookupWasCube)	{
								newFuncString = QString("textureCube(%1,%2)").arg(samplerName).arg(samplerCoord).arg(samplerBias);
							}
							else	{
								newFuncString = QString("IMG_PIXEL(%1,%2,%3)").arg(samplerName).arg(samplerCoord).arg(samplerBias);
							}
							//qDebug() << "\tnewFuncString is " << newFuncString << ", range to replace is " << fullFuncRangeToReplace.location << "/" << fullFuncRangeToReplace.length;
							targetLine.replace(fullFuncRangeToReplace.location, fullFuncRangeToReplace.length, newFuncString);
						}
						else	{
							qDebug() << "\tERR: variable count wrong searching for texture lookup:\n" << targetLine << "\n" << tmpVarArray;
							break;
						}
						//qDebug() << "\tafter replacing, targetLine is " << targetLine;
					}
					//	else the sampler in this texture lookup isn't a sampler being controlled by the ISF environment...
					else	{
						//qDebug() << "\tskipping this line, sampler isn't controlled by ISF environment...";
						tmpRange.length = 0;
					}
				}
			} while (tmpRange.length > 0);
		}
	};
	
	
	//	make a mutable string- this is what we're building the results for the source code in
	QString			tmpMutString;
	tmpMutString.append("\n");
	
	
	int				passIndex = 0;
	//	run through the array of frag shader sources- the goal is to copy everything BEFORE the mainImage() function into the string
	for (const QString & rawFragString : rawFragStrings)	{
		//	get the variable swap dict, i'll need it for find-and-replacing...
		QJsonObject		varSwapNameDict = varSwapNameDicts.at(passIndex).toObject();
		//	run through the frag shader source, line by line
		
		StringRange		thisLineRange;
		QTextStream		ts(const_cast<QString*>(&rawFragString));
		QString			line = ts.readLine();
		while (!line.isNull())	{
			//	update the range of this line within the raw frag string (we need to know the range because we're going to need to lex function calls that may span multiple lines)
			thisLineRange.location = thisLineRange.location + thisLineRange.length;
			thisLineRange.length = line.length() + 1;
			//	make sure that the range is within the bounds of the raw frag string!
			if (thisLineRange.location + thisLineRange.length >= rawFragString.length())
				thisLineRange.length = rawFragString.length() - thisLineRange.location;
			
			//	if this is the mainImage function, stop- we're done, this pass we're only copying the stuff before the mainImage function
			if (line.contains(QRegularExpression("[\\r\\n\\s]*void[\\s]+mainImage")))	{
				break;
			}
			//	else we're still before the mainImage function- we have to do some find-and-replacing while copying stuff
			else	{
				QString			newLine;
				newLine.append(line);
				
				LineFindAndReplaceBlock(newLine, varSwapNameDict, rawFragString, StringRange(thisLineRange.location, newLine.length()));
				
				tmpMutString.append(newLine);
				tmpMutString.append("\n");
			}
			line = ts.readLine();
		}
		
		++passIndex;
	}
	
	
	
	
	//	make the "main" function entry
	tmpMutString.append("void main() {");
	tmpMutString.append("\n");
	
	
	
	
	//			this next bit here goes through every pass and replaces the contents of the mainImage functions
	
	//	we need to know if there are multiple passes, and the index of the pass we're parsing
	bool			multiplePasses = (rawFragStrings.count()>1) ? true : false;
	passIndex = 0;
	//	run through the array of frag shader sources again- this time we're going to find "mainImage" and convert all the code within it
	for (const QString & rawFragString : rawFragStrings)	{
		//	if there are multiple passes, we have to start off by adding a bracket with an if/else defining the PASSINDEX
		if (multiplePasses)	{
			if (passIndex == 0)
				tmpMutString.append(QString("\tif (PASSINDEX == %1)\t{").arg(passIndex));
			else
				tmpMutString.append(QString("\telse if (PASSINDEX == %1)\t{").arg(passIndex));
		}
		
		//	get the variable swap dict
		QJsonObject		varSwapNameDict = varSwapNameDicts.at(passIndex).toObject();
		//	now we have to do all the conversion!
		bool			beforeMainImage = true;
		bool			afterMainImage = false;
		int				mainImageFunctionBracketCount = 0;	//	...in order to determine when i'm parsing text within the mainImage() function i need to keep a count of the brackets- when it hits 0, i've left the function
		QString			fragColorVarNameString("fragColor");	//	the 'mainImage' function in a shadertoy passes in a 4-element vec named 'fragColor' by default (gl_FragColor in "old school" GLSL) and a 2-element vec named "fragCoord" by default.  the variable names (fragColor and fragCoord) may be different, so we parse them here
		QString			fragCoordVarNameString("fragCoord");	//	see above
		StringRange		thisLineRange;
		StringRange		mainImageFunctionRange;	//	the range of the fll "mainImage" function (and all its vars and stuff) in the raw frag string
		StringRange		firstBracketOfMainImageFunction;
		
		QTextStream		ts(const_cast<QString*>(&rawFragString));
		QString			line = ts.readLine();
		while (!line.isNull())	{
			//	update the range of this line within the raw frag string (we need to know the range because we're going to need to lex function calls that may span multiple lines)
			thisLineRange.location = thisLineRange.location + thisLineRange.length;
			thisLineRange.length = line.length() + 1;
			//	make sure that the range is within the bounds of the raw frag string!
			if (thisLineRange.location + thisLineRange.length >= rawFragString.length())
				thisLineRange.length = rawFragString.length() - thisLineRange.location;
			
			//	shadertoy shaders don't have a main() function- instead they have a mainImage() function, which isn't really compatible with the GLSL 1.2 environment i wrote all this for back in the day, and continue to support
			QRegularExpression		mainImageRegex("[\\r\\n\\s]*void[\\s]+(mainImage)");
			if (line.contains(mainImageRegex))	{
				//	first of all, reset the mainImage function bracket count stuff, then increment it (if appropriate)
				mainImageFunctionBracketCount = 0;
				
				//	the default variable names passed to mainImage (fragColor and fragCoord) may be different in this shader- we have to capture them to find out
				QRegularExpressionMatch		mainImageRegexMatch = mainImageRegex.match(line);
				StringRange					mainImageStringRange;
				mainImageStringRange.location = mainImageRegexMatch.capturedStart(1);
				mainImageStringRange.length = mainImageRegexMatch.capturedLength(1);
				mainImageStringRange.location += thisLineRange.location;
				QStringList					mainImageVars;
				mainImageFunctionRange = LexFunctionCall(rawFragString, mainImageStringRange, mainImageVars);
				if (mainImageVars.count() >= 1)	{
					fragColorVarNameString = QString();
					QStringList			tmpArray = mainImageVars.at(0).split(QRegularExpression("[^\\w_]"));
					if (tmpArray.count()>0)
						fragColorVarNameString = tmpArray.at(tmpArray.count()-1);
				}
				if (mainImageVars.count() >= 2)	{
					fragCoordVarNameString = QString();
					QStringList			tmpArray = mainImageVars.at(1).split(QRegularExpression("[^\\w_]"));
					if (tmpArray.count()>0)
						fragCoordVarNameString = tmpArray.at(tmpArray.count()-1);
				}
				//	i still need to know where the first bracket of the mainImage() function is (it may be on another line, so i have to find it now)
				StringRange			tmpRange(mainImageFunctionRange.location+mainImageFunctionRange.length, 1);
				while (firstBracketOfMainImageFunction.length<1)	{
					if (rawFragString.mid(tmpRange.location, tmpRange.length) == "{")	{
						firstBracketOfMainImageFunction = tmpRange;
					}
					++tmpRange.location;
				}
				
				
				//	if this line contains the bracket after the mainImage function...
				if ((firstBracketOfMainImageFunction.length>0) &&
				((firstBracketOfMainImageFunction.location+firstBracketOfMainImageFunction.length)<=(thisLineRange.location+thisLineRange.length)))	{
					//	update the bracket count (only update the count if this line contains the first bracket after the mainImage function)
					mainImageFunctionBracketCount += line.split(QRegularExpression("{")).count();
					mainImageFunctionBracketCount -= line.split(QRegularExpression("}")).count();
					//	we're no longer before the mainImage function...
					beforeMainImage = false;
				}
			}
			//	else this line isn't the mainImage function
			else	{
				//	create a mutable string, populate it initially with the line i'm enumerating
				QString			newLine;
				newLine.append(line);
				
				//	if i'm before the mainImage function, empty it- we already handled this content in the previous pass
				if (beforeMainImage && !afterMainImage)	{
					//	if this line contains the bracket after the mainImage function...
					if ((firstBracketOfMainImageFunction.length>0)	&&
					((firstBracketOfMainImageFunction.location+firstBracketOfMainImageFunction.length)<=(thisLineRange.location+thisLineRange.length)))	{
						//	update the bracket count (only update the count if this line contains the first bracket after the mainImage function!)
						mainImageFunctionBracketCount += line.split("{").count();
						mainImageFunctionBracketCount -= line.split("}").count();
						//	we're no longer before the mainImage function...
						beforeMainImage = false;
						//	empty the line, then copy everything after the first bracket of the main function (which is in this line!) to it
						newLine = QString("");
						StringRange			tmpRange;
						tmpRange.location = firstBracketOfMainImageFunction.location + 1;
						tmpRange.length = thisLineRange.location + thisLineRange.length - tmpRange.location;
						if (tmpRange.length > 0)
							newLine.append(rawFragString.mid(tmpRange.location, tmpRange.length));
					}
					//	else this line doesn't contain the bracket after the mainImage function
					else	{
						//	empty the string- we're "before the main image", but also before the first bracket of the mainImage function- we are most likely somewhere before or within the mainImage function declaration
						newLine = QString("");
					}
				}
				//	if i'm still within the mainImage function, replace occurrences of fragCoord and fragColor with their GLSL 1.2 equivalents
				else if (!beforeMainImage && !afterMainImage)	{
					QString				regexString;
					regexString = QString("([^\\w])(%1)([^\\w])").arg(fragColorVarNameString);
					newLine.replace(QRegularExpression(regexString), QString("\\1gl_FragColor\\3"));
					//	now i want to replace "fragCoord" with "gl_FragCoord", but "fragCoord" is vec2, and "gl_FragCoord" is vec4.
					//	first try to replace "fragCoord." (fragCoord-period) with "gl_FragColor."
					regexString = QString("([^\\w])(%1\\.)([a-z])").arg(fragCoordVarNameString);
					newLine.replace(QRegularExpression(regexString), QString("\\1gl_FragCoord.\\3"));
					//	now try to just replace "fragCoord" with "gl_FragColor.xy"
					regexString = QString ("([^\\w])(%1)([^\\w])").arg(fragCoordVarNameString);
					newLine.replace(QRegularExpression(regexString), QString("\\1gl_FragCoord.xy\\3"));
					
					
					//	this does the replacing on lines that start with the values i'm replacing...
					regexString = QString("(^%1)([^\\w])").arg(fragColorVarNameString);
					newLine.replace(QRegularExpression(regexString), QString("gl_FragColor\\2"));
					//	now i want to replace "fragCoord" with "gl_FragCoord", but "fragCoord" is vec2, and "gl_FragCoord" is vec4.
					//	first try to replace "fragCoord." (fragCoord-period) with "gl_FragColor."
					regexString = QString("(^%1\\.)([^\\w])").arg(fragCoordVarNameString);
					newLine.replace(QRegularExpression(regexString), QString("gl_FragCoord\\2"));
					//	now try to just replace "fragCoord" with "gl_FragColor.xy"
					regexString = QString("(^%1)([^\\w])").arg(fragCoordVarNameString);
					newLine.replace(QRegularExpression(regexString), QString("gl_FragCoord\\2"));
					
					
					//	now do all the find-and-replacing
					LineFindAndReplaceBlock(newLine, varSwapNameDict, rawFragString, StringRange(thisLineRange.location, newLine.length()));
					
					
					//	update the count of mainImage function brackets in both directions
					mainImageFunctionBracketCount += newLine.split("{").count();
					mainImageFunctionBracketCount -= newLine.split("}").count();
					//	if the mainImage bracket count hits 0, then i'm no longer within the mainImage function!
					if (mainImageFunctionBracketCount <= 0)	{
						afterMainImage = true;
						break;
					}
					
					//	if there are multiple passes, everything is indented- add a tab
					if (multiplePasses)
						newLine.insert(0, "\t");
				}
				
				
				if (newLine.length() > 0)	{
					tmpMutString.append("\n");
					tmpMutString.append(newLine);
				}
			}
			
			line = ts.readLine();
		}
		
		
		//	if there are multiple passes, don't forget to close the PASSINDEX bracket!
		if (multiplePasses)	{
			tmpMutString.append("\n\t}\n");
		}
		
		//	increment the pass index!
		++passIndex;
	}
	
	//	close our main function
	//if (multiplePasses)
		tmpMutString.append("\n}");
	tmpMutString.append("\n");
	
	
	
	
	//	run through the array of frag shader sources again- this time we're going to copy everything after "mainImage"...
	passIndex = 0;
	for (const QString & rawFragString : rawFragStrings)	{
		//	get the variable swap dict, i'll need it for find-and-replacing...
		QJsonObject		varSwapNameDict = varSwapNameDicts.at(passIndex).toObject();
		//	set up a bunch of other vars used to track where i am within the main image
		bool			beforeMainImage = true;
		bool			afterMainImage = false;
		int				mainImageFunctionBracketCount = 0;	//	...in order to determine when i'm parsing text within the mainImage() function i need to keep a count of the brackets- when it hits 0, i've left the function!
		StringRange		thisLineRange;
		StringRange		mainImageFunctionRange;
		StringRange		firstBracketOfMainImageFunction;
		
		QTextStream		ts(const_cast<QString*>(&rawFragString));
		QString			line = ts.readLine();
		while (!line.isNull())	{
			//	update the range of this line within the raw frag string (we need to know the range because we're going to need to lex function calls that may span multiple lines)
			thisLineRange.location = thisLineRange.location + thisLineRange.length;
			thisLineRange.length = line.length() + 1;
			//	make sure that the range is within the bounds of the raw frag string!
			if (thisLineRange.location + thisLineRange.length >= rawFragString.length())
				thisLineRange.length = rawFragString.length() - thisLineRange.location;
			
			QRegularExpression		mainImageRegex("[\\r\\n\\s]*void[\\s]+(mainImage)");
			if (line.contains(mainImageRegex))	{
				//	first of all, reset the mainImage function bracket count stuff, then increment it (if appropriate)
				mainImageFunctionBracketCount = 0;
				//	the default variable names passed to mainImage (fragColor and fragCoord) may be different in this shader- we have to capture them to find out
				QRegularExpressionMatch		mainImageRegexMatch = mainImageRegex.match(line);
				StringRange					mainImageStringRange;
				mainImageStringRange.location = mainImageRegexMatch.capturedStart(1);
				mainImageStringRange.length = mainImageRegexMatch.capturedLength(1);
				mainImageStringRange.location += thisLineRange.location;
				QStringList					mainImageVars;
				mainImageFunctionRange = LexFunctionCall(rawFragString, mainImageStringRange, mainImageVars);
				//	i still need to know where the first bracket of the mainImage() function is (it may be on another line, so i have to find it now)
				StringRange			tmpRange(mainImageFunctionRange.location+mainImageFunctionRange.length, 1);
				while (firstBracketOfMainImageFunction.length<1)	{
					if (rawFragString.mid(tmpRange.location, tmpRange.length) == "{")	{
						firstBracketOfMainImageFunction = tmpRange;
					}
					++tmpRange.location;
				}
				
				//	if this line contains the bracket after the mainImage function...
				if ((firstBracketOfMainImageFunction.length>0) &&
				((firstBracketOfMainImageFunction.location+firstBracketOfMainImageFunction.length)<=(thisLineRange.location+thisLineRange.length)))	{
					//	update the bracket count (only update the count if this line contains the first bracket after the mainImage function!)
					mainImageFunctionBracketCount += line.split(QRegularExpression("{")).count();
					mainImageFunctionBracketCount -= line.split(QRegularExpression("}")).count();
					//	we're not longer before the mainImage function...
					beforeMainImage = false;
				}
			}
			//	else this line isn't the mainImage function
			else	{
				//	if i'm before the mainImage function, do nothing
				if (beforeMainImage && !afterMainImage)	{
					//	if this line contains the bracket after the mainImage function...
					if ((firstBracketOfMainImageFunction.length>0) &&
					((firstBracketOfMainImageFunction.location+firstBracketOfMainImageFunction.length)<=(thisLineRange.location+thisLineRange.length)))	{
						//	update the bracket count (only update the count if this line contains the first bracket after the mainImage function!)
						mainImageFunctionBracketCount += line.split("{").count();
						mainImageFunctionBracketCount -= line.split("}").count();
						//	we're not longer before the mainImage function...
						beforeMainImage = false;
					}
					//	else this line doesn't contain the bracket after the mainImage function
					else	{
						//	do nothing- we're "before the main image", but also before the first bracket of the mainImage function- we are most likely somewhere before or within the mainImage function declaration
					}
				}
				//	if i'm still within the mainImage function, replace occurrences of fragCoord and fragColor with their GLSL 1.2 equivalents
				else if (!beforeMainImage && !afterMainImage)	{
					//	update the count of mainImage function brackets in both directions
					mainImageFunctionBracketCount += line.split("{").count();
					mainImageFunctionBracketCount -= line.split("}").count();
					//	if the mainImage bracket count hits 0, then i'm no longer within the mainImage function!
					if (mainImageFunctionBracketCount <= 0)
						afterMainImage = true;
				}
				//	if i'm after the mainImage function...
				else if (!beforeMainImage && afterMainImage)	{
					QString				newLine;
					newLine.append(line);
					
					LineFindAndReplaceBlock(newLine, varSwapNameDict, rawFragString, StringRange(thisLineRange.location,newLine.length()));
					
					tmpMutString.append(newLine);
					tmpMutString.append("\n");
				}
			}
			
			
			line = ts.readLine();
		}
		
		
		//	increment the pass index
		++passIndex;
	}
	
	
	/*
	//	figure out if the source code uses the mouse var
	bool			usesMouseVar = tmpMutString.contains(QRegularExpression("([^\\w_])(iMouse)([^\\w_])"));
	//	if i'm using the mouse var, make 2d input for it
	if (usesMouseVar)	{
		QJsonObject		tmpInput;
		tmpInput.insert("NAME", QJsonValue(QString("iMouse")));
		tmpInput.insert("TYPE", QJsonValue(QString("point2D")));
		
		QJsonArray		tmpInputs = suppEntries.value("INPUTS").toArray();
		tmpInputs.append(tmpInput);
		suppEntries.insert("INPUTS", QJsonValue(tmpInputs));
	}
	*/
	
	
	//	we're pretty much done, we just have to turn the supplemental entries dict into a JSON string and return it along with the modified source code
	QJsonDocument			tmpDoc(suppEntries);
	QString					tmpJSONString(tmpDoc.toJson(QJsonDocument::Indented));
	return QString("/*\n%1\n*/\n\n%2").arg(tmpJSONString).arg(tmpMutString);
}

