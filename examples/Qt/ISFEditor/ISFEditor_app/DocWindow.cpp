#include "DocWindow.h"
#include "ui_DocWindow.h"

#include <QJsonDocument>
#include <QDebug>
#include <QTableView>
#include <QListWidget>
#include <QFileInfo>
#include <QDir>
#include <QFileDialog>
#include <QSettings>
#include <QScrollBar>
#include <QScreen>

#include "SimpleSourceCodeEditor.h"
#include "LoadingWindow.h"
#include "ISFController.h"
#include "LevenshteinCalc.h"
#include "GLSLSandboxConverter.h"
#include "ShadertoyConverter.h"
#include "AutoUpdater.h"
#include "AboutWindow.h"
#include "Preferences.h"
#include "ReportProblemDialog.h"




#define VVDELETE(x) { if (x!=nullptr) { delete x; x=nullptr; } }
static DocWindow * globalDocWindow = nullptr;

using namespace std;
using namespace VVGL;
using namespace VVISF;




DocWindow::DocWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::DocWindow)
{
	//qDebug() << __PRETTY_FUNCTION__;
	
	globalDocWindow = this;
	
	ui->setupUi(this);
	
	//	reload the colors and syntax documents for all the text views
	reloadColorsAndSyntaxFormats();
	
	//	set up the frag shader editor so tmp files are auto-saved
	connect(ui->fragShaderEditor, &QPlainTextEdit::textChanged, [&]()	{
		lock_guard<recursive_mutex>		lock(propLock);
		//	update the ivar so we know there have been edits
		_fragEditsPerformed = true;
		//	kill the save timer if it exists
		if (_tmpFileSaveTimer != nullptr)	{
			_tmpFileSaveTimer->stop();
			delete _tmpFileSaveTimer;
			_tmpFileSaveTimer = nullptr;
		}
		//	if the file is saved in a tmp dir, start a timer to save it again in a couple seconds
		if (_fragFilePath!=nullptr && _fragFilePath->contains(QDir::tempPath()))	{
			_tmpFileSaveTimer = new QTimer(this);
			connect(_tmpFileSaveTimer, SIGNAL(timeout()), this, SLOT(tmpSaveTimerSlot()));
			_tmpFileSaveTimer->start(2000);
		}
	});
	//	set up the vert shader editor so tmp files are auto-saved
	connect(ui->vertShaderEditor, &QPlainTextEdit::textChanged, [&]()	{
		lock_guard<recursive_mutex>		lock(propLock);
		//	update the ivar so we know there have been edits
		_vertEditsPerformed = true;
		//	kill the save timer if it exists
		if (_tmpFileSaveTimer != nullptr)	{
			_tmpFileSaveTimer->stop();
			delete _tmpFileSaveTimer;
			_tmpFileSaveTimer = nullptr;
		}
		//	if the file is saved in a tmp dir, start a timer to save it again in a couple seconds
		if (_vertFilePath!=nullptr && _vertFilePath->contains(QDir::tempPath()))	{
			_tmpFileSaveTimer = new QTimer(this);
			connect(_tmpFileSaveTimer, SIGNAL(timeout()), this, SLOT(tmpSaveTimerSlot()));
			_tmpFileSaveTimer->start(2000);
		}
	});
	//	save window position on app quit
	connect(qApp, &QCoreApplication::aboutToQuit, this, &DocWindow::appQuitEvent);
}
DocWindow::~DocWindow()	{
	delete ui;
	
	lock_guard<recursive_mutex>		lock(propLock);
	
	//	kill the save timer if it exists
	if (_tmpFileSaveTimer != nullptr)	{
		_tmpFileSaveTimer->stop();
		delete _tmpFileSaveTimer;
		_tmpFileSaveTimer = nullptr;
	}
	//	clear out the old paths and file contents
	VVDELETE(_fragFilePath);
	VVDELETE(_fragFilePathContentsOnOpen);
	VVDELETE(_vertFilePath);
	VVDELETE(_vertFilePathContentsOnOpen);
	//	delete any tmp files that may exist
	
}




