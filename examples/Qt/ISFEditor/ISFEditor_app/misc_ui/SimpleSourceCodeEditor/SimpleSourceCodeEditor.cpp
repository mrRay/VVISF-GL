#include "SimpleSourceCodeEditor.h"

//#include <QTextBlock>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QStringListModel>
#include <QAbstractItemView>
#include <QScrollBar>
#include <QPointer>
#include <QTimer>
//#include <QApplication>
#include <QPainter>

#include <regex>
//#include <string>
//#include <iostream>

#include "Highlighter.h"
#include "LineNumberArea.h"
#include "FindDialog.h"






namespace SimpleSourceCodeEdit	{




SimpleSourceCodeEditor::SimpleSourceCodeEditor(QWidget * inParent) :
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
SimpleSourceCodeEditor::~SimpleSourceCodeEditor()
{
	//qDebug() << __PRETTY_FUNCTION__;
	
	std::lock_guard<std::recursive_mutex>		lock(errLock);
	if (errLineNumbers != nullptr)	{
		delete errLineNumbers;
		errLineNumbers = nullptr;
	}
}


void SimpleSourceCodeEditor::setErrorLineNumbers(const QVector<int> & inVect)	{
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
void SimpleSourceCodeEditor::setErrorLineNumbers(const QVector<int> * inVect)	{
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
void SimpleSourceCodeEditor::loadSyntaxDefinitionDocument(const QJsonDocument & inDocument)
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
void SimpleSourceCodeEditor::openFindDialog()	{
	FindDialog		*finder = new FindDialog(findOpts, this);
	//	get the find opts and return code from the dialog, then delete it right away
	int				returnCode = finder->exec();
	findOpts = finder->findOpts();
	delete finder;
	
	//	if we didn't get a return code then the dialog was a success and we need to search for something
	if (!returnCode)	{
		findNext();
	}
	else	{
		//	do nothing- user clicked 'cancel'
	}
}
void SimpleSourceCodeEditor::findNext()	{
	//qDebug() << __PRETTY_FUNCTION__;

	//	assemble the find flags enum we'll need later
	QTextDocument::FindFlags		ff = QTextDocument::FindFlags();
	if (findOpts.caseSensitive)
		ff |= QTextDocument::FindCaseSensitively;
	if (findOpts.entireWord)
		ff |= QTextDocument::FindWholeWords;
	
	//	make the regex options, and then the regex
	QRegularExpression::PatternOptions	regexOpts = QRegularExpression::NoPatternOption;
	if (findOpts.caseSensitive)
		regexOpts |= QRegularExpression::CaseInsensitiveOption;
	regexOpts |= QRegularExpression::OptimizeOnFirstUsageOption;
	QRegularExpression		regex(findOpts.searchString, regexOpts);
	
	//	get the current cursor- we're going to start searching from here
	QTextCursor		startCursor = textCursor();
	startCursor.clearSelection();
	QTextCursor		searchCursor = startCursor;
	QTextCursor		resultsCursor;
	
	//	figure out which block contains the start cursor, and how many blocks there are
	QTextDocument	*td = document();
	int				totalBlocks = td->blockCount();
	
	//	run the search for the information from the find options
	if (findOpts.regex)
		resultsCursor = td->find(regex, searchCursor, ff);
	else
		resultsCursor = td->find(findOpts.searchString, searchCursor, ff);
	
	//	if we got a non-null cursor then we found something
	if (!resultsCursor.isNull())	{
		//	select the text described by the cursor and return, we're done
		setTextCursor(resultsCursor);
		return;
	}
	
	//	...if we're here, we haven't found a result yet- run through the text blocks to the end of the doc, running the search again for each block
	for (int i=searchCursor.blockNumber()+1; i<totalBlocks; ++i)	{
		searchCursor.setPosition(td->findBlockByNumber(i).position(), QTextCursor::MoveAnchor);
		if (findOpts.regex)
			resultsCursor = td->find(regex, searchCursor, ff);
		else
			resultsCursor = td->find(findOpts.searchString, searchCursor, ff);
		//	if i found the text, select it and return, we're done
		if (!resultsCursor.isNull())	{
			setTextCursor(resultsCursor);
			return;
		}
	}
	
	//	...if we're here, we still haven't found a result- loop around to the beginning of the doc and start searching again, stopping when we hit the initial search cursor
	for (int i=0; i<startCursor.blockNumber(); ++i)	{
		searchCursor.setPosition(td->findBlockByNumber(i).position(), QTextCursor::MoveAnchor);
		if (findOpts.regex)
			resultsCursor = td->find(regex, searchCursor, ff);
		else
			resultsCursor = td->find(findOpts.searchString, searchCursor, ff);
		//	if i found the text, select it and return, we're done
		if (!resultsCursor.isNull())	{
			setTextCursor(resultsCursor);
			return;
		}
	}
	
	//	...if we're here, we still haven't found a result- we have almost looped through the entire document by now.  we just need to check the area in the original block prior to 'startCursor'
	searchCursor.setPosition(startCursor.block().position(), QTextCursor::MoveAnchor);
	if (findOpts.regex)
		resultsCursor = td->find(regex, searchCursor, ff);
	else
		resultsCursor = td->find(findOpts.searchString, searchCursor, ff);
	if (!resultsCursor.isNull())	{
		//	we can only use 'resultsCursor' if its position is prior to the position of the start cursor
		if (resultsCursor.position() < startCursor.position())	{
			//	we found the thing we're looking for!
			setTextCursor(resultsCursor);
			return;
		}
		//	else the results cursor is after the start cursor- do nothing, disregard it
	}
	//	else the results cursor was null- we've searched the entire document and haven't found anything.
	//	maybe beep or something?
}
void SimpleSourceCodeEditor::findPrevious()	{
	//qDebug() << __PRETTY_FUNCTION__;

	//	assemble the find flags enum we'll need later
	QTextDocument::FindFlags		ff = QTextDocument::FindFlags();
	if (findOpts.caseSensitive)
		ff |= QTextDocument::FindCaseSensitively;
	if (findOpts.entireWord)
		ff |= QTextDocument::FindWholeWords;
	//	we're searching backwards, so we need this flag too...
	ff |= QTextDocument::FindBackward;
	
	//	make the regex options, and then the regex
	QRegularExpression::PatternOptions	regexOpts = QRegularExpression::NoPatternOption;
	if (findOpts.caseSensitive)
		regexOpts |= QRegularExpression::CaseInsensitiveOption;
	regexOpts |= QRegularExpression::OptimizeOnFirstUsageOption;
	QRegularExpression		regex(findOpts.searchString, regexOpts);
	
	//	get the current cursor- we're going to start searching from here
	QTextCursor		startCursor = textCursor();
	QTextCursor		searchCursor = startCursor;
	searchCursor.clearSelection();
	QTextCursor		resultsCursor;
	
	//	figure out which block contains the start cursor, and how many blocks there are
	QTextDocument	*td = document();
	int				totalBlocks = td->blockCount();
	
	//	run the search for the information from the find options
	if (findOpts.regex)
		resultsCursor = td->find(regex, searchCursor, ff);
	else
		resultsCursor = td->find(findOpts.searchString, searchCursor, ff);
	
	//	if we got a non-null cursor result and that result starts on the same char as the start cursor....we have to run the search again.
	if (!resultsCursor.isNull() && resultsCursor==startCursor)	{
		//	if this was the first thing in the block then null the result- we found the text we started with
		if (searchCursor.selectionStart() - searchCursor.block().position() == 0)	{
			resultsCursor = QTextCursor();
		}
		//	else this wasn't the first thing in the block- decrement the cursor and search again
		else	{
			searchCursor.setPosition(resultsCursor.selectionStart()-1, QTextCursor::MoveAnchor);
			if (findOpts.regex)
				resultsCursor = td->find(regex, searchCursor, ff);
			else
				resultsCursor = td->find(findOpts.searchString, searchCursor, ff);
		}
	}
	
	//	if we got a non-null cursor then we found something
	if (!resultsCursor.isNull())	{
		//	select the text described by the cursor and return, we're done
		setTextCursor(resultsCursor);
		return;
	}
	
	//	...if we're here, we haven't found a result yet- run through the text blocks to the beginning of the doc, running the search again for each block
	for (int i=searchCursor.blockNumber()-1; i>=0; --i)	{
		QTextBlock		tmpBlock = td->findBlockByNumber(i);
		searchCursor.setPosition(tmpBlock.position()+tmpBlock.length()-1, QTextCursor::MoveAnchor);
		if (findOpts.regex)
			resultsCursor = td->find(regex, searchCursor, ff);
		else
			resultsCursor = td->find(findOpts.searchString, searchCursor, ff);
		//	if i found the text, select it and return, we're done
		if (!resultsCursor.isNull())	{
			setTextCursor(resultsCursor);
			return;
		}
	}
	
	//	...if we're here, we still haven't found a result- loop around to the beginning of the doc and start searching again, stopping when we hit the initial search cursor
	for (int i=totalBlocks-1; i>startCursor.blockNumber(); --i)	{
		QTextBlock		tmpBlock = td->findBlockByNumber(i);
		searchCursor.setPosition(tmpBlock.position()+tmpBlock.length()-1, QTextCursor::MoveAnchor);
		if (findOpts.regex)
			resultsCursor = td->find(regex, searchCursor, ff);
		else
			resultsCursor = td->find(findOpts.searchString, searchCursor, ff);
		//	if i found the text, select it and return, we're done
		if (!resultsCursor.isNull())	{
			setTextCursor(resultsCursor);
			return;
		}
	}
	
	//	...if we're here, we still haven't found a result- we have almost looped through the entire document by now.  we just need to check the area in the original block prior to 'startCursor'
	{
		QTextBlock		tmpBlock = td->findBlockByNumber(startCursor.blockNumber());
		searchCursor.setPosition(tmpBlock.position()+tmpBlock.length()-1, QTextCursor::MoveAnchor);
		if (findOpts.regex)
			resultsCursor = td->find(regex, searchCursor, ff);
		else
			resultsCursor = td->find(findOpts.searchString, searchCursor, ff);
		if (!resultsCursor.isNull())	{
			//	we can only use 'resultsCursor' if its position is prior to the position of the start cursor
			if (resultsCursor.position() > startCursor.position())	{
				//	we found the thing we're looking for!
				setTextCursor(resultsCursor);
				return;
			}
			//	else the results cursor is after the start cursor- do nothing, disregard it
		}
		//	else the results cursor was null- we've searched the entire document and haven't found anything.
		//	maybe beep or something?
	}
}
void SimpleSourceCodeEditor::setFindStringFromCursor()	{
	findOpts.searchString = textCursor().selectedText();
}

	
void SimpleSourceCodeEditor::resizeEvent(QResizeEvent * inEvent)
{
	QPlainTextEdit::resizeEvent(inEvent);
	
	QRect		cr = contentsRect();
	lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
}
void SimpleSourceCodeEditor::keyPressEvent(QKeyEvent *inEvent)
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
		QPointer<SimpleSourceCodeEditor>		saveSelfPtr(this);
		QKeyEvent				eventCopy = *inEvent;
		QTimer::singleShot(2000, [=]()	{
			if (saveSelfPtr != nullptr)
				saveSelfPtr->maybeOpenCompleterEvent(eventCopy);
		});
		return;
	}
}
void SimpleSourceCodeEditor::focusInEvent(QFocusEvent *e)
{
	if (completer)
		completer->setWidget(this);
	QPlainTextEdit::focusInEvent(e);
}


void SimpleSourceCodeEditor::updateLineNumberAreaWidth(int newBlockCount)
{
	Q_UNUSED(newBlockCount);
	setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}
void SimpleSourceCodeEditor::highlightCurrentLine()
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
void SimpleSourceCodeEditor::updateLineNumberArea(const QRect & rect, int dy)
{
	if (dy)
		lineNumberArea->scroll(0, dy);
	else
		lineNumberArea->update(0, rect.y(), lineNumberArea->width(), rect.height());
	
	if (rect.contains(viewport()->rect()))
		updateLineNumberAreaWidth(0);
}
void SimpleSourceCodeEditor::insertCompletion(const QString& completion)
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
void SimpleSourceCodeEditor::closeCompleter()
{
	//qDebug() << __PRETTY_FUNCTION__;
	
	if (completer == nullptr)
		return;
	QAbstractItemView		*popup = completer->popup();
	if (popup == nullptr || !popup->isVisible())
		return;
	popup->hide();
}


void SimpleSourceCodeEditor::updateCompleterUsingTextUnderCursor()
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
void SimpleSourceCodeEditor::maybeOpenCompleterEvent(const QKeyEvent & inEvent)
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


void SimpleSourceCodeEditor::lineNumberAreaPaintEvent(QPaintEvent * event)
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
int SimpleSourceCodeEditor::lineNumberAreaWidth()
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




}	//	namespace SimpleSourceCodeEdit

