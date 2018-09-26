#ifndef JSONGUIINPUTAUDIOFFT_H
#define JSONGUIINPUTAUDIOFFT_H

#include <QWidget>

#include "VVISF.hpp"




namespace Ui {
	class JSONGUIInputAudioFFT;
}

using namespace VVISF;




class JSONGUIInputAudioFFTWidget : public QWidget
{
	Q_OBJECT

public:
	explicit JSONGUIInputAudioFFTWidget(const ISFAttrRef & inRef, QWidget *parent = nullptr);
	~JSONGUIInputAudioFFTWidget();

private:
	Ui::JSONGUIInputAudioFFT *ui;
};

#endif // JSONGUIINPUTAUDIOFFT_H
