#include "LoadingWindow.h"
#include "ui_LoadingWindow.h"

#include <QDebug>
#include <QFile>
#include <QDir>
//#include <QFileSystemModel>
//#include <QTimer>
#include <QSettings>
#include <QItemSelection>
//#include <QErrorMessage>
#include <QScrollArea>
#include <QDir>

#include "DocWindow.h"
#include "ISFController.h"
#include "JGMTop.h"
#include "DynamicVideoSource.h"
#include "LoadingWindowFileListModel.h"
#include "MediaFile.h"




using namespace std;

static LoadingWindow * globalLoadingWindow = nullptr;




LoadingWindow::LoadingWindow(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::LoadingWindow)
{
	//qDebug() << __PRETTY_FUNCTION__;
	
	globalLoadingWindow = this;
	
	ui->setupUi(this);
	
	//	the spinboxes for setting rendering res need sane min/maxes
	ui->renderResWidthWidget->blockSignals(true);
	ui->renderResHeightWidget->blockSignals(true);
	ui->renderResWidthWidget->setMinimum(1);
	ui->renderResWidthWidget->setMaximum(16384);
	ui->renderResHeightWidget->setMinimum(1);
	ui->renderResHeightWidget->setMaximum(16384);
	ui->renderResWidthWidget->blockSignals(false);
	ui->renderResHeightWidget->blockSignals(false);
	
	//	the DynamicVideoSource class has a signal which is emitted when its list of sources change
	//connect(GetDynamicVideoSource(), &DynamicVideoSource::listOfStaticSourcesUpdated, this, &LoadingWindow::listOfVideoSourcesUpdated);
	connect(GetDynamicVideoSource(), &DynamicVideoSource::listOfStaticSourcesUpdated, this, &LoadingWindow::listOfVideoSourcesUpdated);
	//connect(GetDynamicVideoSource(), SIGNAL(listOfStaticSourcesUpdated), this, SLOT(listOfVideoSourcesUpdated));
	connect(ui->videoSourceComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &LoadingWindow::videoSourceChanged);
	
	//	we do this later, on an even that fires when the window opens, because i think it's screwing things up
	/*
	//	bump the slot to populate the pop-up button with the list of sources
	listOfVideoSourcesUpdated(GetDynamicVideoSource());
	*/
	
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
		//qDebug() << "\tfound a path stored in the user settings! (" << lastUsedPath.toString() << ")";
		QString			tmpStr = lastUsedPath.toString();
		tmpStr.replace("~", QDir::homePath());
		if (QDir(tmpStr).exists())	{
			setBaseDirectory(tmpStr);
		}
		else	{
			//qDebug() << "\terr: the dir to load doesn't exist, falling back to default dir";
			setBaseDirectory(defaultDirToLoad);
		}
	}
	else
		setBaseDirectory(defaultDirToLoad);
	
	//	restore the window position
	if (settings.contains("LoadingWindowGeometry"))	{
		restoreGeometry(settings.value("LoadingWindowGeometry").toByteArray());
	}
	
	
	
	//qDebug() << "\t" << __PRETTY_FUNCTION__ << " - FINISHED";
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
	qDebug() << __PRETTY_FUNCTION__;
	
	//QThread		*mainThread = QApplication::instance()->thread();
	//qDebug() << "main thread is currently " << mainThread;
	
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
	
	//	make sure the window's visible
	if (!isVisible())
		show();
	
	//	tell the ISF controller to load the passed file- the ISF controller will tell the doc window (and myself) to load when it's ready
	ISFController		*isfc = GetISFController();
	if (isfc != nullptr)
		isfc->loadFile(n);
}
void LoadingWindow::on_saveFile()	{
	qDebug() << __PRETTY_FUNCTION__;
}




