#include "SettingModel.h"
#include "common/Util.h"
#include <QApplication>

static const QString REG_RUN = "HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run";
static const QString MAIN_SHORTCUT = "MainShortcut";
static const QString MAX_RESULT = "MaxResult";
static const QString MAIN_OPACITY = "MainOpacity";
static const QString FONT_FAMILY = "FontFamily";
SettingModel::SettingModel(QObject *parent)
	: QObject(parent)
	, settings_(Util::getConfigPath(), QSettings::IniFormat)
{
	maxResult_ = settings_.value(MAX_RESULT).toInt();
	if (maxResult_ <= 0) {
		maxResult_ = 5;
	}
}

void SettingModel::setMainShortcutText(const QString &keyText)
{
	settings_.setValue(MAIN_SHORTCUT, keyText);
}

QString SettingModel::mainShortcutText() const
{
	return settings_.value(MAIN_SHORTCUT).toString();
}

void SettingModel::setAutoStart(bool isAutoStart)
{
	QString appName = QApplication::applicationName();
	QSettings nativeSettings(REG_RUN, QSettings::NativeFormat);
	if (isAutoStart) {
		QString appPath = QApplication::applicationFilePath();
		nativeSettings.setValue(appName, appPath.replace("/", "\\"));
	} else {
		nativeSettings.remove(appName);
	}
}

bool SettingModel::autoStart() const
{
	QString appName = QApplication::applicationName();
	QSettings nativeSettings(REG_RUN, QSettings::NativeFormat);
	QString path = nativeSettings.value(appName).toString();
	return (QApplication::applicationFilePath().replace("/", "\\") == path);
}

void SettingModel::setMaxResult(int count)
{
	maxResult_ = count;
	settings_.setValue(MAX_RESULT, count);
}

int SettingModel::maxResult() const
{
	return maxResult_;
}

void SettingModel::setMainOpacity(int level)
{
	settings_.setValue(MAIN_OPACITY, level);
}

int SettingModel::mainOpacity() const
{
	int opacity = settings_.value(MAIN_OPACITY).toInt();
	return opacity <= 0 ? 1 : opacity;
}

void SettingModel::setFontFamily(const QString &font)
{
	settings_.setValue(FONT_FAMILY, font);
}

QString SettingModel::fontFamily() const
{
	QString font = settings_.value(FONT_FAMILY).toString();
	if (font.isEmpty()) {
		font = "Microsoft YaHei";
	}
	return font;
}
