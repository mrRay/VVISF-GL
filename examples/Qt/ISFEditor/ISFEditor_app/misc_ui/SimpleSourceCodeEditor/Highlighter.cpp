#include "Highlighter.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>

#if defined(Q_OS_WIN)
#define __PRETTY_FUNCTION__ __FUNCSIG__
#endif




namespace SimpleSourceCodeEdit	{




Highlighter::Highlighter(QTextDocument * inParent)
	: QSyntaxHighlighter(inParent)
{
	//	configure some default comment start and end expressions
	commentSingleExpr = QRegularExpression("//[^\n]*");
	commentStartExpr = QRegularExpression("/\\*");
	commentEndExpr = QRegularExpression("\\*/");
	
	//xxxFmt.setFontWeight(QFont::Bold);
	
	variablesFmt.setForeground(Qt::darkMagenta);
	typeAndClassNamesFmt.setForeground(Qt::darkGreen);
	functionsFmt.setForeground(Qt::darkBlue);
	sdkFunctionsFmt.setForeground(Qt::blue);
	keywordsFmt.setForeground(Qt::darkYellow);
	pragmasFmt.setForeground(Qt::green);
	numbersFmt.setForeground(Qt::magenta);
	quotationsFmt.setForeground(Qt::darkRed);
	commentFmt.setForeground(QColor(255,194,0,255));
}
void Highlighter::loadSyntaxDefinitionDocument(const QJsonDocument & inDocument)	{
	//qDebug() << __PRETTY_FUNCTION__;
	if (!inDocument.isObject()) {
		qDebug() << "\tERR: document is not a JSON object, bailing, " << __PRETTY_FUNCTION__;
		return;
	}
	
	highlightRules.clear();
	
	QJsonObject		tmpDocObj = inDocument.object();
	HighlightRule	tmpRule;
	
	if (tmpDocObj.contains("VARIABLES") && tmpDocObj["VARIABLES"].isArray())	{
		tmpRule.format = variablesFmt;
		QJsonArray		tmpArray = tmpDocObj["VARIABLES"].toArray();
		for (const QJsonValue & tmpVal : tmpArray)	{
			if (tmpVal.isString())	{
				tmpRule.pattern = QRegularExpression(QString("\\b%1\\b").arg(tmpVal.toString()));
				highlightRules.append(tmpRule);
			}
		}
	}
	
	if (tmpDocObj.contains("TYPE_AND_CLASS_NAMES") && tmpDocObj["TYPE_AND_CLASS_NAMES"].isArray())	{
		tmpRule.format = typeAndClassNamesFmt;
		QJsonArray		tmpArray = tmpDocObj["TYPE_AND_CLASS_NAMES"].toArray();
		for (const QJsonValue & tmpVal : tmpArray)	{
			if (tmpVal.isString())	{
				tmpRule.pattern = QRegularExpression(QString("\\b%1\\b").arg(tmpVal.toString()));
				highlightRules.append(tmpRule);
			}
		}
	}
	
	if (tmpDocObj.contains("FUNCTION_REGEXES") && tmpDocObj["FUNCTION_REGEXES"].isArray())	{
		tmpRule.format = functionsFmt;
		QJsonArray		tmpArray = tmpDocObj["FUNCTION_REGEXES"].toArray();
		for (const QJsonValue & tmpVal : tmpArray)	{
			if (tmpVal.isString())	{
				tmpRule.pattern = QRegularExpression(tmpVal.toString());
				highlightRules.append(tmpRule);
			}
		}
	}
	
	if (tmpDocObj.contains("SDK_FUNCTIONS") && tmpDocObj["SDK_FUNCTIONS"].isArray())	{
		tmpRule.format = sdkFunctionsFmt;
		QJsonArray		tmpArray = tmpDocObj["SDK_FUNCTIONS"].toArray();
		for (const QJsonValue & tmpVal : tmpArray)	{
			if (tmpVal.isString())	{
				tmpRule.pattern = QRegularExpression(QString("\\b%1\\b").arg(tmpVal.toString()));
				highlightRules.append(tmpRule);
			}
		}
	}
	
	if (tmpDocObj.contains("KEYWORDS") && tmpDocObj["KEYWORDS"].isArray())	{
		tmpRule.format = keywordsFmt;
		QJsonArray		tmpArray = tmpDocObj["KEYWORDS"].toArray();
		for (const QJsonValue & tmpVal : tmpArray)	{
			if (tmpVal.isString())	{
				tmpRule.pattern = QRegularExpression(QString("\\b%1\\b").arg(tmpVal.toString()));
				highlightRules.append(tmpRule);
			}
		}
	}
	
	if (tmpDocObj.contains("PRAGMA_REGEXES") && tmpDocObj["PRAGMA_REGEXES"].isArray())	{
		tmpRule.format = pragmasFmt;
		QJsonArray		tmpArray = tmpDocObj["PRAGMA_REGEXES"].toArray();
		for (const QJsonValue & tmpVal : tmpArray)	{
			if (tmpVal.isString())	{
				tmpRule.pattern = QRegularExpression(tmpVal.toString());
				highlightRules.append(tmpRule);
			}
		}
	}
	
	if (tmpDocObj.contains("NUMBER_REGEXES") && tmpDocObj["NUMBER_REGEXES"].isArray())	{
		tmpRule.format = numbersFmt;
		QJsonArray		tmpArray = tmpDocObj["NUMBER_REGEXES"].toArray();
		for (const QJsonValue & tmpVal : tmpArray)	{
			if (tmpVal.isString())	{
				tmpRule.pattern = QRegularExpression(tmpVal.toString());
				highlightRules.append(tmpRule);
			}
		}
	}
	if (tmpDocObj.contains("QUOTATION_REGEXES") && tmpDocObj["QUOTATION_REGEXES"].isArray())	{
		tmpRule.format = quotationsFmt;
		QJsonArray		tmpArray = tmpDocObj["QUOTATION_REGEXES"].toArray();
		for (const QJsonValue & tmpVal : tmpArray)	{
			if (tmpVal.isString())	{
				tmpRule.pattern = QRegularExpression(tmpVal.toString());
				highlightRules.append(tmpRule);
			}
		}
	}
	
	if (tmpDocObj.contains("SINGLE_LINE_COMMENT_REGEX") && tmpDocObj["SINGLE_LINE_COMMENT_REGEX"].isString())	{
		commentSingleExpr = QRegularExpression(tmpDocObj["SINGLE_LINE_COMMENT_REGEX"].toString());
	}
	if (tmpDocObj.contains("MULTI_LINE_COMMENT_START_REGEX") && tmpDocObj["MULTI_LINE_COMMENT_START_REGEX"].isString()) {
		//	no format to set here, we're not making a rule- just an expression!
		commentStartExpr = QRegularExpression(tmpDocObj["MULTI_LINE_COMMENT_START_REGEX"].toString());
	}
	if (tmpDocObj.contains("MULTI_LINE_COMMENT_END_REGEX") && tmpDocObj["MULTI_LINE_COMMENT_END_REGEX"].isString()) {
		//	no format to set here, we're not making a rule- just an expression!
		commentEndExpr = QRegularExpression(tmpDocObj["MULTI_LINE_COMMENT_END_REGEX"].toString());
	}
	
	if (tmpDocObj.contains("SINGLE_LINE_COMMENT_REGEX") && tmpDocObj["SINGLE_LINE_COMMENT_REGEX"].isString())	{
		tmpRule.format = commentFmt;
		tmpRule.pattern = QRegularExpression(tmpDocObj["SINGLE_LINE_COMMENT_REGEX"].toString());
		highlightRules.append(tmpRule);
	}
}


void Highlighter::highlightBlock(const QString & inText)
{
	//qDebug() << __PRETTY_FUNCTION__ << ": " << inText;
	//	run through all of the highlight rules, checked the passed string against each
	foreach (const HighlightRule & tmpRule, highlightRules) {
		QRegularExpressionMatchIterator		matchIterator = tmpRule.pattern.globalMatch(inText);
		while (matchIterator.hasNext()) {
			QRegularExpressionMatch		tmpMatch = matchIterator.next();
			setFormat(tmpMatch.capturedStart(), tmpMatch.capturedLength(), tmpRule.format);
		}
	}
	//	set the current block state
	setCurrentBlockState(HBS_OK);
	
	//	check this line- figure out if there's a single-line comment, and if so, the index where the single-line comment starts
	int		singleLineCommentStartIndex = -1;
	if (inText.contains(commentSingleExpr))	{
		singleLineCommentStartIndex = commentSingleExpr.match(inText).capturedStart();
	}
	
	//	if there isn't an open multi-line comment, look for the beginning of one in the passed text
	int		startIndex = 0;
	if (previousBlockState() != HBS_OpenComment)	{
		startIndex = inText.indexOf(commentStartExpr);
	}
	//	if the multi-line comment beginning occurred after the single-line comment start, we need to ignore it!
	if (startIndex>=0 && singleLineCommentStartIndex>=0 && startIndex>singleLineCommentStartIndex)
		startIndex = -1;
	
	//	if we found the beginning of a multi-line comment...
	while (startIndex >= 0) {
		QRegularExpressionMatch		tmpMatch = commentEndExpr.match(inText, startIndex);
		int			endIndex = tmpMatch.capturedStart();
		int			commentLength = 0;
		if (endIndex == -1) {
			setCurrentBlockState(HBS_OpenComment);
			commentLength = inText.length() - startIndex;
		}
		else	{
			commentLength = endIndex - startIndex + tmpMatch.capturedLength();
		}
		setFormat(startIndex, commentLength, commentFmt);
		startIndex = inText.indexOf(commentStartExpr, startIndex + commentLength);
	}
	
}




}	//	namespace SimpleSourceCodeEdit
