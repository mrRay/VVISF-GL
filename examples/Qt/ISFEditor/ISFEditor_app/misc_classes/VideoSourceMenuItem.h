#ifndef VIDEOSOURCEMENUITEM_H
#define VIDEOSOURCEMENUITEM_H

#include <QString>
#include <QVariant>
#include <QCameraInfo>









class VideoSourceMenuItem
{
public:
	VideoSourceMenuItem(const QString & inStr, const QVariant & inVar) : string(inStr), variant(inVar) {
		
	}
	QString		string;
	QVariant	variant;
};




#endif // VIDEOSOURCEMENUITEM_H
