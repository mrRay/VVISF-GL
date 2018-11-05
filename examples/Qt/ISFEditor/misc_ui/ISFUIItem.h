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
#include "InterAppVideoSource.h"
#include "QDoubleSlider.h"




using namespace VVGL;
using namespace VVISF;




class ISFUIItem : public QGroupBox
{
	Q_OBJECT
public:
	explicit ISFUIItem(const ISFAttrRef & inAttr, QWidget * inParent=nullptr);
	~ISFUIItem();

	QString name() { return _name; }
	ISFVal getISFVal();

private slots:
	void pointWidgetUsed();
	void interAppVideoCBUsed(int newIndex);
	void audioCBUsed(int newIndex);
	
private:
	QString			_name;
	
	ISFValType		type = ISFValType_None;
	ISFAttrRef		attr = nullptr;
	
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
	QComboBox		*interAppVideoCB = nullptr;
	QComboBox		*audioSourceCB = nullptr;
	
	QHash<QString, QVariant>	userInfoDict;	//	used to store float flag and max val for audio-type inputs
	
	InterAppVideoSource		*interAppSrc = nullptr;
	GLBufferRef				interAppBuffer = nullptr;
	
	void refreshInterAppVideoCB();
	void refreshAudioSourceCB();
};




#endif // ISFUIITEM_H