void DocWindow::updateContentsFromISFController()	{
	//qDebug() << __PRETTY_FUNCTION__;
	
	//ISFSceneRef		scene = GetISFController()->getScene();
	//ISFDocRef		doc = (scene==nullptr) ? nullptr : scene->doc();
	ISFController	*isfc = GetISFController();
	if (isfc == nullptr)
		return;
	ISFDocRef		doc = isfc->getCurrentDoc();
	//if (doc != nullptr)
	//	cout << "doc is " << *doc;
	
	//	create an array of the names of the doc's variables and passes
	QStringList		localVarStrings;
	if (doc != nullptr)	{
		for (const ISFAttrRef & inputAttr : doc->inputs())	{
			QString		attrName = QString::fromStdString( inputAttr->name() );
			localVarStrings.append(attrName);
		}
		for (const string & passName : doc->renderPasses())	{
			if (passName.length() > 0)	{
				QString		tmpStr = QString::fromStdString(passName);
				localVarStrings.append(tmpStr);
			}
		}
	}
	qDebug() << "\tlocalVarStrings are " << localVarStrings;
	
	lock_guard<recursive_mutex>		lock(propLock);
	
	//	kill the save timer if it exists
	if (_tmpFileSaveTimer != nullptr)	{
		_tmpFileSaveTimer->stop();
		delete _tmpFileSaveTimer;
		_tmpFileSaveTimer = nullptr;
	}
	
	//	clear out the old paths and file contents
	VVDELETE(_fragFilePath);
	VVDELETE(_fragFilePathContentsOnOpen);
	VVDELETE(_vertFilePath);
	VVDELETE(_vertFilePathContentsOnOpen);
	
	//	pass the local variable names to the text views displaying source code
	ui->fragShaderEditor->setLocalVariableNames(localVarStrings);
	ui->vertShaderEditor->setLocalVariableNames(localVarStrings);
	ui->compiledFragShader->setLocalVariableNames(localVarStrings);
	ui->compiledVertShader->setLocalVariableNames(localVarStrings);
	
	if (doc != nullptr)	{
		//	get the frag file path from the doc
		_fragFilePath = new QString( QString::fromStdString(doc->path()) );
		//qDebug() << "_fragFilePath is " << *_fragFilePath;
		//	check for a vert file by using the common recognized extensions for vert shaders
		QFileInfo		fragFileInfo(*_fragFilePath);
		QString			tmpPath = QString("%1/%2.vs").arg(fragFileInfo.dir().absolutePath()).arg(fragFileInfo.completeBaseName());
		//qDebug() << "\tchecking for vert file at " << tmpPath;
		if (QFileInfo::exists(tmpPath))	{
			//qDebug() << "\tfound the file!";
			_vertFilePath = new QString(tmpPath);
		}
		else	{
			tmpPath = QString("%1/%2.vert").arg(fragFileInfo.dir().absolutePath()).arg(fragFileInfo.completeBaseName());
			//qDebug() << "\tchecking for vert file at " << tmpPath;
			if (QFileInfo::exists(tmpPath))	{
				//qDebug() << "\tfound the file!";
				_vertFilePath = new QString(tmpPath);
			}
		}
		
		
		QString			errString;
		
		//	if there's a frag file, load its contents and populate the editor
		if (_fragFilePath != nullptr)	{
			QFile			tmpFragFile(*_fragFilePath);
			if (tmpFragFile.open(QFile::ReadOnly))	{
				QTextStream		rStream(&tmpFragFile);
				_fragFilePathContentsOnOpen = new QString( rStream.readAll() );
				tmpFragFile.close();
			}
		}
		if (_fragFilePathContentsOnOpen == nullptr)	{
			ui->fragShaderEditor->clear();
			ui->fragShaderEditor->setErrorLineNumbers(nullptr);
		}
		else	{
			//	set the contents of the frag shader editor if the text changed
			const QString			&origFragShaderTxt = ui->fragShaderEditor->toPlainText();
			if (origFragShaderTxt != *_fragFilePathContentsOnOpen)	{
				//	preserve the cursor location if the text hasn't changed significantly
				bool					preserveCursor = false;
				int						origCursorLineNumber = 0;
				
				int						maxLen = std::max(origFragShaderTxt.length(), _fragFilePathContentsOnOpen->length());
				int						minLen = std::min(origFragShaderTxt.length(), _fragFilePathContentsOnOpen->length());
				//	if there's more than a 15% change in the total # of chars
				if (double(maxLen)/double(minLen) > 1.15)	{
					//	do nothing (we do not want to preserve the cursor, the change is too great)
				}
				//	else if the levenshtein difference is < 0.25
				//else if (CalculateLevenshtein(origFragShaderTxt, *_fragFilePathContentsOnOpen) < 0.25)	{
					preserveCursor = true;
					origCursorLineNumber = ui->fragShaderEditor->textCursor().blockNumber();
				//}
				ui->fragShaderEditor->setPlainText(*_fragFilePathContentsOnOpen);
				if (preserveCursor)	{
					QTextCursor				newCursor = ui->fragShaderEditor->textCursor();
					newCursor.setPosition(0);
					for (int i=0; i<origCursorLineNumber; ++i)
						newCursor.movePosition(QTextCursor::Down);
					ui->fragShaderEditor->setTextCursor(newCursor);
				}
			}
			else	{
				//qDebug() << "frag shader hasn't changed since opened, not updating presently visible contents";
				//qDebug() << "*****************************";
				//qDebug() << "orig text is " << origFragShaderTxt;
				//qDebug() << "*****************************";
				//qDebug() << "current text is " << *_fragFilePathContentsOnOpen;
				//qDebug() << "*****************************";
			}
			//	assemble a vector containing the line numbers with errors
			QVector<int>		tmpLineNos;
			auto				tmpErrs = GetISFController()->getSceneJSONErrors();
			if (tmpErrs.size() > 0)
				errString.append("JSON errors:\n");
			for (auto tmpErrPair : tmpErrs)	{
				tmpLineNos.append(tmpErrPair.first);
				errString.append( QString::fromStdString(tmpErrPair.second) );
				errString.append("\n");
			}
			tmpErrs = GetISFController()->getSceneFragErrors();
			if (tmpErrs.size() > 0)
				errString.append("Fragment shader errros:\n");
			for (auto tmpErrPair : tmpErrs)	{
				tmpLineNos.append(tmpErrPair.first);
				errString.append( QString::fromStdString(tmpErrPair.second) );
				errString.append("\n");
			}
			
			if (tmpErrs.size() > 0)
				errString.append("\n\n");
			//	give the vector of line nmbers for errors to the frag shader editor
			ui->fragShaderEditor->setErrorLineNumbers(tmpLineNos);
		}
		
		
		//	if there's a vert file, load its contents and populate the editor
		if (_vertFilePath != nullptr)	{
			QFile			tmpVertFile(*_vertFilePath);
			if (tmpVertFile.open(QFile::ReadOnly))	{
				QTextStream		rStream(&tmpVertFile);
				_vertFilePathContentsOnOpen = new QString( rStream.readAll() );
				tmpVertFile.close();
			}
		}
		if (_vertFilePathContentsOnOpen == nullptr)	{
			ui->vertShaderEditor->clear();
			ui->vertShaderEditor->setErrorLineNumbers(nullptr);
		}
		else	{
			//	set the contents of the vert shader editor
			const QString			&origVertShaderTxt = ui->vertShaderEditor->toPlainText();
			if (origVertShaderTxt != *_vertFilePathContentsOnOpen)	{
				//	preserve the cursor location if the text hasn't changed significantly
				bool					preserveCursor = false;
				int						origCursorLineNumber = 0;
				
				int						maxLen = std::max(origVertShaderTxt.length(), _vertFilePathContentsOnOpen->length());
				int						minLen = std::min(origVertShaderTxt.length(), _vertFilePathContentsOnOpen->length());
				//	if there's more than a 15% change in the total # of chars
				if (double(maxLen)/double(minLen) > 1.15)	{
					//	do nothing (we do not want to preserve the cursor, the change is too great)
				}
				//	else if the levenshtein difference is < 0.25
				//else if (CalculateLevenshtein(origVertShaderTxt, *_vertFilePathContentsOnOpen) < 0.25)	{
					preserveCursor = true;
					origCursorLineNumber = ui->vertShaderEditor->textCursor().blockNumber();
				//}
				ui->vertShaderEditor->setPlainText(*_vertFilePathContentsOnOpen);
				if (preserveCursor)	{
					QTextCursor				newCursor = ui->vertShaderEditor->textCursor();
					newCursor.setPosition(0);
					for (int i=0; i<origCursorLineNumber; ++i)
						newCursor.movePosition(QTextCursor::Down);
					ui->vertShaderEditor->setTextCursor(newCursor);
				}
			}
			else	{
				//qDebug() << "vert shader hasn't changed since opened, not updating presently visible contents";
			}
			//	assemble a vector containing the line numbers with errors
			QVector<int>		tmpLineNos;
			auto				vertErrs = GetISFController()->getSceneVertErrors();
			if (vertErrs.size() > 0)
				errString.append("Vertex shader errors:\n");
			for (auto vertErrPair : vertErrs)	{
				tmpLineNos.append(vertErrPair.first);
				errString.append( QString::fromStdString(vertErrPair.second) );
				errString.append("\n");
			}
			//	give the vector of line numbers for errors to the frag shader editor
			ui->vertShaderEditor->setErrorLineNumbers(tmpLineNos);
		}
		
		
		if (errString.length() < 1)
			ui->compilerErrorsTextWidget->setPlainText( QString("No compiler errors!  Yay!") );
		else
			ui->compilerErrorsTextWidget->setPlainText( errString );
		//ui->compiledFragShader->setPlainText( QString::fromStdString(scene->fragmentShaderString()) );
		//ui->compiledVertShader->setPlainText( QString::fromStdString(scene->vertexShaderString()) );
		ui->compiledFragShader->setPlainText(isfc->getCompiledFragmentShaderString());
		ui->compiledVertShader->setPlainText(isfc->getCompiledVertexShaderString());
		if (doc->jsonString()==nullptr)
			ui->parsedJSON->setPlainText("");
		else
			ui->parsedJSON->setPlainText( QString::fromStdString(*doc->jsonString()) );
		
		//	if there's an error string...
		if (errString.length() > 0)	{
			//	if the error side of the splitter is collapsed, open it up halfway
			QList<int>		sizes = ui->errSplitter->sizes();
			if (sizes.size()>1 && sizes[1]<1)	{
				sizes[1] = sizes[0]/2;
				sizes[0] = sizes[1];
				ui->errSplitter->setSizes(sizes);
			}
		}
		
		//	tell the json GUI widget to load its state from the ISF controller
		ui->jsonGUIWidget->loadDocFromISFController();
	}
	else	{
		ui->fragShaderEditor->setPlainText(QString(""));
		ui->fragShaderEditor->setLocalVariableNames(QStringList());
		ui->vertShaderEditor->setPlainText(QString(""));
		ui->vertShaderEditor->setLocalVariableNames(QStringList());
		
		ui->compilerErrorsTextWidget->setPlainText( "" );
		ui->compiledFragShader->setPlainText( "" );
		ui->compiledFragShader->setLocalVariableNames(QStringList());
		ui->compiledVertShader->setPlainText( "" );
		ui->compiledVertShader->setLocalVariableNames(QStringList());
		ui->parsedJSON->setPlainText( "" );
		
		ui->jsonGUIWidget->loadDocFromISFController();
	}
}
bool DocWindow::saveOpenFile()	{
	qDebug() << __PRETTY_FUNCTION__;
	
	bool			returnMe = false;
	QString			currentFragString = ui->fragShaderEditor->toPlainText();
	QString			currentVertString = ui->vertShaderEditor->toPlainText();
	
	{
		lock_guard<recursive_mutex>		lock(propLock);
		
		//	kill the save timer if it exists
		if (_tmpFileSaveTimer != nullptr)	{
			_tmpFileSaveTimer->stop();
			delete _tmpFileSaveTimer;
			_tmpFileSaveTimer = nullptr;
		}
		
		bool			fragContentsChanged = false;
		bool			vertContentsChanged = false;
		
		if ((_fragFilePathContentsOnOpen==nullptr && currentFragString.length()>0)	||
		(_fragFilePathContentsOnOpen!=nullptr && *_fragFilePathContentsOnOpen!=currentFragString))	{
			fragContentsChanged = true;
		}
		if ((_vertFilePathContentsOnOpen==nullptr && currentVertString.length()>0)	||
		(_vertFilePathContentsOnOpen!=nullptr && *_vertFilePathContentsOnOpen!=currentVertString))	{
			vertContentsChanged = true;
		}
		
		//	if the frag file is in the tmp dir, we're going to pretend that its contents have changed so it gets written to disk
		if (!fragContentsChanged && _fragFilePath!=nullptr && _fragFilePath->contains( QDir::tempPath() ))
			fragContentsChanged = true;
		if (!vertContentsChanged && _vertFilePath!=nullptr && _vertFilePath->contains( QDir::tempPath() ))
			vertContentsChanged = true;
		
		if (!fragContentsChanged && !vertContentsChanged)	{
			qDebug() << "\tbailing- neither frag nor vert shaders changed...";
			return true;
		}
		
		//	if the file path is nil or this is a tmp file, open an alert so the user can supply a name and save location for the file
		if (_fragFilePath==nullptr || _fragFilePath->contains(QDir::tempPath()))	{
			qDebug() << "\tpresently-viewed file is a tmp file...";
			LoadingWindow	*lw = GetLoadingWindow();
			QString			pathToSave;
			if (lw == nullptr)
				pathToSave = QString("");
			else	{
				pathToSave = QFileDialog::getSaveFileName(lw,
					tr("Save shader as:"),
					lw->getBaseDirectory(),
					tr("Text (*.fs)"));
			}
			qDebug() << "\tpathToSave is " << pathToSave;
			if (pathToSave.length() < 1)	{
				//qDebug() << "\tinvalid path, don't save!";
			}
			else	{
				QFileInfo		saveFileInfo(pathToSave);
				QString			noExtPathToSave = QString("%1/%2").arg(saveFileInfo.dir().absolutePath()).arg(saveFileInfo.completeBaseName());
				qDebug() << "\tnoExtPathToSave is " << noExtPathToSave;
				
				if (fragContentsChanged)	{
					QString			localWritePath = QString("%1.fs").arg(noExtPathToSave);
					qDebug() << "\tsaving frag to file " << localWritePath;
					VVDELETE(_fragFilePath);
					_fragFilePath = new QString(localWritePath);
					VVDELETE(_fragFilePathContentsOnOpen)
					_fragFilePathContentsOnOpen = new QString(currentFragString);
				
					QFile			wFile(localWritePath);
					if (wFile.open(QIODevice::WriteOnly))	{
						QTextStream		wStream(&wFile);
						wStream << currentFragString;
						wFile.close();
						_fragEditsPerformed = false;
						returnMe = true;
					}
				}
				if (vertContentsChanged)	{
					QString			localWritePath = QString("%1.vs").arg(noExtPathToSave);
					qDebug() << "\tsaving vert to file " << localWritePath;
					VVDELETE(_vertFilePath);
					_vertFilePath = new QString(localWritePath);
					VVDELETE(_vertFilePathContentsOnOpen)
					_vertFilePathContentsOnOpen = new QString(currentVertString);
				
					QFile			wFile(localWritePath);
					if (wFile.open(QIODevice::WriteOnly))	{
						QTextStream		wStream(&wFile);
						wStream << currentVertString;
						wFile.close();
						_vertEditsPerformed = false;
						returnMe = true;
					}
				}
			
				if (fragContentsChanged || vertContentsChanged)	{
					if (lw != nullptr)
						lw->on_loadFile(pathToSave);
				}
			}
			
			
			
			
		}
		//	else the file path is non-nil and not in tmp, so just save it to disk
		else	{
			qDebug() << "\tpresently-viewed file is NOT a tmp file...";
			
			if (fragContentsChanged)	{
				if (_fragFilePath != nullptr)	{
					qDebug() << "\tsaving frag file to " << *_fragFilePath;
					VVDELETE(_fragFilePathContentsOnOpen);
					_fragFilePathContentsOnOpen = new QString(currentFragString);
					
					QFile		wFile(*_fragFilePath);
					if (wFile.open(QIODevice::WriteOnly))	{
						QTextStream		wStream(&wFile);
						wStream << currentFragString;
						wFile.close();
						_fragEditsPerformed = false;
						returnMe = true;
					}
					else	{
						qDebug() << "\tERR: could not open frag file for writing! " << *_fragFilePath;
					}
				}
			}
			if (vertContentsChanged)	{
				if (_vertFilePath != nullptr)	{
					qDebug() << "\tsaving vert file to " << *_vertFilePath;
					VVDELETE(_vertFilePathContentsOnOpen);
					_vertFilePathContentsOnOpen = new QString(currentVertString);
					
					QFile		wFile(*_vertFilePath);
					if (wFile.open(QIODevice::WriteOnly))	{
						QTextStream		wStream(&wFile);
						wStream << currentVertString;
						wFile.close();
						_vertEditsPerformed = false;
						returnMe = true;
					}
					else	{
						qDebug() << "\tERR: could not open vert file for writing! " << *_vertFilePath;
					}
				}
			}
		}
	}
	
	return returnMe;
}
void DocWindow::reloadFileFromTableView()	{
}
bool DocWindow::contentsNeedToBeSaved()	{
	lock_guard<recursive_mutex>		lock(propLock);
	bool			returnMe = false;
	if (_fragEditsPerformed)	{
		QString			currentContents = ui->fragShaderEditor->toPlainText();
		if (currentContents.length() > 0)	{
			if (_fragFilePathContentsOnOpen==nullptr || *_fragFilePathContentsOnOpen!=currentContents)	{
				returnMe = true;
			}
			else if (_fragFilePath != nullptr && _fragFilePath->startsWith(QDir::tempPath()))	{
				returnMe = true;
			}
		}
	}
	if (_vertEditsPerformed)	{
		QString			currentContents = ui->vertShaderEditor->toPlainText();
		if (currentContents.length() > 0)	{
			if (_vertFilePathContentsOnOpen==nullptr || *_vertFilePathContentsOnOpen!=currentContents)	{
				returnMe = true;
			}
			else if (_vertFilePath != nullptr && _vertFilePath->startsWith(QDir::tempPath()))	{
				returnMe = true;
			}
		}
	}
	return returnMe;
}
QString DocWindow::fragFilePath()	{
	lock_guard<recursive_mutex>		lock(propLock);
	QString		returnMe = (_fragFilePath==nullptr) ? QString("") : QString(*_fragFilePath);
	return returnMe;
}
void DocWindow::pauseAutoSaveTimer()	{
	lock_guard<recursive_mutex>		lock(propLock);
	if (_tmpFileSaveTimer != nullptr)	{
		_tmpFileSaveTimer->stop();
		delete _tmpFileSaveTimer;
		_tmpFileSaveTimer = nullptr;
	}
}
void DocWindow::reloadColorsAndSyntaxFormats()	{
	//qDebug() << __PRETTY_FUNCTION__;
	
	//	tell the source code editors to load their language files
	QFile		tmpFile(":/shader language files/shader language files/ISF_GLSL_1_5.json");
	if (tmpFile.open(QFile::ReadOnly))	{
		QJsonDocument		tmpDoc = QJsonDocument::fromJson(tmpFile.readAll());
		if (tmpDoc.isEmpty())
			qDebug() << "\terr: doc was empty";
		tmpFile.close();
		
		QSettings		settings;
		QColor			bgColor; 
		QColor			txtColor;
		QColor			lineBGColor;
		QColor			selTxtColor;
		QColor			selBGColor;
		QColor			insertColor;
		
		if (settings.contains("color_txt_txt"))
			txtColor = settings.value("color_txt_txt").value<QColor>();
		else
			txtColor = QColor("#b4b4b4");
		
		if (settings.contains("color_txt_linebg"))
			lineBGColor = settings.value("color_txt_linebg").value<QColor>();
		else
			lineBGColor = QColor("#414141");
		
		if (settings.contains("color_txt_bg"))
			bgColor = settings.value("color_txt_bg").value<QColor>();
		else
			bgColor = QColor("#1a1a1a");
		
		if (settings.contains("color_txt_seltxt"))
			selTxtColor = settings.value("color_txt_seltxt").value<QColor>();
		else
			selTxtColor = QColor("#000000");
		
		if (settings.contains("color_txt_selbg"))
			selBGColor = settings.value("color_txt_selbg").value<QColor>();
		else
			selBGColor = QColor("#ffff50");
		
		//QString			stylesheetString = "QPlainTextEdit {background-color:" + bgColor.name() + "; color:" + txtColor.name() + ";}";
		QString			stylesheetString = "QPlainTextEdit {";
		stylesheetString += " background-color:" + bgColor.name() + ";";
		stylesheetString += " color:" + txtColor.name() + ";";
		//stylesheetString += " caret-color:" + txtColor.name() + ";";
		stylesheetString += " selection-background-color:" + selBGColor.name() + ";";
		stylesheetString += " selection-color:" + selTxtColor.name() + ";";
		stylesheetString += "}";
		
		QTextCharFormat		fmt;
		fmt.setForeground(insertColor);
		
		QPalette			palette;
		palette.setColor(QPalette::Window, bgColor);
		palette.setColor(QPalette::Base, bgColor);
		palette.setColor(QPalette::WindowText, txtColor);
		palette.setColor(QPalette::Text, txtColor);
		palette.setColor(QPalette::Highlight, selBGColor);
		palette.setColor(QPalette::HighlightedText, selTxtColor);

		QScrollBar			*vScroll;
		int					vScrollVal = 0;

		vScroll = ui->fragShaderEditor->verticalScrollBar();
		if (vScroll != nullptr)
			vScrollVal = vScroll->value();
		ui->fragShaderEditor->loadSyntaxDefinitionDocument(tmpDoc);
		ui->fragShaderEditor->setStyleSheet(stylesheetString);
		ui->fragShaderEditor->setLineBGColor(lineBGColor);
		//ui->fragShaderEditor->mergeCurrentCharFormat(fmt);
		//ui->fragShaderEditor->setPalette(palette);
		ui->fragShaderEditor->setPlainText(ui->fragShaderEditor->toPlainText());
		if (vScroll != nullptr)
			vScroll->setValue(vScrollVal);
		
		vScroll = ui->vertShaderEditor->verticalScrollBar();
		if (vScroll != nullptr)
			vScrollVal = vScroll->value();
		ui->vertShaderEditor->loadSyntaxDefinitionDocument(tmpDoc);
		ui->vertShaderEditor->setStyleSheet(stylesheetString);
		ui->vertShaderEditor->setLineBGColor(lineBGColor);
		//ui->vertShaderEditor->mergeCurrentCharFormat(fmt);
		//ui->vertShaderEditor->setPalette(palette);
		ui->vertShaderEditor->setPlainText(ui->vertShaderEditor->toPlainText());
		if (vScroll != nullptr)
			vScroll->setValue(vScrollVal);
		
		vScroll = ui->compiledVertShader->verticalScrollBar();
		if (vScroll != nullptr)
			vScrollVal = vScroll->value();
		ui->compiledVertShader->loadSyntaxDefinitionDocument(tmpDoc);
		ui->compiledVertShader->setStyleSheet(stylesheetString);
		ui->compiledVertShader->setLineBGColor(lineBGColor);
		//ui->compiledVertShader->mergeCurrentCharFormat(fmt);
		//ui->compiledVertShader->setPalette(palette);
		ui->compiledVertShader->setPlainText(ui->compiledVertShader->toPlainText());
		if (vScroll != nullptr)
			vScroll->setValue(vScrollVal);
		
		vScroll = ui->compiledFragShader->verticalScrollBar();
		if (vScroll != nullptr)
			vScrollVal = vScroll->value();
		ui->compiledFragShader->loadSyntaxDefinitionDocument(tmpDoc);
		ui->compiledFragShader->setStyleSheet(stylesheetString);
		ui->compiledFragShader->setLineBGColor(lineBGColor);
		//ui->compiledFragShader->mergeCurrentCharFormat(fmt);
		//ui->compiledFragShader->setPalette(palette);
		ui->compiledFragShader->setPlainText(ui->compiledFragShader->toPlainText());
		if (vScroll != nullptr)
			vScroll->setValue(vScrollVal);
	}
	else
		qDebug() << "ERR: couldn't open shader lang files, " << __PRETTY_FUNCTION__;
}









