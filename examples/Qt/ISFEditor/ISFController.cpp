#include "ISFController.h"

#include <QMessageBox>
#include <QDebug>
#include <QSharedPointer>
#include <QScrollArea>
#include <QLayout>

#include <vector>
#include <utility>
#include <regex>
#include <sstream>

#include "LoadingWindow.h"
#include "DocWindow.h"
#include "OutputWindow.h"
#include "DynamicVideoSource.h"




static ISFController * globalISFController = nullptr;

using namespace std;
using namespace VVGL;
using namespace VVISF;




ISFController::ISFController()	{
	qDebug() << __PRETTY_FUNCTION__;
	
	globalISFController = this;
	connect(qApp, SIGNAL(aboutToQuit()), this, SLOT(aboutToQuit()));
	
	//	the output window has a buffer view which emits a signal every redraw- use that to drive rendering
	OutputWindow		*ow = GetOutputWindow();
	GLBufferQWidget		*bufferView = (ow==nullptr) ? nullptr : ow->bufferView();
	if (bufferView == nullptr)
		qDebug() << "ERR: bufferView nil in " << __PRETTY_FUNCTION__;
	else
		connect(bufferView, &GLBufferQWidget::aboutToRedraw, this, &ISFController::widgetRedrawSlot);
}
ISFController::~ISFController()	{
	//	we have to explicitly free the spacer item
	if (spacerItem != nullptr)	{
		LoadingWindow			*lw = GetLoadingWindow();
		QScrollArea				*scrollArea = (lw==nullptr) ? nullptr : lw->getScrollArea();
		QWidget					*scrollWidget = (scrollArea==nullptr) ? nullptr : scrollArea->widget();
		QLayout					*scrollLayout = (scrollWidget==nullptr) ? nullptr : scrollWidget->layout();
		if (scrollLayout != nullptr)	{
			scrollLayout->removeItem(spacerItem);
		}
		
		delete spacerItem;
		spacerItem = nullptr;
	}
}


