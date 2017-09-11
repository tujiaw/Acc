#pragma once

#include <QWidget>
#include "ui_SettingWidget.h"

class SettingWidget : public QWidget
{
	Q_OBJECT

public:
	SettingWidget(QWidget *parent = Q_NULLPTR);
	~SettingWidget();

private slots:
	void slotFirst();
	void slotSecond();

protected:
	void keyPressEvent(QKeyEvent *e);

private:
	Ui::SettingWidget ui;
};
