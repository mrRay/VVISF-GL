#ifndef CODEEDITOR_H
#define CODEEDITOR_H

#include <mutex>

#include <QPlainTextEdit>
#include <QSyntaxHighlighter>
#include <QRegularExpression>
#include <QObject>
#include <QVector>
#include <QCompleter>

class QPaintEvent;
class QResizeEvent;
class QSize;
class QWidget;

class LineNumberArea;
class Highlighter;








class SimpleSourceCodeEdit : public QPlainTextEdit
{
	Q_OBJECT
	
public:
	SimpleSourceCodeEdit(QWidget * inParent = nullptr);
	~SimpleSourceCodeEdit();
	
	//	the passed line numbers will be flagged for display as errors of some sort in the UI, and the signal 'selectedErrorAtLine' will be emitted when the cursor navigates over any of these lines
	void setErrorLineNumbers(const QVector<int> & inVect);
	void setErrorLineNumbers(const QVector<int> * inVect);
	
	//	the passed QJsonDocument is expected to be a JSON object with the following key/value pairs which define syntax coloring and auto-completion content.  when syntax coloring is applied, it is applied in the order in which the following key/value pairs are listed (so if a word matches multiple patterns- like a keyword that has been commented out- the last will take precedence):
	//		VARIABLES : array of strings, each string is a defined variable name
	//		TYPE_AND_CLASS_NAMES : array of strings, each string is the name of a data type or class
	//		FUNCTION_REGEXES : array of strings, each string is a regex pattern that will match arbitrary functions
	//		SDK_FUNCTIONS : array of strings, each string is a defined function name
	//		KEYWORDS : array of strings, each string is a keyword
	//		PRAGMA_REGEXES : array of strings, each string is a regex pattern that will match a recognized pragma
	//		NUMBER_REGEXES : array of strings, each string is a regex pattern that will match numeric values
	//		QUOTATION_REGEXES : array of strings, each string is a regex pattern that will match quotations
	//		MULTI_LINE_COMMENT_START_REGEX : string with a regex pattern that matches the start of a multi-line comment
	//		MULTI_LINE_COMMENT_END_REGEX : string with a regex pattern that matches the close of a multi-line comment
	//		SINGLE_LINE_COMMENT_REGEX : string with a regex pattern that matches a single-line comment
	void loadSyntaxDefinitionDocument(const QJsonDocument & inDocument);

signals:
	void selectedErrorAtLine(int selectedErrorLineIndex);


protected:
	void resizeEvent(QResizeEvent * inEvent) override;
	void keyPressEvent(QKeyEvent *e) override;
	void focusInEvent(QFocusEvent *e) override;
	
private slots:
	void updateLineNumberAreaWidth(int newBlockCount);
	void highlightCurrentLine();
	void updateLineNumberArea(const QRect & rect, int dy);
	void insertCompletion(const QString &completion);
	void closeCompleter();
	
private:
	QWidget			*lineNumberArea = nullptr;	//	do not explicitly free
	Highlighter		*highlighter = nullptr;	//	do not explicitly free
	QCompleter		*completer = nullptr;	//	do not explicitly free
	
	std::recursive_mutex	errLock;
	QVector<int>			*errLineNumbers = nullptr;	//	must be deleted!
	
	std::recursive_mutex	timestampLock;
	ulong					lastKeyEventTimestamp = 0;
	
	void updateCompleterUsingTextUnderCursor();
	void maybeOpenCompleterEvent(const QKeyEvent & inEvent);
	inline void resetKeyEventTimestamp() { std::lock_guard<std::recursive_mutex> tmpLock(timestampLock); lastKeyEventTimestamp=0; };
public:
	//	called by LineNumberArea's paintEvent- not actually for general use!
	void lineNumberAreaPaintEvent(QPaintEvent * event);
	int lineNumberAreaWidth();
};








class LineNumberArea : public QWidget
{
	Q_OBJECT
	
public:
	LineNumberArea(SimpleSourceCodeEdit * inEditor);
	QSize sizeHint() const override;

protected:
	void paintEvent(QPaintEvent * event) override;

private:
	SimpleSourceCodeEdit			*codeEditor;
};








class Highlighter : public QSyntaxHighlighter
{
	Q_OBJECT
	
public:
	Highlighter(QTextDocument * inParent = nullptr);
	void loadSyntaxDefinitionDocument(const QJsonDocument & inDocument);
	
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








#endif // CODEEDITOR_H
