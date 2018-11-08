#include "SimpleSourceCodeEdit.h"

#include <QPainter>
#include <QTextBlock>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QAbstractItemView>
#include <QScrollBar>
#include <QStringListModel>
#include <QApplication>
#include <QTimer>
#include <QPointer>

#include <regex>
#include <string>
#include <iostream>






/*	========================================	*/
#pragma mark --------------------- SimpleSourceCodeEdit






SimpleSourceCodeEdit::SimpleSourceCodeEdit(QWidget * inParent) :
	QPlainTextEdit(inParent)
{
	lineNumberArea = new LineNumberArea(this);
	highlighter = new Highlighter(this->document());
	errLineNumbers = new QVector<int>();
	
	connect(this, SIGNAL(blockCountChanged(int)), this, SLOT(updateLineNumberAreaWidth(int)));
	connect(this, SIGNAL(updateRequest(QRect,int)), this, SLOT(updateLineNumberArea(QRect,int)));
	connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(highlightCurrentLine()));
	
	setLineWrapMode(QPlainTextEdit::NoWrap);
	QFont		tmpFont;
	tmpFont.setFamily("Courier");
	tmpFont.setFixedPitch(true);
	tmpFont.setPointSize(12);
	setFont(tmpFont);
	
#if (QT_VERSION >= QT_VERSION_CHECK(5, 10, 0))
	int			tmpDist = fontMetrics().horizontalAdvance(QLatin1Char('9')) * 4;
#else
	int			tmpDist = fontMetrics().boundingRect(QLatin1Char('9')).width() * 4;
#endif
	setTabStopDistance(tmpDist);
	
	updateLineNumberAreaWidth(0);
	highlightCurrentLine();
	
	completer = new QCompleter(this);
	completer->setModelSorting(QCompleter::CaseInsensitivelySortedModel);
	completer->setCaseSensitivity(Qt::CaseInsensitive);
	completer->setWrapAround(false);
	
	completer->setWidget(this);
	//completer->setFilterMode(Qt::MatchContains);
	completer->setCompletionMode(QCompleter::PopupCompletion);
	completer->setCaseSensitivity(Qt::CaseInsensitive);
	
	connect(completer, SIGNAL(activated(QString)), this, SLOT(insertCompletion(QString)));
	connect(this, SIGNAL(selectionChanged()), this, SLOT(closeCompleter()));
	
	setAttribute(Qt::WA_DeleteOnClose);
}
SimpleSourceCodeEdit::~SimpleSourceCodeEdit()
{
	//qDebug() << __PRETTY_FUNCTION__;
	
	std::lock_guard<std::recursive_mutex>		lock(errLock);
	if (errLineNumbers != nullptr)	{
		delete errLineNumbers;
		errLineNumbers = nullptr;
	}
}


