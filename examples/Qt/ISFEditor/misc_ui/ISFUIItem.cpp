#include "ISFUIItem.h"

#include <algorithm>

#include <QDebug>
#include <QVBoxLayout>
#include <QColorDialog>
#include <QTimer>




ISFUIItem::ISFUIItem(const ISFAttrRef & inAttr, QWidget * inParent) : QGroupBox(inParent)	{
	//qDebug() << __PRETTY_FUNCTION__;
	
	name = QString::fromStdString(inAttr->getName());
	type = inAttr->getType();
	
	string		tmpLabel = inAttr->getLabel();
	if (tmpLabel.length() < 1)
		setTitle(name);
	else	{
		QString		tmpTitle = QString("%1 (%2)").arg(QString::fromStdString(tmpLabel)).arg(name);
		setTitle(tmpTitle);
	}
	
	setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
	
	QLayout		*myLayout = new QHBoxLayout(this);
	setLayout(myLayout);
	myLayout = layout();
	
	switch (type)	{
	case ISFValType_None:
		break;
	case ISFValType_Event:
		eventWidget = new QPushButton("Button", this);
		myLayout->addWidget(eventWidget);
		connect(eventWidget, &QAbstractButton::pressed, [&]()	{
			eventNeedsSending = true;
		});
		break;
	case ISFValType_Bool:
		boolWidget = new QCheckBox("Toggle", this);
		boolWidget->setChecked(inAttr->getCurrentVal().getBoolVal());
		myLayout->addWidget(boolWidget);
		//connect(boolWidget, SIGNAL(stateChanged(int)), this, SLOT(boolWidgetUsed(int)));
		break;
	case ISFValType_Long:
		{
			longCBWidget = new QComboBox(this);
			longCBWidget->clear();
			
			vector<string>		labelArray = inAttr->getLabelArray();
			vector<int32_t>		valArray = inAttr->getValArray();
			int					labelArraySize = labelArray.size();
			int					valArraySize = valArray.size();
			long				defaultValAsLong = inAttr->getDefaultVal().getLongVal();
			int					defaultValIndex = -1;
			int					tmpIndex = 0;
			//	if there's a val array, use it to populate the combo box
			if (valArraySize != 0)	{
				for (auto valIt=valArray.begin(); valIt!=valArray.end(); ++valIt)	{
					long		tmpLong = *valIt;
					if (tmpIndex < labelArraySize)	{
						QString		tmpKey = QString::fromStdString(labelArray[tmpIndex]);
						longCBWidget->addItem(tmpKey, QVariant(static_cast<qlonglong>(tmpLong)));
					}
					else	{
						QString		tmpKey = QString("%1").arg(tmpLong);
						longCBWidget->addItem(tmpKey, QVariant(static_cast<qlonglong>(tmpLong)));
					}
					
					if (tmpLong == defaultValAsLong)
						defaultValIndex = tmpIndex;
					++tmpIndex;
				}
			}
			//	else use the attribute's min/max vals
			else	{
				ISFVal		minVal = inAttr->getMinVal();
				ISFVal		maxVal = inAttr->getMaxVal();
				for (int i=std::min(minVal.getLongVal(),maxVal.getLongVal()); i<=std::max(minVal.getLongVal(),maxVal.getLongVal()); ++i)	{
					QString		tmpKey = QString("%1").arg(i);
					longCBWidget->addItem(tmpKey, QVariant(static_cast<qlonglong>(i)));
					
					if (i == defaultValAsLong)
						defaultValIndex = tmpIndex;
					++tmpIndex;
				}
			}
			//	if we found a default val index, apply it
			if (defaultValIndex >= 0)
				longCBWidget->setCurrentIndex(defaultValIndex);
			//	add the widget to the layout
			myLayout->addWidget(longCBWidget);
		}
		break;
	case ISFValType_Float:
		sliderWidget = new QDoubleSlider(this);
		sliderWidget->setOrientation(Qt::Horizontal);
		myLayout->addWidget(sliderWidget);
		break;
	case ISFValType_Point2D:
		{
			xFieldWidget = new QDoubleSpinBox(this);
			yFieldWidget = new QDoubleSpinBox(this);
		
			ISFVal		tmpVal = inAttr->getCurrentVal();
			ISFVal		tmpMin = inAttr->getMinVal();
			ISFVal		tmpMax = inAttr->getMaxVal();
			
			if (tmpMin.isPoint2DVal() && tmpMax.isPoint2DVal())	{
				xFieldWidget->setRange(tmpMin.getPointValByIndex(0), tmpMax.getPointValByIndex(0));
				yFieldWidget->setRange(tmpMin.getPointValByIndex(1), tmpMax.getPointValByIndex(1));
			}
			else	{
				xFieldWidget->setRange(0.0, 1.0);
				yFieldWidget->setRange(0.0, 1.0);
			}
			if (tmpVal.isPoint2DVal())	{
				xFieldWidget->setValue(tmpVal.getPointValByIndex(0));
				yFieldWidget->setValue(tmpVal.getPointValByIndex(1));
			}
			
			myLayout->addWidget(xFieldWidget);
			myLayout->addWidget(yFieldWidget);
		
			connect( xFieldWidget, SIGNAL(valueChanged(double)), this, SLOT(pointWidgetUsed(double)) );
			connect( yFieldWidget, SIGNAL(valueChanged(double)), this, SLOT(pointWidgetUsed(double)) );
		}
		break;
	case ISFValType_Color:
		{
			//	set the color ivar from the attribute's current value
			ISFVal		currentVal = inAttr->getCurrentVal();
			if (currentVal.isColorVal())	{
				int			tmpColor[4];
				for (int i=0; i<4; ++i)	{
					tmpColor[i] = static_cast<int>(currentVal.getColorValByChannel(i) * 255.0);
				}
				color = QColor(tmpColor[0], tmpColor[1], tmpColor[2], tmpColor[3]);
			}
		
			//	make the button we'll see in the UI- this is what will open the color picker
			colorButton = new QPushButton("Open Picker", this);
		
			//	make a label that will display the color so the user knows what color is presently set
			colorLabel = new QLabel(this);
			colorLabel->setAutoFillBackground(true);
			QPalette		tmpPalette = colorLabel->palette();
			tmpPalette.setColor(colorLabel->backgroundRole(), color);
			colorLabel->setAutoFillBackground(true);
			colorLabel->setPalette(tmpPalette);
		
			//	the color button should open a color dialog
			connect(colorButton, &QAbstractButton::clicked, [&](bool /*inChecked*/)	{
				//	make a color dialog
				QColorDialog		*colorDialog = new QColorDialog(this);
				colorDialog->setOptions( QColorDialog::DontUseNativeDialog );
				colorDialog->setAttribute( Qt::WA_DeleteOnClose );
				//	the color dialog should update the local color ivar and the label's background color as its color is updated
				connect(colorDialog, &QColorDialog::currentColorChanged, [&](const QColor & inColor)	{
					color = inColor;
			
					QPalette		tmpPalette = colorLabel->palette();
					tmpPalette.setColor(colorLabel->backgroundRole(), color);
					colorLabel->setPalette(tmpPalette);
				});
				//	open the color dialog
				colorDialog->open();
			});
		
			//	add the button and label to the layout
			myLayout->addWidget(colorButton);
			myLayout->addWidget(colorLabel);
		}
		break;
	case ISFValType_Cube:
		break;
	case ISFValType_Image:
		break;
	case ISFValType_Audio:
		break;
	case ISFValType_AudioFFT:
		break;
	}
	
}

