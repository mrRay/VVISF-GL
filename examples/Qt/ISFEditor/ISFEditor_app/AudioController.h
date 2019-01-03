#ifndef AUDIOCONTROLLER_H
#define AUDIOCONTROLLER_H

#include <QObject>
#include <QScopedPointer>
#include <QAudioInput>
#include <QAudioDeviceInfo>
#include <QLinkedList>

#include <mutex>

#include "VVGL.hpp"
#include "ISFAudioBufferList.h"

#include "fftreal_wrapper.h"
#include "FFTRealFixLenParam.h"




#define fftQuality 512




class AudioController : public QObject
{
	Q_OBJECT
public:
	explicit AudioController(QObject *parent = nullptr);
	
	QList<QAudioDeviceInfo> getListOfDevices();
	QAudioDeviceInfo * currentDeviceInfo();
	void updateAudioResults();
	VVGL::GLBufferRef getAudioImageBuffer(const int & inWidth = 0);
	VVGL::GLBufferRef getAudioFFTBuffer(const int & inWidth = 0);
	
	void moveToThread(QThread * inTargetThread, VVGL::GLBufferPoolRef inThreadBufferPool=nullptr, VVGL::GLTexToTexCopierRef inThreadTexCopier=nullptr);

signals:
	Q_SIGNAL void listOfAudioDevicesUpdated();

public slots:

private:
	std::recursive_mutex			_lock;
	
	VVGL::GLCPUToTexCopierRef				_audioUploader;
	VVGL::GLCPUToTexCopierRef				_fftUploader;
	
	QScopedPointer<QAudioDeviceInfo>	_device;
	QAudioFormat					_format;
	QScopedPointer<QAudioInput>		_input;
	
	QLinkedList<ISFAudioBufferList>		_bufferList;
	ISFAudioBufferList				_lastABL;	//	the last ABL we processed
	VVGL::GLBufferRef						_audioBuffer = nullptr;
	VVGL::GLBufferRef						_fftBuffer = nullptr;
	
	typedef FFTRealFixLenParam::DataType        DataType;
	FFTRealWrapper								m_fft;
	//QVector<DataType>							m_input;
	QVector<DataType>							m_output;
};




AudioController * GetAudioController();




#endif // AUDIOCONTROLLER_H
