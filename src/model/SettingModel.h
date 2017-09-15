#pragma once

#include <QObject>
#include <QKeySequence>
#include <QSettings>

class SettingModel : public QObject
{
	Q_OBJECT
public:
	SettingModel(QObject *parent = 0);
	void sync();
	void revertDefault();

	void setMainShortcutText(const QString &keyText);
	QString mainShortcutText() const;

	void setAutoStart(bool isAutoStart);
	bool autoStart() const;

	void setMaxResult(int count);
	int maxResult() const;

	void setMainOpacity(int level);
	int mainOpacity() const;

	void setFontFamily(const QString &font, bool isBold);
	QString fontFamily() const;
	bool isBold() const;
	
	void setEnableOpenUrl(bool enable);
	bool enableOpenUrl() const;

	void setSearchEngine(bool enable, const QString &text);
	QPair<bool , QString> searchEngine() const;

private:
	QSettings settings_;
};
