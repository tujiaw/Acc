#pragma once

#include <QAbstractListModel>
#include <QSharedDataPointer>
#include <QPixmap>
#include <QIcon>
#include <QThread>

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
	QString jianpin;
	QIcon icon;
};

class WorkerThread : public QThread
{
	Q_OBJECT
public:
	WorkerThread(QObject *parent = 0);

signals :
	void resultReady(const QList<QSharedDataPointer<LnkData>> &data);

protected:
	void run();
};

class LnkModel : public QAbstractListModel
{
	Q_OBJECT

public:
	LnkModel(QObject *parent);
	~LnkModel();
	void load();
	void filter(const QString &text);
	int totalCount() const;
	int showCount() const;
	bool isExist(const QString &key);

protected:
	int rowCount(const QModelIndex &parent = QModelIndex()) const;
	QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

private:
	void handleResult(const QList<QSharedDataPointer<LnkData>> &data);

private:
	QList<QSharedDataPointer<LnkData>> pdata_;
	QList<QSharedDataPointer<LnkData>> pfilterdata_;
};