void DocWindow::on_actionNew_triggered()	{
	//qDebug() << __PRETTY_FUNCTION__;
	LoadingWindow		*lw = GetLoadingWindow();
	if (lw != nullptr)
		lw->on_createNewFile(true);
}
void DocWindow::on_actionSave_triggered()	{
	//qDebug() << __PRETTY_FUNCTION__;
	if (!saveOpenFile())	{
		QMessageBox::warning(GetDocWindow(), "", "The file could not be saved", QMessageBox::Ok);
	}
}
void DocWindow::on_actionImport_from_GLSLSandbox_triggered()	{
	//qDebug() << __PRETTY_FUNCTION__;

	GLSLSandboxConverter		*conv = new GLSLSandboxConverter(GetLoadingWindow());
	int				returnCode = conv->exec();
	//qDebug() << "returnCode is " << returnCode;
	if (!returnCode)	{
		LoadingWindow		*lw = GetLoadingWindow();
		if (lw != nullptr)	{
			lw->finishedConversionDisplayFile(conv->exportedISFPath());
		}
	}
	delete conv;
}
void DocWindow::on_actionImport_from_Shadertoy_triggered()	{
	//qDebug() << __PRETTY_FUNCTION__;

	ShadertoyConverter		*conv = new ShadertoyConverter(GetLoadingWindow());
	int				returnCode = conv->exec();
	//qDebug() << "returnCode is " << returnCode;
	if (!returnCode)	{
		LoadingWindow		*lw = GetLoadingWindow();
		if (lw != nullptr)	{
			lw->finishedConversionDisplayFile(conv->exportedISFPath());
		}
	}
	delete conv;
}
void DocWindow::on_actionPreferences_triggered()	{
	//qDebug() << __PRETTY_FUNCTION__;
	Preferences		*p = GetPreferences();
	if (p != nullptr)	{
		p->show();
	}
}
void DocWindow::on_actionQuit_triggered()	{
	//qDebug() << __PRETTY_FUNCTION__;
	QCoreApplication::quit();
}




