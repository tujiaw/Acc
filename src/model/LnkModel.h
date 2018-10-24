#pragma once

#include <QAbstractListModel>
#include <QSharedDataPointer>
#include <QPixmap>
#include <QIcon>
#include <QThread>
#include <QString>

class LnkData : public QSharedData
{
public:
	QVariant toVariant() const
	{
		QVariantMap result;
		result["lnkName"] = lnkName;
		result["targetPath"] = targetPath;
		result["icon"] = icon;
		return result;
	}

	QString key() const
	{
		return lnkName + "-" + targetPath;
	}

	QString lnkName;
	QString lnkPath;
	QString targetName;
	QString targetPath;
	QString pinyin;
	QIcon icon;
};

class WorkerThread : public QThread
{
	Q_OBJECT
public:
	WorkerThread(QObject *parent = 0);
    ~WorkerThread();
    void go(const QStringList &pathList);

signals:
	void resultReady(const QList<QSharedPointer<LnkData>> &data);

protected:
	void run();

private:
    QStringList pathList_;
};

class LnkModel : public QAbstractListModel
{
	Q_OBJECT

public:
	LnkModel(QObject *parent);
	~LnkModel();
	void load();
    void asyncAddNotExist(const QString &path);
    void asyncAddNotExist(const QStringList &pathList);
    void asyncAdd(const QStringList &pathList);
	void filter(const QString &text);
	int totalCount() const;
	int showCount() const;
	bool isExist(const QString &key);

protected:
	int rowCount(const QModelIndex &parent = QModelIndex()) const;
	QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

private:
	void handleResult(const QList<QSharedPointer<LnkData>> &data);

private:
	QList<QSharedPointer<LnkData>> pdata_;
	QList<QSharedPointer<LnkData>> pfilterdata_;
};
