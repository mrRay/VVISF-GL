#ifndef JSONGUIPASSWIDGET_H
#define JSONGUIPASSWIDGET_H

#include <QWidget>
class QLabel;
class QComboBox;
class QLineEdit;
class QLabelClickable;
class QLabelDrag;

#include "JGMObject.h"



namespace Ui {
	class JSONGUIPassWidget;
}




class JSONGUIPassWidget : public QWidget
{
	Q_OBJECT

public:
	explicit JSONGUIPassWidget(const JGMPassRef & inRef, QWidget *parent = nullptr);
	~JSONGUIPassWidget();
	
	virtual void prepareUIItems();
	virtual void refreshUIItems();
	
	//void setDropEdge(const Qt::Edge & n) { _dropEdge=n; }
	//Qt::Edge dropEdge() { return _dropEdge; }
	
protected:
	virtual void dragEnterEvent(QDragEnterEvent * e) Q_DECL_OVERRIDE;
	virtual void dragMoveEvent(QDragMoveEvent * e) Q_DECL_OVERRIDE;
	virtual void dragLeaveEvent(QDragLeaveEvent * e) Q_DECL_OVERRIDE;
	virtual void dropEvent(QDropEvent * e) Q_DECL_OVERRIDE;
	virtual void paintEvent(QPaintEvent * e) Q_DECL_OVERRIDE;
	
	void prepareDragLabel(QLabelDrag * dragLabel);
	void prepareDeleteLabel(QLabelClickable * deleteLabel);
	void prepareBufferNameEdit(QLineEdit * bufferNameEdit);
	void preparePBufferCBox(QCheckBox * pbufferCBox);
	void prepareFBufferCBox(QCheckBox * fbufferCBox);
	void prepareCustWidthEdit(QLineEdit * custWidthEdit);
	void prepareCustHeightEdit(QLineEdit * custHeightEdit);
	
	void refreshPassTitleLabel(QLabel * passNameLabel);
	void refreshBufferNameEdit(QLineEdit * bufferNameEdit);
	void refreshPBufferCBox(QCheckBox * pbufferCBox);
	void refreshFBufferCBox(QCheckBox * fbufferCBox);
	void refreshCustWidthEdit(QLineEdit * custWidthEdit);
	void refreshCustHeightEdit(QLineEdit * custHeightEdit);

private:
	Ui::JSONGUIPassWidget *ui;
	
	JGMPassRef			_pass = nullptr;
	Qt::Edge			_dropEdge = Qt::LeftEdge;
};




#endif // JSONGUIPASSWIDGET_H
