#ifndef ISFAUDIOBUFFERLIST_H
#define ISFAUDIOBUFFERLIST_H

#include <QObject>
#include <QByteArray>
#include <QAudioDeviceInfo>
#include <QAudioFormat>
#include <QString>
#include <QLinkedList>





class ISFAudioBufferList : public QObject
{
	Q_OBJECT
	
public:
	int			numberOfFrames;
	int			numberOfChannels;

public:
	explicit ISFAudioBufferList(QObject * inParent = nullptr) : QObject(inParent) { _bufferData=QByteArray(); _format=QAudioDeviceInfo::defaultInputDevice().preferredFormat(); generalInit(); }
	explicit ISFAudioBufferList(QByteArray & inBufferData, const QAudioFormat & inFmt, QObject *inParent = nullptr) : QObject(inParent), _bufferData(inBufferData), _format(inFmt) { generalInit(); };
	explicit ISFAudioBufferList(const ISFAudioBufferList & n, QObject * inParent=nullptr) : QObject(inParent), _bufferData(n._bufferData), _format(n._format) { generalInit(); }
	explicit ISFAudioBufferList(const QLinkedList<ISFAudioBufferList> & n, QObject * inParent=nullptr) : QObject(inParent) {
		_bufferData = QByteArray();
		if (n.size() > 0)
			_format = n.first()._format;
		else
			_format = QAudioDeviceInfo::defaultInputDevice().preferredFormat();
		for (const ISFAudioBufferList & tmpBuffer : n)	{
			_bufferData.append(tmpBuffer._bufferData);
		}
		generalInit();
	}
	
	ISFAudioBufferList & operator=(ISFAudioBufferList & rhs) { _bufferData.clear(); _bufferData.append(rhs._bufferData); _format=rhs._format; numberOfFrames=rhs.numberOfFrames; numberOfChannels=rhs.numberOfChannels; return *this; }
	
	operator QString() const { return QString("<ISFABL %1 samples>").arg(numberOfFrames); }
	float * floatPtr() { return reinterpret_cast<float*>( _bufferData.data() ); }

private:
	QByteArray		_bufferData;
	QAudioFormat	_format;
	
	void generalInit()	{
		int			bytesPerSample = _format.sampleSize() / 8;
		numberOfFrames = _bufferData.size() / bytesPerSample;
		numberOfChannels = 1;
	}
};




#endif // ISFAUDIOBUFFERLIST_H