#include "Preferences.h"
#include "ui_Preferences.h"

#include <QSettings>
#include <QColor>
#include <QVariant>

#include "DocWindow.h"




static Preferences		*_globalPrefsWindow = nullptr;




Preferences::Preferences(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::Preferences)
{
	ui->setupUi(this);
	
	QObject::connect(ui->color_txt_txt, &QLabelClickable::clicked, this, &Preferences::colorLabelClicked);
	QObject::connect(ui->color_txt_bg, &QLabelClickable::clicked, this, &Preferences::colorLabelClicked);
	
	QObject::connect(ui->color_txt_var, &QLabelClickable::clicked, this, &Preferences::colorLabelClicked);
	QObject::connect(ui->color_txt_typeClass, &QLabelClickable::clicked, this, &Preferences::colorLabelClicked);
	QObject::connect(ui->color_txt_funcs, &QLabelClickable::clicked, this, &Preferences::colorLabelClicked);
	QObject::connect(ui->color_txt_sdkFuncs, &QLabelClickable::clicked, this, &Preferences::colorLabelClicked);
	QObject::connect(ui->color_txt_keywords, &QLabelClickable::clicked, this, &Preferences::colorLabelClicked);
	QObject::connect(ui->color_txt_pragmas, &QLabelClickable::clicked, this, &Preferences::colorLabelClicked);
	QObject::connect(ui->color_txt_numbers, &QLabelClickable::clicked, this, &Preferences::colorLabelClicked);
	QObject::connect(ui->color_txt_quotes, &QLabelClickable::clicked, this, &Preferences::colorLabelClicked);
	QObject::connect(ui->color_txt_comment, &QLabelClickable::clicked, this, &Preferences::colorLabelClicked);
	
	upateLocalUI();
}

void Preferences::colorLabelClicked()	{
	QLabelClickable		*clickedLabel = qobject_cast<QLabelClickable*>(sender());
	if (clickedLabel == nullptr)
		return;
	QString				clickedName = clickedLabel->objectName();
	QPalette			tmpPalette = clickedLabel->palette();
	QColor				tmpColor = tmpPalette.color(clickedLabel->backgroundRole());
	
	//	make a color dialog
	QColorDialog		*colorDialog = new QColorDialog(this);
	colorDialog->setOptions( QColorDialog::DontUseNativeDialog );
	colorDialog->setCurrentColor(tmpColor);
	//colorDialog->setOptions( QColorDialog::NoButtons );
	colorDialog->setAttribute( Qt::WA_DeleteOnClose );
	//	open a color picker dialog
	connect(colorDialog, &QColorDialog::currentColorChanged, [clickedName,clickedLabel](const QColor & inColor)	{
		//	update the QSettings
		QSettings			settings;
		settings.setValue(clickedName, QVariant(inColor));
		//	update the label's background color
		QPalette			tmpPalette = clickedLabel->palette();
		tmpPalette.setColor(clickedLabel->backgroundRole(), inColor);
		clickedLabel->setPalette(tmpPalette);
		//	the various text views need to update their colors and then redisplay their contents
		DocWindow		*dw = GetDocWindow();
		if (dw != nullptr)
			dw->reloadColorsAndSyntaxFormats();
	});
	//	open the color dialog
	colorDialog->open();
}
Preferences::~Preferences()
{
	delete ui;
}



void Preferences::upateLocalUI()	{
	QSettings		settings;
	
	QStringList		keys = {
		"color_txt_bg",
		"color_txt_txt",
		"color_txt_var",
		"color_txt_typeClass",
		"color_txt_funcs",
		"color_txt_sdkFuncs",
		"color_txt_keywords",
		"color_txt_pragmas",
		"color_txt_numbers",
		"color_txt_quotes",
		"color_txt_comment"
	};
	for (const QString & str : keys)	{
		QColor		tmpColor;
		QLabel		*label = nullptr;
		if (str == "color_txt_bg")	{
			tmpColor = settings.value(str).value<QColor>();
			label = ui->color_txt_bg;
		}
		else if (str == "color_txt_txt")	{
			tmpColor = settings.value(str).value<QColor>();
			label = ui->color_txt_txt;
		}
		else if (str == "color_txt_var")	{
			tmpColor = settings.value(str).value<QColor>();
			label = ui->color_txt_var;
		}
		else if (str == "color_txt_typeClass")	{
			tmpColor = settings.value(str).value<QColor>();
			label = ui->color_txt_typeClass;
		}
		else if (str == "color_txt_funcs")	{
			tmpColor = settings.value(str).value<QColor>();
			label = ui->color_txt_funcs;
		}
		else if (str == "color_txt_sdkFuncs")	{
			tmpColor = settings.value(str).value<QColor>();
			label = ui->color_txt_sdkFuncs;
		}
		else if (str == "color_txt_keywords")	{
			tmpColor = settings.value(str).value<QColor>();
			label = ui->color_txt_keywords;
		}
		else if (str == "color_txt_pragmas")	{
			tmpColor = settings.value(str).value<QColor>();
			label = ui->color_txt_pragmas;
		}
		else if (str == "color_txt_numbers")	{
			tmpColor = settings.value(str).value<QColor>();
			label = ui->color_txt_numbers;
		}
		else if (str == "color_txt_quotes")	{
			tmpColor = settings.value(str).value<QColor>();
			label = ui->color_txt_quotes;
		}
		else if (str == "color_txt_comment")	{
			tmpColor = settings.value(str).value<QColor>();
			label = ui->color_txt_comment;
		}
		
		if (label != nullptr)	{
			QPalette		tmpPalette = label->palette();
			tmpPalette.setColor(label->backgroundRole(), tmpColor);
			label->setAutoFillBackground(true);
			label->setPalette(tmpPalette);
		}
	}
	
	if (settings.contains("GL4") && settings.value("GL4").toBool())	{
		ui->gl4CheckBox->blockSignals(true);
		ui->gl4CheckBox->setChecked(true);
		ui->gl4CheckBox->blockSignals(false);
	}
}




Preferences * GetPreferences()	{
	if (_globalPrefsWindow == nullptr)	{
		_globalPrefsWindow = new Preferences();
	}
	return _globalPrefsWindow;
}


void Preferences::on_gl4CheckBox_stateChanged(int arg1)	{
	qDebug() << __PRETTY_FUNCTION__;
	
	//	change the value in the settings
	QSettings			settings;
	bool				newVal = (arg1==Qt::Checked) ? true : false;
	qDebug() << "\tnewVal is " << newVal;
	settings.setValue("GL4", QVariant(newVal));
	//	open a dialog informing the user that this change will take effect on app restart
	QMessageBox::warning(this, "", "You have to restart the ISF Editor for this change to take effect!", QMessageBox::Ok);
}