void DocWindow::on_actionFind_triggered()	{
	SimpleSourceCodeEditor		*focusedEd = focusedSourceCodeEditor();
	if (focusedEd == nullptr)
		return;
	
	focusedEd->openFindDialog();
}
void DocWindow::on_actionFind_Previous_triggered()	{
	SimpleSourceCodeEditor		*focusedEd = focusedSourceCodeEditor();
	if (focusedEd == nullptr)
		return;
	
	focusedEd->findPrevious();
}
void DocWindow::on_actionFind_Next_triggered()	{
	SimpleSourceCodeEditor		*focusedEd = focusedSourceCodeEditor();
	if (focusedEd == nullptr)
		return;
	
	focusedEd->findNext();
}
void DocWindow::on_actionUse_selection_for_next_Find_triggered()	{
	SimpleSourceCodeEditor		*focusedEd = focusedSourceCodeEditor();
	if (focusedEd == nullptr)
		return;
	
	focusedEd->setFindStringFromCursor();
}








void DocWindow::on_actionCheck_for_Updates_triggered()	{
	//qDebug() << __PRETTY_FUNCTION__;
	AutoUpdater		*aa = GetGlobalAutoUpdater();
	if (aa != nullptr)	{
		aa->checkForUpdates();
	}
	else	{
		qDebug() << "ERR: global auto updater nil!";
	}
}
void DocWindow::on_actionAbout_triggered()	{
	//qDebug() << __PRETTY_FUNCTION__;
	AboutWindow		*aw = GetAboutWindow();
	if (aw != nullptr)	{
		aw->show();
	}
}
void DocWindow::on_actionGet_Help_triggered()	{
	//qDebug() << __PRETTY_FUNCTION__;
	ReportProblemDialog		*rpd = GetReportProblemDialog();
	if (rpd != nullptr)	{
		rpd->show();
	}
}








