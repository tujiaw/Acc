#include "HitsModel.h"
#include "common/Util.h"
#include <QTextStream>
#include <QDateTime>
#include <QDebug>
#include <QTimer>

HitsModel::HitsModel(QObject *parent)
	: QObject(parent)
{
    timer_ = new QTimer(this);
    timer_->setInterval(3000);
    timer_->setSingleShot(true);
    connect(timer_, &QTimer::timeout, this, &HitsModel::save);

	init();
}

HitsModel::~HitsModel()
{
	save();
}

void HitsModel::init()
{
    data_.clear();
	QString hitspath = Util::getConfigDir() + "/hits.json";
	QFile file(hitspath);
	if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
		QByteArray json = file.readAll();
        QVariantList ls = Util::json2list(json);
        for (const QVariant &item : ls) {
            QVariantMap vm = item.toMap();
            HitData data;
            if (!vm["title"].toString().isEmpty()) {
                data.type = (HitType)vm["type"].toInt();
                data.hits = vm["hits"].toInt();
                data.lasttime = vm["lasttime"].toString();
                data.title = vm["title"].toString();
                data.subtitle = vm["subtitle"].toString();
                data_.push_back(data);
            }
        }
	}
	else {
		qWarning() << "open read file failed, path:" << hitspath;
	}
}

void HitsModel::save()
{
	QString hitspath = Util::getConfigDir() + "/hits.json";
	QFile file(hitspath);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
		QTextStream out(&file);
        out.setCodec("utf-8");
        qSort(data_.begin(), data_.end(), [](const HitData &left, const HitData &right) -> bool {
            return left.hits > right.hits;
		});
        QVariantList ls;
        for (const auto &item : data_) {
            QVariantMap vm;
            vm["type"] = item.type;
            vm["hits"] = item.hits;
            vm["lasttime"] = item.lasttime;
            vm["title"] = item.title;
            vm["subtitle"] = item.subtitle;
            ls.push_back(vm);
        }
		out << Util::list2json(ls);
	}
	else {
		qWarning() << "open write file failed, path:" << hitspath;
	}
}

void HitsModel::delaySave()
{
    timer_->stop();
    timer_->start();
}

void HitsModel::increase(HitType type, const QString &title, const QString &subtitle)
{
    bool isUpdate = false;
	QString lastime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    for (auto &item : data_) {
        if (item.type == type && item.title == title && item.subtitle == subtitle) {
            item.hits += 1;
            item.lasttime = lastime;
            isUpdate = true;
            break;
        }
    }

    if (!isUpdate) {
        HitData item;
        item.type = type;
        item.title = title;
        item.subtitle = subtitle;
        item.hits = 1;
        item.lasttime = lastime;
        data_.push_back(item);
    }
    
    delaySave();
}

int HitsModel::hits(HitType type, const QString &title, const QString &subtitle)
{
    for (auto &item : data_) {
        if (item.type == type && item.title == title && item.subtitle == subtitle) {
            return item.hits;
        }
    }
    return 0;
}

