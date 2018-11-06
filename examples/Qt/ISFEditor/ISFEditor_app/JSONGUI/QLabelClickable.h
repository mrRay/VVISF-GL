#ifndef QLABELCLICKABLE_H
#define QLABELCLICKABLE_H

#include <QLabel>
#include <QWidget>




class QLabelClickable : public QLabel
{
	Q_OBJECT
	
public:
	explicit QLabelClickable(QWidget * inParent=nullptr, Qt::WindowFlags f=Qt::WindowFlags()) : QLabel(inParent,f) {}
	~QLabelClickable() {}
	
signals:
	void clicked();
	
protected:
	void mousePressEvent(QMouseEvent * inEvent)	{
		QLabel::mousePressEvent(inEvent);
		emit clicked();
	}
};




#endif // QLABELCLICKABLE_H