#include "JSONGUIInputWidget.h"

#include <QObject>
#include <QLabel>
#include <QComboBox>
#include <QLineEdit>
#include <QDebug>

#include "JSONScrollWidget.h"
#include "JGMTop.h"
#include "QLabelClickable.h"
#include "QLabelDrag.h"




JSONGUIInputWidget::JSONGUIInputWidget(const JGMInputRef & inInput, QWidget *parent) :
	QWidget(parent),
	_input(inInput)
{
	setAcceptDrops(true);
}




void JSONGUIInputWidget::dragEnterEvent(QDragEnterEvent * e)	{
	//qDebug() << __PRETTY_FUNCTION__;
	const QMimeData		*md = e->mimeData();
	if (md!=nullptr && md->hasFormat("text/JGMInputDrag"))	{
		QPoint		tmpPoint = e->pos();
		QSize		tmpSize = frameSize();
		if (tmpPoint.y() > tmpSize.height()/2)
			_dropEdge = Qt::BottomEdge;
		else
			_dropEdge = Qt::TopEdge;
		update();
		e->acceptProposedAction();
	}
}
void JSONGUIInputWidget::dragMoveEvent(QDragMoveEvent * e)	{
	//qDebug() << __PRETTY_FUNCTION__;
	QPoint		tmpPoint = e->pos();
	QSize		tmpSize = frameSize();
	if (tmpPoint.y() > tmpSize.height()/2)
		_dropEdge = Qt::BottomEdge;
	else
		_dropEdge = Qt::TopEdge;
	update();
	e->accept();
}
void JSONGUIInputWidget::dragLeaveEvent(QDragLeaveEvent * e)	{
	//qDebug() << __PRETTY_FUNCTION__;
	_dropEdge = Qt::LeftEdge;
	update();
	e->accept();
}
void JSONGUIInputWidget::dropEvent(QDropEvent * e)	{
	qDebug() << __PRETTY_FUNCTION__;
	
	const QMimeData		*md = e->mimeData();
	QVariant			mdvar = QVariant(md->data("text/JGMInputDrag"));
	int					srcIndex = mdvar.toInt();
	
	JGMTop			*top = _input->top();
	int				dstIndex = top->indexOfInput(*_input);
	if (srcIndex == dstIndex)
		return;
	if (_dropEdge == Qt::BottomEdge)
		++dstIndex;
	if (dstIndex > srcIndex)
		--dstIndex;

	qDebug() << "accepting drop from " << srcIndex << ", dstIndex is " << dstIndex;
	JGMCInputArray		&inputsC = top->inputsContainer();
	QVector<JGMInputRef>		&inputsV = inputsC.contents();
	inputsV.move(srcIndex, dstIndex);
	
	_dropEdge = Qt::LeftEdge;
	update();
	e->accept();
	
	QTimer::singleShot(50, []()	{
		RecreateJSONAndExport();
	});
}
void JSONGUIInputWidget::paintEvent(QPaintEvent * e)	{
	//qDebug() << __PRETTY_FUNCTION__;
	
	QRect			bounds(QPoint(), frameSize());
	QRect			drawRect;
	switch (_dropEdge)	{
	case Qt::TopEdge:
		drawRect = QRect(
			bounds.topLeft().x(),
			bounds.topLeft().y(),
			bounds.size().width(),
			13.0);
		break;
	case Qt::BottomEdge:
		drawRect = QRect(
			bounds.bottomLeft().x(),
			bounds.bottomLeft().y() - 13.0,
			bounds.size().width(),
			13.0);
		break;
	default:
		return;
	}
	
	
	QPainter		painter(this);
	painter.fillRect(drawRect, QBrush(QColor(255,0,0,255)));
}




