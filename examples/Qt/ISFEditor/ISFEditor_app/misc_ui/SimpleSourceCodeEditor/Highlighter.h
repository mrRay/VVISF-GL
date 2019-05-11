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
	void setLocalVariableNames(const QStringList & inStrList);
	void setSelectedText(const QString & inStr);
	
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
	
	QVector<HighlightRule>		syntaxDocHighlightRules;
	QVector<HighlightRule>		localVarHighlightRules;
	QVector<HighlightRule>		selTextHighlightRules;
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
	QTextCharFormat		bgSelTextFmt;
};




}	//	namespace SimpleSourceCodeEdit




#endif // HIGHLIGHTER_H