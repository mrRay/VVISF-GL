#ifndef JSONGUIGROUP_H
#define JSONGUIGROUP_H

#include <QObject>
#include <QJsonObject>
#include <QSharedPointer>

class JGMTop;




//	JSON GUI Model Object
//	represents a single item/object in the JSON GUI Model.  inputs, passes, and pbuffers are objects.
class JGMObject : public QObject	{
	Q_OBJECT
public:
	explicit JGMObject(const QJsonObject & inJsonObj, JGMTop * inTop, QObject *parent=nullptr);
	
	bool contains(const QString & n) { return _dict.contains(n); }
	QJsonValue value(const QString & n) { if (!_dict.contains(n)) return QJsonValue(); return _dict[n]; }
	void setValue(const QString & k, const QJsonValue & v) { if (v.isUndefined()) _dict.remove(k); else _dict[k]=v; }
	JGMTop * top() { return _top; }
	virtual QJsonObject createExportObject() { return _dict; }

protected:
	QJsonObject			_dict;	//	the JSON dict from the ISF file corresponding to the given object (input/pass/pbuffer)
	JGMTop			*_top = nullptr;
};




//	JSON GUI Model- Input
//	represents a single input from the JSON portion of an ISF file
class JGMInput : public JGMObject	{
	Q_OBJECT
public:
	explicit JGMInput(const QJsonObject & inJsonObj, JGMTop * inTop, QObject *parent=nullptr);
	//virtual QJsonObject createExportObject() override;
};

using JGMInputRef = QSharedPointer<JGMInput>;




//	JSON GUI Model- Pass
//	represents a single render pass from the JSON portion of an ISF file
class JGMPass : public JGMObject	{
	Q_OBJECT
public:
	explicit JGMPass(const QJsonObject & inJsonObj, JGMTop * inTop, QObject *parent=nullptr);
	//virtual QJsonObject createExportObject() override;
};

using JGMPassRef = QSharedPointer<JGMPass>;




#endif // JSONGUIGROUP_H
