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

    void slotIndexAdd();
    void slotIndexRemove();
    void slotIndexUp();
    void slotIndexDown();
    void slotIndexResult(const QString &err, const QString &indexName);

private:
    QListWidgetItem* addIndexItem(const QString &name);
    void addIndexItemList(const QStringList &nameList);
    void updateIndexStatus();

private:
	Ui::SettingWidget ui;
};
