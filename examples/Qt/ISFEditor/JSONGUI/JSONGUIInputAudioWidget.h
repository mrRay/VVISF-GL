#ifndef JSONGUIINPUTAUDIO_H
#define JSONGUIINPUTAUDIO_H

#include <QWidget>

#include "VVISF.hpp"




namespace Ui {
	class JSONGUIInputAudio;
}

using namespace VVISF;




class JSONGUIInputAudioWidget : public QWidget
{
	Q_OBJECT

public:
	explicit JSONGUIInputAudioWidget(const ISFAttrRef & inRef, QWidget *parent = nullptr);
	~JSONGUIInputAudioWidget();

private:
	Ui::JSONGUIInputAudio *ui;
};

#endif // JSONGUIINPUTAUDIO_H
