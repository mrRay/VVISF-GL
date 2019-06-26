#ifndef DISPLAYLINKDRIVER_H
#define DISPLAYLINKDRIVER_H

#include <QObject>
#include <functional>




class DisplayLinkDriver : public QObject
{
	Q_OBJECT
	
	using DisplayLinkCallback = std::function<void()>;
	
public:
	//	moves the passed context to the thread (to self), creates a buffer pool & thread copier using the ctx, will use the ctx to perform all rendering
	explicit DisplayLinkDriver(QObject * parent = nullptr);
	~DisplayLinkDriver();
	
	void performCallback();
	
	void setDisplayLinkCallback(const DisplayLinkCallback & n=nullptr);
	
public slots:
	Q_SLOT void start();
	Q_SLOT void stop();
	
private:
	void					*_displayLink = NULL;	//	really a CVDisplayLinkRef
	
	DisplayLinkCallback		_displayLinkCallback = nullptr;
};




#endif // DISPLAYLINKDRIVER_H