void SimpleSourceCodeEdit::setErrorLineNumbers(const QVector<int> & inVect)	{
	{
		std::lock_guard<std::recursive_mutex>		lock(errLock);
		if (errLineNumbers != nullptr)
			delete errLineNumbers;
		if (inVect.length() < 1)
			errLineNumbers = nullptr;
		else
			errLineNumbers = new QVector<int>(inVect);
	}
	
	highlightCurrentLine();
}
void SimpleSourceCodeEdit::setErrorLineNumbers(const QVector<int> * inVect)	{
	{
		std::lock_guard<std::recursive_mutex>		lock(errLock);
		if (errLineNumbers != nullptr)
			delete errLineNumbers;
		if (inVect ==nullptr)
			errLineNumbers = nullptr;
		else
			errLineNumbers = new QVector<int>(*inVect);
	}
	
	highlightCurrentLine();
}
void SimpleSourceCodeEdit::loadSyntaxDefinitionDocument(const QJsonDocument & inDocument)
{
	//qDebug() << __PRETTY_FUNCTION__;
	
	//	reset the last key event timestamp- we don't want any one-shot timers triggering the open completer callback to open anything
	resetKeyEventTimestamp();
	
	//	create a QStringList containing all the (non-regex!) words
	QStringList		tmpList;
	if (inDocument.isObject())	{
		QJsonObject		tmpDocObj = inDocument.object();

		if (tmpDocObj.contains("VARIABLES") && tmpDocObj["VARIABLES"].isArray())	{
			QJsonArray		tmpArray = tmpDocObj["VARIABLES"].toArray();
			for (const QJsonValue & tmpVal : tmpArray)	{
				if (tmpVal.isString())	{
					QString		tmpString = tmpVal.toString().trimmed();
					if (!tmpList.contains(tmpString))
						tmpList.append(tmpString);
				}
			}
		}
		
		if (tmpDocObj.contains("TYPE_AND_CLASS_NAMES") && tmpDocObj["TYPE_AND_CLASS_NAMES"].isArray())	{
			QJsonArray		tmpArray = tmpDocObj["TYPE_AND_CLASS_NAMES"].toArray();
			for (const QJsonValue & tmpVal : tmpArray)	{
				if (tmpVal.isString())	{
					QString		tmpString = tmpVal.toString().trimmed();
					if (!tmpList.contains(tmpString))
						tmpList.append(tmpString);
				}
			}
		}
		
		if (tmpDocObj.contains("SDK_FUNCTIONS") && tmpDocObj["SDK_FUNCTIONS"].isArray())	{
			QJsonArray		tmpArray = tmpDocObj["SDK_FUNCTIONS"].toArray();
			for (const QJsonValue & tmpVal : tmpArray)	{
				if (tmpVal.isString())	{
					QString		tmpString = tmpVal.toString().trimmed();
					if (!tmpList.contains(tmpString))
						tmpList.append(tmpString);
				}
			}
		}
		
		tmpList.sort(Qt::CaseInsensitive);
	}
	
	QAbstractItemModel		*targetModel = new QStringListModel(tmpList, completer);
	completer->setModel(targetModel);
	
	//	pass the doc to the highlighter, which needs to construct a bunch of regex stuff from its contents
	if (highlighter != nullptr) {
		highlighter->loadSyntaxDefinitionDocument(inDocument);
	}
}

	
void SimpleSourceCodeEdit::resizeEvent(QResizeEvent * inEvent)
{
	QPlainTextEdit::resizeEvent(inEvent);
	
	QRect		cr = contentsRect();
	lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
}
void SimpleSourceCodeEdit::keyPressEvent(QKeyEvent *inEvent)
{
	//qDebug() << __PRETTY_FUNCTION__ << ", timestamp is " << inEvent->timestamp() << ", text is " << inEvent->text();
	
	using namespace std;
	
	if (completer && completer->popup()->isVisible()) {
		// The following keys are forwarded by the completer to the widget
		switch (inEvent->key()) {
		case Qt::Key_Enter:
		case Qt::Key_Return:
		case Qt::Key_Escape:
		case Qt::Key_Tab:
		case Qt::Key_Backtab:
			//qDebug() << "\tignoring...";
			inEvent->ignore();
			return; // let the completer do default behavior
		default:
			break;
		}
	}
	
	//	update the last key event timestamp
	{
		std::lock_guard<std::recursive_mutex>		tmpLock(timestampLock);
		lastKeyEventTimestamp = inEvent->timestamp();
	}
	//	if this was a shortcut (cmd-E/ctrl-E)
	bool		isShortcut = ((inEvent->modifiers() & Qt::ControlModifier) && inEvent->key() == Qt::Key_E);
	if (isShortcut) {
		//	populate the completer with the text under the cursor
		updateCompleterUsingTextUnderCursor();
		//	if there are valid completions
		if (completer->completionCount() > 0)	{
			//	open the pop-up button immediately
			QRect		tmpRect = cursorRect();
			tmpRect.setWidth(completer->popup()->sizeHintForColumn(0) + completer->popup()->verticalScrollBar()->sizeHint().width());
			completer->complete(tmpRect);
		}
		return;
	}
	
	
	//	...if i'm here, this wasn't a shortcut
	
	
	switch (inEvent->key())	{
	//	if this was a left/right arrow key event, close the popup immediately.  we have to check for this first because arrow keys are considered a modifier, and modifiers should not automatically close the pop-up...
	case Qt::Key_Left:
	case Qt::Key_Right:
		//qDebug() << "\tdirectional key used, closing popup and moving cursor
		closeCompleter();
		//	pass the key event to the super
		QPlainTextEdit::keyPressEvent(inEvent);
		break;
	//	if this was a return/enter key, we want to add a newline- and also as much whitespace as exists at the beginning of this line
	case Qt::Key_Enter:
	case Qt::Key_Return:
		{
			//	we know for a fact that the completer isn't open because enter and return are explicitly checked for if there's a completer at this method's beginning
			
			//	select the line under the current cursor, copy it into a std::string
			QTextCursor		tmpCurs = textCursor();
			tmpCurs.select(QTextCursor::LineUnderCursor);
			string			lineString = tmpCurs.selectedText().toStdString();
			
			//	pass the key event to the super
			QPlainTextEdit::keyPressEvent(inEvent);
			
			//	search the line that was under the current cursor for all the whitespace in the beginning- if i found stuff, append it
			regex			regex("^[\\s]*");
			smatch			matches;
			if (regex_search(lineString, matches, regex))	{
				insertPlainText( QString::fromStdString(matches[0]) );
			}
		}
		break;
	default:
		//	pass the key event to the super
		QPlainTextEdit::keyPressEvent(inEvent);
		break;
	}
	
	
	
	
	//	if this was a shift/cmd/opt/ctrl modifier, i can return now- i'm not going to do anything popup- or completer-related
	if (inEvent->modifiers() != Qt::NoModifier && inEvent->text().length()<1)	{
		//qDebug() << "\tpure modifier keypress, bailing!";
		return;
	}
	
	//	if this isn't a character, close the popup immediately and return
	if (inEvent->text().length() < 1)	{
		//qDebug() << "\tnon-character key event, closing completer popup and bailing!";
		closeCompleter();
		return;
	}
	
	
	
	
	//	if the popup is already open
	if (completer->popup()!=nullptr && completer->popup()->isVisible()) {
		static QString		eow("~!@#$%^&*()+{}|:\"<>?,./;'[]\\= ");
		//	if this latest keystroke ended a word
		if (eow.contains(inEvent->text().right(1))) {
			//	close the popup immediately
			closeCompleter();
			//	zero the last key event timestamp so pending events don't open the window agin
			//resetKeyEventTimestamp();
			return;
		}
		//	else the keystroke didn't end a word
		else	{
			//	update the completer's partial text immediately
			updateCompleterUsingTextUnderCursor();
			//	zero the last key event so pending events don't open the window again
			resetKeyEventTimestamp();
			return;
		}
	}
	//	else the popup isn't already open
	else	{
		//	call the maybe open completer event after a delay
		QPointer<SimpleSourceCodeEdit>		saveSelfPtr(this);
		QKeyEvent				eventCopy = *inEvent;
		QTimer::singleShot(2000, [=]()	{
			if (saveSelfPtr != nullptr)
				saveSelfPtr->maybeOpenCompleterEvent(eventCopy);
		});
		return;
	}
}
void SimpleSourceCodeEdit::focusInEvent(QFocusEvent *e)
{
	if (completer)
		completer->setWidget(this);
	QPlainTextEdit::focusInEvent(e);
}


