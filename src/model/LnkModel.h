#pragma once

#include <QAbstractListModel>
#include <QSharedDataPointer>
#include <QFileSystemWatcher>
#include <QPixmap>
#include <QIcon>
#include <QString>
#include <functional>

class LnkData : public QSharedData
{
public:
    enum Type {
        TUnkown = 0,
        TPath = 1,
        TIndexDir = 2,
        TSearchEngine = 3,
    };

    LnkData()
        : type(TUnkown)
    {
    }

	QVariant toVariant() const
	{
		QVariantMap result;
        result["type"] = (int)type;
		result["name"] = name;
		result["path"] = path;
		result["icon"] = icon;
		return result;
	}

    Type type;
	QString name;
	QString path;
	QString pinyin;
    QString searchText;
	QIcon icon;
};

class ModelData : public QObject {
public:
    ModelData(QObject *parent) : QObject(parent){}
    virtual ~ModelData() {}
    void setBreak(const std::function<bool()> &fn) { fn_ = fn; }
    bool isBreak() { if (fn_) return fn_(); return false; }
    virtual QList<QSharedPointer<LnkData>> filter(const QString &text) = 0;
private:
    std::function<bool()> fn_;
};

class InitModelData : public ModelData {
public:
    InitModelData(QObject *parent = 0);
    QList<QSharedPointer<LnkData>> filter(const QString &text) override;

private:
    QList<QSharedPointer<LnkData>> datalist_;
};

class LnkModelData : public ModelData {
public:
    LnkModelData(QObject *parent = 0);
    QList<QSharedPointer<LnkData>> filter(const QString &text) override;
    void load(const QString &key, const QString &dir, const QString &filter);
    bool removeSearcher(const QString &name);
};

class CmdModelData : public ModelData {
public:
    CmdModelData(QObject *parent = 0);
    QList<QSharedPointer<LnkData>> filter(const QString &text) override;
    void updateIndexDir();

private:
    QList<QSharedPointer<LnkData>> datalist_;
};

class LnkModel : public QAbstractListModel
{
	Q_OBJECT

public:
	LnkModel(QObject *parent);
	~LnkModel();
    void insertModelData(const QString &name, QSharedPointer<ModelData> modelData);
    void removeModelData(const QString &name);
    QSharedPointer<ModelData> getModelData(const QString &name);

	void filter(const QString &text);
    const QString& head() const { return head_; }
    bool isBreak() const;
	int showCount() const;

protected:
	int rowCount(const QModelIndex &parent = QModelIndex()) const;
	QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

private:
    QStringList namelist_;
    QMap<QString, QSharedPointer<ModelData>> modeldata_;
	QList<QSharedPointer<LnkData>> pfilterdata_;
    QString head_;
};
