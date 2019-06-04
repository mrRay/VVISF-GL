#ifndef FILELOADEVENTFILTER_H
#define FILELOADEVENTFILTER_H




class FileLoadEventFilter : public QObject
{
public:
	FileLoadEventFilter(QObject * inParent=nullptr) : QObject(inParent)	{
	}
	
	virtual bool eventFilter(QObject * watched, QEvent * event) override;
};




#endif // FILELOADEVENTFILTER_H
