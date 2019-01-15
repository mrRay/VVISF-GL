#ifndef CODEEDITOR_H
#define CODEEDITOR_H

#include <mutex>

#include <QPlainTextEdit>
#include <QCompleter>

#include "FindOpts.h"




namespace SimpleSourceCodeEdit	{


class Highlighter;


class SimpleSourceCodeEditor : public QPlainTextEdit
{
	Q_OBJECT
	
public:
	SimpleSourceCodeEditor(QWidget * inParent = nullptr);
	~SimpleSourceCodeEditor();
	
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
	FindOpts findOpts() { return _findOpts; }
	void setFindOpts(const FindOpts & n) { _findOpts = n; }

signals:
	void selectedErrorAtLine(int selectedErrorLineIndex);

public slots:
	Q_SLOT void openFindDialog();
	Q_SLOT void findNext();
	Q_SLOT void findPrevious();
	Q_SLOT void setFindStringFromCursor();

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
	QColor			currentLineColor;	//	the color drawn behind the current line (the line with the insertion point)
	
	std::recursive_mutex	errLock;
	QVector<int>			*errLineNumbers = nullptr;	//	must be deleted!
	
	std::recursive_mutex	timestampLock;
	ulong					lastKeyEventTimestamp = 0;
	
	FindOpts		_findOpts;	//	last search opts
	
	void updateCompleterUsingTextUnderCursor();
	void maybeOpenCompleterEvent(const QKeyEvent & inEvent);
	inline void resetKeyEventTimestamp() { std::lock_guard<std::recursive_mutex> tmpLock(timestampLock); lastKeyEventTimestamp=0; };
public:
	//	called by LineNumberArea's paintEvent- not actually for general use!
	void lineNumberAreaPaintEvent(QPaintEvent * event);
	int lineNumberAreaWidth();
};




}	//	namespace SimpleSourceCodeEdit




#endif // CODEEDITOR_H
