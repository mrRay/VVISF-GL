#ifndef JSONGUIDICTGROUP_H
#define JSONGUIDICTGROUP_H

#include <QObject>
#include <QMap>

#include "JGMObject.h"

class JGMTop;




enum JGMDictType	{
	JGMDictType_PBuffer
};



//	JSON GUI Model Container- Dict
//	an object that uses a hash to contain some kind of data related to the JSON GUI
class JGMCDict : public QObject
{
	Q_OBJECT
	
public:
	explicit JGMCDict(const JGMDictType & inType, JGMTop * inTop, QObject *parent = nullptr);
	
	JGMDictType groupType() { return _groupType; }
	
protected:
	JGMDictType		_groupType = JGMDictType_PBuffer;
	JGMTop				*_top = nullptr;
};



//	JSON GUI Model Container- PBufferDict
//	stores JSONGUIPBufferRef instances at keys (key is pbuffer name, pbuffer contains the JSON dict describing the pbuffer)
class JGMCPBufferDict : public JGMCDict
{
	Q_OBJECT
	
public:
	explicit JGMCPBufferDict(JGMTop * inTop, QObject *parent=nullptr);
	
	JGMPBufferRef value(const QString & n);
	QHash<QString,JGMPBufferRef> & contents() { return _contents; }
	
private:
	QHash<QString,JGMPBufferRef>		_contents;
};




#endif // JSONGUIDICTGROUP_H
