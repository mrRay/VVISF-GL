#ifndef LINENUMBERAREA_H
#define LINENUMBERAREA_H

#include <QWidget>




namespace SimpleSourceCodeEdit	{

class SimpleSourceCodeEditor;


class LineNumberArea : public QWidget
{
	Q_OBJECT
	
public:
	LineNumberArea(SimpleSourceCodeEditor * inEditor);
	QSize sizeHint() const override;

protected:
	void paintEvent(QPaintEvent * event) override;

private:
	SimpleSourceCodeEditor			*codeEditor;
};




}	//	namespace SimpleSourceCodeEdit




#endif // LINENUMBERAREA_H