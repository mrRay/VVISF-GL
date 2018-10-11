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
		Type_App
	};
	
	MediaFile() {}
	MediaFile(const Type & inType, const QString & inPath);
	MediaFile(const QCameraInfo & inCameraInfo);
	
	inline Type type() const { return _type; };
	QString name() const;
	QString path() const;
	QCameraInfo cameraInfo() const;
	
	bool operator==(const MediaFile & n);
	
private:
	Type		_type = Type_None;
	QVariant	resourceLocator;	//	QCameraInfo if it's Type_Cam, string if it's mov or img
};




//	we want to be able to store a MediaFile as a QVariant
Q_DECLARE_METATYPE(MediaFile)
extern int QVariantMediaFileUserType;
//	we want to be able to store a QCameraInfo as a QVariant in the MediaFile!
Q_DECLARE_METATYPE(QCameraInfo)
extern int QVariant_videoSourceMenuItem_userType;




#endif // MEDIAFILE_H