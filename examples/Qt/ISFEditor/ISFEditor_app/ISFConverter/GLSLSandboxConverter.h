#ifndef GLSLSANDBOXCONVERTER_H
#define GLSLSANDBOXCONVERTER_H

#include <QDialog>




namespace Ui {
	class GLSLSandboxConverter;
}


class GLSLSandboxConverter : public QDialog
{
	Q_OBJECT

public:
	explicit GLSLSandboxConverter(QWidget *parent = nullptr);
	~GLSLSandboxConverter();
	
	QString exportedISFPath() { return _exportedISFPath; }

public slots:
	Q_SLOT void cancelClicked();
	Q_SLOT void okClicked();

private:
	Ui::GLSLSandboxConverter *ui;
	QString				_exportedISFPath;
	
	QString convertShaderSource(const QString & rawFragString, const QJsonObject & suppEntries);
};




#endif // GLSLSANDBOXCONVERTER_H
