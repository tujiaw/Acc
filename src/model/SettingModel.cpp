#include "SettingModel.h"
#include "common/Util.h"

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

