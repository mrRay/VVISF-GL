#include "OutputWindow.h"
#include "ui_OutputWindow.h"

#include <QDebug>
#include <QSettings>
#include <QScreen>

#include "ISFPassTarget.hpp"
#include "ISFController.h"




using namespace VVGL;
using namespace VVISF;
static OutputWindow * globalOutputWindow = nullptr;




OutputWindow::OutputWindow(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::OutputWindow)
{
	//qDebug() << __PRETTY_FUNCTION__;

	interAppOutput = new InterAppOutput();

	//	disable the close button!
	setWindowFlags((windowFlags() | Qt::CustomizeWindowHint) & (~Qt::WindowCloseButtonHint));
	
	ui->setupUi(this);
	
	globalOutputWindow = this;
	
	//	we want to know when the widget draws its first frame because we can't create the global shared context/buffer pool until we can get a base ctx from the widget
	//connect(ui->bufferView, SIGNAL(frameSwapped()), this, SLOT(widgetDrewItsFirstFrame()));
	
	//	we need to shut stuff down and delete contexts gracefull on quit
	connect(qApp, SIGNAL(aboutToQuit()), this, SLOT(aboutToQuit()));
	
	//	the gl widget publishes a signal when the user clicks & drags in it
	connect(ui->bufferView, &MouseEventISFWidget::mouseMoved, [&](VVGL::Point normMouseEventLoc, VVGL::Point absMouseEventLoc)	{
		emit outputWindowMouseMoved(normMouseEventLoc, absMouseEventLoc);
	});
	
	//	restore the window position
	QSettings		settings;
	if (settings.contains("OutputWindowGeometry"))	{
		restoreGeometry(settings.value("OutputWindowGeometry").toByteArray());
	}
	else	{
		QWidget			*window = this->window();
		if (window != nullptr)	{
			QRect		winFrame = window->frameGeometry();
			QScreen		*screen = QGuiApplication::screenAt(winFrame.center());
			if (screen != nullptr)	{
				QRect		screenFrame = screen->geometry();
				winFrame.moveBottomRight(screenFrame.bottomRight());
				window->setGeometry(winFrame);
			}
		}
	}
	
	//	tell the widget to draw a single frame.  for some reason, GL widgets on os x don't have their internal sizes set properly when they draw their first frame.
	//ui->bufferView->drawBuffer(nullptr);
	//	tell the buffer view to start rendering
	ui->bufferView->startRendering();
}

OutputWindow::~OutputWindow()	{
	delete ui;
}

ISFGLBufferQWidget * OutputWindow::bufferView()	{
	return ui->bufferView;
}
void OutputWindow::drawBuffer(const VVGL::GLBufferRef & n)	{
	if (n == nullptr)
		return;
	ui->bufferView->drawBuffer(n);
	QString			resText = QString("%1 x %2").arg(int(n->srcRect.size.width)).arg(int(n->srcRect.size.height));
	if (ui->imageResLabel->text() != resText)	{
		//perform_async([&,resText](){
			ui->imageResLabel->setText(resText);
		//}, ui->imageResLabel);
	}
	
	if (interAppOutput != nullptr)
		interAppOutput->publishBuffer(n);
	
}
void OutputWindow::updateContentsFromISFController()	{
	//	get the ISFController, and from it retrieve the doc that is currently loaded
	ISFController		*isfc = GetISFController();
	if (isfc == nullptr)
		return;
	//ISFSceneRef			tmpScene = isfc->getScene();
	//ISFDocRef			tmpDoc = (tmpScene==nullptr) ? nullptr : tmpScene->doc();
	ISFDocRef			tmpDoc = isfc->getCurrentDoc();
	int					maxPassCount = (tmpDoc==nullptr) ? 0 : (int)tmpDoc->renderPasses().size();
	int					imageInputsCount = (tmpDoc==nullptr) ? 0 : (int)tmpDoc->imageInputs().size();
	int					audioInputsCount = (tmpDoc==nullptr) ? 0 : (int)tmpDoc->audioInputs().size();
	
	//	update the pop-up button with the list of rendering passes
	QComboBox		*cb = ui->renderPassComboBox;
	if (cb != nullptr)	{
		cb->blockSignals(true);
	
		//	'main output' menu item has a variant of -1
		cb->clear();
		cb->addItem(QString("Main Output"), QVariant(-1));
	
		//	pass menu items have variants starting at 0
		if (maxPassCount > 1)	{
			cb->insertSeparator(cb->count());
		
			for (int i=0; i<maxPassCount; ++ i)	{
				QString		passName = QString::fromStdString(tmpDoc->renderPasses().at(i));
				if (passName.size() == 0)
					passName = QString("PASSINDEX %1").arg(i);
				cb->addItem(passName, QVariant(i));
			}
		}
		//	image input menu items have variants starting at 100
		if (imageInputsCount > 0)	{
			cb->insertSeparator(cb->count());
		
			for (int i=0; i<imageInputsCount; ++i)	{
				ISFAttrRef	inputRef = tmpDoc->imageInputs().at(i);
				QString		inputName = (inputRef==nullptr) ? QString("") : QString::fromStdString(inputRef->name());
				cb->addItem(inputName, QVariant(100+i));
			}
		}
		//	image input menu items have variants starting at 200
		if (audioInputsCount > 0)	{
			cb->insertSeparator(cb->count());
		
			for (int i=0; i<audioInputsCount; ++i)	{
				ISFAttrRef	inputRef = tmpDoc->audioInputs().at(i);
				QString		inputName = (inputRef==nullptr) ? QString("") : QString::fromStdString(inputRef->name());
				cb->addItem(inputName, QVariant(200+i));
			}
		}
	
		cb->blockSignals(false);
	}
	
}
int OutputWindow::selectedIndexToDisplay()	{
	return ui->renderPassComboBox->currentData().toInt();
}