void SimpleSourceCodeEdit::updateLineNumberAreaWidth(int newBlockCount)
{
	Q_UNUSED(newBlockCount);
	setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}
void SimpleSourceCodeEdit::highlightCurrentLine()
{
	QList<QTextEdit::ExtraSelection>		extraSelections;
	
	//	if the doc is editable, draw a highlight behind the current line
	if (!isReadOnly())	{
		QTextEdit::ExtraSelection		tmpSel;
		
		QColor			lineColor = QColor(Qt::yellow).lighter(160);
		
		tmpSel.format.setBackground(lineColor);
		tmpSel.format.setProperty(QTextFormat::FullWidthSelection, true);
		tmpSel.cursor = textCursor();
		tmpSel.cursor.clearSelection();
		
		extraSelections.append(tmpSel);
	}
	
	//	draw selections behind the error lines
	{
		std::lock_guard<std::recursive_mutex>		lock(errLock);
		if (errLineNumbers != nullptr)	{
			for (int i=0; i<errLineNumbers->length(); ++i)	{
				int			errLineNumber = errLineNumbers->at(i);
				
				QTextEdit::ExtraSelection		tmpSel;
				
				QColor		lineColor = QColor(Qt::red).lighter(160);
				
				tmpSel.format.setBackground(lineColor);
				tmpSel.format.setProperty(QTextFormat::FullWidthSelection, true);
				
				tmpSel.cursor = textCursor();
				tmpSel.cursor.clearSelection();
				tmpSel.cursor.setPosition(0, QTextCursor::MoveAnchor);
				
				if (!tmpSel.cursor.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor, errLineNumber-1))	{
					continue;
				}
				
				extraSelections.append(tmpSel);
			}
		}
	}
	
	setExtraSelections(extraSelections);
	
	//	if the currently-selected line of text is on an error, publish a signal
	{
		QTextCursor		tmpCursor = textCursor();
		tmpCursor.movePosition(QTextCursor::StartOfLine);
		int				lineCount = 1;
		while (tmpCursor.positionInBlock() > 0) {
			tmpCursor.movePosition(QTextCursor::Up);
			++lineCount;
		}
		QTextBlock		tmpBlock = tmpCursor.block().previous();
		while (tmpBlock.isValid())	{
			lineCount += tmpBlock.lineCount();
			tmpBlock = tmpBlock.previous();
		}
		
		bool			fireSignal = false;
		{
			std::lock_guard<std::recursive_mutex>		lock(errLock);
			if (errLineNumbers != nullptr)	{
				if (errLineNumbers->contains(lineCount))	{
					fireSignal = true;
				}
			}
		}
		
		if (fireSignal)
			emit selectedErrorAtLine(lineCount);
	}
}
void SimpleSourceCodeEdit::updateLineNumberArea(const QRect & rect, int dy)
{
	if (dy)
		lineNumberArea->scroll(0, dy);
	else
		lineNumberArea->update(0, rect.y(), lineNumberArea->width(), rect.height());
	
	if (rect.contains(viewport()->rect()))
		updateLineNumberAreaWidth(0);
}
void SimpleSourceCodeEdit::insertCompletion(const QString& completion)
{
	if (completer->widget() != this)
		return;
	/*
	QTextCursor tc = textCursor();
	int extra = completion.length() - completer->completionPrefix().length();
	tc.movePosition(QTextCursor::Left);
	tc.movePosition(QTextCursor::EndOfWord);
	tc.insertText(completion.right(extra));
	setTextCursor(tc);
	*/
	QTextCursor		tc = textCursor();
	tc.select(QTextCursor::WordUnderCursor);
	tc.removeSelectedText();
	tc.insertText(completion);
}
void SimpleSourceCodeEdit::closeCompleter()
{
	//qDebug() << __PRETTY_FUNCTION__;
	
	if (completer == nullptr)
		return;
	QAbstractItemView		*popup = completer->popup();
	if (popup == nullptr || !popup->isVisible())
		return;
	popup->hide();
}


