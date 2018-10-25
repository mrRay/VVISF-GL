#ifndef JSONGUIINPUTAUDIO_H
#define JSONGUIINPUTAUDIO_H

#include "JSONGUIInputWidget.h"




namespace Ui {
	class JSONGUIInputAudio;
}




class JSONGUIInputAudioWidget : public JSONGUIInputWidget
{
	Q_OBJECT

public:
	explicit JSONGUIInputAudioWidget(const JGMInputRef & inRef, QWidget *parent = nullptr);
	~JSONGUIInputAudioWidget();
	
	virtual void prepareUIItems() override;
	virtual void refreshUIItems() override;

private:
	Ui::JSONGUIInputAudio *ui;
};

#endif // JSONGUIINPUTAUDIO_H
