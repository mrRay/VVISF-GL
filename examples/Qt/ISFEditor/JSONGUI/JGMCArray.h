#ifndef JSONGUIARRAYGROUP_H
#define JSONGUIARRAYGROUP_H

#include <QObject>
#include <QVector>

//class JSONGUIHolderObject;
//class JSONGUIHolderObjectRef;

#include "JGMObject.h"

class JGMTop;




enum JGMCArrayType	{
	JGMCArrayType_Input,
	JGMCArrayType_Pass
};




//	JSON GUI Model Container- Array
//	base class that holds "some kind of array" (either an array of inputs or an array of passes)
class JGMCArray : public QObject
{
	Q_OBJECT
public:
	explicit JGMCArray(const JGMCArrayType & inType, JGMTop * inTop, QObject *parent = nullptr);
	
	JGMCArrayType groupType() { return _groupType; }

protected:
	JGMCArrayType	_groupType = JGMCArrayType_Input;	//	what "type" of group this is (inputs/passes/etc)
	JGMTop			*top = nullptr;
};




//	JSON GUI Model Container- Input Array
//	a class that holds an array of inputs
class JGMCInputArray : public JGMCArray
{
	Q_OBJECT
public:
	explicit JGMCInputArray(JGMTop * inTop, QObject *parent=nullptr);
	QVector<JGMInputRef> & contents() { return _contents; }
private:
	QVector<JGMInputRef>		_contents;
};




//	JSON GUI Model Container- PassArray
class JGMCPassArray : public JGMCArray
{
	Q_OBJECT
public:
	explicit JGMCPassArray(JGMTop * inTop, QObject *parent=nullptr);
	QVector<JGMPassRef> & contents() { return _contents; }
private:
	QVector<JGMPassRef>		_contents;
};




#endif // JSONGUIARRAYGROUP_H
