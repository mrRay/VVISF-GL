#include "JSONGUIInputAudioWidget.h"
#include "ui_JSONGUIInputAudio.h"




JSONGUIInputAudioWidget::JSONGUIInputAudioWidget(const ISFAttrRef & inRef, QWidget *parent) :
	QWidget(parent),
	ui(new Ui::JSONGUIInputAudio)
{
	ui->setupUi(this);
}

JSONGUIInputAudioWidget::~JSONGUIInputAudioWidget()
{
	delete ui;
}
