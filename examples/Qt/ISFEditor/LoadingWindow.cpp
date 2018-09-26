#include "LoadingWindow.h"
#include "ui_LoadingWindow.h"

#include <QDebug>
#include <QFile>
#include <QDir>
#include <QFileSystemModel>
//#include <QTimer>
#include <QSettings>
#include <QItemSelection>
//#include <QErrorMessage>
#include <QScrollArea>
#include <QDir>

#include "DocWindow.h"
#include "ISFController.h"




using namespace std;

static LoadingWindow * globalLoadingWindow = nullptr;




LoadingWindow::LoadingWindow(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::LoadingWindow)
{
	qDebug() << __PRETTY_FUNCTION__;
	
	globalLoadingWindow = this;
	
	ui->setupUi(this);
	
	//	the spinboxes for setting rendering res need sane min/maxes
	ui->renderResWidthWidget->setMinimum(1);
	ui->renderResWidthWidget->setMaximum(16384);
	ui->renderResHeightWidget->setMinimum(1);
	ui->renderResHeightWidget->setMaximum(16384);
	
	//	figure out what directory's contents we want to display, and use it to set the base directory
	QString			defaultDirToLoad;
#ifdef Q_OS_MAC
		defaultDirToLoad = QString("~/Library/Graphics/ISF");
		//defaultDirToLoad = QString("~/Documents/VDMX5/VDMX5/supplemental resources/ISF tests+tutorials");
		defaultDirToLoad.replace("~", QDir::homePath());
#endif
	QSettings		settings;
	QVariant		lastUsedPath = settings.value("baseDir");
	if (!lastUsedPath.isNull())	{
		qDebug() << "\tfound a path stored in the user settings! (" << lastUsedPath.toString() << ")";
		QString			tmpStr = lastUsedPath.toString();
		tmpStr.replace("~", QDir::homePath());
		if (QDir(tmpStr).exists())	{
			setBaseDirectory(tmpStr);
		}
		else	{
			qDebug() << "\terr: the dir to load doesn't exist, falling back to default dir";
			setBaseDirectory(defaultDirToLoad);
		}
	}
	else
		setBaseDirectory(defaultDirToLoad);
	
	//	restore the window position
	if (settings.contains("LoadingWindowGeometry"))	{
		restoreGeometry(settings.value("LoadingWindowGeometry").toByteArray());
	}
	
	//	we need to load a new file as the selection changes
	QItemSelectionModel		*selModel = ui->filterListView->selectionModel();
	if (selModel != nullptr)	{
		//connect(selModel, &QItemSelectionModel::selectionChanged, this, &LoadingWindow::newFileSelected);
		connect(selModel, &QItemSelectionModel::selectionChanged, [&](const QItemSelection &selected, const QItemSelection &deselected)	{
			QList<QModelIndex>		selectedIndexes = selected.indexes();
			if (selectedIndexes.count() < 1)
				return;
			QModelIndex		firstIndex = selectedIndexes.first();
			if (!firstIndex.isValid())
				return;
			QVariant		selectedPath = firstIndex.data(QFileSystemModel::FilePathRole);
			if (selectedPath.isNull())
				return;
			//qDebug() << "\tshould be loading file " << selectedPath.toString();
			
			QString			selectedPathString = selectedPath.toString();
			//GetISFController()->loadFile(selectedPathString);
			//GetDocWindow()->loadFile(selectedPathString);
			
			on_loadFile(selectedPathString);
		});
	}
}

LoadingWindow::~LoadingWindow()
{
	delete ui;
}

QScrollArea * LoadingWindow::getScrollArea()	{
	return ui->scrollArea;
}
QSpinBox * LoadingWindow::getWidthSB() { return ui->renderResWidthWidget; }
QSpinBox * LoadingWindow::getHeightSB() { return ui->renderResHeightWidget; }
void LoadingWindow::on_createNewFile()	{
	//	make new tmp file, populate its contents
	QString			tmpFilePath = QString();
	QFile			tmpFragShaderFile( QString("%1/ISFTesterTmpFile.fs").arg(QDir::tempPath()) );
	if (tmpFragShaderFile.open(QIODevice::WriteOnly))	{
		tmpFilePath = tmpFragShaderFile.fileName();
		qDebug() << "\ttmpFragShaderFile's path is " << tmpFilePath;
	
		QFile			newFileTemplate(":/resources/NewFileTemplate.txt");
		if (newFileTemplate.open(QFile::ReadOnly))	{
			QTextStream		rStream(&newFileTemplate);
			QString			tmpString = rStream.readAll();
			
			QTextStream		wStream(&tmpFragShaderFile);
			wStream << tmpString;
		
			newFileTemplate.close();
		}
	
		tmpFragShaderFile.close();
	}
	//	load the temp file!
	on_loadFile(tmpFilePath);
}
void LoadingWindow::on_loadFile(const QString & n)	{
	//	tell the ISF controller to load the passed file- the ISF controller will tell the doc window (and myself) to load when it's ready
	GetISFController()->loadFile(n);
}
void LoadingWindow::on_saveFile()	{
	qDebug() << __PRETTY_FUNCTION__;
}




