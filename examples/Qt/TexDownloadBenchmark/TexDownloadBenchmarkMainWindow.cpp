#include "TexDownloadBenchmarkMainWindow.h"
#include "ui_TexDownloadBenchmarkMainWindow.h"

//#include <QOpenGLContext>
#include <QPainter>
#include <QTimer>
#include <QFile>
#include <QDebug>
#include <iostream>




using namespace std;
using namespace VVGL;
using namespace VVISF;




TexDownloadBenchmarkMainWindow::TexDownloadBenchmarkMainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::TexDownloadBenchmarkMainWindow)
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
	ui->tt_rect->setChecked(true);
	
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


void TexDownloadBenchmarkMainWindow::startTestClicked()
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
		
		GetGlobalBufferPool()->context()->makeCurrentIfNotCurrent();
		
		while (testCount < 1000)	{
			workMethod();
		}

		endTime = make_shared<Timestamp>();

		string			tmpCPPString = FmtString("%0.2f FPS download",double(testCount)/((*endTime - *startTime).getTimeInSeconds()));
		QString			tmpString = QString::fromStdString(tmpCPPString);
		//QString		tmpString = QString("%1 FPS download").arg(QString::number());
		ui->resultsLabel->setText(tmpString);
	});



}
void TexDownloadBenchmarkMainWindow::checkImageClicked()
{
	cout << __PRETTY_FUNCTION__ << endl;
	
	prepForWork();
}


/*	========================================	*/
#pragma mark --------------------- private


void TexDownloadBenchmarkMainWindow::prepForWork()
{
	cout << __PRETTY_FUNCTION__ << endl;
	
	texBuffer = nullptr;
	
	//	load the included ISF file
	ISFSceneRef			renderScene = CreateISFSceneRef();
	QString				tmpPath(":/files/CellMod.fs");
	QFile				tmpFile(tmpPath);
	if (!tmpFile.open(QFile::ReadOnly | QFile::Text))	{
        qDebug() << "ERR: could not open CellMod file, " << __PRETTY_FUNCTION__;
		return;
	}
	QTextStream		tmpStream(&tmpFile);
	QString			fileContentsString = tmpStream.readAll();
	std::string		fileContents = fileContentsString.toStdString();
	tmpFile.close();
	
	ISFDocRef		tmpDoc = make_shared<ISFDoc>(fileContents, ISFVertPassthru_GL2, nullptr);
	//cout << "\tisf doc is " << *tmpDoc << endl;
	renderScene->useDoc(tmpDoc);
	
	//	render a frame of the appropriate dimensions/texture type
	texBuffer = createTexForWork();
	renderScene->renderToBuffer(texBuffer, texBuffer->size, 1.0);
	cout << "\tsource texture is " << *texBuffer << endl;
	ui->bufferView->drawBuffer(texBuffer);
	
	//	prime the downloader with a couple frames so when we start streaming we'll be pulling stuff out right away
	GetGlobalBufferPool()->context()->makeCurrentIfNotCurrent();
	GLBufferRef			validFrame = nullptr;
	texToCPU->clearStream();
	for (int i=0; i<texToCPU->queueSize()+1; ++i)	{
		GLBufferRef		targetCPU = createCPUBufferForWork();
		GLBufferRef		downloadedCPU = texToCPU->streamTexToCPU(texBuffer, targetCPU, true);
		//GLBufferRef		downloadedCPU = texToCPU->streamTexToCPU(texBuffer, nullptr, true);
		
		if (i == texToCPU->queueSize())	{
			if (downloadedCPU == nullptr)
				cout << "\tERR: download failed for some reason, " << __PRETTY_FUNCTION__ << endl;
			else	{
				cout << "\tpreviewed CPU-based buffer is " << *downloadedCPU << endl;
				QImage			tmpQImg(static_cast<uchar *>(downloadedCPU->cpuBackingPtr), downloadedCPU->size.width, downloadedCPU->size.height, QImage::Format_RGBA8888);
				QPixmap			tmpPixmap = QPixmap::fromImage(tmpQImg);
				ui->cpuPreview->setPixmap(tmpPixmap);
			}
		}
	}
	
	
	//	clear the start/end times and the test count
	startTime = nullptr;
	endTime = nullptr;
	testCount = 0;
}
GLBufferRef TexDownloadBenchmarkMainWindow::createTexForWork()
{
	cout << __PRETTY_FUNCTION__ << endl;
	
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
	
	GLBufferPoolRef	bp = GetGlobalBufferPool();
	Size			tmpSize(ui->widthField->value(), ui->heightField->value());
	GLBufferRef		returnMe = bp->createBufferRef(desc, tmpSize, nullptr, Size(), true);
	returnMe->parentBufferPool = bp;
	
	return returnMe;
}

