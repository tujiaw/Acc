#pragma once

#include <QWidget>
#include "ui_SettingWidget.h"
#include "model/SettingModel.h"

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
    void slotIndexTableSelectChanged();

    void slotIndexOpen();
    void slotIndexSave();
    void slotIndexRemove();
    void slotIndexUp();
    void slotIndexDown();
    void slotIndexResult(const QString &err, const QString &indexName);

private:
    void addIndexItem(const IndexInfo &info);
    void updateIndexModel();
    int getSelectRow() const;
    void clearIndexTableAllRow();
    QList<IndexInfo> getCurrentIndexList() const;

private:
	Ui::SettingWidget ui;
};
