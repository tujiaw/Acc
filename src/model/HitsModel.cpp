#include "HitsModel.h"
#include "common/Util.h"
#include <QTextStream>
#include <QDateTime>
#include <QDebug>

HitsModel::HitsModel(QObject *parent)
	: QObject(parent)
{
	load();
}

HitsModel::~HitsModel()
{
	unload();
}

void HitsModel::load()
{
	QString hitspath = Util::getConfigDir() + "/hits.json";
	QFile file(hitspath);
	if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
		QByteArray json = file.readAll();
		data_ = Util::json2list(json);
	}
	else {
		qWarning() << "open read file failed, path:" << hitspath;
	}
}

void HitsModel::unload()
{
	QString hitspath = Util::getConfigDir() + "/hits.json";
	QFile file(hitspath);
	if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
		QTextStream out(&file);
		qSort(data_.begin(), data_.end(), [](const QVariant &left, const QVariant &right) -> bool { 
			return left.toMap()["hits"].toInt() > right.toMap()["hits"].toInt();
		});
		out << Util::list2json(data_);
	}
	else {
		qWarning() << "open write file failed, path:" << hitspath;
	}
}

void HitsModel::increase(const QString &title, const QString &subtitle)
{
	QString lastime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
	for (int i = 0; i < data_.size(); i++) {
		QVariantMap vm = data_[i].toMap();
		if (vm["title"] == title && vm["subtitle"] == subtitle) {
			int hits = vm["hits"].toInt() + 1;
			vm["hits"] = hits;
			vm["lasttime"] = lastime;
			data_[i] = vm;
			return;
		}
	}

	QVariantMap newVm;
	newVm["title"] = title;
	newVm["subtitle"] = subtitle;
	newVm["hits"] = 1;
	newVm["lasttime"] = lastime;
	data_.push_back(newVm);
}

int HitsModel::hits(const QString &title, const QString &subtitle)
{
	for (int i = 0; i < data_.size(); i++) {
		QVariantMap vm = data_[i].toMap();
		if (vm["title"] == title && vm["subtitle"] == subtitle) {
			return vm["hits"].toInt();
		}
	}
	return 0;
}