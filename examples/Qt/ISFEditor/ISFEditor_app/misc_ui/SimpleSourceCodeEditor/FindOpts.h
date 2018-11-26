#ifndef FINDOPTS_H
#define FINDOPTS_H

#include <QString>
#include <QRegularExpression>




namespace SimpleSourceCodeEdit	{




struct FindOpts	{
	QString			searchString = QString("");
	bool			caseSensitive = true;
	bool			entireWord = true;
	bool			regex = true;
	
	bool isValid()	{
		if (searchString.isNull() || searchString.length()<1)
			return false;
		if (regex)	{
			QRegularExpression		regex(searchString);
			if (!regex.isValid())
				return false;
		}
		return true;
	}
};




}	//	namespace SimpleSourceCodeEdit




#endif // FINDOPTS_H
