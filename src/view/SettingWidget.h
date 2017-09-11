#pragma once

#include <QWidget>
#include "ui_SettingWidget.h"

class SettingWidget : public QWidget
{
	Q_OBJECT

public:
	SettingWidget(QWidget *parent = Q_NULLPTR);
	~SettingWidget();

private:
	Ui::SettingWidget ui;
};