VVGL::GLBufferRef TexDownloadBenchmarkMainWindow::createCPUBufferForWork()
{
	//cout << __PRETTY_FUNCTION__ << endl;
	
	//	we're constructing the buffer manually here, which...probably isn't the best idea
	GLBuffer::Descriptor	desc;
	
	desc.type = GLBuffer::Type_CPU;
	desc.target = GLBuffer::Target_None;
	desc.pixelFormat = static_cast<GLBuffer::PixelFormat>(pixelFormatGroup.checkedId());
	desc.internalFormat = static_cast<GLBuffer::InternalFormat>(internalFormatGroup.checkedId());
	desc.pixelType = static_cast<GLBuffer::PixelType>(pixelTypeGroup.checkedId());
	desc.cpuBackingType = GLBuffer::Backing_Internal;
	desc.gpuBackingType = GLBuffer::Backing_None;
	desc.texRangeFlag = false;
	desc.texClientStorageFlag = false;
	desc.msAmount = 0;
	desc.localSurfaceID = 0;
	
	//GLBufferRef		returnMe = GetGlobalBufferPool()->createBufferRef(desc, cpuBuffer->srcRect.size, nullptr, Size(), true);
	//returnMe->parentBufferPool = GetGlobalBufferPool();
	
	VVGL::Size			tmpSize(ui->widthField->value(), ui->heightField->value());
	uint32_t		tmpThing = desc.bytesPerRowForWidth(64);
	++tmpThing;
	void			*bufferMemory = malloc(desc.backingLengthForSize(tmpSize));
	GLBufferPoolRef	bp = GetGlobalBufferPool();
	//cout << "\tcpu buffer should be sized " << tmpSize << endl;
	GLBufferRef		returnMe = bp->fetchMatchingFreeBuffer(desc, tmpSize);
	if (returnMe == nullptr)	{
		//cout << "\tactually creating a CPU-based buffer!\n";
		returnMe = bp->createBufferRef(desc, tmpSize, bufferMemory, tmpSize, true);
		returnMe->parentBufferPool = bp;
		returnMe->backingID = GLBuffer::BackingID_Pixels;
		returnMe->backingContext = bufferMemory;
		returnMe->backingReleaseCallback = [](GLBuffer & , void* inReleaseContext)	{
			free(inReleaseContext);
		};
	}
	
	return returnMe;
}

void TexDownloadBenchmarkMainWindow::workMethod()
{
	if (texBuffer != nullptr)	{
		GLBufferRef		newCPU = createCPUBufferForWork();
		GLBufferRef		outCPU = texToCPU->streamTexToCPU(texBuffer, nullptr, true);
	}
	
	++testCount;
}


/*	========================================	*/
#pragma mark --------------------- private slots


void TexDownloadBenchmarkMainWindow::widgetDrewItsFirstFrame()
{
	cout << __PRETTY_FUNCTION__ << endl;
	
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
	
	//	make the texture downloader
	texToCPU = CreateGLTexToCPUCopierRef();
	texToCPU->setQueueSize(1);	//	no discernible effect
}
void TexDownloadBenchmarkMainWindow::aboutToQuit()
{
	cout << __PRETTY_FUNCTION__ << endl;
	
	//ui->bufferView->stopRenderingImmediately();
	SetGlobalBufferPool(nullptr);
}

TexDownloadBenchmarkMainWindow::~TexDownloadBenchmarkMainWindow()
{
	delete ui;
}
