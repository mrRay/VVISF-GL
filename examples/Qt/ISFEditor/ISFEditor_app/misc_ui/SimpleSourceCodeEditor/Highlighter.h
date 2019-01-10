#ifndef HIGHLIGHTER_H
#define HIGHLIGHTER_H

#include <QSyntaxHighlighter>
#include <QRegularExpression>
#include <QSettings>




namespace SimpleSourceCodeEdit	{




class Highlighter : public QSyntaxHighlighter
{
	Q_OBJECT
	
public:
	Highlighter(QTextDocument * inParent = nullptr);
	void loadSyntaxDefinitionDocument(const QJsonDocument & inDocument);
	void loadColorsFromSettings();
	
protected:
	void highlightBlock(const QString & inText) override;
	
private:
	struct HighlightRule	{
		QRegularExpression		pattern;
		QTextCharFormat			format;
	};
	
	enum HighlighterBlockState	{
		HBS_Undefined = -1,
		HBS_OK = 0,
		HBS_OpenComment = 1
	};
	
	QVector<HighlightRule>		highlightRules;
	QRegularExpression			commentSingleExpr;
	QRegularExpression			commentStartExpr;
	QRegularExpression			commentEndExpr;
	
	//QTextCharFormat		plainTextFmt;
	
	QTextCharFormat		variablesFmt;
	QTextCharFormat		typeAndClassNamesFmt;
	QTextCharFormat		functionsFmt;
	QTextCharFormat		sdkFunctionsFmt;
	QTextCharFormat		keywordsFmt;
	QTextCharFormat		pragmasFmt;
	QTextCharFormat		numbersFmt;
	QTextCharFormat		quotationsFmt;
	QTextCharFormat		commentFmt;
};




}	//	namespace SimpleSourceCodeEdit




#endif // HIGHLIGHTER_H