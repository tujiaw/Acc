#pragma once

#include <QObject>
#include <QMap>
#include "common/Constants.h"
#include "model/LnkModel.h"
#include "model/SettingModel.h"
#include "model/HitsModel.h"

class Acc : public QObject
{
	Q_OBJECT

public:
	static Acc* instance();
	void destory();
	void openWidget(const QString &id);
	void closeWidget(const QString &id);
	void hideWidget(const QString &id);
    void minimizeWidget(const QString &id);
    void restoreWidget(const QString &id);
	QWidget* getContentWidget(const QString &id);
	void setWindowOpacity(const QString &id, int level);
    void setBindWallpaper(int index);

	LnkModel* getLnkModel();
	SettingModel* getSettingModel();
	HitsModel* getHitsModel();

signals:
	void sigSetMainShortcut(const QString &textKey);
	void sigClearResult();

private:
	Acc();
	~Acc();

private:
	QMap<QString, QWidget*> widgets_;
	LnkModel *lnkModel_;
	SettingModel *settingModel_;
	HitsModel *hitsModel_;
};
