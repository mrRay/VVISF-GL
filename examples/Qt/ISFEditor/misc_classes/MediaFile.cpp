#include "MediaFile.h"

#include <QFileInfo>




int QVariantMediaFileUserType = -1;




MediaFile::MediaFile(const Type & inType, const QString & inPath) : _type(inType), resourceLocator(inPath)	{
	if (QVariantMediaFileUserType < 0)	{
		QVariantMediaFileUserType = qRegisterMetaType<MediaFile>();
	}
	if (QVariant_videoSourceMenuItem_userType < 0)	{
		QVariant_videoSourceMenuItem_userType = qRegisterMetaType<QCameraInfo>();
	}
}
MediaFile::MediaFile(const QCameraInfo & inCameraInfo) : _type(Type_Cam), resourceLocator(QVariant::fromValue(inCameraInfo)) {
	if (QVariantMediaFileUserType < 0)	{
		QVariantMediaFileUserType = qRegisterMetaType<MediaFile>();
	}
	if (QVariant_videoSourceMenuItem_userType < 0)	{
		QVariant_videoSourceMenuItem_userType = qRegisterMetaType<QCameraInfo>();
	}
}




QString MediaFile::name() const	{
	switch (_type)	{
	case Type_None:
		return QString("None");
	case Type_Cam:
		return resourceLocator.value<QCameraInfo>().description();
	case Type_Mov:
	case Type_Img:
		return QFileInfo(path()).baseName();
	case Type_App:
		return QString("OtherApp");
	}
	return QString("???");
}
QString MediaFile::path() const	{
	if (_type==Type_Mov || _type==Type_Img)
		return resourceLocator.toString();
	return QString();
}
QCameraInfo MediaFile::cameraInfo() const	{
	if (_type==Type_Cam)
		return resourceLocator.value<QCameraInfo>();
	return QCameraInfo();
}




bool MediaFile::operator==(const MediaFile & n)	{
	if (_type!=n.type())
		return false;
	switch (_type)	{
	case Type_None:
		return true;
	case Type_Cam:
		return (cameraInfo() == n.cameraInfo());
	case Type_Mov:
	case Type_Img:
		return (path() == n.path());
	case Type_App:
		return true;
	}
	return false;
}

