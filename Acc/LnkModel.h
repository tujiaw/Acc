#pragma once

#include <QAbstractListModel>
#include <QSharedDataPointer>
#include <QPixmap>

class LnkData : public QSharedData
{
public:
	LnkData(){}
	LnkData(const LnkData &other)
		: QSharedData(other)
		, name(other.name)
		, basename(other.basename)
		, path(other.path) {}

	QVariant toVariant() const
	{
		QVariantMap result;
		result["name"] = name;
		result["basename"] = basename;
		result["path"] = path;
		result["pixmap"] = pixmap;
		return result;
	}

	QString name;
	QString basename;
	QString path;
	QPixmap pixmap;
	QString pinyin;
	QString jianpin;
};

class LnkModel : public QAbstractListModel
{
	Q_OBJECT

public:
	LnkModel(QObject *parent);
	~LnkModel();
	QStringList getAllLnk() const;
	void filter(const QString &text);
	int totalCount() const;
	int showCount() const;

protected:
	int rowCount(const QModelIndex &parent = QModelIndex()) const;
	QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

private:
	QList<QSharedDataPointer<LnkData>> pdata_;
	QList<QSharedDataPointer<LnkData>> pfilterdata_;
};