void ISFController::aboutToQuit()	{
	std::lock_guard<std::recursive_mutex>		tmpLock(sceneLock);
	scene = nullptr;
}
void ISFController::loadFile(const QString & inPathToLoad)	{
	//qDebug() << __PRETTY_FUNCTION__;
	
	if (GetGlobalBufferPool() == nullptr)
		return;
	
	std::lock_guard<std::recursive_mutex>		tmpLock(sceneLock);
	
	if (scene == nullptr)
		scene = CreateISFSceneRef();
	if (scene == nullptr)
		return;
	
	sceneVertErrors.clear();
	sceneFragErrors.clear();

	scene->setThrowExceptions(true);
	//	tell the scene to load the file, catch exceptions so we can throw stuff
	try	{
		scene->useFile(inPathToLoad.toStdString());
	}
	catch (ISFErr & exc)	{
		QString		errString = QString("%1, %2").arg(QString::fromStdString(exc.general), QString::fromStdString(exc.specific));
		QMessageBox::warning(GetLoadingWindow(), "", errString, QMessageBox::Ok);
	}
	catch (...)	{
	}
	
	
	
	/*
	if (QThread::currentThread() == qApp->thread())
		qDebug() << "\tcurrent thread in load file is main thread";
	else
		qDebug() << "\tcurrent thread in load file is NOT main thread!";
	*/
	//	tell the scene to render a frame, so the ISFController can pull its compiled shaders and populate its UI items
	try	{
		scene->createAndRenderABuffer();
	}
	catch (ISFErr & exc)	{
		//QString		errString = QString("%1, %2").arg(QString::fromStdString(exc.general), QString::fromStdString(exc.specific));
		//QMessageBox::warning(GetLoadingWindow(), "", errString, QMessageBox::Ok);
		//qDebug() << "\tERR: caught exception rendering first frame, " << __PRETTY_FUNCTION__;
		
		
		
		//	this block finds the "line number" in the pased string, addes "line delta" to it, and optionally adds both to a vector of syntax errors
		auto		GLSLErrLogLineNumberChanger = [](string * _lineIn, int _lineDelta, vector<pair<int,string>> * _syntaxErrArray)	{
			//qDebug() << "GLSLErrLogLineNumberChanger()";
			//	if the passed line was null just return immediately
			if (_lineIn==nullptr || _lineIn->length()<1)
				return std::string("");
			regex			regex("([0-9]+[\\W])([0-9]+)");
			smatch			matches;
			//	if i couldn't find any matches in the passed line or i found the wrong # of matches just return the passed line
			if (!regex_search(*_lineIn, matches, regex))	{
				//qDebug() << "\tdidn't find a line number in this line...";
				return *_lineIn;
			}
			if (matches.size() != 3)	{
				qDebug() << "\tERR: incorrect num elements, cannot correct line nos. in " << __PRETTY_FUNCTION__;
				return *_lineIn;
			}
			
			//	...if i'm here i captured the vals and i need to construct a new line...
			
			string			errString("");
			
			//	first, copy everything in the line *before* the regex
			errString.append( matches.prefix() );
			//	now copy the first thing i captured (index 1 in the array)- this is presumably the file number
			errString.append(matches[1]);
			//	the second thing i captured (index 2 in the array) is the line number- modify it, then add it to the string
			int			tmpInt = stoi(matches[2]) + _lineDelta;
			errString.append( FmtString("%d", tmpInt) );
			//	copy everything in the line *after* the regex
			errString.append( matches.suffix() );
			
			//	if we need to record the syntax errors in an array...
			if (_syntaxErrArray != nullptr)	{
				//	now make a pair that associates the corrected line number with the corrected line number string, and add it to the array
				_syntaxErrArray->emplace_back( pair<int,string>(tmpInt, errString) );
			}
			
			//cout << "\terrString is \"" << errString << "\"" << endl;
			return errString;
		};
		
		
		
		
		string					errString("");
		map<string,string>		*details = &exc.details;
		//	if there's a vertex error log, parse it line-by-line, adjusting the line numbers to cmpensate for changes to the shader
		auto					tmpLogIt = details->find("vertErrLog");
		if (tmpLogIt != details->end())	{
			//qDebug() << "\tvert err log...";
			string		*tmpLog = &tmpLogIt->second;
			//cout << "\tvert err log is " << *tmpLog << endl;
			
			//	figure out the difference in line numbers between the compiled vert shader and the raw ISF file
			int					lineDelta = 0;
			//string				compiledVertSrc = (*details)["vertSrc"];
			auto				compiledVertSrcIt = details->find("vertSrc");
			const string		&compiledVertSrc = compiledVertSrcIt->second;
			ISFDocRef			tmpDoc = scene->getDoc();
			string				*precompiledVertSrc = (tmpDoc==nullptr) ? nullptr : tmpDoc->getVertShaderSource();
			
			if (precompiledVertSrc != nullptr)	{
				//	the compiled vertex shader has stuff added to both the beginning and the end- the first line added to the end of the raw vertex shader source is:
				string				firstAppendedLine("\nvoid vv_vertShaderInit(void)\t{");
				auto				firstAppendedLineIndex = compiledVertSrc.find(firstAppendedLine);
				if (firstAppendedLineIndex != std::string::npos)	{
					string				appendedString = compiledVertSrc.substr(firstAppendedLineIndex);
					int					numberOfAppendedLines = NumLines(appendedString);
					lineDelta = NumLines(*precompiledVertSrc) - (NumLines(compiledVertSrc) - numberOfAppendedLines);
				}
				else	{
					firstAppendedLine = string("\nvoid isf_vertShaderInit(void)\t{");
					firstAppendedLineIndex = compiledVertSrc.find(firstAppendedLine);
					if (firstAppendedLineIndex != std::string::npos)	{
						string				appendedString = compiledVertSrc.substr(firstAppendedLineIndex);
						int					numberOfAppendedLines = NumLines(appendedString);
						lineDelta = NumLines(*precompiledVertSrc) - (NumLines(compiledVertSrc) - numberOfAppendedLines);
					}
					else	{
						cout << "\tERR: couldnt find beginning of addition to vert shader, " << __PRETTY_FUNCTION__ << endl;
					}
				}
				
				//	run through each line of the log, adjusting the line numbers
				istringstream		ss(*tmpLog);
				string				tmpLine;
				while (getline(ss, tmpLine))	{
					string		modString = GLSLErrLogLineNumberChanger(&tmpLine, lineDelta, &sceneVertErrors);
					//cout << "\tmod vert err log is " << modString << endl;
				}
				
			}
		}
		
		
		
		
		//	if there's a fragment error log, parse it line-by-line, adjusting the line numbers to cmpensate for changes to the shader
		tmpLogIt = details->find("fragErrLog");
		if (tmpLogIt != details->end())	{
			//qDebug() << "\tfrag err log...";
			string		*tmpLog = &tmpLogIt->second;
			//cout << "\tfrag err log is " << *tmpLog << endl;
			
			//	figure out the difference in line numbers between the compield frag shader and the raw ISF file
			int					lineDelta = 0;
			auto				compiledFragSrcIt = details->find("fragSrc");
			const string		&compiledFragSrc = compiledFragSrcIt->second;
			ISFDocRef			tmpDoc = scene->getDoc();
			string				*precompiledFragSrc = (tmpDoc==nullptr) ? nullptr : tmpDoc->getFragShaderSource();
			string				*jsonSrc = (tmpDoc==nullptr) ? nullptr : tmpDoc->getJSONSourceString();
			if (precompiledFragSrc!=nullptr && jsonSrc!=nullptr)	{
				int		precompiledLineCount = NumLines(*precompiledFragSrc);
				int		jsonLineCount = NumLines(*jsonSrc);
				int		compiledLineCount = NumLines(compiledFragSrc);
				//cout << "\tprecompLineCount " << precompiledLineCount << " jsonLineCount " << jsonLineCount << " compLineCount " << compiledLineCount << endl;
				lineDelta = (precompiledLineCount + jsonLineCount) - compiledLineCount;
				//cout << "\tlineDelta is " << lineDelta << endl;
				//lineDelta = (PRECOMPILED_NUM_LINES + JSON_NUM_LINES) - COMPILED_NUM_LINES;
			}
			
			//	run through each line of the log, adjusting the line numbers
			istringstream		ss(*tmpLog);
			string				tmpLine;
			while (getline(ss, tmpLine))	{
				string		modString = GLSLErrLogLineNumberChanger(&tmpLine, lineDelta, &sceneFragErrors);
				//cout << "\tmod frag err log is " << modString << endl;
			}
			
		}
		
		
		
	}
	catch (...)	{
	}
	
	
	
	//	populate the loading window- this creates the UI items, populates text fields, etc
	populateLoadingWindowUI();
	
	//	tell the doc window to update its contents
	GetDocWindow()->updateContentsFromISFController();
}

