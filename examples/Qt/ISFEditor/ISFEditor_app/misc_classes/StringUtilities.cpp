#include "StringUtilities.h"

#include <QStringList>




StringRange LexFunctionCall(const QString & stringToLex, StringRange funcNameRange, QStringList & varArray)	{
	int					searchStartIndex = funcNameRange.location + funcNameRange.length;
	int					lexIndex = searchStartIndex;
	int					openGroupingCount = 0;
	StringRange			substringRange;
	substringRange.location = searchStartIndex + 1;
	do	{
		switch (stringToLex.at(lexIndex).toLatin1())	{
		case '(':
		case '{':
			++openGroupingCount;
			break;
		case ')':
		case '}':
			--openGroupingCount;
			if (openGroupingCount == 0)	{
				substringRange.length = lexIndex - substringRange.location;
				QString			groupString = stringToLex.mid(substringRange.location, substringRange.length);
				if (!groupString.isNull())	{
					groupString = groupString.trimmed();
					varArray.append(groupString);
				}
			}
			break;
		case ',':
			if (openGroupingCount == 1)	{
				substringRange.length = lexIndex - substringRange.location;
				QString			groupString = stringToLex.mid(substringRange.location, substringRange.length);
				if (!groupString.isNull())	{
					groupString = groupString.trimmed();
					varArray.append(groupString);
				}
				substringRange.location = lexIndex + 1;
			}
			break;
		}
		++lexIndex;
		if (lexIndex >= stringToLex.length())
			return StringRange();
	} while (openGroupingCount > 0);
	StringRange			rangeToReplace(funcNameRange.location, lexIndex-funcNameRange.location);
	return rangeToReplace;
}

