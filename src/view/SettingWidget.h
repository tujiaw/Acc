#pragma once

#include <QWidget>
#include "ui_SettingWidget.h"

class SettingWidget : public QWidget
{
	Q_OBJECT

public:
	SettingWidget(QWidget *parent = Q_NULLPTR);
	~SettingWidget();
	void init();

private slots:
	void slotMaxResultChanged(const QString &text);
	void slotHotkeyConfirm();
	void slotAutoStartChanged(int state);
	void slotOpacityChanged();
	void slotCurrentFontChanged(const QFont &font);

protected:
	void keyPressEvent(QKeyEvent *e);

private:
	Ui::SettingWidget ui;
};
