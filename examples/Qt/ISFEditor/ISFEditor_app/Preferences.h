#ifndef PREFERENCES_H
#define PREFERENCES_H

#include <QWidget>




namespace Ui {
	class Preferences;
}

class Preferences : public QWidget
{
	Q_OBJECT

public:
	explicit Preferences(QWidget *parent = nullptr);
	~Preferences();
	
public slots:
	Q_SLOT void upateLocalUI();
	Q_SLOT void colorLabelClicked();

private slots:
	void on_gl4CheckBox_stateChanged(int arg1);

private:
	Ui::Preferences *ui;
};




Preferences * GetPreferences();




#endif // PREFERENCES_H
