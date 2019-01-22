#include "GLSLSandboxConverter.h"
#include "ui_GLSLSandboxConverter.h"

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

#if defined(Q_OS_WIN)
#define __PRETTY_FUNCTION__ __FUNCSIG__
#endif




GLSLSandboxConverter::GLSLSandboxConverter(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::GLSLSandboxConverter)
{
	qDebug() << __PRETTY_FUNCTION__;
	//setWindowModality(Qt::ApplicationModal);
	setWindowModality(Qt::WindowModal);
	ui->setupUi(this);
#if defined(Q_OS_MAC)
	ui->destinationLabel->setText("(converted file will be created in ~/Library/Graphics/ISF)");
#elif defined(Q_OS_WIN)
	ui->destinationLabel->setText("(converted file will be created in \\ProgramData\\ISF)");
#endif
}

GLSLSandboxConverter::~GLSLSandboxConverter()
{
	qDebug() << __PRETTY_FUNCTION__;
	delete ui;
}




void GLSLSandboxConverter::cancelClicked()	{
	qDebug() << __PRETTY_FUNCTION__;
	done(1);
}
void GLSLSandboxConverter::okClicked()	{
	qDebug() << __PRETTY_FUNCTION__;
	
	QString			rawURLString = ui->lineEdit->text();
	//QString			rawURLString("http://glslsandbox.com/e#50324.0");
	QStringList		rawURLComponents = rawURLString.split("e#");
	if (rawURLComponents.size() < 2)	{
		qDebug() << "err: couldnt parse user-supplied URL";
		QString		errString = QString("Error: cannot parse URL \"%1\"").arg(rawURLString);
		QMessageBox::warning(GetLoadingWindow(), "", errString, QMessageBox::Ok);
		done(1);
		return;
	}
	
	QString			shaderIDString = rawURLComponents.at(1);
	QString			sourceBlobURL = QString("http://glslsandbox.com/item/%1").arg(shaderIDString);
	QJsonDocument	jsonDoc;
	
	//	this bit downloads the contents of the url synchronously
	{
		QUrl				url(sourceBlobURL);
		QNetworkRequest		req(url);
		QNetworkAccessManager	nm;
		QNetworkReply		*reply = nm.get(req);
		if (reply == nullptr)	{
			done(2);
			return;
		}
		
		QEventLoop			tmpLoop;
		connect(reply, SIGNAL(finished()), &tmpLoop, SLOT(quit()));
		tmpLoop.exec();
		
		jsonDoc = QJsonDocument::fromJson( reply->readAll() );
		reply->deleteLater();
	}
	
	if (jsonDoc.isEmpty() || !jsonDoc.isObject())	{
		QMessageBox::warning(GetLoadingWindow(), "", "Error: downloaded data is empty", QMessageBox::Ok);
		done(3);
		return;
	}
	
	QJsonObject		parsedDownload = jsonDoc.object();
	if (!parsedDownload.contains("code"))	{
		QMessageBox::warning(GetLoadingWindow(), "", "Error: data format has changed!", QMessageBox::Ok);
		done(4);
		return;
	}
	QJsonValue		tmpVal = parsedDownload.value("code");
	if (!tmpVal.isString())	{
		QMessageBox::warning(GetLoadingWindow(), "", "Error: data format has changed!", QMessageBox::Ok);
		done(5);
		return;
	}
	QString			rawShaderSource = tmpVal.toString();
	
	//	convert the shader source string
	QJsonObject		suppEntries;
	suppEntries.insert(QString("DESCRIPTION"), QJsonValue( QString("Automatically converted from %1").arg(rawURLString) ));
	QString			convertedShaderSource = convertShaderSource(rawShaderSource, suppEntries);
	//	export the string to the user-library ISF folder
#if defined(Q_OS_MAC)
	QString			exportPath = QString("%1/Library/Graphics/ISF/gs_%2.fs").arg(QDir::homePath()).arg(shaderIDString);
#elif defined(Q_OS_WIN)
	QDir			tmpDir = QDir::root();
	if (!tmpDir.cd("ProgramData"))	{
		tmpDir.mkdir("ProgramData");
		tmpDir.cd("ProgramData");
	}
	if (!tmpDir.cd("ISF"))	{
		tmpDir.mkdir("ISF");
		tmpDir.cd("ISF");
	}
	QString			exportPath = QString("%1/gs_%2.fs").arg(tmpDir.path()).arg(shaderIDString);
#endif
	//qDebug() << "exportPath is " << exportPath;
	
	QFile		wFile(exportPath);
	if (wFile.open(QIODevice::WriteOnly))	{
		_exportedISFPath = exportPath;
		
		QTextStream		wStream(&wFile);
		wStream << convertedShaderSource;
		wFile.close();
	}
	
	
	done(0);
}


