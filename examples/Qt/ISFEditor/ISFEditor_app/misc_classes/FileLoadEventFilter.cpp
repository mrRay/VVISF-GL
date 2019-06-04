#include "FileLoadEventFilter.h"

#include <QDebug>

#include "LoadingWindow.h"




bool FileLoadEventFilter::eventFilter(QObject * watched, QEvent * event)	{
	//qDebug() << __PRETTY_FUNCTION__;
	//return false;
	
	
	QString			fileToOpen("");
	
	switch (event->type())	{
	case QEvent::FileOpen:
		event->accept();
		qDebug() << "Event FileOpen, " << static_cast<QFileOpenEvent*>(event)->file();
		fileToOpen = static_cast<QFileOpenEvent*>(event)->file();
		break;
	case QEvent::DragEnter:
	case QEvent::DragLeave:
		event->accept();
		qDebug() << "Event DragEnter";
		break;
	case QEvent::Drop:
		{
			event->accept();
			const QMimeData		*mimeData = static_cast<QDropEvent *>(event)->mimeData();
			// If there is one file (not more) we open it
			if (mimeData->urls().length() == 1) {
				//QString		fileName = mimeData->urls().first().toLocalFile();
				fileToOpen = mimeData->urls().first().toLocalFile();
				//qDebug() << "Event Drop, " << fileName;
			}
		}
		break;
	
	default:
		return false;
	}
	
	if (fileToOpen.length() > 0)	{
		QTimer::singleShot(500, [&,fileToOpen]()	{
			//	tell the loading window to load the base directory of the file that was dragged in and- if it was a file- select/load that file
			LoadingWindow		*lw = GetLoadingWindow();
			if (lw != nullptr)	{
				QFileInfo		fileInfo(fileToOpen);
				if (fileInfo.exists())	{
					if (fileInfo.isDir())	{
						lw->setBaseDirectory(fileToOpen);
					}
					else	{
						lw->setBaseDirectory(fileInfo.dir().path());
						lw->selectFile(fileToOpen);
					}
				}
				if (fileInfo.exists() && !fileInfo.isDir())	{
		
				}
			}
		});
	}
	
	return true;
	
}