void JSONGUIInputWidget::prepareDragLabel(QLabelDrag * dragLabel)	{
	JGMTop		*top = _input->top();
	if (top != nullptr)	{
		//	configure the drag label so it's "text/JGMInputDrag" and its data is the index of my input
		dragLabel->mimeType = QString("text/JGMInputDrag");
		//ui->dragLabel->dragVariant = QVariant( QString("frisky dingo") );
		dragLabel->dragVariant = QVariant( top->indexOfInput(*_input) );
		
	}
}
void JSONGUIInputWidget::prepareInputNameEdit(QLineEdit * inputNameEdit)	{
	QObject::disconnect(inputNameEdit, 0, 0, 0);
	
	//	pressing enter on a line
	QObject::connect(inputNameEdit, &QLineEdit::editingFinished, [&]()	{
		//qDebug() << __FUNCTION__ << " QLineEdit::editingFinished";
		
		JGMTop		*top = (_input==nullptr) ? nullptr : _input->top();
		if (top == nullptr)
			return;
		
		QString		origAttrName = _input->value("NAME").toString();
		QString		newString = inputNameEdit->text();
		//	if the new input name isn't valid (because an input by that name already exists)
		if (top->getInputNamed(newString)!=nullptr)	{
			//	if the new input name is the same as the existing name
			if (origAttrName == newString)	{
				//	deselect the text, remove focus from the widget
				inputNameEdit->deselect();
				inputNameEdit->clearFocus();
			}
			//	else the new input name is different- just...not valid (probably a dupe)
			else	{
				//	reset the displayed text and return (should still be editing)
				inputNameEdit->setText(origAttrName);
			}
		}
		//	else the new input name is valid
		else	{
			//	update the attribute
			_input->setValue("NAME", QJsonValue(newString));
			//	deselect the text, remove focus from the widget
			inputNameEdit->deselect();
			inputNameEdit->clearFocus();
			//	recreate the json & export the file
			RecreateJSONAndExport();
		}
		
	});
}
void JSONGUIInputWidget::prepareLabelField(QLineEdit * labelField)	{
	QObject::disconnect(labelField, 0, 0, 0);
	
	QObject::connect(labelField, &QLineEdit::editingFinished, [&]()	{
		if (_input == nullptr)
			return;
		QString		tmpString = labelField->text();
		if (tmpString.length() < 1)
			_input->setValue("LABEL", QJsonValue::Undefined);
		else
			_input->setValue("LABEL", tmpString);
		RecreateJSONAndExport();
	});
}
void JSONGUIInputWidget::prepareTypeCBox(QComboBox * typeCB)	{
	QObject::disconnect(typeCB, 0, 0, 0);
	
	typeCB->clear();
	typeCB->addItem( "event" );
	typeCB->addItem( "bool" );
	typeCB->addItem( "long" );
	typeCB->addItem( "float" );
	typeCB->addItem( "point2D" );
	typeCB->addItem( "color" );
	typeCB->addItem( "image" );
	typeCB->addItem( "audio" );
	typeCB->addItem( "audioFFT" );
	
	QObject::connect(typeCB, QOverload<const QString &>::of(&QComboBox::activated), [&](const QString & inText)	{
		if (_input == nullptr)
			return;
		_input->setValue("TYPE", inText);
		RecreateJSONAndExport();
	});
}
void JSONGUIInputWidget::prepareDeleteLabel(QLabelClickable * deleteLabel)	{
	QObject::disconnect(deleteLabel, 0, 0, 0);
	
	QObject::connect(deleteLabel, &QLabelClickable::clicked, [&]()	{
		if (!_input.isNull())	{
			if (_input->top()->deleteInput(_input))	{
				RecreateJSONAndExport();
			}
		}
	});
}




void JSONGUIInputWidget::refreshInputNameEdit(QLineEdit * inputNameEdit)	{
	//qDebug() << __PRETTY_FUNCTION__;
	QString		tmpString = (_input==nullptr) ? "" : _input->value("NAME").toString();
	inputNameEdit->setText(tmpString);
}
void JSONGUIInputWidget::refreshLabelField(QLineEdit * labelField)	{
	QString		tmpString = (_input==nullptr) ? "" : _input->value("LABEL").toString();
	labelField->setText(tmpString);
}
void JSONGUIInputWidget::refreshTypeCBox(QComboBox * typeCB)	{
	QString		tmpString = (_input==nullptr) ? "" : _input->value("TYPE").toString();
	typeCB->setCurrentText(tmpString);
}