QString GLSLSandboxConverter::convertShaderSource(const QString & rawFragString, const QJsonObject & suppEntries)	{
	QString			tmpMutString;
	
	QTextStream		ts(const_cast<QString*>(&rawFragString));
	QString			line = ts.readLine();
	
	bool			hasAnotherUniform = false;
	bool			declaresBackbuffer = false;
	QString			backbufferName;
	bool			backbufferWasRect = false;
	bool			declaresSurfacePosition = false;
	
	QRegularExpression		timeUniformRegex("uniform[\\s]+float[\\s]+time;");
	QRegularExpression		mouseUniformRegex("uniform[\\s]+vec2[\\s]+mouse;");
	QRegularExpression		resUniformRegex("uniform[\\s]+vec2[\\s]+resolution;");
	QRegularExpression		samplerUniformRegex("uniform[\\s]+sampler2D(Rect)?[\\s]+[bB]");
	QRegularExpression		surfacePosUniformRegex("((varying)|(uniform))[\\s]+vec2[\\s]+surfacePosition");
	QRegularExpression		mouseOrResUniformRegex("uniform[\\s]+vec2[\\s]+((mouse)|(resolution))[\\s]*,[\\s]*((mouse)|(resolution));");
	
	while (!line.isNull())	{
		qDebug() << "\tprocessing line " << line;
		
		QRegularExpressionMatch		timeRegexMatch = timeUniformRegex.match(line);
		if (!timeRegexMatch.hasMatch())	{
			QRegularExpressionMatch		mouseRegexMatch = mouseUniformRegex.match(line);
			if (!mouseRegexMatch.hasMatch())	{
				QRegularExpressionMatch		resRegexMatch = resUniformRegex.match(line);
				if (!resRegexMatch.hasMatch())	{
					QRegularExpressionMatch		samplerRegexMatch = samplerUniformRegex.match(line);
					if (!samplerRegexMatch.hasMatch())	{
						QRegularExpressionMatch		surfacePosRegexMatch = surfacePosUniformRegex.match(line);
						if (!surfacePosRegexMatch.hasMatch())	{
							QRegularExpressionMatch		mouseOrResRegexMatch = mouseOrResUniformRegex.match(line);
							if (!mouseOrResRegexMatch.hasMatch())	{
								//	if there's a "uniform" in this line, log it so i can flag the file as having another uniform (which can potentially be controlled externally)
								StringRange		tmpRange;
								QString			uniformString("uniform");
								
								if (line.contains(uniformString))	{
									hasAnotherUniform = true;
									tmpRange.location = line.indexOf(uniformString);
									tmpRange.length = uniformString.length();
								}
								
								//	remove any and all texture2DRect function calls from this line (there may be more than one!) and replace with the appropriate macro for accessing the texture
								QString			newLine(line);
								do	{
									bool			textureLookupWas2D = false;
									tmpRange.location = newLine.indexOf("texture2DRect(");
									if (tmpRange.location >= 0)
										tmpRange.length = QString("texture2DRect").length();
									else	{
										tmpRange.location = newLine.indexOf("texture2D(");
										if (tmpRange.location >= 0)	{
											tmpRange.length = QString("texture2D").length();
											textureLookupWas2D = true;
										}
									}
									if (tmpRange.location >= 0)	{
										//qDebug() << "\t\tline matches a texture lookup: " << newLine;
										StringRange		funcNameRange = tmpRange;
										QStringList		tmpVarArray;
										StringRange		fullFuncRangeToReplace = LexFunctionCall(newLine, funcNameRange, tmpVarArray);
										if (tmpVarArray.length() == 2)	{
											QString			newFuncString;
											QString			samplerName = tmpVarArray.at(0);
											QString			samplerCoord = tmpVarArray.at(1);
											if (textureLookupWas2D)	{
												newFuncString = QString("IMG_NORM_PIXEL(%1,mod(%2,1.0))").arg(samplerName).arg(samplerCoord);
											}
											else	{
												newFuncString = QString("IMG_PIXEL(%1,%2)").arg(samplerName).arg(samplerCoord);
											}
											newLine.replace(fullFuncRangeToReplace.location, fullFuncRangeToReplace.length, newFuncString);
										}
										else if (tmpVarArray.length() == 3)	{
											QString			newFuncString;
											QString			samplerName = tmpVarArray.at(0);
											QString			samplerCoord = tmpVarArray.at(1);
											QString			samplerBias = tmpVarArray.at(2);
											if (textureLookupWas2D)	{
												newFuncString = QString("IMG_NORM_PIXEL(%1,mod(%2,1.0),%3)").arg(samplerName).arg(samplerCoord).arg(samplerBias);
											}
											else	{
												newFuncString = QString("IMG_PIXEL(%1,%2,%3)").arg(samplerName).arg(samplerCoord).arg(samplerBias);
											}
											newLine.replace(fullFuncRangeToReplace.location, fullFuncRangeToReplace.length, newFuncString);
										}
										else	{
											qDebug() << "ERR: variable count wrong searching for texture lookup: " << newLine << ", " << tmpVarArray;
											break;
										}
									}
								} while (tmpRange.length>0);
								
								tmpMutString.append("\n");
								tmpMutString.append(newLine);
							}
						}
						//	else there's a surface position variable
						else	{
							declaresSurfacePosition = true;
						}
					}
					//	else there's a backbuffer var declaration, figure out what kind of sampler it is (2D or RECT) and pull its exact name
					else	{
						declaresBackbuffer = true;
						StringRange		tmpRange;
						tmpRange.location = line.indexOf("sampler2DRect");
						if (tmpRange.location >= 0)	{
							tmpRange.length = QString("sampler2DRect").length();
							backbufferWasRect = true;
						}
						QRegularExpression		tmpRegex("uniform[\\s]+sampler2D(Rect)?[\\s]+([^;]+);");
						QRegularExpressionMatch		tmpRegexMatch = tmpRegex.match(line);
						qDebug() << "capturedTexts are " << tmpRegexMatch.capturedTexts();
						if (tmpRegexMatch.hasMatch() && tmpRegexMatch.capturedTexts().length()==3)	{
							backbufferName = tmpRegexMatch.captured(2);
							qDebug() << "\tbackbufferName discovered to be " << backbufferName;
						}
					}
				}
			}
		}
		
		line = ts.readLine();
	}
	qDebug() << "tmpMutString is " << tmpMutString;
	
	//	figure out if the source code uses the mouse var and backbuffer
	bool			usesMouseVar = QRegularExpression("([^a-zA-Z0-9_])(mouse)([^a-zA-Z0-9_])").match(tmpMutString).hasMatch();
	bool			usesBackbufferVar = false;
	if (!backbufferName.isNull())	{
		QString			tmpRegexString = QString("([^a-zA-Z0-9_])(%1)([^a-zA-Z0-9_])").arg(backbufferName);
		usesBackbufferVar = QRegularExpression(tmpRegexString).match(tmpMutString).hasMatch();
	}
	bool			usesSurfacePositionVar = QRegularExpression("([^a-zA-Z0-9_])(surfacePosition)([^a-zA-Z0-9_])").match(tmpMutString).hasMatch();
	
	//	assemble the JSON dict that describes the filter.
	QJsonObject		isfDict;
	QJsonArray		tmpArray;
	QJsonObject		tmpObject;
	//	add any supplemental entries passed in with the method
	for (const QString & tmpKey : suppEntries.keys())
		isfDict.insert(tmpKey, suppEntries[tmpKey]);
	//	put it in an "automatically Converted" category by default
	tmpArray = { QString("Automatically Converted"), QString("GLSLSandbox") };
	isfDict.insert(QString("CATEGORIES"), QJsonValue(tmpArray));
	//	make an input (if the mouse is being used), populate the inputs array
	tmpArray = {};
	if (usesMouseVar)	{
		tmpObject = QJsonObject();
		tmpObject.insert(QString("NAME"), QJsonValue(QString("mouse")));
		tmpObject.insert(QString("TYPE"), QJsonValue(QString("point2D")));
		QJsonArray		minArray = { 0.0, 0.0 };
		tmpObject.insert(QString("MIN"), QJsonValue(minArray));
		QJsonArray		maxArray = { 1.0, 1.0 };
		tmpObject.insert(QString("MAX"), QJsonValue(maxArray));
		
		tmpArray.append(QJsonValue(tmpObject));
	}
	isfDict.insert(QString("INPUTS"), QJsonValue(tmpArray));
	
	//	if there's a backbuffer...
	if (usesBackbufferVar && !backbufferName.isNull())	{
		//	make a persistent buffer for it
		//tmpArray = {};
		//tmpArray.append(QJsonValue(backbufferName));
		
		//	make the last render pass target the backbuffer
		tmpObject = QJsonObject();
		tmpObject.insert(QString("TARGET"), QJsonValue(backbufferName));
		tmpObject.insert(QString("PERSISTENT"), QJsonValue(true));
		
		tmpArray = {};
		tmpArray.append(QJsonValue(tmpObject));
		
		isfDict.insert(QString("PASSES"), QJsonValue(tmpArray));
	}
	
	
	//	replace the "time" and "resolution" vars
	QString					tmpString = tmpMutString;
	QRegularExpression		timeVarRegex("([^a-zA-Z0-9_])(time)([^a-zA-Z0-9_])");
	if (tmpString.contains(timeVarRegex))
		tmpString.replace(timeVarRegex, QString("\\1TIME\\3"));
	QRegularExpression		resVarRegex("([^a-zA-Z0-9_])(resolution)([^a-zA-Z0-9_])");
	if (tmpString.contains(resVarRegex))
		tmpString.replace(resVarRegex, QString("\\1RENDERSIZE\\3"));
	
	if (declaresSurfacePosition && usesSurfacePositionVar)	{
		QRegularExpression		surfacePosVarRegex("([^a-zA-Z0-9_])(surfacePosition)([^a-zA-Z0-9_])");
		if (tmpString.contains(surfacePosVarRegex))
			tmpString.replace(surfacePosVarRegex, QString("\\1isf_FragNormCoord\\3"));
	}
	
	
	//	...finally assemble the final string
	
	return QString("/*\n%1\n*/\n\n%2").arg(QString(QJsonDocument(isfDict).toJson())).arg(tmpString);
}
