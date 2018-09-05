#ifndef TEXDOWNLOADBENCHMARKMAINWINDOW_H
#define TEXDOWNLOADBENCHMARKMAINWINDOW_H

#include <QMainWindow>
#include <QButtonGroup>

#include <VVGL.hpp>
#include <VVISF.hpp>




namespace Ui {
	class TexDownloadBenchmarkMainWindow;
}

class TexDownloadBenchmarkMainWindow : public QMainWindow
{
	Q_OBJECT

public:
	explicit TexDownloadBenchmarkMainWindow(QWidget *parent = nullptr);
	~TexDownloadBenchmarkMainWindow();
	
public slots:
	Q_SLOT void startTestClicked();
	Q_SLOT void checkImageClicked();
	



private:
	void prepForWork();
	VVGL::GLBufferRef createTexForWork();
	VVGL::GLBufferRef createCPUBufferForWork();
	void workMethod();
	
private slots:
	Q_SLOT void widgetDrewItsFirstFrame();
	Q_SLOT void aboutToQuit();

private:
	Ui::TexDownloadBenchmarkMainWindow		*ui;
	
	QButtonGroup				textureTypeGroup;
	QButtonGroup				pixelFormatGroup;
	QButtonGroup				internalFormatGroup;
	QButtonGroup				pixelTypeGroup;
	
	//VVGL::GLBufferRef			cpuBuffer;	//	CPU-based, created when you start a test or check.  this gets copied into a buffer of the appropriate target/format, which is then downloaded
	VVGL::GLBufferRef			texBuffer;	// texture-based, created when you start a test or check.  this is the texture we're downloading!
	VVGL::TimestampRef			startTime = nullptr;	//	null until the test is started.  used to calculate how long the test takes.
	int							testCount = 0;
	VVGL::TimestampRef			endTime = nullptr;
	
	VVGL::GLTexToCPUCopierRef	texToCPU = nullptr;
};

#endif // TEXDOWNLOADBENCHMARKMAINWINDOW_H