void ISFController::widgetRedrawSlot(GLBufferQWidget * n)	{
	//qDebug() << __PRETTY_FUNCTION__;
	
	//	get a "source buffer" from the dynamic video source
	DynamicVideoSource		*dvs = GetDynamicVideoSource();
	if (dvs == nullptr)
		return;
	GLBufferRef				newSrcBuffer = dvs->getBuffer();
	
	lock_guard<recursive_mutex>		tmpLock(sceneLock);
	
	//	if there's no scene, display the source buffer and bail
	if (scene == nullptr)	{
		n->drawBuffer(newSrcBuffer);
		return;
	}
	
	//	if there's no doc or a null doc, display the source buffer and bail
	ISFDocRef	tmpDoc = scene->getDoc();
	if (tmpDoc == nullptr)	{
		n->drawBuffer(newSrcBuffer);
		return;
	}
	string		scenePath = tmpDoc->getPath();
	if (scenePath.length() < 1)	{
		n->drawBuffer(newSrcBuffer);
		return;
	}
	
	//	apply the source buffer to the scene as "inputImage"
	scene->setBufferForInputNamed(newSrcBuffer, string("inputImage"));
	
	//	run through the UI items, pushing their values to the scene
	for (const QPointer<ISFUIItem> & itemPtr : sceneItemArray)	{
		if (itemPtr.isNull())
			continue;
		//itemPtr.data()
		QString		itemName = itemPtr.data()->getName();
		ISFVal		itemVal = itemPtr.data()->getISFVal();
		if (itemVal.getType() != ISFValType_None)
			scene->setValueForInputNamed(itemVal, itemName.toStdString());
	}
	
	//	render the scene!
	GLBufferRef						newBuffer = nullptr;
	map<int32_t,GLBufferRef>		tmpPassDict;
	try	{
		Size		tmpSize = renderSize;
		if (sceneIsFilter && newSrcBuffer!=nullptr)
			tmpSize = newSrcBuffer->srcRect.size;
		newBuffer = scene->createAndRenderABuffer(tmpSize, &tmpPassDict, GetGlobalBufferPool());
	}
	catch (ISFErr & exc)	{
		cout << "ERR: " << __PRETTY_FUNCTION__ << "-> caught exception: " << exc.getTypeString() << ": " << exc.general << ", " << exc.specific << endl;
		cout << "\tgen: " << exc.general << endl;
		cout << "\tspec: " << exc.specific << endl;
		map<string,string>		&details = exc.details;
		for (const auto & it : details)	{
			cout << "\t\t" << it.first << " : " << it.second << endl;
		}
	}
	
	n->drawBuffer(newBuffer);
	//cout << "newBuffer is " << *newBuffer << endl;
	//cout << "tmp pass dict:\n";
	//for (const auto & it : tmpPassDict)	{
	//	cout << "\t" << it.first << " : " << *it.second << endl;
	//}
}

