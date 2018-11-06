#include "QDoubleSlider.h"



QDoubleSlider::QDoubleSlider(QWidget * inParent) : QSlider(inParent)	{
	qDebug() << __PRETTY_FUNCTION__;
	setMinimum(INT_MIN);
	setMaximum(INT_MAX);
	_doubleMinValue = 0.0;
	_doubleMaxValue = 1.0;
	connect(this, &QAbstractSlider::sliderMoved, this, &QDoubleSlider::intSliderMoved);
}
QDoubleSlider::~QDoubleSlider()	{
	qDebug() << __PRETTY_FUNCTION__;
}




double QDoubleSlider::doubleMinValue() {
	return _doubleMinValue;
}
void QDoubleSlider::setDoubleMinValue(const double & n)	{
	_doubleMinValue = n;
	emit doubleRangeChanged(_doubleMinValue, _doubleMaxValue);
}


double QDoubleSlider::doubleMaxValue() {
	return _doubleMaxValue;
}
void QDoubleSlider::setDoubleMaxValue(const double & n)	{
	_doubleMaxValue = n;
	emit doubleRangeChanged(_doubleMinValue, _doubleMaxValue);
}


double QDoubleSlider::doubleValue()	{
	double			tmpMin = static_cast<double>(INT_MIN);
	double			tmpMax = static_cast<double>(INT_MAX);
	double			tmpVal = static_cast<double>(value());
	
	double			tmpNormVal = (tmpVal - tmpMin) / (tmpMax - tmpMin);
	
	double			newDoubleVal = (tmpNormVal * (_doubleMaxValue - _doubleMinValue)) + _doubleMinValue;
	return newDoubleVal;
}



void QDoubleSlider::intSliderMoved(int newVal)	{
	/*
	double			tmpMin = static_cast<double>(INT_MIN);
	double			tmpMax = static_cast<double>(INT_MAX);
	double			tmpVal = static_cast<double>(newVal);
	
	double			tmpNormVal = (tmpVal - tmpMin) / (tmpMax - tmpMin);
	
	double			newDoubleVal = (tmpNormVal * (doubleMaxValue - _doubleMinValue)) + _doubleMinValue;
	
	emit doubleSliderMoved(newDoubleVal);
	*/
	
	Q_UNUSED(newVal);
	emit doubleSliderMoved(doubleValue());
	
}