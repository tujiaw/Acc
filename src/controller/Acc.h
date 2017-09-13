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

	LnkModel* getLnkModel();
	SettingModel* getSettingModel();
	HitsModel* getHitsModel();

signals:
	void sigSetMainShortcut(const QString &textKey);

private:
	Acc();
	~Acc();

private:
	QMap<QString, QWidget*> widgets_;
	LnkModel *lnkModel_;
	SettingModel *settingModel_;
	HitsModel *hitsModel_;
};
