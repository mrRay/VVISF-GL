#ifndef LOADINGWINDOWFILELISTMODEL_H
#define LOADINGWINDOWFILELISTMODEL_H

#include <QFileSystemModel>




class LoadingWindowFileListModel : public QFileSystemModel
{
	Q_OBJECT
	
public:
	LoadingWindowFileListModel(QObject * parent=nullptr);
	
	virtual Qt::DropActions supportedDropActions() const override;
	virtual Qt::ItemFlags flags(const QModelIndex &index) const override;
	
	//virtual bool canDropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) override;
	virtual bool canDropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) const override;
	virtual bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) override;
};




#endif // LOADINGWINDOWFILELISTMODEL_H