void LoadingWindow::closeEvent(QCloseEvent * event)	{
	QSettings		settings;
	settings.setValue("LoadingWindowGeometry", saveGeometry());
	
	QWidget::closeEvent(event);
}




void LoadingWindow::on_loadUserISFsButton_clicked()	{
	qDebug() << __PRETTY_FUNCTION__;
}
void LoadingWindow::on_loadSystemISFsButton_clicked()	{
	qDebug() << __PRETTY_FUNCTION__;
}
void LoadingWindow::on_halveRenderRes_clicked()	{
	qDebug() << __PRETTY_FUNCTION__;
}
void LoadingWindow::on_doubleRenderRes_clicked()	{
	qDebug() << __PRETTY_FUNCTION__;
}
void LoadingWindow::on_renderResWidthWidget_valueChanged(int arg1)	{
	qDebug() << __PRETTY_FUNCTION__;
}
void LoadingWindow::on_renderResHeightWidget_valueChanged(int arg1)	{
	qDebug() << __PRETTY_FUNCTION__;
}
void LoadingWindow::on_saveUIValsToDefault_clicked()	{
	qDebug() << __PRETTY_FUNCTION__;
}
/*
void LoadingWindow::newFileSelected(const QItemSelection &selected, const QItemSelection &deselected)	{
	QList<QModelIndex>		selectedIndexes = selected.indexes();
	if (selectedIndexes.count() < 1)
		return;
	QModelIndex				firstIndex = selectedIndexes.first();
	if (!firstIndex.isValid())
		return;
	QVariant				selectedPath = firstIndex.data(QFileSystemModel::FilePathRole);
	if (selectedPath.isNull())
		return;
	//qDebug() << "\tshould be loading file " << selectedPath.toString();
	
	GetISFController()->loadFile(selectedPath.toString());
}
*/


void LoadingWindow::setBaseDirectory(const QString & inBaseDir)	{
	qDebug() << __PRETTY_FUNCTION__ << ", " << inBaseDir;
	
	if (baseDirectory == inBaseDir)
		return;
	
	QDir		tmpDir(inBaseDir);
	if (!tmpDir.exists() || !tmpDir.isReadable())	{
		qDebug() << "\tERR: passed dir doesn't exist or is not readable (" << inBaseDir << "), " << __PRETTY_FUNCTION__;
		return;
	}
	
	baseDirectory = inBaseDir;
	
	
	//	store the base directory in the settings for next time (try to store the relative path)
	QSettings		settings;
	if (baseDirectory.startsWith(QDir::homePath(), Qt::CaseSensitive))	{
		QString			relativePath = baseDirectory;
		relativePath.replace(QDir::homePath(), "~");
		settings.setValue("baseDir", relativePath);
	}
	else	{
		settings.setValue("baseDir", baseDirectory);
	}
	settings.sync();
	
	
	//	get the existing directory model from the list view
	QAbstractItemModel		*oldModel = ui->filterListView->model();
	
	//	make a new model, pass it to the list view (or pass null if we couldn't make a model)
	QFileSystemModel		*newModel = new QFileSystemModel(this);
	newModel->setReadOnly(true);
	//newModel->setFilter(QDir::Files | QDir::NoSymLinks | QDir::NoDotAndDotDot | QDir::Readable);
	newModel->setNameFilters(QStringList(QString("*.fs")));
	newModel->setNameFilterDisables(false);
	ui->filterListView->setModel(newModel);
	ui->filterListView->setRootIndex(newModel->setRootPath(baseDirectory));
	
	//	delete the old model
	if (oldModel != nullptr)
		delete oldModel;
}




LoadingWindow * GetLoadingWindow()	{
	return globalLoadingWindow;
}