ISFUIItem::~ISFUIItem()	{
	//qDebug() << __PRETTY_FUNCTION__;
	
	QLayout		*myLayout = layout();
	if (myLayout != nullptr)	{
		delete myLayout;
	}
}




void ISFUIItem::pointWidgetUsed(double newVal)	{
	qDebug() << __PRETTY_FUNCTION__;
	QObject		*tmpSender = sender();
	if (tmpSender == xFieldWidget)	{
		pointVal.x = newVal;
	}
	else if (tmpSender == yFieldWidget)	{
		pointVal.y = newVal;
	}
}
void ISFUIItem::audioCBUsed(int newIndex)	{
	qDebug() << __PRETTY_FUNCTION__;
}




ISFVal ISFUIItem::getISFVal()	{
	//qDebug() << __PRETTY_FUNCTION__;
	//return ISFNullVal();
	
	
	switch (type)	{
	case ISFValType_None:
		return ISFNullVal();
	case ISFValType_Event:
		if (eventNeedsSending)	{
			eventNeedsSending = false;
			return ISFEventVal(true);
		}
		return ISFEventVal();
	case ISFValType_Bool:
		if (boolWidget == nullptr)
			return ISFBoolVal(false);
		return ISFBoolVal(boolWidget->isChecked());
	case ISFValType_Long:
		if (longCBWidget == nullptr)
			return ISFLongVal(0);
		return ISFLongVal( longCBWidget->currentData().toLongLong() );
	case ISFValType_Float:
		if (sliderWidget == nullptr)
			return ISFFloatVal(0.0);
		return ISFFloatVal( sliderWidget->doubleValue() );
	case ISFValType_Point2D:
		if (xFieldWidget==nullptr || yFieldWidget==nullptr)
			return ISFPoint2DVal(0.0, 0.0);
		return ISFPoint2DVal( xFieldWidget->value(), yFieldWidget->value() );
	case ISFValType_Color:
		return ISFColorVal(color.redF(), color.greenF(), color.blueF(), color.alphaF());
	case ISFValType_Cube:
	case ISFValType_Image:
	case ISFValType_Audio:
	case ISFValType_AudioFFT:
		break;
	}
	
	return ISFNullVal();
}
