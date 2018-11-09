#ifndef LOADINGWINDOW_H
#define LOADINGWINDOW_H

#include <QWidget>
#include <QSpinBox>
class QItemSelection;
class QScrollArea;
class DynamicVideoSource;




namespace Ui {
	class LoadingWindow;
}




class LoadingWindow : public QWidget
{
	Q_OBJECT

public:
	explicit LoadingWindow(QWidget *parent = nullptr);
	~LoadingWindow();
	
	QScrollArea * getScrollArea();
	QSpinBox * getWidthSB();
	QSpinBox * getHeightSB();
	inline QString getBaseDirectory() { return baseDirectory; }
	void setBaseDirectory(const QString & inBaseDir);
	
	void on_createNewFile();
	void on_loadFile(const QString & n);
	void on_saveFile();
	
protected:
	virtual void closeEvent(QCloseEvent * event) Q_DECL_OVERRIDE;
	virtual void showEvent(QShowEvent * event) Q_DECL_OVERRIDE;
	void appQuitEvent();

public slots:
	Q_SLOT void loadUserISFsButtonClicked();
	Q_SLOT void loadSystemISFsButtonClicked();
	Q_SLOT void halveRenderResClicked();
	Q_SLOT void doubleRenderResClicked();
	Q_SLOT void renderResWidthWidgetValueChanged();
	Q_SLOT void renderResHeightWidgetValueChanged();
	Q_SLOT void saveUIValsToDefaultClicked();
	/*
	void newFileSelected(const QItemSelection &selected, const QItemSelection &deselected);
	*/
	void listOfVideoSourcesUpdated(DynamicVideoSource * inSrc);
	void videoSourceChanged(int arg1);
	
private:
	//static LoadingWindow	*globalIvar;
	Ui::LoadingWindow		*ui;
	QString					baseDirectory = QString("");
};




//	gets the global singleton for this class, which is created in main()
LoadingWindow * GetLoadingWindow();




#endif // LOADINGWINDOW_H