void SimpleSourceCodeEdit::updateCompleterUsingTextUnderCursor()
{
	if (completer == nullptr)
		return;
	
	QTextCursor		tmpCursor = textCursor();
	tmpCursor.select(QTextCursor::WordUnderCursor);
	QString			partialText = tmpCursor.selectedText();
	completer->setCompletionPrefix(partialText);
	if (completer->popup()!=nullptr && completer->completionModel()!=nullptr)
		completer->popup()->setCurrentIndex(completer->completionModel()->index(0, 0));
}
void SimpleSourceCodeEdit::maybeOpenCompleterEvent(const QKeyEvent & inEvent)
{
	//qDebug() << __PRETTY_FUNCTION__ << ", timestamp is " << inEvent.timestamp();
	
	if (completer == nullptr)
		return;
	
	{
		std::lock_guard<std::recursive_mutex>		tmpLock(timestampLock);
		//	if the last key event timestamp differs from the timestamp of the passed event, bail
		if (inEvent.timestamp() != lastKeyEventTimestamp)	{
			//qDebug() << "\ttimestamp differs, bailing!";
			return;
		}
	}
	
	//	if the text under the cursor is an "end of word" character, bail- we're not going to open anything
	static QString		eow("~!@#$%^&*()+{}|:\"<>?,./;'[]\\= "); // end of word
	QTextCursor			tmpCursor = textCursor();
	tmpCursor.select(QTextCursor::WordUnderCursor);
	if (eow.contains(tmpCursor.selectedText().right(1)))	{
		//qDebug() << "\tlast-entered char is an end of word char, bailing";
		return;
	}
	
	//	populate the completer with the text under the cursor
	updateCompleterUsingTextUnderCursor();
	
	//	open the pop-up button immediately if it isn't already visible
	if (completer->popup() != nullptr && !completer->popup()->isVisible() && completer->completionCount()>0)	{
		//qDebug() << "\topening the completer!";
		QRect		tmpRect = cursorRect();
		tmpRect.setWidth(completer->popup()->sizeHintForColumn(0) + completer->popup()->verticalScrollBar()->sizeHint().width());
		completer->complete(tmpRect);
	}
	//else
		//qDebug() << "\tcompleter is already visible!";
}


