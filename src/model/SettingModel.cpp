#include "SettingModel.h"
#include "common/Util.h"
#include <QApplication>

static const QString REG_RUN = "HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run";
static const QString MAIN_SHORTCUT = "MainShortcut";
static const QString MAX_RESULT = "MaxResult";
static const QString MAIN_OPACITY = "MainOpacity";
static const QString FONT_FAMILY = "FontFamily";
static const QString FONT_BOLD = "FontBold";
static const QString OPEN_URL_ON = "OpenUrlOn";
static const QString SEARCH_ENGINE_ON = "SearchEngineOn";
static const QString SEARCH_ENGINE = "SearchEngine";
static const QString BIND_WALLPAPER_ON = "BindWallpaperOn";
static const QString BIND_WALLPAPER_INDEX = "BindWallpaperIndex";
static const QString INDEX_LIST = "IndexList";
static const QString DIR_MAX_LIMIT = "DirMaxLimit";
static const QString FILTER_SUFFIX = "FilterSuffix";
static const QString HOST = "Host";

SettingModel::SettingModel(QObject *parent)
	: QObject(parent)
	, settings_(Util::getConfigPath(), QSettings::IniFormat)
{
    QList<IndexInfo> infoList = getIndexList();
    foreach(const IndexInfo &info, infoList) {
        tableList_.append(info.key);
    }
}

void SettingModel::sync()
{
	settings_.sync();
}

void SettingModel::revertDefault()
{
	settings_.clear();
	setAutoStart(false);
	settings_.sync();
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
	settings_.setValue(MAX_RESULT, count);
}

int SettingModel::maxResult() const
{
	int count = settings_.value(MAX_RESULT).toInt();
	return count <= 0 ? 5 : count;
}

void SettingModel::setMainOpacity(int level)
{
	settings_.setValue(MAIN_OPACITY, level);
}

int SettingModel::mainOpacity() const
{
	int opacity = settings_.value(MAIN_OPACITY).toInt();
	return opacity <= 0 ? 10 : opacity;
}

void SettingModel::setFontFamily(const QString &font, bool isBold)
{
	settings_.setValue(FONT_FAMILY, font);
	settings_.setValue(FONT_BOLD, isBold);
}

QString SettingModel::fontFamily() const
{
	QString font = settings_.value(FONT_FAMILY).toString();
	if (font.isEmpty()) {
		font = "Microsoft YaHei";
	}
	return font;
}

bool SettingModel::isBold() const
{
	return settings_.value(FONT_BOLD).toBool();
}

void SettingModel::setEnableOpenUrl(bool enable)
{
	settings_.setValue(OPEN_URL_ON, enable);
}

bool SettingModel::enableOpenUrl() const
{
	if (settings_.value(OPEN_URL_ON).isValid()) {
		return settings_.value(OPEN_URL_ON).toBool();
	}
	return true;
}

void SettingModel::setSearchEngine(bool enable, const QString &text)
{
	settings_.setValue(SEARCH_ENGINE_ON, enable);
	settings_.setValue(SEARCH_ENGINE, text);
}

QPair<bool, QString> SettingModel::searchEngine() const
{
	bool enable = true;
	if (settings_.value(SEARCH_ENGINE_ON).isValid()) {
		enable = settings_.value(SEARCH_ENGINE_ON).toBool();
	}
	QString name = settings_.value(SEARCH_ENGINE).toString();
	name = name.isEmpty() ? tr("Baidu") : name;
	return qMakePair(enable, name);
}

void SettingModel::setBindWallpaper(bool enable, int index)
{
    settings_.setValue(BIND_WALLPAPER_ON, enable);
    settings_.setValue(BIND_WALLPAPER_INDEX, index);
}

QPair<bool, int> SettingModel::bindWallpaperUrl() const
{
    bool enable = settings_.value(BIND_WALLPAPER_ON, true).toBool();
    int index = settings_.value(BIND_WALLPAPER_INDEX, 0).toInt();
    return qMakePair(enable, index);
}

IndexInfo SettingModel::getIndex(const QString &key) const
{
    static IndexInfo s_emptyInfo;
    QList<IndexInfo> infoList = getIndexList();
    foreach(const IndexInfo &info, infoList) {
        if (info.key == key) {
            return info;
        }
    }
    return s_emptyInfo;
}

void SettingModel::setIndexList(const QList<IndexInfo> &infoList)
{
    QVariantList ls;
    tableList_.clear();
    foreach(const IndexInfo &info, infoList) {
        ls.append(info.toMap());
        tableList_.append(info.key);
    }
    settings_.setValue(INDEX_LIST, QVariant(ls));
}

QList<IndexInfo> SettingModel::getIndexList() const
{
    QVariantList ls = settings_.value(INDEX_LIST).toList();
    QList<IndexInfo> infoList;
    foreach(const QVariant &v, ls) {
        IndexInfo info;
        info.fromMap(v.toMap());
        infoList.append(info);
    }
    return infoList;
}

bool SettingModel::containsTable(const QString &table) const
{
    return tableList_.contains(table);
}

void SettingModel::setDirMaxLimit(int limit)
{
    settings_.setValue(DIR_MAX_LIMIT, limit);
}

int SettingModel::getDirMaxLimit() const
{
    return settings_.value(DIR_MAX_LIMIT, 10000).toInt();
}


void SettingModel::setFilterSuffix(const QString &suffix)
{
    settings_.setValue(FILTER_SUFFIX, suffix);
}

QString SettingModel::getFilterSuffix() const
{
    return settings_.value(FILTER_SUFFIX).toString();
}

QStringList SettingModel::filterSuffix() const
{
    return getFilterSuffix().split(";", QString::SkipEmptyParts);
}

QString SettingModel::host() const
{
	return settings_.value(HOST).toString();
}