#include "TexUploadBenchmarkMainWindow.h"
#include "ui_TexUploadBenchmarkMainWindow.h"

//#include <QOpenGLContext>
#include <QPainter>
#include <QTimer>
#include <iostream>




using namespace std;
using namespace VVGL;




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
	
	//	populate the texture type group
	textureTypeGroup.addButton(ui->tt_2d, GLBuffer::Target_2D);
	textureTypeGroup.addButton(ui->tt_rect, GLBuffer::Target_Rect);
	ui->tt_2d->setChecked(true);
	
	//	populate the pixel format group
	pixelFormatGroup.addButton(ui->pf_rgba, GLBuffer::PF_RGBA);
	pixelFormatGroup.addButton(ui->pf_bgra, GLBuffer::PF_BGRA);
	ui->pf_rgba->setChecked(true);
	
	//	populate the internal format group
	internalFormatGroup.addButton(ui->if_rgba, GLBuffer::IF_RGBA);
	internalFormatGroup.addButton(ui->if_rgba8, GLBuffer::IF_RGBA8);
	ui->if_rgba->setChecked(true);
	
	//	populate the pixel type group
	pixelTypeGroup.addButton(ui->pt_ubyte, GLBuffer::PT_UByte);
	pixelTypeGroup.addButton(ui->pt_u8888rev, GLBuffer::PT_UInt_8888_Rev);
	ui->pt_u8888rev->setChecked(true);
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
		
		GetGlobalBufferPool()->getContext()->makeCurrentIfNotCurrent();
		
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
}


/*	========================================	*/
#pragma mark --------------------- private


void TexUploadBenchmarkMainWindow::prepForWork()
{
	cout << __PRETTY_FUNCTION__ << endl;
	
	cpuBuffer = nullptr;
	
	//	load the raw image from the file
	QImage			rawImg(":/files/IMG_0885.JPG");
	//	use QPainter to resize the image and get a bitmap of the dimensions specified in the UI
	QSize			tmpSize(ui->widthField->value(), ui->heightField->value());
	QImage			*tmpImg = new QImage(tmpSize, QImage::Format_ARGB32);
	QPainter		tmpPainter(tmpImg);
	QRect			tmpSrcFrame(QPoint(0,0), rawImg.size());
	QRect			tmpDstFrame(QPoint(0,0), tmpSize);
	tmpPainter.drawImage(tmpDstFrame, rawImg, tmpSrcFrame);
	tmpPainter.end();
	//	draw the QImage in the label
	ui->cpuPreview->setPixmap(QPixmap::fromImage(*tmpImg));
	
	//	make a CPU-based buffer from the QImage- this is what we'll be uploading
	cpuBuffer = CreateCPUBufferForQImage(tmpImg);
	
	//	prime the uploader with a couple frames so when we start streaming we'll be pulling stuff out right away
	GetGlobalBufferPool()->getContext()->makeCurrentIfNotCurrent();
	cpuToTex->clearStream();
	for (int i=0; i<cpuToTex->getQueueSize()+1; ++i)	{
		GLBufferRef		targetTex = createTexForWork();
		GLBufferRef		uploadedTex = cpuToTex->streamCPUToTex(cpuBuffer, targetTex, true);
		if (i == cpuToTex->getQueueSize())	{
			if (uploadedTex == nullptr)
				cout << "\tERR: upload failed for some reason, " << __PRETTY_FUNCTION__ << endl;
			else
				cout << "\tpreviewed buffer is " << *uploadedTex << endl;
			ui->bufferView->drawBuffer(uploadedTex);
		}
	}
	
	//	clear the start/end times and the test count
	startTime = nullptr;
	endTime = nullptr;
	testCount = 0;
}
GLBufferRef TexUploadBenchmarkMainWindow::createTexForWork()
{
	//	we're constructing the buffer manually here, which...probably isn't the best idea
	GLBuffer::Descriptor	desc;
	
	desc.type = GLBuffer::Type_Tex;
	desc.target = static_cast<GLBuffer::Target>(textureTypeGroup.checkedId());
	desc.pixelFormat = static_cast<GLBuffer::PixelFormat>(pixelFormatGroup.checkedId());
	desc.internalFormat = static_cast<GLBuffer::InternalFormat>(internalFormatGroup.checkedId());
	desc.pixelType = static_cast<GLBuffer::PixelType>(pixelTypeGroup.checkedId());
	desc.cpuBackingType = GLBuffer::Backing_None;
	desc.gpuBackingType = GLBuffer::Backing_Internal;
	desc.texRangeFlag = false;
	desc.texClientStorageFlag = false;
	desc.msAmount = 0;
	desc.localSurfaceID = 0;
	
	GLBufferRef		returnMe = GetGlobalBufferPool()->createBufferRef(desc, cpuBuffer->srcRect.size, nullptr, Size(), true);
	returnMe->parentBufferPool = GetGlobalBufferPool();
	
	return returnMe;
}
void TexUploadBenchmarkMainWindow::workMethod()
{
	if (cpuBuffer != nullptr)	{
		GLBufferRef		newTex = createTexForWork();
		GLBufferRef		outTex = cpuToTex->streamCPUToTex(cpuBuffer,newTex, true);
	}
	
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
	
	//cout << "\tversion is " << GLVersionToString(widgetCtx->version) << endl;
	
	//	disconnect immediately- we're only doing this because we need to create the shared context from the widget's context
	disconnect(ui->bufferView, SIGNAL(frameSwapped()), this, SLOT(widgetDrewItsFirstFrame()));
	
	//	make the global buffer pool, using a newly-created context that shares the base global context
	CreateGlobalBufferPool(widgetCtx->newContextSharingMe());
	
	//	don't tell the widget to start rendering- doing so will cause it to start rendering at 60fps
	//ui->bufferView->startRendering();
	
	//	tell the widget to draw a single frame.  for some reason, GL widgets on os x don't have their internal sizes set properly when they draw their first frame.
	ui->bufferView->drawBuffer(nullptr);
	
	//	make the texture uploader
	cpuToTex = CreateGLCPUToTexCopierRef();
	cpuToTex->setQueueSize(1);	//	no discernible effect
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
