#pragma once

#include <QWidget>
#include "ui_SettingWidget.h"

class SettingWidget : public QWidget
{
	Q_OBJECT

public:
	SettingWidget(QWidget *parent = Q_NULLPTR);
	~SettingWidget();
	void readData();
	void writeData(QObject *sender = nullptr);

private slots:
	void slotMaxResultChanged(const QString &text);
	void slotHotkeyConfirm();
	void slotAutoStartChanged(int state);
	void slotOpacityChanged();
	void slotCurrentFontChanged(int index);
	void slotBoldChanged(int state);
	void slotDefaultActivated(const QString &link);

private:
	Ui::SettingWidget ui;
};
