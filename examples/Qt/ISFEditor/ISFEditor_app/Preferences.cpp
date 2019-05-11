#include "Preferences.h"
#include "ui_Preferences.h"

#include <QSettings>
#include <QColor>
#include <QVariant>

#include <QColorDialog>
#include <QDebug>
#include <QMessageBox>

#include "DocWindow.h"




static Preferences		*_globalPrefsWindow = nullptr;




Preferences::Preferences(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::Preferences)
{
	ui->setupUi(this);
	
	QObject::connect(ui->color_reset_button, &QPushButton::clicked, this, &Preferences::resetDefaultColorsClicked);
	QObject::connect(ui->color_txt_txt, &QLabelClickable::clicked, this, &Preferences::colorLabelClicked);
	QObject::connect(ui->color_txt_bg, &QLabelClickable::clicked, this, &Preferences::colorLabelClicked);
	
	QObject::connect(ui->color_txt_linebg, &QLabelClickable::clicked, this, &Preferences::colorLabelClicked);
	QObject::connect(ui->color_txt_seltxt, &QLabelClickable::clicked, this, &Preferences::colorLabelClicked);
	QObject::connect(ui->color_txt_selbg, &QLabelClickable::clicked, this, &Preferences::colorLabelClicked);
	QObject::connect(ui->color_txt_seltxt_alt, &QLabelClickable::clicked, this, &Preferences::colorLabelClicked);
	QObject::connect(ui->color_txt_selbg_alt, &QLabelClickable::clicked, this, &Preferences::colorLabelClicked);
	
	QObject::connect(ui->color_txt_var, &QLabelClickable::clicked, this, &Preferences::colorLabelClicked);
	QObject::connect(ui->color_txt_typeClass, &QLabelClickable::clicked, this, &Preferences::colorLabelClicked);
	QObject::connect(ui->color_txt_funcs, &QLabelClickable::clicked, this, &Preferences::colorLabelClicked);
	QObject::connect(ui->color_txt_sdkFuncs, &QLabelClickable::clicked, this, &Preferences::colorLabelClicked);
	QObject::connect(ui->color_txt_keywords, &QLabelClickable::clicked, this, &Preferences::colorLabelClicked);
	QObject::connect(ui->color_txt_pragmas, &QLabelClickable::clicked, this, &Preferences::colorLabelClicked);
	QObject::connect(ui->color_txt_numbers, &QLabelClickable::clicked, this, &Preferences::colorLabelClicked);
	QObject::connect(ui->color_txt_quotes, &QLabelClickable::clicked, this, &Preferences::colorLabelClicked);
	QObject::connect(ui->color_txt_comment, &QLabelClickable::clicked, this, &Preferences::colorLabelClicked);
	
	updateLocalUI();
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
void Preferences::resetDefaultColorsClicked()	{
	QSettings		settings;
	QColor			tmpColor;
	tmpColor = QColor("#1a1a1a");
	settings.setValue("color_txt_bg", QVariant(tmpColor));
	tmpColor = QColor("#b4b4b4");
	settings.setValue("color_txt_txt", QVariant(tmpColor));
	tmpColor = QColor("#000000");
	settings.setValue("color_txt_seltxt", QVariant(tmpColor));
	tmpColor = QColor("#ffff50");
	settings.setValue("color_txt_selbg", QVariant(tmpColor));
	
	tmpColor = QColor("#000000");
	settings.setValue("color_txt_seltxt_alt", QVariant(tmpColor));
	tmpColor = QColor("#999950");
	settings.setValue("color_txt_selbg_alt", QVariant(tmpColor));
	
	tmpColor = QColor("#aaffff");
	settings.setValue("color_txt_var", QVariant(tmpColor));
	tmpColor = QColor("#aa00ff");
	settings.setValue("color_txt_typeClass", QVariant(tmpColor));
	tmpColor = QColor("#55aaff");
	settings.setValue("color_txt_funcs", QVariant(tmpColor));
	tmpColor = QColor("#55aaff");
	settings.setValue("color_txt_sdkFuncs", QVariant(tmpColor));
	tmpColor = QColor("#ffffff");
	settings.setValue("color_txt_keywords", QVariant(tmpColor));
	tmpColor = QColor("#00ff00");
	settings.setValue("color_txt_pragmas", QVariant(tmpColor));
	tmpColor = QColor("#ff3737");
	settings.setValue("color_txt_numbers", QVariant(tmpColor));
	tmpColor = QColor("#ff3737");
	settings.setValue("color_txt_quotes", QVariant(tmpColor));
	tmpColor = QColor("#ffc737");
	settings.setValue("color_txt_comment", QVariant(tmpColor));
	//	update my local UI...
	updateLocalUI();
	//	the various text views need to update their colors and then redisplay their contents
	DocWindow		*dw = GetDocWindow();
	if (dw != nullptr)
		dw->reloadColorsAndSyntaxFormats();
}
Preferences::~Preferences()
{
	delete ui;
}



void Preferences::updateLocalUI()	{
	QSettings		settings;
	
	QStringList		keys = {
		"color_txt_bg",
		"color_txt_txt",
		"color_txt_linebg",
		"color_txt_seltxt",
		"color_txt_selbg",
		"color_txt_seltxt_alt",
		"color_txt_selbg_alt",
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
			label = ui->color_txt_bg;
		}
		else if (str == "color_txt_txt")	{
			label = ui->color_txt_txt;
		}
		else if (str == "color_txt_linebg")	{
			label = ui->color_txt_linebg;
		}
		else if (str == "color_txt_seltxt")	{
			label = ui->color_txt_seltxt;
		}
		else if (str == "color_txt_selbg")	{
			label = ui->color_txt_selbg;
		}
		else if (str == "color_txt_seltxt_alt")	{
			label = ui->color_txt_seltxt_alt;
		}
		else if (str == "color_txt_selbg_alt")	{
			label = ui->color_txt_selbg_alt;
		}
		else if (str == "color_txt_var")	{
			label = ui->color_txt_var;
		}
		else if (str == "color_txt_typeClass")	{
			label = ui->color_txt_typeClass;
		}
		else if (str == "color_txt_funcs")	{
			label = ui->color_txt_funcs;
		}
		else if (str == "color_txt_sdkFuncs")	{
			label = ui->color_txt_sdkFuncs;
		}
		else if (str == "color_txt_keywords")	{
			label = ui->color_txt_keywords;
		}
		else if (str == "color_txt_pragmas")	{
			label = ui->color_txt_pragmas;
		}
		else if (str == "color_txt_numbers")	{
			label = ui->color_txt_numbers;
		}
		else if (str == "color_txt_quotes")	{
			label = ui->color_txt_quotes;
		}
		else if (str == "color_txt_comment")	{
			label = ui->color_txt_comment;
		}
		
		if (label != nullptr)	{
			tmpColor = settings.value(str).value<QColor>();
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
