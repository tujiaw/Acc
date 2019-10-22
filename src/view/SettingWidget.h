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
	void slotOpacityChanged();
	void slotCurrentFontChanged(int index);
	void slotDefaultActivated(const QString &link);
    void slotMyBlog(const QString &link);
	void slotSearchEngineActivated(const QString &text);
    void slotWallpaperIndex(int index);

private:
	Ui::SettingWidget ui;
};
