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

	void setAutoStart(bool isAutoStart);
	bool autoStart() const;

	void setMaxResult(int count);
	int maxResult() const;

private:
	QSettings settings_;
	int maxResult_;
};
