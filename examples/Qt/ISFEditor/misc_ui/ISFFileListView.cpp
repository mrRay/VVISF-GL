#include "ISFFileListView.h"

#include <QDebug>
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDragLeaveEvent>
#include <QDropEvent>
#include <QMimeData>



ISFFileListView::ISFFileListView(QWidget * parent) : QListView(parent)	{
	//setDragEnabled(true);
	setAcceptDrops(true);
	setDragDropMode(QAbstractItemView::DropOnly);
	setDropIndicatorShown(true);
	//setSelectionMode(QAbstractItemView::ExtendedSelection);
}
ISFFileListView::~ISFFileListView()	{

}

/*
void ISFFileListView::dragEnterEvent(QDragEnterEvent * e)	{
	qDebug() << __PRETTY_FUNCTION__;
	const QMimeData		*md = e->mimeData();
	//QStringList			fmts = md->formats();
	//qDebug() << "\tfmts are " << fmts;
	if (md->hasFormat("text/uri-list"))	{
		e->acceptProposedAction();
	}
}
void ISFFileListView::dragMoveEvent(QDragMoveEvent * e)	{
	qDebug() << __PRETTY_FUNCTION__;
	e->accept();
}
void ISFFileListView::dragLeaveEvent(QDragLeaveEvent * e)	{
	qDebug() << __PRETTY_FUNCTION__;
	e->accept();
}
void ISFFileListView::dropEvent(QDropEvent * e)	{
	qDebug() << __PRETTY_FUNCTION__;
	e->accept();
}
*/
