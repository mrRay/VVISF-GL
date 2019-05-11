#include "ISFUIItem.h"

#include <algorithm>

#include <QDebug>
#include <QVBoxLayout>
#include <QColorDialog>
#include <QTimer>

#include "AudioController.h"
#include "OutputWindow.h"




using namespace std;
using namespace VVGL;
using namespace VVISF;




ISFUIItem::ISFUIItem(const ISFAttrRef & inAttr, QWidget * inParent) : QGroupBox(inParent)	{
	//qDebug() << __PRETTY_FUNCTION__;
	
	_name = QString::fromStdString(inAttr->name());
	type = inAttr->type();
	attr = inAttr;
	
	string		tmpLabel = inAttr->label();
	if (tmpLabel.length() < 1)
		setTitle(_name);
	else	{
		QString		tmpTitle = QString("%1 (%2)").arg(QString::fromStdString(tmpLabel)).arg(_name);
		setTitle(tmpTitle);
	}
	
	setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
	
	//QLayout		*myLayout = new QHBoxLayout(this);
	//setLayout(myLayout);
	//myLayout = layout();
	QLayout		*myLayout = nullptr;
	
	switch (type)	{
	case ISFValType_None:
		break;
	case ISFValType_Event:
		myLayout = new QHBoxLayout(this);
		setLayout(myLayout);
		myLayout = layout();
		
		eventWidget = new QPushButton("Button", this);
		myLayout->addWidget(eventWidget);
		connect(eventWidget, &QAbstractButton::pressed, [&]()	{
			eventNeedsSending = true;
		});
		break;
	case ISFValType_Bool:
		myLayout = new QHBoxLayout(this);
		setLayout(myLayout);
		myLayout = layout();
		
		boolWidget = new QCheckBox("Toggle", this);
		boolWidget->setChecked(inAttr->currentVal().getBoolVal());
		myLayout->addWidget(boolWidget);
		//connect(boolWidget, SIGNAL(stateChanged(int)), this, SLOT(boolWidgetUsed(int)));
		break;
	case ISFValType_Long:
		{
			vector<string>		labelArray = inAttr->labelArray();
			vector<int32_t>		valArray = inAttr->valArray();
			int					labelArraySize = int(labelArray.size());
			int					valArraySize = int(valArray.size());
			long				defaultValAsLong = inAttr->defaultVal().getLongVal();
			int					defaultValIndex = -1;
			int					tmpIndex = 0;
			//	if there's a val array, we're making a combo box no matter what
			if (valArraySize != 0)	{
				longCBWidget = new QComboBox(this);
				longCBWidget->clear();
				
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
				
				//	if we found a default val index, apply it
				if (defaultValIndex >= 0)
					longCBWidget->setCurrentIndex(defaultValIndex);
				
				//	add the widget to the layout
				myLayout = new QHBoxLayout(this);
				setLayout(myLayout);
				myLayout = layout();
			
				myLayout->addWidget(longCBWidget);
			}
			//	else use the attribute's min/max vals
			else	{
				ISFVal		minVal = inAttr->minVal();
				ISFVal		maxVal = inAttr->maxVal();
				int			actualMin = std::min( minVal.getLongVal(), maxVal.getLongVal() );
				int			actualMax = std::max( minVal.getLongVal(), maxVal.getLongVal() );
				
				//	if the delta between the min and max vals is > 10, make a text field + slider.  if <= 10, make a pop-up button.
				if ((actualMax-actualMin) > 10)	{
					longFieldWidget = new QSpinBox(this);
					
					longFieldWidget->setRange(actualMin, actualMax);
					
					longSliderWidget = new QSlider(this);
					longSliderWidget->setOrientation(Qt::Horizontal);
					longSliderWidget->setRange(actualMin, actualMax);
					
					//	add the widgets to the layout
					myLayout = new QVBoxLayout(this);
					setLayout(myLayout);
					myLayout = layout();
					
					myLayout->addWidget(longFieldWidget);
					myLayout->addWidget(longSliderWidget);
					
					//	connect the widgets so the text field/slider update one another!
					connect( longFieldWidget, SIGNAL(editingFinished()), this, SLOT(longWidgetUsed()) );
					//connect( longSliderWidget, SIGNAL(sliderMoved()), this, SLOT(longWidgetUsed()) );
					connect( longSliderWidget, &QSlider::sliderMoved, this, &ISFUIItem::longWidgetUsed );
				}
				else	{
					longCBWidget = new QComboBox(this);
					longCBWidget->clear();
					
					for (int i=std::min(minVal.getLongVal(),maxVal.getLongVal()); i<=std::max(minVal.getLongVal(),maxVal.getLongVal()); ++i)	{
						QString		tmpKey = QString("%1").arg(i);
						longCBWidget->addItem(tmpKey, QVariant(static_cast<qlonglong>(i)));
					
						if (i == defaultValAsLong)
							defaultValIndex = tmpIndex;
						++tmpIndex;
					}
					
					//	if we found a default val index, apply it
					if (defaultValIndex >= 0)	{
						longCBWidget->setCurrentIndex(defaultValIndex);
					}
					
					//	add the widget to the layout
					myLayout = new QHBoxLayout(this);
					setLayout(myLayout);
					myLayout = layout();
			
					myLayout->addWidget(longCBWidget);
				}
				
			}
			
			
		}
		/*
		{
			longCBWidget = new QComboBox(this);
			longCBWidget->clear();
			
			vector<string>		labelArray = inAttr->labelArray();
			vector<int32_t>		valArray = inAttr->valArray();
			int					labelArraySize = int(labelArray.size());
			int					valArraySize = int(valArray.size());
			long				defaultValAsLong = inAttr->defaultVal().getLongVal();
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
				ISFVal		minVal = inAttr->minVal();
				ISFVal		maxVal = inAttr->maxVal();
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
			myLayout = new QHBoxLayout(this);
			setLayout(myLayout);
			myLayout = layout();
			
			myLayout->addWidget(longCBWidget);
		}
		*/
		break;
	case ISFValType_Float:
		{
			floatSliderWidget = new QDoubleSlider(this);
			
			floatSliderWidget->setOrientation(Qt::Horizontal);
			
			ISFVal			&tmpVal = inAttr->minVal();
			if (tmpVal.isFloatVal())
				floatSliderWidget->setDoubleMinValue( tmpVal.getDoubleVal() );
			
			tmpVal = inAttr->maxVal();
			if (tmpVal.isFloatVal())
				floatSliderWidget->setDoubleMaxValue( tmpVal.getDoubleVal() );
			
			tmpVal = inAttr->defaultVal();
			if (tmpVal.isFloatVal())
				floatSliderWidget->setDoubleValue( tmpVal.getDoubleVal() );
			
			//	add the widget to the layout
			myLayout = new QHBoxLayout(this);
			setLayout(myLayout);
			myLayout = layout();
			
			myLayout->addWidget(floatSliderWidget);
		}
		break;
	case ISFValType_Point2D:
		{
			xFieldWidget = new QDoubleSpinBox(this);
			yFieldWidget = new QDoubleSpinBox(this);
			
			xSliderWidget = new QDoubleSlider(this);
			xSliderWidget->setOrientation(Qt::Horizontal);
			ySliderWidget = new QDoubleSlider(this);
			ySliderWidget->setOrientation(Qt::Horizontal);
		
			ISFVal		tmpVal = inAttr->currentVal();
			ISFVal		tmpMin = inAttr->minVal();
			ISFVal		tmpMax = inAttr->maxVal();
			
			if (tmpMin.isPoint2DVal() && tmpMax.isPoint2DVal())	{
				xFieldWidget->setRange(tmpMin.getPointValByIndex(0), tmpMax.getPointValByIndex(0));
				yFieldWidget->setRange(tmpMin.getPointValByIndex(1), tmpMax.getPointValByIndex(1));
				xSliderWidget->setDoubleRange(tmpMin.getPointValByIndex(0), tmpMax.getPointValByIndex(0));
				ySliderWidget->setDoubleRange(tmpMin.getPointValByIndex(1), tmpMax.getPointValByIndex(1));
			}
			else	{
				//xFieldWidget->setRange(0.0, 1.0);
				//yFieldWidget->setRange(0.0, 1.0);
				xFieldWidget->setRange(-16384, 16384);
				yFieldWidget->setRange(-16384, 16384);
				xSliderWidget->setDoubleRange(-16384.0, 16384.0);
				ySliderWidget->setDoubleRange(16384.0, 16384.0);
			}
			if (tmpVal.isPoint2DVal())	{
				xFieldWidget->setValue(tmpVal.getPointValByIndex(0));
				yFieldWidget->setValue(tmpVal.getPointValByIndex(1));
				xSliderWidget->setDoubleValue(tmpVal.getPointValByIndex(0));
				ySliderWidget->setDoubleValue(tmpVal.getPointValByIndex(1));
			}
			
			
			//	make a vbox layout
			myLayout = new QVBoxLayout(this);
			setLayout(myLayout);
			myLayout = layout();
			
			//	make an hbox layout- we want the text fields next to one another in this sub layout...
			QLayout		*subLayout = new QHBoxLayout(this);
			qobject_cast<QBoxLayout*>(myLayout)->addLayout(subLayout);
			
			subLayout->addWidget(xFieldWidget);
			subLayout->addWidget(yFieldWidget);
			
			//	...and we want the sliders below the text fields, in the vbox layout
			myLayout->addWidget(xSliderWidget);
			myLayout->addWidget(ySliderWidget);
			
			
			
			/*
			//	add the widget to the layout
			myLayout = new QHBoxLayout(this);
			setLayout(myLayout);
			myLayout = layout();
			
			myLayout->addWidget(xFieldWidget);
			myLayout->addWidget(yFieldWidget);
			*/
			connect( xFieldWidget, SIGNAL(editingFinished()), this, SLOT(pointWidgetUsed()) );
			connect( yFieldWidget, SIGNAL(editingFinished()), this, SLOT(pointWidgetUsed()) );
			connect( xSliderWidget, &QDoubleSlider::doubleSliderMoved, this, &ISFUIItem::pointWidgetUsed );
			connect( ySliderWidget, &QDoubleSlider::doubleSliderMoved, this, &ISFUIItem::pointWidgetUsed );
			connect(GetOutputWindow(), &OutputWindow::outputWindowMouseMoved, this, &ISFUIItem::outputWindowMouseUsed);
		}
		break;
	case ISFValType_Color:
		{
			//	set the color ivar from the attribute's current value
			ISFVal		currentVal = inAttr->currentVal();
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
				colorDialog->setOptions( QColorDialog::DontUseNativeDialog | QColorDialog::ShowAlphaChannel );
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
			
			//	add the widget to the layout
			myLayout = new QHBoxLayout(this);
			setLayout(myLayout);
			myLayout = layout();
			
			myLayout->addWidget(colorButton);
			myLayout->addWidget(colorLabel);
		}
		break;
	case ISFValType_Cube:
		break;
	case ISFValType_Image:
		{
			//	make a combo box
			interAppVideoCB = new QComboBox(this);
			
			//	make the video source
			interAppSrc = new InterAppVideoSource(this);
			
			//	configure the video source to update my buffer every time there's a new frame
			connect(interAppSrc, &VideoSource::frameProduced, [&](VVGL::GLBufferRef n)	{
				interAppBuffer = n;
			});
			
			//	configure the video source to repopulate the combo box when its list of static sources gets udpated
			connect(interAppSrc, &VideoSource::staticSourceUpdated, [&](VideoSource * n)	{
				Q_UNUSED(n);
				if (interAppVideoCB == nullptr)
					return;
				
				//	stop the inter-app source
				interAppSrc->stop();
				
				//	store the index and MediaFile that was originally selected
				int			origIndex = interAppVideoCB->currentIndex();
				MediaFile	origMediaFile = (origIndex<0) ? MediaFile() : interAppVideoCB->currentData().value<MediaFile>();
				//	we're going to try to find the original index again after we repopulate the menu...
				int			newIndex = -1;
				
				interAppVideoCB->blockSignals(true);
				
				interAppVideoCB->clear();
				
				//	each menu item has a name (the video source name) and a value (a QVariant<MediaFile> that can be loaded by interAppSrc)
				
				//	make a menu item with a null item that will simply use the input image frame
				interAppVideoCB->addItem(QString("<Use main video input>"), QVariant::fromValue(MediaFile()));
				
				//	now make menu items for the inter-app video sources
				QList<MediaFile>		vidSrcs = interAppSrc->createListOfStaticMediaFiles();
				for (const MediaFile & n : vidSrcs)	{
					interAppVideoCB->addItem(n.name(), QVariant::fromValue(n));
				}
				
				//	if there was something originally selected, select it again
				if (origIndex >= 0)	{
					//	find the index corresponding to the originally-selected  media file
					newIndex = interAppVideoCB->findData(QVariant::fromValue(origMediaFile));
					//	select the new index
					if (newIndex < 0)	{
						//qDebug() << "ERR: couldn't find index for file " << origMediaFile << ", has it disappeared?";
						interAppVideoCB->setCurrentIndex(-1);
					}
					else
						interAppVideoCB->setCurrentIndex(newIndex);
				}
				
				interAppVideoCB->blockSignals(false);
				
				//	if there was something originally selected but we couldn't select it again, select somethign else
				//	we're doing this here because we want the signal to fire...
				if (origIndex >= 0 && newIndex < 0)	{
					if (interAppVideoCB->maxCount() > 0)
						interAppVideoCB->setCurrentIndex(0);
				}
			});
			
			//	configure the combo box to tell the video source to load a new file when its selection changes
			connect(interAppVideoCB, QOverload<int>::of(&QComboBox::currentIndexChanged), [&](int newIndex)	{
				Q_UNUSED(newIndex);
				MediaFile		selectedMediaFile = interAppVideoCB->currentData().value<MediaFile>();
				interAppSrc->loadFile(selectedMediaFile);
				interAppBuffer = nullptr;
			});
			
			//	make the video source emit a signal that its sources have been updated
			emit interAppSrc->staticSourceUpdated(interAppSrc);
			
			//	add the widget to the layout
			myLayout = new QHBoxLayout(this);
			setLayout(myLayout);
			myLayout = layout();
			
			myLayout->addWidget(interAppVideoCB);
			
		}
		break;
	case ISFValType_Audio:
	case ISFValType_AudioFFT:
		audioLabel = new QLabel(this);
		audioLabel->setText("System's default audio input used!");
		
		//	add the widget to the layout
		myLayout = new QHBoxLayout(this);
		setLayout(myLayout);
		myLayout = layout();
		
		myLayout->addWidget(audioLabel);
		break;
	}
	
}

