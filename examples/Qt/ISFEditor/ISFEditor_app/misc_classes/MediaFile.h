#ifndef MEDIAFILE_H
#define MEDIAFILE_H

#include <QVariant>
#include <QCameraInfo>




class MediaFile
{
public:
	enum Type	{
		Type_None = 0,
		Type_Cam,
		Type_Mov,
		Type_Img,
		Type_App,
		Type_ISF
	};
	
	MediaFile() : _name(""), resourceLocator() {}
	MediaFile(const Type & inType, const QString & inName, const QString & inOtherString);
	MediaFile(const Type & inType, const QString & inPath);
	MediaFile(const QCameraInfo & inCameraInfo);
	
	static QString StringForType(const MediaFile::Type & n);
	
	inline Type type() const { return _type; };
	QString name() const;
	QString path() const;
	QString syphonUUID() const;
	QCameraInfo cameraInfo() const;
	
	bool operator==(const MediaFile & n) const;
	bool operator<(const MediaFile & n) const;
	
	
	operator QString() const	{
		switch (_type)	{
		case Type_None:
			return QString("<MediaFile %1>").arg(MediaFile::StringForType(_type));
		case Type_App:
		case Type_Mov:
		case Type_Img:
		case Type_Cam:
		case Type_ISF:
			return QString("<MediaFile %1 %2>").arg(MediaFile::StringForType(_type)).arg(name());
		}
	}
	
	
private:
	Type		_type = Type_None;
	QString		_name;
	QVariant	resourceLocator;	//	QCameraInfo if it's Type_Cam, string if it's mov or img
};




//	we want to be able to store a MediaFile as a QVariant
Q_DECLARE_METATYPE(MediaFile)
extern int QVariantMediaFileUserType;
//	we want to be able to store a QCameraInfo as a QVariant in the MediaFile!
Q_DECLARE_METATYPE(QCameraInfo)
extern int QVariant_videoSourceMenuItem_userType;




#endif // MEDIAFILE_H