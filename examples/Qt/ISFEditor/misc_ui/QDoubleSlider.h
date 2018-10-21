#ifndef QDOUBLESLIDER_H
#define QDOUBLESLIDER_H

#include <QSlider>
#include <QDebug>

#include <algorithm>




class QDoubleSlider : public QSlider	{
	Q_OBJECT
	
public:
	explicit QDoubleSlider(QWidget * inParent=nullptr) : QSlider(inParent)	{
		setMinimum(INT_MIN);
		setMaximum(INT_MAX);
		_doubleMinValue = 0.0;
		_doubleMaxValue = 1.0;
		//connect(this, &QAbstractSlider::sliderMoved, this, &QDoubleSlider::intSliderMoved);
		//connect(this, SIGNAL(sliderMoved(int)), this, SLOT(intSliderMoved(int)));
		connect(this, &QAbstractSlider::sliderMoved, [&](int newVal)	{
			Q_UNUSED(newVal);
			emit doubleSliderMoved(doubleValue());
		});
	}
	
	
	double doubleMinValue() {
		return _doubleMinValue;
	}
	void setDoubleMinValue(const double & n)	{
		_doubleMinValue = n;
		emit doubleRangeChanged(_doubleMinValue, _doubleMaxValue);
	}
	
	
	double doubleMaxValue() {
		return _doubleMaxValue;
	}
	void setDoubleMaxValue(const double & n)	{
		_doubleMaxValue = n;
		emit doubleRangeChanged(_doubleMinValue, _doubleMaxValue);
	}
	
	
	double doubleValue()	{
		double			tmpMin = static_cast<double>(INT_MIN);
		double			tmpMax = static_cast<double>(INT_MAX);
		double			tmpVal = static_cast<double>(value());
		
		double			tmpNormVal = (tmpVal - tmpMin) / (tmpMax - tmpMin);
		
		double			newDoubleVal = (tmpNormVal * (_doubleMaxValue - _doubleMinValue)) + _doubleMinValue;
		return newDoubleVal;
	}
	void setDoubleValue(const double & n)	{
		qDebug() << __PRETTY_FUNCTION__ << "... " << n;
		double			tmpDoubleVal = std::max<double>( std::min<double>( _doubleMaxValue,n ), _doubleMinValue );
		double			tmpNormVal = ((tmpDoubleVal-_doubleMinValue)/(_doubleMaxValue-_doubleMinValue));
		double			tmpMin = static_cast<double>(INT_MIN);
		double			tmpMax = static_cast<double>(INT_MAX);
		int				tmpVal = static_cast<int>( (tmpNormVal * (tmpMax-tmpMin)) + tmpMin );
		setValue(tmpVal);
	}

signals:
	void doubleRangeChanged(double newMin, double newMax);
	void doubleSliderMoved(double newValue);
	
private:
	double		_doubleMinValue;
	double		_doubleMaxValue;
};




#endif // QDOUBLESLIDER_H