ISFUIItem::~ISFUIItem()	{
	//qDebug() << __PRETTY_FUNCTION__;
	
	QLayout		*myLayout = layout();
	if (myLayout != nullptr)	{
		delete myLayout;
	}
	
	if (interAppSrc != nullptr)	{
		interAppSrc->stop();
		delete interAppSrc;
		interAppSrc = nullptr;
	}
}




void ISFUIItem::outputWindowMouseUsed(VVGL::Point normMouseEventLoc, VVGL::Point absMouseEventLoc)	{
	//qDebug() << __PRETTY_FUNCTION__;
	
	//	if there's no attr, or there's an attr but the attr has either a null min val or a null max val...
	if (attr==nullptr || (attr!=nullptr && (attr->minVal().isNullVal() || attr->maxVal().isNullVal())))	{
		//	...since there's no min AND max we can't use the normalized event loc, we have to use the absolute event loc.  just make sure it's within the acceptable range.
		Point			newPointVal = absMouseEventLoc;
		ISFVal			tmpMin = attr->minVal();
		ISFVal			tmpMax = attr->maxVal();
		if (!tmpMin.isNullVal())	{
			newPointVal.x = std::max(newPointVal.x, tmpMin.getPointValByIndex(0));
			newPointVal.y = std::max(newPointVal.y, tmpMin.getPointValByIndex(1));
		}
		if (!tmpMax.isNullVal())	{
			newPointVal.x = std::min(newPointVal.x, tmpMax.getPointValByIndex(0));
			newPointVal.y = std::max(newPointVal.y, tmpMax.getPointValByIndex(1));
		}
		pointVal = newPointVal;
		
		xFieldWidget->blockSignals(true);
		xFieldWidget->setValue(newPointVal.x);
		xFieldWidget->blockSignals(false);
		
		yFieldWidget->blockSignals(true);
		yFieldWidget->setValue(newPointVal.y);
		yFieldWidget->blockSignals(false);
	}
	//	else there's an attr and the attr has both a min and a max val- use the normalized mouse location to calculate the value
	else	{
		ISFVal			tmpMin = attr->minVal();
		ISFVal			tmpMax = attr->maxVal();
		Point			newPointVal;
		newPointVal.x = (normMouseEventLoc.x * (tmpMax.getPointValByIndex(0) - tmpMin.getPointValByIndex(0))) + tmpMin.getPointValByIndex(0);
		newPointVal.y = (normMouseEventLoc.y * (tmpMax.getPointValByIndex(1) - tmpMin.getPointValByIndex(1))) + tmpMin.getPointValByIndex(1);
		pointVal = newPointVal;
		
		xFieldWidget->blockSignals(true);
		xFieldWidget->setValue(newPointVal.x);
		xFieldWidget->blockSignals(false);
		
		yFieldWidget->blockSignals(true);
		yFieldWidget->setValue(newPointVal.y);
		yFieldWidget->blockSignals(false);
	}
}
void ISFUIItem::longWidgetUsed()	{
	//qDebug() << __PRETTY_FUNCTION__;
	QObject		*rawSender = sender();
	if (rawSender == nullptr)
		return;
	QSpinBox		*tmpSpinBoxSender = qobject_cast<QSpinBox*>(rawSender);
	QSlider			*tmpSliderSender = qobject_cast<QSlider*>(rawSender);
	
	if (tmpSpinBoxSender == longFieldWidget)	{
		int			tmpVal = longFieldWidget->value();
		longSliderWidget->blockSignals(true);
		longSliderWidget->setValue(tmpVal);
		longSliderWidget->blockSignals(false);
	}
	else if (tmpSliderSender == longSliderWidget)	{
		int			tmpVal = longSliderWidget->value();
		longFieldWidget->blockSignals(true);
		longFieldWidget->setValue(tmpVal);
		longFieldWidget->blockSignals(false);
	}
}
void ISFUIItem::pointWidgetUsed()	{
	//qDebug() << __PRETTY_FUNCTION__;
	QObject		*rawSender = sender();
	if (rawSender == nullptr)
		return;
	QDoubleSpinBox		*tmpSpinBoxSender = qobject_cast<QDoubleSpinBox*>(rawSender);
	QDoubleSlider		*tmpSliderSender = qobject_cast<QDoubleSlider*>(rawSender);
	
	if (tmpSpinBoxSender == xFieldWidget)	{
		//pointVal.x = newVal;
		pointVal.x = tmpSpinBoxSender->value();
		xSliderWidget->blockSignals(true);
		xSliderWidget->setDoubleValue(pointVal.x);
		xSliderWidget->blockSignals(false);
	}
	else if (tmpSpinBoxSender == yFieldWidget)	{
		//pointVal.y = newVal;
		pointVal.y = tmpSpinBoxSender->value();
		ySliderWidget->blockSignals(true);
		ySliderWidget->setDoubleValue(pointVal.y);
		ySliderWidget->blockSignals(false);
	}
	else if (tmpSliderSender == xSliderWidget)	{
		pointVal.x = tmpSliderSender->doubleValue();
		xFieldWidget->blockSignals(true);
		xFieldWidget->setValue(pointVal.x);
		xFieldWidget->blockSignals(false);
	}
	else if (tmpSliderSender == ySliderWidget)	{
		pointVal.y = tmpSliderSender->doubleValue();
		yFieldWidget->blockSignals(true);
		yFieldWidget->setValue(pointVal.y);
		yFieldWidget->blockSignals(false);
	}
	
}
void ISFUIItem::interAppVideoCBUsed(int newIndex)	{
	Q_UNUSED(newIndex);
}
void ISFUIItem::audioCBUsed(int newIndex)	{
	Q_UNUSED(newIndex);
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
		{
			if (longCBWidget != nullptr)	{
				return ISFLongVal( longCBWidget->currentData().toLongLong() );
			}
			else if (longSliderWidget != nullptr)	{
				return ISFLongVal( longSliderWidget->value() );
			}
			else
				return ISFLongVal(0);
		}
	case ISFValType_Float:
		if (floatSliderWidget == nullptr)
			return ISFFloatVal(0.0);
		return ISFFloatVal( floatSliderWidget->doubleValue() );
	case ISFValType_Point2D:
		if (xFieldWidget==nullptr || yFieldWidget==nullptr)
			return ISFPoint2DVal(0.0, 0.0);
		return ISFPoint2DVal( xFieldWidget->value(), yFieldWidget->value() );
	case ISFValType_Color:
		return ISFColorVal(color.redF(), color.greenF(), color.blueF(), color.alphaF());
	case ISFValType_Image:
		{
#if defined(Q_OS_WIN)
			interAppSrc->renderABuffer();
#else
#endif
			return ISFVal(ISFValType_Image, interAppBuffer);
		}
		break;
	case ISFValType_Cube:
		break;
	case ISFValType_Audio:
		{
			AudioController		*ac = GetAudioController();
			if (ac != nullptr)	{
				int					maxVal = 0;
				if (attr != nullptr)	{
					ISFVal				tmpVal = attr->maxVal();
					if (!tmpVal.isNullVal())
						maxVal = tmpVal.getLongVal();
				}
				
				return ISFImageVal(ac->getAudioImageBuffer(maxVal));
			}
		}
		break;
	case ISFValType_AudioFFT:
		{
			AudioController		*ac = GetAudioController();
			if (ac != nullptr)	{
				int					maxVal = 0;
				if (attr != nullptr)	{
					ISFVal				tmpVal = attr->maxVal();
					if (!tmpVal.isNullVal())
						maxVal = tmpVal.getLongVal();
				}
				
				return ISFImageVal(ac->getAudioFFTBuffer(maxVal));
			}
		}
		break;
	}
	
	return ISFNullVal();
}







void ISFUIItem::refreshInterAppVideoCB()	{
	
}
void ISFUIItem::refreshAudioSourceCB()	{
}

