#include "TexUploadBenchmarkMainWindow.h"
#include "ui_TexUploadBenchmarkMainWindow.h"

//#include <QOpenGLContext>
#include <QTimer>
#include <iostream>




using namespace std;
using namespace VVGL;




enum UploadPixelFormat	{
	UploadPixelFormat_RGBA = 0,
	UploadPixelFormat_BGRA = 1
};

enum UploadInternalFormat	{
	UploadInternalFormat_RGBA = 0,
	UploadInternalFormat_RGBA8 = 1
};

enum UploadPixelType	{
	UploadPixelType_UB = 0,
	UploadPixelType_8888_REV = 1,
	UploadPixelType_8888 = 2
};




TexUploadBenchmarkMainWindow::TexUploadBenchmarkMainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::TexUploadBenchmarkMainWindow)
{
	cout << __PRETTY_FUNCTION__ << endl;
	
	ui->setupUi(this);
	
	//	we want to know when the widget draws its first frame because we can't create the global shared context/buffer pool until we can get a base ctx from the widget
	connect(ui->bufferView, SIGNAL(frameSwapped()), this, SLOT(widgetDrewItsFirstFrame()));
	//	we need to shut stuff down and delete contexts gracefull on quit
	connect(qApp, SIGNAL(aboutToQuit()), this, SLOT(aboutToQuit()));
	
	//	we need to know when the user starts the test, or clicks 'checkImage'
	connect(ui->checkButton, SIGNAL(clicked()), this, SLOT(checkImageClicked()));
	connect(ui->startButton, SIGNAL(clicked()), this, SLOT(startTestClicked()));
}


/*	========================================	*/
#pragma mark --------------------- public slots


void TexUploadBenchmarkMainWindow::startTestClicked()
{
	cout << __PRETTY_FUNCTION__ << endl;
	
	prepForWork();
	
	//ui->resultsLabel->setText("Running the test...");
	//ui->resultsLabel->update();
	
	QTimer::singleShot(0, [&] {
		ui->resultsLabel->setText("Test starting in 5 seconds...");
		ui->resultsLabel->update();
	});

	QTimer::singleShot(5000, [&] {
		if (startTime == nullptr)
			startTime = make_shared<Timestamp>();

		while (testCount < 1000)	{
			workMethod();
		}

		endTime = make_shared<Timestamp>();

		string			tmpCPPString = FmtString("%0.2f FPS upload",double(testCount)/((*endTime - *startTime).getTimeInSeconds()));
		QString			tmpString = QString::fromStdString(tmpCPPString);
		//QString		tmpString = QString("%1 FPS upload").arg(QString::number());
		ui->resultsLabel->setText(tmpString);
	});



}
void TexUploadBenchmarkMainWindow::checkImageClicked()
{
	cout << __PRETTY_FUNCTION__ << endl;
	
	prepForWork();
	
	GLBufferRef		tmpTex = cpuToTex->streamCPUToTex(cpuBuffer);
	ui->bufferView->drawBuffer(tmpTex);
}


/*	========================================	*/
#pragma mark --------------------- private


void TexUploadBenchmarkMainWindow::prepForWork()
{
	cout << __PRETTY_FUNCTION__ << endl;
	
	cpuBuffer = nullptr;
	
	
	QImage			rawImg(":/files/IMG_0885.JPG");
	
	QSize			tmpSize(ui->widthField->value(), ui->heightField->value());
	QImage			*tmpImg = new QImage(tmpSize, QImage::Format_ARGB32);
	QPainter		tmpPainter(tmpImg);
	QRect			tmpSrcFrame(QPoint(0,0), rawImg.size());
	QRect			tmpDstFrame(QPoint(0,0), tmpSize);
	tmpPainter.drawImage(tmpDstFrame, rawImg, tmpSrcFrame);
	tmpPainter.end();
	
	//cpuBuffer = CreateBufferForQImage(tmpImg);
	cpuBuffer = CreateCPUBufferForQImage(tmpImg);
	
	//	prime the uploader with a couple frames so when we start streaming we'll be pulling stuff out right away
	GLBufferRef			validFrame = nullptr;
	cpuToTex->clearStream();
	for (int i=0; i<cpuToTex->getQueueSize()+1; ++i)	{
		GLBufferRef		tmpTex = cpuToTex->streamCPUToTex(cpuBuffer);
	}
	
	
	//	clear the start/end times and the test count
	startTime = nullptr;
	endTime = nullptr;
	testCount = 0;
}
void TexUploadBenchmarkMainWindow::workMethod()
{
	if (cpuBuffer == nullptr)
		return;
	
	GLBufferRef		tmpTex = cpuToTex->streamCPUToTex(cpuBuffer);
	
	++testCount;
}


/*	========================================	*/
#pragma mark --------------------- private slots


void TexUploadBenchmarkMainWindow::widgetDrewItsFirstFrame()
{
	cout << __PRETTY_FUNCTION__ << endl;
	
	//	get the widget's context- if it's null, the widget's context doesn't exist yet and this method shouldn't have been called!
	GLContextRef		widgetCtx = ui->bufferView->getContext();
	if (widgetCtx == nullptr)
		return;
	
	//	disconnect immediately- we're only doing this because we need to create the shared context from the widget's context
	disconnect(ui->bufferView, SIGNAL(frameSwapped()), this, SLOT(widgetDrewItsFirstFrame()));
	
	//	make the global buffer pool, using a newly-created context that shares the base global context
	CreateGlobalBufferPool(widgetCtx->newContextSharingMe());
	
	//	tell the widget to start rendering?
	//ui->bufferView->startRendering();
	
	/*
	//	load the image file we include with the sample app, convert it to a VVGLBufferRef
	//QImage				tmpImg(":/files/SampleImg.png");
	QImage				tmpImg(":/files/IMG_0885.JPG");
	GLBufferRef			imgBuffer = CreateBufferForQImage(&tmpImg);
	//	pass the image buffer to the GL view
	ui->bufferView->drawBuffer(imgBuffer);
	ui->bufferView->update();
	*/
	
	cpuToTex = CreateGLCPUToTexCopierRef();
	//cpuToTex->setQueueSize(4);	//	no discernible effect
}
void TexUploadBenchmarkMainWindow::aboutToQuit()
{
	cout << __PRETTY_FUNCTION__ << endl;
	
	//ui->bufferView->stopRenderingImmediately();
	SetGlobalBufferPool(nullptr);
}

TexUploadBenchmarkMainWindow::~TexUploadBenchmarkMainWindow()
{
	delete ui;
}
