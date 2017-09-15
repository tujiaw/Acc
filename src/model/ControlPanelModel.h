#pragma once

#include <QObject>
#include <QIcon>

class ControlPanelModel : public QObject
{
	Q_OBJECT
public:
	struct Item
	{
		QString title;
		QString subtitle;
		QString path;
		QIcon icon;
	};

	ControlPanelModel(QObject *parent = 0);
	void addItem(const ControlPanelModel::Item &item);
	const QList<ControlPanelModel::Item>& items() const;

private:
	QList<ControlPanelModel::Item> data_;
};
