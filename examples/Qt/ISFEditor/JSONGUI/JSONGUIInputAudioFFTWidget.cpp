#include "JSONGUIInputAudioFFTWidget.h"
#include "ui_JSONGUIInputAudioFFT.h"




JSONGUIInputAudioFFTWidget::JSONGUIInputAudioFFTWidget(const ISFAttrRef & inRef, QWidget *parent) :
	QWidget(parent),
	ui(new Ui::JSONGUIInputAudioFFT)
{
	ui->setupUi(this);
}

JSONGUIInputAudioFFTWidget::~JSONGUIInputAudioFFTWidget()
{
	delete ui;
}