void OutputWindow::closeEvent(QCloseEvent * event)	{
	QSettings		settings;
	settings.setValue("OutputWindowGeometry", saveGeometry());
	
	QWidget::closeEvent(event);
}
void OutputWindow::showEvent(QShowEvent * event)	{
	QWidget::showEvent(event);
	
	ui->displayAlphaToggle->setChecked(false);
}
void OutputWindow::moveEvent(QMoveEvent * ev)
{
	QWidget::moveEvent(ev);
	
	double			ltbbm = devicePixelRatio();
	const QSize	&	tmpQSize = ui->bufferView->size();
	Size			tmpSize(tmpQSize.width()*ltbbm, tmpQSize.height()*ltbbm);
	
	//lock_guard<recursive_mutex>		lock(ctxLock);
	//scene->setOrthoSize(tmpSize);
	ISFSceneRef		viewScene = ui->bufferView->getScene();
	if (viewScene != nullptr)	{
		viewScene->setSize(tmpSize);
	}
}
/*
void OutputWindow::showEvent(QShowEvent * event)	{
	qDebug() << __PRETTY_FUNCTION__;
	
	Q_UNUSED(event);
	QWidget::showEvent(event);
	
}
*/


void OutputWindow::on_freezeOutputToggle_stateChanged(int arg1)	{
	Q_UNUSED(arg1);
	freezeOutputFlag = ui->freezeOutputToggle->isChecked();
}

void OutputWindow::on_displayAlphaToggle_stateChanged(int arg1)	{
	Q_UNUSED(arg1);
	bool			newVal = ui->displayAlphaToggle->isChecked();
	
	ISFSceneRef		viewScene = ui->bufferView->getScene();
	if (viewScene != nullptr)	{
		ISFDocRef		viewDoc = viewScene->doc();
		if (viewDoc != nullptr)	{
			ISFAttrRef		alphaAttr = viewDoc->input(std::string("viewAlpha"));
			if (alphaAttr != nullptr)
				alphaAttr->setCurrentVal(ISFBoolVal(newVal));
		}
	}
}
/*
void OutputWindow::widgetDrewItsFirstFrame()	{
	//qDebug() << __PRETTY_FUNCTION__;
	
	//if (QThread::currentThread() == qApp->thread())
	//	qDebug() << "\tcurrent thread in widget draw first frame is main thread";
	//else
	//	qDebug() << "\tcurrent thread in widget draw first frame is NOT main thread!";
	
	//	get the widget's context- if it's null, the widget's context doesn't exist yet and this method shouldn't have been called!
	GLContextRef		widgetCtx = ui->bufferView->glContextRef();
	if (widgetCtx == nullptr)
		return;
	
	//cout << "\tversion is " << GLVersionToString(widgetCtx->version) << endl;
	
	//	disconnect immediately- we're only doing this because we need to create the shared context from the widget's context
	disconnect(ui->bufferView, SIGNAL(frameSwapped()), this, SLOT(widgetDrewItsFirstFrame()));
	
	//	make the global buffer pool, using a newly-created context that shares the base global context
	CreateGlobalBufferPool(widgetCtx->newContextSharingMe());
	
	//	don't tell the widget to start rendering- doing so will cause it to start rendering at 60fps
	//ui->bufferView->startRendering();
	
	//	tell the widget to draw a single frame.  for some reason, GL widgets on os x don't have their internal sizes set properly when they draw their first frame.
	ui->bufferView->drawBuffer(nullptr);
}
*/
void OutputWindow::aboutToQuit()	{
	qDebug() << __PRETTY_FUNCTION__;
	
	QSettings		settings;
	settings.setValue("OutputWindowGeometry", saveGeometry());
	
	//	we want to clear all the rendering resources- if we don't, Qt will crash on quit, which is harmless but tacky and should be avoided
	
	//	set the global buffer pool to null
	SetGlobalBufferPool(nullptr);
	//	the ISFPassTarget class creates a tex-to-tex copier for copying textures: delete it
	VVISF::ISFPassTarget::cleanup();
	
	//	...other GL resources owned by other classes will be freed using the same signal
}





OutputWindow * GetOutputWindow()	{
	return globalOutputWindow;
}