void LoadingWindow::closeEvent(QCloseEvent * event)	{
	QSettings		settings;
	settings.setValue("LoadingWindowGeometry", saveGeometry());
	
	QWidget::closeEvent(event);
}
void LoadingWindow::showEvent(QShowEvent * event)	{
	QWidget::showEvent(event);
	
	//	bump the slot to populate the pop-up button with the list of sources.
	listOfVideoSourcesUpdated(GetDynamicVideoSource());
}




void LoadingWindow::loadUserISFsButtonClicked()	{
	qDebug() << __PRETTY_FUNCTION__;
	
	QString			dirToLoad("~/Library/Graphics/ISF");
	dirToLoad.replace("~", QDir::homePath());
	setBaseDirectory(dirToLoad);
}
void LoadingWindow::loadSystemISFsButtonClicked()	{
	qDebug() << __PRETTY_FUNCTION__;
	
	QString			dirToLoad("/Library/Graphics/ISF");
	setBaseDirectory(dirToLoad);
}
void LoadingWindow::halveRenderResClicked()	{
	qDebug() << __PRETTY_FUNCTION__;
	ISFController		*isfc = GetISFController();
	if (isfc == nullptr)
		return;
	Size			origSize = isfc->getRenderSize();
	Size			newSize(origSize.width/2., origSize.height/2.);
	
	ui->renderResWidthWidget->blockSignals(true);
	ui->renderResHeightWidget->blockSignals(true);
	
	ui->renderResWidthWidget->setValue(newSize.width);
	ui->renderResHeightWidget->setValue(newSize.height);
	
	ui->renderResWidthWidget->blockSignals(false);
	ui->renderResHeightWidget->blockSignals(false);
	
	ui->renderResWidthWidget->update();
	ui->renderResHeightWidget->update();
	
	isfc->setRenderSize(newSize);
}
void LoadingWindow::doubleRenderResClicked()	{
	qDebug() << __PRETTY_FUNCTION__;
	ISFController		*isfc = GetISFController();
	if (isfc == nullptr)
		return;
	Size			origSize = isfc->getRenderSize();
	Size			newSize(origSize.width*2., origSize.height*2.);
	
	ui->renderResWidthWidget->blockSignals(true);
	ui->renderResHeightWidget->blockSignals(true);
	
	ui->renderResWidthWidget->setValue(newSize.width);
	ui->renderResHeightWidget->setValue(newSize.height);
	
	ui->renderResWidthWidget->blockSignals(false);
	ui->renderResHeightWidget->blockSignals(false);
	
	ui->renderResWidthWidget->update();
	ui->renderResHeightWidget->update();
	
	isfc->setRenderSize(newSize);
}
void LoadingWindow::renderResWidthWidgetValueChanged()	{
	qDebug() << __PRETTY_FUNCTION__;
	ISFController		*isfc = GetISFController();
	if (isfc == nullptr)
		return;
	Size			tmpSize(ui->renderResWidthWidget->value(), ui->renderResHeightWidget->value());
	isfc->setRenderSize(tmpSize);
}
void LoadingWindow::renderResHeightWidgetValueChanged()	{
	qDebug() << __PRETTY_FUNCTION__;
	ISFController		*isfc = GetISFController();
	if (isfc == nullptr)
		return;
	Size			tmpSize(ui->renderResWidthWidget->value(), ui->renderResHeightWidget->value());
	isfc->setRenderSize(tmpSize);
}
void LoadingWindow::saveUIValsToDefaultClicked()	{
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
void LoadingWindow::listOfVideoSourcesUpdated(DynamicVideoSource * inSrc)	{
	qDebug() << __PRETTY_FUNCTION__;
	
	if (inSrc == nullptr)
		return;
	
	//if (qApp->thread() == QThread::currentThread())
	//	qDebug() << "reloading list of sources on main thread";
	//else
	//	qDebug() << "ERR: reloading list of sources on NON-MAIN thread!";
	
	
	
	ui->videoSourceComboBox->blockSignals(true);
	//QObject::disconnect(ui->videoSourceComboBox, 0, 0, 0);
	
	//	clear the list of video sources
	ui->videoSourceComboBox->clear();
	
	//	get a new list of video source menu items, use them to populate the combo box
	QList<MediaFile>		newFiles = inSrc->createListOfStaticMediaFiles();
	for (const MediaFile & newFile : newFiles)	{
		//qDebug() << "\titem " << newFile.name();
		ui->videoSourceComboBox->addItem(newFile.name(), QVariant::fromValue(newFile));
	}
	
	//	run through the items in the combo box, select the first item that appears to be playing back
	int				foundIndex = -1;
	for (int i=0; i<ui->videoSourceComboBox->count(); ++i)	{
		QString			tmpItemString = ui->videoSourceComboBox->itemText(i);
		QVariant		tmpItemVariant = ui->videoSourceComboBox->itemData(i);
		//qDebug() << "\tcomparing against camera " << tmpInfo.description();
		if (GetDynamicVideoSource()->playingBackItem(tmpItemVariant.value<MediaFile>()))	{
			foundIndex = i;
			break;
		}
	}
	
	ui->videoSourceComboBox->setCurrentIndex(foundIndex);
	ui->videoSourceComboBox->blockSignals(false);
	
}
void LoadingWindow::videoSourceChanged(int arg1)	{
	qDebug() << __PRETTY_FUNCTION__;
	
	DynamicVideoSource		*dvs = GetDynamicVideoSource();
	if (dvs != nullptr)	{
		//	the combo box stores a QVariant<MediaFile> for each item
		QVariant		selectedData = ui->videoSourceComboBox->currentData();
		if (selectedData.isValid())	{
			MediaFile		selectedMediaFile = selectedData.value<MediaFile>();
			dvs->loadFile(selectedMediaFile);
		}
	}
}




void LoadingWindow::setBaseDirectory(const QString & inBaseDir)	{
	qDebug() << __PRETTY_FUNCTION__ << ", " << inBaseDir;
	
	if (qApp->thread() != QThread::currentThread()) qDebug() << "ERR: thread is not main! " << __PRETTY_FUNCTION__;
	
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
	LoadingWindowFileListModel		*newModel = new LoadingWindowFileListModel(this);
	newModel->setReadOnly(true);
	//newModel->setFilter(QDir::Files | QDir::NoSymLinks | QDir::NoDotAndDotDot | QDir::Readable);
	newModel->setNameFilters(QStringList(QString("*.fs")));
	newModel->setNameFilterDisables(false);
	ui->filterListView->setModel(newModel);
	ui->filterListView->setRootIndex(newModel->setRootPath(baseDirectory));
	
	//	delete the old model
	if (oldModel != nullptr)
		delete oldModel;
	
	//	set a new selection model
	//	we need to load a new file as the selection changes
	QItemSelectionModel		*selModel = ui->filterListView->selectionModel();
	if (selModel != nullptr)	{
		//connect(selModel, &QItemSelectionModel::selectionChanged, this, &LoadingWindow::newFileSelected);
		connect(selModel, &QItemSelectionModel::selectionChanged, [&](const QItemSelection &selected, const QItemSelection &deselected)	{
			qDebug() << __FUNCTION__ << "->&QItemSelectionModel::selectionChanged";
			QList<QModelIndex>		selectedIndexes = selected.indexes();
			if (selectedIndexes.count() < 1)
				return;
			QModelIndex		firstIndex = selectedIndexes.first();
			if (!firstIndex.isValid())
				return;
			QVariant		selectedPath = firstIndex.data(QFileSystemModel::FilePathRole);
			if (selectedPath.isNull())
				return;
			qDebug() << "\tshould be loading file " << selectedPath.toString();
			
			QString			selectedPathString = selectedPath.toString();
			//GetISFController()->loadFile(selectedPathString);
			//GetDocWindow()->loadFile(selectedPathString);
			
			on_loadFile(selectedPathString);
		});
	}
}




LoadingWindow * GetLoadingWindow()	{
	return globalLoadingWindow;
}
