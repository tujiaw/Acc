#include "SettingModel.h"
#include "common/Util.h"
#include <QApplication>

static const QString REG_RUN = "HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run";
static const QString MAIN_SHORTCUT = "MainShortcut";
SettingModel::SettingModel(QObject *parent)
	: QObject(parent)
	, settings_(Util::getConfigPath(), QSettings::IniFormat)
{
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