void ISFController::populateLoadingWindowUI()	{
	//	get the loading window, bail if we can't
	LoadingWindow			*lw = GetLoadingWindow();
	if (lw == nullptr)
		return;
	
	//	get the scroll area's widget and its layout, which is where we're going to be adding stuff
	QScrollArea				*scrollArea = lw->getScrollArea();
	QWidget					*scrollWidget = (scrollArea==nullptr) ? nullptr : scrollArea->widget();
	QLayout					*scrollLayout = (scrollWidget==nullptr) ? nullptr : scrollWidget->layout();
	if (scrollLayout == nullptr)
		return;
	
	//	get a lock
	lock_guard<recursive_mutex>		tmpLock(sceneLock);
	for (const QPointer<ISFUIItem> sceneItemPtr : sceneItemArray)	{
		if (sceneItemPtr.isNull())
			continue;
		//	remove the widget from the layout and hide it
		scrollLayout->removeWidget(sceneItemPtr.data());
		sceneItemPtr->hide();
		//	remove the widget's layout item from the layout
		QLayoutItem			*itemLayoutItem = scrollLayout->itemAt(scrollLayout->indexOf(sceneItemPtr.data()));
		if (itemLayoutItem != nullptr)
			scrollLayout->removeItem(itemLayoutItem);
		//	delete the widget
		delete sceneItemPtr.data();
	}
	//	clear the array of items
	sceneItemArray.clear();
	//	if there's a spacer item, remove it from the layout
	if (spacerItem != nullptr)	{
		scrollLayout->removeItem(spacerItem);
	}
	//	else there's no spacer item- create one
	else	{
		spacerItem = new QSpacerItem(10, 10, QSizePolicy::Expanding, QSizePolicy::Expanding);
	}
	
	//	if something's wrong and there's no scene, bail here
	if (scene == nullptr)
		return;
	
	//	run through the scene's inputs- we want to make a UI item for each...
	vector<ISFAttrRef>		sceneInputs = scene->getInputs();
	for (auto it=sceneInputs.rbegin(); it!=sceneInputs.rend(); ++it)	{
		ISFAttrRef		attrib = *it;
		if (attrib == nullptr)
			continue;
		//	if the attrib ISN'T the image filter's input image...
		if (!attrib->getIsFilterInputImage())	{
			//	make the UI item, store a weak ptr to it in the vector, add the UI item to the layout
			ISFUIItem		*newWidget = new ISFUIItem(attrib);
			QPointer<ISFUIItem>			newWidgetPtr(newWidget);
			sceneItemArray.append(newWidgetPtr);
			scrollLayout->addWidget(newWidgetPtr);
		}
	}
	
	//	add the spacer at the bottom so the UI items are pushed to the top of the scroll area
	if (spacerItem != nullptr)	{
		scrollLayout->addItem(spacerItem);
	}
	
	//	populate the render res spinboxes!
	Size			renderSize(1,1);
	if (scene != nullptr)
		renderSize = scene->getOrthoSize();
	QSpinBox		*tmpWidget = nullptr;
	tmpWidget = lw->getWidthSB();
	if (tmpWidget != nullptr)
		tmpWidget->setValue(renderSize.width);
	tmpWidget = lw->getHeightSB();
	if (tmpWidget != nullptr)
		tmpWidget->setValue(renderSize.height);
}
void ISFController::pushNormalizedMouseClickToPoints(const Size & inSize)	{
}
void ISFController::reloadTargetFile()	{
}




ISFController * GetISFController()	{
	if (globalISFController == nullptr)
		globalISFController = new ISFController();
	return globalISFController;
}
