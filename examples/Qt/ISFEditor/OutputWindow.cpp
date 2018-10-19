#include "OutputWindow.h"
#include "ui_OutputWindow.h"

#include <QDebug>
#include <QSettings>

#include "ISFPassTarget.hpp"
#include "ISFController.h"




static OutputWindow * globalOutputWindow = nullptr;




OutputWindow::OutputWindow(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::OutputWindow)
{
	qDebug() << __PRETTY_FUNCTION__;
	
	ui->setupUi(this);
	
	globalOutputWindow = this;
	
	//	we want to know when the widget draws its first frame because we can't create the global shared context/buffer pool until we can get a base ctx from the widget
	connect(ui->bufferView, SIGNAL(frameSwapped()), this, SLOT(widgetDrewItsFirstFrame()));
	
	//	we need to shut stuff down and delete contexts gracefull on quit
	connect(qApp, SIGNAL(aboutToQuit()), this, SLOT(aboutToQuit()));
	
	//	restore the window position
	QSettings		settings;
	if (settings.contains("OutputWindowGeometry"))	{
		restoreGeometry(settings.value("OutputWindowGeometry").toByteArray());
	}
	
	//	tell the buffer view to start rendering
	ui->bufferView->startRendering();
}

OutputWindow::~OutputWindow()	{
	delete ui;
}

GLBufferQWidget * OutputWindow::bufferView()	{
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
}
void OutputWindow::updateContentsFromISFController()	{
	//	get the ISFController, and from it retrieve the doc that is currently loaded
	ISFController		*isfc = GetISFController();
	if (isfc == nullptr)
		return;
	//ISFSceneRef			tmpScene = isfc->getScene();
	//ISFDocRef			tmpDoc = (tmpScene==nullptr) ? nullptr : tmpScene->getDoc();
	ISFDocRef			tmpDoc = isfc->getCurrentDoc();
	int					maxPassCount = (tmpDoc==nullptr) ? 0 : (int)tmpDoc->getRenderPasses().size();
	int					imageInputsCount = (tmpDoc==nullptr) ? 0 : (int)tmpDoc->getImageInputs().size();
	int					audioInputsCount = (tmpDoc==nullptr) ? 0 : (int)tmpDoc->getAudioInputs().size();
	
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
				QString		passName = QString::fromStdString(tmpDoc->getRenderPasses().at(i));
				if (passName.size() == 0)
					passName = QString("PASSINDEX %1").arg(i);
				cb->addItem(passName, QVariant(i));
			}
		}
		//	image input menu items have variants starting at 100
		if (imageInputsCount > 0)	{
			cb->insertSeparator(cb->count());
		
			for (int i=0; i<imageInputsCount; ++i)	{
				ISFAttrRef	inputRef = tmpDoc->getImageInputs().at(i);
				QString		inputName = (inputRef==nullptr) ? QString("") : QString::fromStdString(inputRef->getName());
				cb->addItem(inputName, QVariant(100+i));
			}
		}
		//	image input menu items have variants starting at 200
		if (audioInputsCount > 0)	{
			cb->insertSeparator(cb->count());
		
			for (int i=0; i<audioInputsCount; ++i)	{
				ISFAttrRef	inputRef = tmpDoc->getAudioInputs().at(i);
				QString		inputName = (inputRef==nullptr) ? QString("") : QString::fromStdString(inputRef->getName());
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

void OutputWindow::on_renderPassComboBox_currentIndexChanged(int index)	{

}

void OutputWindow::on_freezeOutputToggle_stateChanged(int arg1)	{

}

void OutputWindow::on_displayAlphaToggle_stateChanged(int arg1)	{

}

void OutputWindow::widgetDrewItsFirstFrame()	{
	//qDebug() << __PRETTY_FUNCTION__;
	/*
	if (QThread::currentThread() == qApp->thread())
		qDebug() << "\tcurrent thread in widget draw first frame is main thread";
	else
		qDebug() << "\tcurrent thread in widget draw first frame is NOT main thread!";
	*/
	//	get the widget's context- if it's null, the widget's context doesn't exist yet and this method shouldn't have been called!
	GLContextRef		widgetCtx = ui->bufferView->getContext();
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
void OutputWindow::aboutToQuit()	{
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
