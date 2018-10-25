#ifndef JSONGUIINPUTWIDGET_H
#define JSONGUIINPUTWIDGET_H

#include <QWidget>
class QLabel;
class QComboBox;
class QLineEdit;
class QLabelClickable;
class QLabelDrag;

#include "JGMObject.h"




class JSONGUIInputWidget : public QWidget
{
	Q_OBJECT
public:
	explicit JSONGUIInputWidget(const JGMInputRef & inInput, QWidget *parent = nullptr);
	
protected:
	virtual void dragEnterEvent(QDragEnterEvent * e) Q_DECL_OVERRIDE;
	virtual void dragMoveEvent(QDragMoveEvent * e) Q_DECL_OVERRIDE;
	virtual void dragLeaveEvent(QDragLeaveEvent * e) Q_DECL_OVERRIDE;
	virtual void dropEvent(QDropEvent * e) Q_DECL_OVERRIDE;
	virtual void paintEvent(QPaintEvent * e) Q_DECL_OVERRIDE;
	
	void prepareDragLabel(QLabelDrag * dragLabel);
	void prepareInputNameEdit(QLineEdit * inputNameEdit);
	void prepareLabelField(QLineEdit * labelField);
	void prepareTypeCBox(QComboBox * typeCB);
	void prepareDeleteLabel(QLabelClickable * deleteLabel);
	
	void refreshInputNameEdit(QLineEdit * inputNameEdit);
	void refreshLabelField(QLineEdit * labelField);
	void refreshTypeCBox(QComboBox * typeCB);
	
	virtual void prepareUIItems() = 0;
	virtual void refreshUIItems() = 0;
	
protected:
	JGMInputRef			_input = nullptr;
	Qt::Edge			_dropEdge = Qt::LeftEdge;
};




#endif // JSONGUIINPUTWIDGET_H