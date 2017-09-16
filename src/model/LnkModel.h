#pragma once

#include <QAbstractListModel>
#include <QSharedDataPointer>
#include <QPixmap>
#include <QIcon>
class LnkData : public QSharedData
{
public:
	LnkData(){}

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
	QList<QSharedDataPointer<LnkData>> pdata_;
	QList<QSharedDataPointer<LnkData>> pfilterdata_;
};
