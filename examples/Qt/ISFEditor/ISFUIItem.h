#ifndef ISFUIITEM_H
#define ISFUIITEM_H

#include <QGroupBox>
#include <QPushButton>
#include <QCheckBox>
#include <QComboBox>
#include <QSlider>
#include <QDoubleSpinBox>
#include <QHash>
#include <QString>
#include <QVariant>
#include <QColor>
#include <QLabel>

#include "VVISF.hpp"
#include "ISFRemoteImageClient.h"
#include "QDoubleSlider.h"




using namespace VVGL;
using namespace VVISF;




class ISFUIItem : public QGroupBox
{
	Q_OBJECT
public:
	explicit ISFUIItem(const ISFAttrRef & inAttr, QWidget * inParent=nullptr);
	~ISFUIItem();

	QString getName() { return name; }
	ISFVal getISFVal();

private slots:
	void pointWidgetUsed(double newVal);
	void audioCBUsed(int newIndex);
	
private:
	QString			name;
	
	ISFValType		type = ISFValType_None;
	
	//	all of these widgets are owned by my layout
	QPushButton		*eventWidget = nullptr;
	bool			eventNeedsSending = false;
	QCheckBox		*boolWidget = nullptr;
	QComboBox		*longCBWidget = nullptr;
	QDoubleSlider	*sliderWidget = nullptr;
	
	QDoubleSpinBox	*xFieldWidget = nullptr;
	QDoubleSpinBox	*yFieldWidget = nullptr;
	Point			pointVal = Point(0.0, 0.0);
	QPushButton		*colorButton = nullptr;
	QLabel			*colorLabel = nullptr;
	QColor			color = QColor(0, 0, 0, 0);
	QComboBox		*audioSourceCB = nullptr;
	
	QHash<QString, QVariant>	userInfoDict;	//	used to store float flag and max val for audio-type inputs
	
	ISFRemoteImageClient	*remoteImageClient = nullptr;
	QString					*remoteImageClientLastSelectedName = nullptr;
};




#endif // ISFUIITEM_H
