#ifndef JSONGUITOP_H
#define JSONGUITOP_H

#include <QObject>
#include <QJsonObject>
#include <QSharedPointer>

#include "JGMCArray.h"
#include "JGMCDict.h"
#include "JGMObject.h"




//	JSON GUI Model- Top
//	the topmost object in the JSON GUI Model- contains inputs, passes, and pbuffers
class JGMTop : public QObject
{
	Q_OBJECT
public:
	//	inISFObj is the full-blown ISF dict from the file
	explicit JGMTop(const QJsonObject & inISFObj, QObject *parent = nullptr) :
		QObject(parent),
		_isfDict(inISFObj),
		_inputs(this),
		_passes(this)
		//_buffers(this)
	{
	}
	
	~JGMTop() {}
	
	QJsonObject & isfDict() { return _isfDict; }
	JGMCInputArray & inputsContainer() { return _inputs; }
	JGMCPassArray & passesContainer() { return _passes; }
	//JGMCPBufferDict & buffersContainer() { return _buffers; }
	
	JGMInputRef getInputNamed(const QString & n);
	QVector<JGMPassRef> getPassesRenderingToBufferNamed(const QString & n);
	//JGMPBufferRef getPersistentBufferNamed(const QString & n);
	JGMPassRef getPersistentPassNamed(const QString & n);
	int indexOfInput(const JGMInput & n);
	int indexOfPass(const JGMPass & n);
	QString createNewInputName();
	
	bool deleteInput(const JGMInputRef & n);
	bool deletePass(const JGMPassRef & n);
	
	//	creates the JSON object that can be written to disk
	QJsonObject createJSONExport();

private:
	QJsonObject				_isfDict;
	
	JGMCInputArray		_inputs;
	JGMCPassArray		_passes;
	//JGMCPBufferDict		_buffers;
	
	QJsonArray createJSONForInputs();
	QJsonArray createJSONForPasses();
};

using JGMTopRef = QSharedPointer<JGMTop>;
using JGMTopWRef = QPointer<JGMTop>;




#endif // JSONGUITOP_H
