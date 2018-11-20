#ifndef SHADERTOYCONVERTER_H
#define SHADERTOYCONVERTER_H

#include <QDialog>




namespace Ui {
	class ShadertoyConverter;
}


class ShadertoyConverter : public QDialog
{
	Q_OBJECT

public:
	explicit ShadertoyConverter(QWidget *parent = nullptr);
	~ShadertoyConverter();
	
	QString exportedISFPath() { return _exportedISFPath; }

public slots:
	Q_SLOT void cancelClicked();
	Q_SLOT void okClicked();

private:
	Ui::ShadertoyConverter *ui;
	QString				_exportedISFPath;
	
	//QString convertShaderSource(const QString & rawFragString, const QJsonObject & suppEntries);
	QString convertShaderSource(const QStringList & inSortedShaderSourceArray, QJsonObject & inSuppEntries, const QJsonArray & inPassVarNameSwapDicts);
};




#endif // SHADERTOYCONVERTER_H
