#include "LoadingWindowFileListModel.h"

#include <QStringListModel>
#include <QDebug>
#include <QMimeData>
#include <QUrl>

#include "LoadingWindow.h"




LoadingWindowFileListModel::LoadingWindowFileListModel(QObject * parent) :
	QFileSystemModel(parent)
{
	qDebug() << __PRETTY_FUNCTION__;
	setReadOnly(true);
	//setReadOnly(false);

}






Qt::DropActions LoadingWindowFileListModel::supportedDropActions() const
{
	return Qt::CopyAction | Qt::MoveAction;
}

Qt::ItemFlags LoadingWindowFileListModel::flags(const QModelIndex &index) const
{
	Qt::ItemFlags defaultFlags = QFileSystemModel::flags(index);

	if (index.isValid())	{
		return Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | defaultFlags;
		//return Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled;
	}
	else	{
		return Qt::ItemIsDropEnabled | defaultFlags;
		//return Qt::ItemIsDropEnabled;
	}
}






bool LoadingWindowFileListModel::canDropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) const
{
	Q_UNUSED(action);
	Q_UNUSED(row);
	Q_UNUSED(parent);
	Q_UNUSED(column);

	if (!data->hasFormat("text/uri-list"))
		return false;

	//if (column > 0)
	//	return false;

	return true;
}
bool LoadingWindowFileListModel::dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent)
{
	qDebug() << __PRETTY_FUNCTION__;
	if (!canDropMimeData(data, action, row, column, parent))
		return false;
	
	
	LoadingWindow		*lw = GetLoadingWindow();
	if (lw != nullptr)	{
		QList<QUrl>		urls = data->urls();
		for (const QUrl & url : urls)	{
			//qDebug() << "url is " << url << ", as string it's " << url.toString();
			QFileInfo		fi(url.toString());
			//qDebug() << "file info is " << fi;
			
			if (fi.isDir())	{
				//qDebug() << "file is dir, path is " << fi.absolutePath();
				lw->setBaseDirectory( fi.absolutePath() );
			}
			else	{
				QDir			parentDir = fi.dir();
				//qDebug() << "file is file, parentDir is " << parentDir;
				//qDebug() << "parentDir absoluteFilePath is " << parentDir.absolutePath();
				//qDebug() << "parentDir canonicalPath is " <<  parentDir.canonicalPath();
				//qDebug() << "parentDir path is " << parentDir.path();
				//QFileInfo		altFI(parentDir.path());
				//qDebug() << "altFI is " << altFI;
				//qDebug() << "altFI path is " << altFI.path();
				//qDebug() << "altFI absolutePath is " << altFI.absolutePath();
				//qDebug() << "altFI absoluteFilePath is " << altFI.absoluteFilePath();
				//qDebug() << "test string is " << parentDir.path().remove(0,7);
				lw->setBaseDirectory( parentDir.path().remove(0,7) );
			}
			break;
		}
	}
	
	return true;
}
