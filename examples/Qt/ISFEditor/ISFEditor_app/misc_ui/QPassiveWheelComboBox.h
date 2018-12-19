#ifndef QPASSIVEWHEELCOMBOBOX_H
#define QPASSIVEWHEELCOMBOBOX_H

#include <QComboBox>




class QPassiveWheelComboBox : public QComboBox
{
	Q_OBJECT

public:
	explicit QPassiveWheelComboBox(QWidget * inParent=nullptr) : QComboBox(inParent) {}
	void wheelEvent(QWheelEvent * e)	{
		if (hasFocus())
			QComboBox::wheelEvent(e);
	}
};




#endif // QPASSIVEWHEELCOMBOBOX_H
