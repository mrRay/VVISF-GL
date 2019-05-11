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




//using namespace VVGL;
//using namespace VVISF;




class ISFUIItem : public QGroupBox
{
	Q_OBJECT
public:
	explicit ISFUIItem(const VVISF::ISFAttrRef & inAttr, QWidget * inParent=nullptr);
	~ISFUIItem();

	QString name() { return _name; }
	VVISF::ISFVal getISFVal();

private slots:
	void outputWindowMouseUsed(VVGL::Point normMouseEventLoc, VVGL::Point absMouseEventLoc);
	void longWidgetUsed();
	void pointWidgetUsed();
	void interAppVideoCBUsed(int newIndex);
	void audioCBUsed(int newIndex);
	
private:
	QString			_name;
	
	VVISF::ISFValType		type = VVISF::ISFValType_None;
	VVISF::ISFAttrRef		attr = nullptr;
	
	//	all of these widgets are owned by my layout
	QPushButton		*eventWidget = nullptr;
	bool			eventNeedsSending = false;
	QCheckBox		*boolWidget = nullptr;
	QComboBox		*longCBWidget = nullptr;
	QSpinBox		*longFieldWidget = nullptr;
	QSlider			*longSliderWidget = nullptr;
	QDoubleSlider	*floatSliderWidget = nullptr;
	
	QDoubleSpinBox	*xFieldWidget = nullptr;
	QDoubleSpinBox	*yFieldWidget = nullptr;
	QDoubleSlider	*xSliderWidget = nullptr;
	QDoubleSlider	*ySliderWidget = nullptr;
	VVGL::Point			pointVal = VVGL::Point(0.0, 0.0);
	QPushButton		*colorButton = nullptr;
	QLabel			*colorLabel = nullptr;
	QColor			color = QColor(0, 0, 0, 0);
	QComboBox		*interAppVideoCB = nullptr;
	QComboBox		*audioSourceCB = nullptr;
	QLabel			*audioLabel = nullptr;
	
	QHash<QString, QVariant>	userInfoDict;	//	used to store float flag and max val for audio-type inputs
	
	InterAppVideoSource		*interAppSrc = nullptr;
	VVGL::GLBufferRef				interAppBuffer = nullptr;
	
	void refreshInterAppVideoCB();
	void refreshAudioSourceCB();
};




#endif // ISFUIITEM_H
