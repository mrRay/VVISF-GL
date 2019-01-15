#ifndef FINDOPTS_H
#define FINDOPTS_H

#include <QString>
#include <QRegularExpression>
#include <QSettings>




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
	
	void saveToSettings()	{
		QSettings		settings;
		settings.setValue("findOpts_searchString", QVariant(searchString));
		settings.setValue("findOpts_caseSensitive", QVariant(caseSensitive));
		settings.setValue("findOpts_entireWord", QVariant(entireWord));
		settings.setValue("findOpts_regex", QVariant(regex));
	}
	void loadFromSettings()	{
		QSettings		settings;
		if (settings.contains("findOpts_searchString"))
			searchString = settings.value("findOpts_searchString").toString();
		if (settings.contains("findOpts_caseSensitive"))
			caseSensitive = settings.value("findOpts_caseSensitive").toBool();
		if (settings.contains("findOpts_entireWord"))
			entireWord = settings.value("findOpts_entireWord").toBool();
		if (settings.contains("findOpts_regex"))
			regex = settings.value("findOpts_regex").toBool();
	}
};




}	//	namespace SimpleSourceCodeEdit




#endif // FINDOPTS_H
