#ifndef ISFFILELISTVIEW_H
#define ISFFILELISTVIEW_H

#include <QObject>
#include <QListView>




class ISFFileListView : public QListView
{
	Q_OBJECT
	
public:
	ISFFileListView(QWidget * parent=nullptr);
	~ISFFileListView();
	
protected:
	/*
	void dragEnterEvent(QDragEnterEvent * e) Q_DECL_OVERRIDE;
	void dragMoveEvent(QDragMoveEvent * e) Q_DECL_OVERRIDE;
	void dragLeaveEvent(QDragLeaveEvent * e) Q_DECL_OVERRIDE;
	void dropEvent(QDropEvent * e) Q_DECL_OVERRIDE;
	*/
};




#endif // ISFFILELISTVIEW_H