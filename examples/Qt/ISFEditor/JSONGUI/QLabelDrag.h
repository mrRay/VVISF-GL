#ifndef QLABELJGMDRAG_H
#define QLABELJGMDRAG_H

#include <QDrag>
#include <QDebug>
#include <QMimeData>




/*		label class that starts a drag.  the drag type and drag data are configurable.		*/
class QLabelDrag : public QLabel
{
public:
	QLabelDrag(QWidget * parent=nullptr) : QLabel(parent)	{}
	
	void mousePressEvent(QMouseEvent * e)	{
		qDebug() << __PRETTY_FUNCTION__;
		
		QDrag		*drag = new QDrag(this);
		drag->setPixmap(this->parentWidget()->grab());
		
		QMimeData	*md = new QMimeData;
		md->setData(mimeType, dragVariant.toByteArray());
		//md->setData(mimeScrollType, QVariant().toByteArray());
		drag->setMimeData(md);
		
		qDebug() << "\tbeginning drag...";
		Qt::DropAction		dropAction = drag->exec();
		qDebug() << "\tdrag->exec() returned...";
	}
	
public:
	QString			mimeType = QString("text/JGMDrag");
	QString			mimeScrollType = QString("text/JGMDrag");
	QVariant		dragVariant = QVariant( QString("XXX") );
};




#endif // QLABELJGMDRAG_H