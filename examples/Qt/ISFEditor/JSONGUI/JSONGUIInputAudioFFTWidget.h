#ifndef JSONGUIINPUTAUDIOFFT_H
#define JSONGUIINPUTAUDIOFFT_H

#include <QWidget>

#include "JSONGUIInput.h"




namespace Ui {
	class JSONGUIInputAudioFFT;
}




class JSONGUIInputAudioFFTWidget : public QWidget, public JSONGUIInput
{
	Q_OBJECT

public:
	explicit JSONGUIInputAudioFFTWidget(const JGMInputRef & inRef, QWidget *parent = nullptr);
	~JSONGUIInputAudioFFTWidget();
	
	virtual void prepareUIItems() override;
	virtual void refreshUIItems() override;

private:
	Ui::JSONGUIInputAudioFFT *ui;
};

#endif // JSONGUIINPUTAUDIOFFT_H
