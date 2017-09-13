#pragma once

#include <QObject>
#include <QVariantList>

class HitsModel : public QObject
{
	Q_OBJECT
public:
	HitsModel(QObject *parent = 0);
	~HitsModel();

	void load();
	void unload();
	void increase(const QString &title, const QString &subtitle);
	int hits(const QString &title, const QString &subtitle);

private:
	QVariantList data_;
};