void SimpleSourceCodeEdit::lineNumberAreaPaintEvent(QPaintEvent * event)
{
	QPainter		painter(lineNumberArea);
	painter.fillRect(event->rect(), Qt::lightGray);
	
	QTextBlock		block = firstVisibleBlock();
	int			blockNumber = block.blockNumber();
	int			top = static_cast<int>(blockBoundingGeometry(block).translated(contentOffset()).top());
	int			bottom = top + static_cast<int>(blockBoundingRect(block).height());
	
	while (block.isValid() && top <= event->rect().bottom())	{
		if (block.isVisible() && bottom >= event->rect().top()) {
			QString		number = QString::number(blockNumber + 1);
			painter.setPen(Qt::black);
			painter.drawText(0, top, lineNumberArea->width(), fontMetrics().height(), Qt::AlignRight, number);
		}
		
		block = block.next();
		top = bottom;
		bottom = top + static_cast<int>(blockBoundingRect(block).height());
		++blockNumber;
	}
}
int SimpleSourceCodeEdit::lineNumberAreaWidth()
{
	int			digits = 1;
	int			max = qMax(1, blockCount());
	while (max >= 10)	{
		max /= 10;
		++digits;
	}
	
#if (QT_VERSION >= QT_VERSION_CHECK(5, 10, 0))
	int			space = fontMetrics().horizontalAdvance(QLatin1Char('9')) * (digits + 1);
	return space;
#else
	return fontMetrics().boundingRect(QLatin1Char('9')).width() * (digits + 1);
#endif
}






/*	========================================	*/
#pragma mark --------------------- LineNumberArea






LineNumberArea::LineNumberArea(SimpleSourceCodeEdit * inEditor) : QWidget(inEditor) {
	codeEditor = inEditor;
}
QSize LineNumberArea::sizeHint() const {
	return QSize(codeEditor->lineNumberAreaWidth(), 0);
}
void LineNumberArea::paintEvent(QPaintEvent * event)	{
	
	codeEditor->lineNumberAreaPaintEvent(event);
}






/*	========================================	*/
#pragma mark --------------------- Highlighter






Highlighter::Highlighter(QTextDocument * inParent)
	: QSyntaxHighlighter(inParent)
{
	//	configure some default comment start and end expressions
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
	
	//	if there isn't an open multi-line comment, look for the beginning of one in the passed text
	int		startIndex = 0;
	if (previousBlockState() != HBS_OpenComment)
		startIndex = inText.indexOf(commentStartExpr);
	
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
