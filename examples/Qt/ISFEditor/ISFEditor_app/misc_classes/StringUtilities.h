#ifndef STRINGUTILITIES_H
#define STRINGUTILITIES_H

#include <QString>
#include <QRegularExpressionMatch>




struct StringRange	{
	StringRange(int inLocation=0, int inLength=0) { location=inLocation; length=inLength; }
	StringRange(const QRegularExpressionMatch & n) { location=n.capturedStart(); length=n.capturedLength(); }
	int			location = 0;
	int			length = 0;
	StringRange & operator=(const StringRange & rhs) { location=rhs.location; length=rhs.length; return *this; }
	StringRange & operator=(const QRegularExpressionMatch & rhs) { location=rhs.capturedStart(); length=rhs.capturedLength(); return *this; }
};




StringRange LexFunctionCall(const QString & stringToLex, StringRange funcNameRange, QStringList & varArray);




#endif // STRINGUTILITIES_H
