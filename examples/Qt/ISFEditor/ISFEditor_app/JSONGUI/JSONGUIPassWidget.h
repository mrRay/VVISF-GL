#ifndef JSONGUIPASSWIDGET_H
#define JSONGUIPASSWIDGET_H

#include <QWidget>
#include <QPointer>
class QLabel;
class QComboBox;
class QCheckBox;
class QLineEdit;
class QLabelClickable;
class QLabelDrag;

#include "JGMObject.h"
#include "JSONScrollWidget.h"



namespace Ui {
	class JSONGUIPassWidget;
}




class JSONGUIPassWidget : public QWidget
{
	Q_OBJECT

public:
	//	my parent is the JSONScrollView that created me!
	explicit JSONGUIPassWidget(const JGMPassRef & inRef, JSONScrollWidget * inScrollWidget, QWidget *parent = nullptr);
	~JSONGUIPassWidget() override;
	
	virtual void prepareToBeDeleted();
	
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
	QPointer<JSONScrollWidget>		_parentScroll = nullptr;
};




#endif // JSONGUIPASSWIDGET_H