void DocWindow::closeEvent(QCloseEvent * event)	{
	QSettings		settings;
	settings.setValue("DocWindowGeometry", saveGeometry());
	
	QWidget::closeEvent(event);
	
	//	tell the app to quit!
	QCoreApplication::quit();
}
void DocWindow::showEvent(QShowEvent * event)	{
	//qDebug() << __PRETTY_FUNCTION__;
	
	//Q_UNUSED(event);
	QWidget::showEvent(event);
	
	//	configure the splitter
	ui->splitter->setCollapsible(2, false);
	QWidget		*jsonTableWidget = ui->splitter->widget(2);
	if (jsonTableWidget != nullptr)	{
		jsonTableWidget->setMinimumSize(QSize(335,335));
	}
	
	QList<int>		tmpSizes;
	tmpSizes.append(99999);
	tmpSizes.append(0);
	tmpSizes.append(0);
	ui->splitter->setSizes(tmpSizes);
	
	//	configure the compiler errors widget
	QFont		tmpFont;
	tmpFont.setFamily("Courier");
	tmpFont.setFixedPitch(true);
#if defined(Q_OS_MAC)
	tmpFont.setPointSize(12);
#else
	tmpFont.setPointSize(9);
#endif
	ui->compilerErrorsTextWidget->setFont(tmpFont);
	
	//	all of the various error/shader/parsed json widgets are read-only...
	ui->compilerErrorsTextWidget->setReadOnly(true);
	ui->compiledVertShader->setReadOnly(true);
	ui->compiledFragShader->setReadOnly(true);
	ui->parsedJSON->setReadOnly(true);
	
	//	configure the error splitter
	tmpSizes.clear();
	tmpSizes.append(9999999);
	tmpSizes.append(0);
	ui->errSplitter->setSizes(tmpSizes);
	
	//	restore the window position
	QSettings		settings;
	if (settings.contains("DocWindowGeometry"))	{
		restoreGeometry(settings.value("DocWindowGeometry").toByteArray());
	}
	else	{
		//QWidget			*window = window();
		//if (window != nullptr)	{
			QRect		winFrame = frameGeometry();
			QRect		contentFrame = geometry();
			QPoint		contentOffset = contentFrame.topRight() - winFrame.topRight();
			QScreen		*screen = QGuiApplication::screenAt(winFrame.center());
			if (screen != nullptr)	{
				QRect		screenFrame = screen->geometry();
				winFrame.moveTopRight(screenFrame.topRight());
				winFrame.translate(contentOffset.x(), contentOffset.y());
				setGeometry(winFrame);
			}
		//}
	}
	
	//	set myself as the parent window for the auto updater!
	AutoUpdater		*aa = GetGlobalAutoUpdater();
	if (aa != nullptr)	{
		aa->setParentWindow(this);
	}
	
}
void DocWindow::appQuitEvent()	{
	//qDebug() << __PRETTY_FUNCTION__;
	
	QSettings		settings;
	settings.setValue("DocWindowGeometry", saveGeometry());
}








