#ifndef TEXUPLOADBENCHMARKMAINWINDOW_H
#define TEXUPLOADBENCHMARKMAINWINDOW_H

#include <QMainWindow>
#include <QButtonGroup>

#include <VVGL.hpp>




namespace Ui {
	class TexUploadBenchmarkMainWindow;
}

class TexUploadBenchmarkMainWindow : public QMainWindow
{
	Q_OBJECT

public:
	explicit TexUploadBenchmarkMainWindow(QWidget *parent = nullptr);
	~TexUploadBenchmarkMainWindow();
	
public slots:
	Q_SLOT void startTestClicked();
	Q_SLOT void checkImageClicked();
	



private:
	void prepForWork();
	VVGL::GLBufferRef createTexForWork();
	void workMethod();
	
private slots:
	Q_SLOT void widgetDrewItsFirstFrame();
	Q_SLOT void aboutToQuit();

private:
	Ui::TexUploadBenchmarkMainWindow		*ui;
	
	QButtonGroup				pixelFormatGroup;
	QButtonGroup				internalFormatGroup;
	QButtonGroup				pixelTypeGroup;
	
	VVGL::GLBufferRef			cpuBuffer;	//	CPU-based, created when you start a test or check.  this gets copied into a buffer of the appropriate target/format, which is then uploaded
	VVGL::TimestampRef			startTime = nullptr;	//	null until the test is started.  used to calculate how long the test takes.
	int							testCount = 0;
	VVGL::TimestampRef			endTime = nullptr;
	VVGL::GLCPUToTexCopierRef	cpuToTex = nullptr;
};

#endif // TEXUPLOADBENCHMARKMAINWINDOW_H
