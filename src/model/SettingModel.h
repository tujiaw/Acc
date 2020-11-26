#pragma once

#include <QObject>
#include <QKeySequence>
#include <QSettings>

struct IndexInfo {
    IndexInfo() = default;
    QString key;
    QString filter;
    QString path;
    QVariantMap toMap() const {
        QVariantMap v;
        v["key"] = key;
        v["filter"] = filter;
        v["path"] = path;
        return v;
    }
    void fromMap(const QVariantMap &v) {
        key = v["key"].toString();
        filter = v["filter"].toString();
        path = v["path"].toString();
    }
    bool isEmpty() const {
        return key.isEmpty() || path.isEmpty() || filter.isEmpty();
    }
};

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

    void setBindWallpaper(bool enable, int index);
    QPair<bool, int> bindWallpaperUrl() const;

    IndexInfo getIndex(const QString &key) const;
    void setIndexList(const QList<IndexInfo> &infoList);
    QList<IndexInfo> getIndexList() const;
    bool containsTable(const QString &table) const;

    void setDirMaxLimit(int limit);
    int getDirMaxLimit() const;

    void setFilterSuffix(const QString &suffix);
    QString getFilterSuffix() const;
    QStringList filterSuffix() const;

	QString host() const;

private:
	QSettings settings_;
    QStringList tableList_;
};
