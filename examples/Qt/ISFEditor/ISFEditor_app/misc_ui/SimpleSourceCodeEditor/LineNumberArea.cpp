#include "LineNumberArea.h"

#include "SimpleSourceCodeEditor.h"




namespace SimpleSourceCodeEdit	{




LineNumberArea::LineNumberArea(SimpleSourceCodeEditor * inEditor) : QWidget(inEditor) {
	codeEditor = inEditor;
}
QSize LineNumberArea::sizeHint() const {
	return QSize(codeEditor->lineNumberAreaWidth(), 0);
}
void LineNumberArea::paintEvent(QPaintEvent * event)	{
	
	codeEditor->lineNumberAreaPaintEvent(event);
}




}	//	namespace SimpleSourceCodeEdit
