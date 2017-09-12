#pragma once

#include <QObject>
#include <QKeySequence>
#include <QSettings>

class SettingModel : public QObject
{
	Q_OBJECT
public:
	SettingModel(QObject *parent = 0);
	void setMainShortcutText(const QString &keyText);
	QString mainShortcutText() const;

private:
	QSettings settings_;
};
