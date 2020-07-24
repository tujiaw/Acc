#pragma once

#include <QAbstractListModel>
#include <QSharedDataPointer>
#include <QFileSystemWatcher>
#include <QPixmap>
#include <QIcon>
#include <QThread>
#include <QString>

class LnkData : public QSharedData
{
public:
    enum Type {
        TUnkown = 0,
        TPath = 1,
        TSearchEngine = 2,
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

class WorkerThread : public QThread
{
	Q_OBJECT
public:
	WorkerThread(QObject *parent = 0);
    ~WorkerThread();
    void go(const QString &indexDir, int maxLimit);

signals:
	void resultReady(const QString &err, const QString &indexDir);

protected:
	void run();

private:
    QString indexDir_;
    int maxLimit_;
};

class LnkModel : public QAbstractListModel
{
	Q_OBJECT

public:
	LnkModel(QObject *parent);
	~LnkModel();

    void initLnk();
    void initSearchEngine();
	void load(const QString &dir = "");
	void filter(const QString &text);
    const QString& head() const { return head_; }
	int showCount() const;
    bool removeSearcher(const QString &name);

protected:
	int rowCount(const QModelIndex &parent = QModelIndex()) const;
	QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

private:
	void handleResult(const QString &err, const QString &indexName);
    void onDirChanged(const QString &path);

private:
    QList<QSharedPointer<LnkData>> pinitdata_;
	QList<QSharedPointer<LnkData>> pfilterdata_;
    QString head_;
    QFileSystemWatcher watcher_;
};