SimpleSourceCodeEditor * DocWindow::focusedSourceCodeEditor()	{
	//	run through the various source code editors, looking for one that has keyboard focus
	SimpleSourceCodeEditor		*focusedEd = nullptr;
	if (ui->fragShaderEditor->hasFocus())
		focusedEd = ui->fragShaderEditor;
	else if (ui->vertShaderEditor->hasFocus())
		focusedEd = ui->vertShaderEditor;
	else if (ui->compiledVertShader->hasFocus())
		focusedEd = ui->compiledVertShader;
	else if (ui->compiledFragShader->hasFocus())
		focusedEd = ui->compiledFragShader;
	else if (ui->parsedJSON->hasFocus())
		focusedEd = ui->parsedJSON;
	return focusedEd;
}
void DocWindow::tmpSaveTimerSlot()	{
	//qDebug() << __PRETTY_FUNCTION__;
	
	QString			currentFragString = ui->fragShaderEditor->toPlainText();
	QString			currentVertString = ui->vertShaderEditor->toPlainText();
	
	{
		lock_guard<recursive_mutex>		lock(propLock);
		
		//	kill the save timer if it exists
		if (_tmpFileSaveTimer != nullptr)	{
			_tmpFileSaveTimer->stop();
			delete _tmpFileSaveTimer;
			_tmpFileSaveTimer = nullptr;
		}
		
		if (_fragFilePath!=nullptr && _fragFilePath->contains( QDir::tempPath() ))	{
			VVDELETE(_fragFilePathContentsOnOpen)
			_fragFilePathContentsOnOpen = new QString(currentFragString);
			
			QFile			wFile(*_fragFilePath);
			if (wFile.open(QIODevice::WriteOnly))	{
				QTextStream		wStream(&wFile);
				wStream << currentFragString;
				wFile.close();
			}
		}
		
		if (_vertFilePath!=nullptr && _vertFilePath->contains( QDir::tempPath() ))	{
			VVDELETE(_vertFilePathContentsOnOpen)
			_vertFilePathContentsOnOpen = new QString(currentVertString);
			
			QFile			wFile(*_vertFilePath);
			if (wFile.open(QIODevice::WriteOnly))	{
				QTextStream		wStream(&wFile);
				wStream << currentVertString;
				wFile.close();
			}
		}
	}
}








DocWindow * GetDocWindow()	{
	return globalDocWindow;
}
