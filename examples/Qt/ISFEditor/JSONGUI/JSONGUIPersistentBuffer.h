#ifndef JSONGUIPERSISTENTBUFFER_H
#define JSONGUIPERSISTENTBUFFER_H

#include <QObject>

class JSONGUIPersistentBuffer : public QObject
{
	Q_OBJECT
public:
	explicit JSONGUIPersistentBuffer(QObject *parent = nullptr);

signals:

public slots:
};

#endif // JSONGUIPERSISTENTBUFFER_H