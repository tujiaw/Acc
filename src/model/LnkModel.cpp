#include "LnkModel.h"

#include <QFileIconProvider>
#include <QDebug>
#include <QStandardPaths>
#include <QImageReader>
#include <QProcess>
#include <thread>
#include "controller/Acc.h"
#include "common/Util.h"
#include "common/FileVersionInfo.h"
#include "common/pinyin.h"
#include "common/LocalSearch.h"

const QFileIconProvider g_iconProvider;
InitModelData::InitModelData(QObject *parent) : ModelData(parent)
{
    QSharedPointer<LnkData> baidu(new LnkData());
    baidu->icon = QIcon(":/images/baidu.png");
    baidu->type = LnkData::TSearchEngine;
    baidu->name = "baidu";
    baidu->searchText = baidu->name;
    baidu->path = QStringLiteral("百度一下你就知道");

    QSharedPointer<LnkData> bing(new LnkData());
    bing->icon = QIcon(":/images/bing.png");
    bing->type = LnkData::TSearchEngine;
    bing->name = "bing";
    bing->searchText = bing->name;
    bing->path = QStringLiteral("必应搜索");

    QSharedPointer<LnkData> google(new LnkData());
    google->icon = QIcon(":/images/google.png");
    google->type = LnkData::TSearchEngine;
    google->name = "google";
    google->searchText = google->name;
    google->path = QStringLiteral("谷歌搜索");

    datalist_.append(baidu);
    datalist_.append(bing);
    datalist_.append(google);
}

QList<QSharedPointer<LnkData>> InitModelData::filter(const QString &text)
{
    QList<QSharedPointer<LnkData>> result;
    QStringList ls = text.split(" ");
    if (!ls.isEmpty()) {
        for (int i = 0; i < datalist_.size() && !isBreak(); i++) {
            if (datalist_.at(i)->searchText.contains(ls.first())) {
                result.append(datalist_.at(i));
            }
        }
    }
    return result;
}

//////////////////////////////////////////////////////////////////////////
LnkModelData::LnkModelData(QObject *parent) : ModelData(parent)
{
    return;
    std::vector<std::vector<std::string>> bindText;
    CFileVersionInfo verinfo;
    QStringList lnkList = Util::getAllLnk();
    QSet<QString> repeat;
    foreach(const QString &lnk, lnkList) {
        QFileInfo info;
        info.setFile(lnk);
        QString target = info.filePath();
        QString linkTarget = info.symLinkTarget();
        // 排除链接目标是：C:/Windows/Installer中的文件
        if (!linkTarget.isEmpty() && info.isSymLink() && !linkTarget.contains("installer", Qt::CaseInsensitive)) {
            target = linkTarget;
            if (!QFile::exists(target)) {
                target = target.remove(" (x86)");
                if (!QFile::exists(target)) {
                    qDebug() << "not found" << linkTarget;
                    continue;
                }
            }
        }

        QSharedPointer<LnkData> p(new LnkData());
        p->type = LnkData::TPath;
        p->name = info.baseName();
        p->path = target;

        QString key = (p->name + p->path).toLower();
        if (repeat.find(key) != repeat.end()) {
            continue;
        }
        repeat.insert(key);

        QString searchText = p->name;

        QString targetName = target.mid(target.lastIndexOf("/") + 1);
        if (p->name != targetName) {
            searchText += " " + targetName;
        }

        QString descName;
        if (verinfo.Create(p->path.toStdWString().c_str())) {
            std::wstring desc = verinfo.GetFileDescription();
            if (!desc.empty()) {
                descName = QString::fromStdWString(desc);
                searchText += " " + descName;
            }
        }

        std::string pinyin = getFullAndInitialWithSeperator(searchText.toStdString());
        std::vector<std::string> row;
        row.push_back(p->name.toStdString());
        row.push_back(p->path.toStdString());
        row.push_back(pinyin + searchText.toStdString());
        bindText.push_back(row);
    }
    LocalSearcher::instance().initData("__base__", bindText);
    qDebug() << "LnkModel init finished, size:" << bindText.size();
}

QList<QSharedPointer<LnkData>> LnkModelData::filter(const QString &text)
{
    QList<QSharedPointer<LnkData>> result;
    if (text.isEmpty() || text[0] == ">") {
        return result;
    }

    QStringList ls = text.split(" ");
    if (!ls.isEmpty()) {
        QList<QVariantMap> datas;
        LocalSearcher::instance().query(ls.last(), datas);
        foreach(const QVariantMap &data, datas) {
            if (!isBreak()) {
                QSharedPointer<LnkData> p(new LnkData());
                p->name = data["name"].toString();
                p->path = data["path"].toString();
                result.append(p);
            }
        }
    }
    return result;
}

void LnkModelData::load(const QString &key, const QString &dir, const QString &filter)
{
    LocalSearcher::instance().initTable(key, dir, filter);
}

bool LnkModelData::removeSearcher(const QString &name)
{
    return LocalSearcher::instance().dropTable(name);
}

//////////////////////////////////////////////////////////////////////////

CmdModelData::CmdModelData(QObject *parent) : ModelData(parent)
{
    QSharedPointer<LnkData> baidu(new LnkData());
    baidu->icon = QIcon(":/images/baidu.png");
    baidu->type = LnkData::TSearchEngine;
    baidu->name = "baidu";
    baidu->searchText = baidu->name;
    baidu->path = QStringLiteral("百度一下你就知道");

    QSharedPointer<LnkData> bing(new LnkData());
    bing->icon = QIcon(":/images/bing.png");
    bing->type = LnkData::TSearchEngine;
    bing->name = "bing";
    bing->searchText = bing->name;
    bing->path = QStringLiteral("必应搜索");

    QSharedPointer<LnkData> google(new LnkData());
    google->icon = QIcon(":/images/google.png");
    google->type = LnkData::TSearchEngine;
    google->name = "google";
    google->searchText = google->name;
    google->path = QStringLiteral("谷歌搜索");

    datalist_.append(baidu);
    datalist_.append(bing);
    datalist_.append(google);

    updateIndexDir();
}

QList<QSharedPointer<LnkData>> CmdModelData::filter(const QString &text)
{
    QList<QSharedPointer<LnkData>> result;
    if (text.isEmpty() || text[0] != '>') {
        return result;
    }

    if (text == ">") {
        return datalist_;
    }

    QList<QVariantMap> datas;
    QStringList textList = text.mid(1).split(" ", QString::SkipEmptyParts);
    if (textList.size() > 1) {
        QString key = textList.first();
        QString searchText = textList.last();
        if (Acc::instance()->getSettingModel()->containsTable(key)) {
            LocalSearcher::instance().query(key, searchText, searchText.length(), datas);
        }
    } else if (textList.size() > 0) {
        QString searchText = textList.first();
        foreach(const QSharedPointer<LnkData> &v, datalist_) {
            if (v->searchText.contains(searchText)) {
                result.append(v);
            }
        }
        if (text.at(text.length() - 1) == ' ' && Acc::instance()->getSettingModel()->containsTable(searchText)) {
            LocalSearcher::instance().query(searchText, "", 0, datas);
        }
    }

    foreach(const QVariantMap &data, datas) {
        if (!isBreak()) {
            QSharedPointer<LnkData> p(new LnkData());
            p->name = data["name"].toString();
            p->path = data["path"].toString();
            result.append(p);
        }
    }

    return result;
}

void CmdModelData::updateIndexDir()
{
    foreach(const QSharedPointer<LnkData> &v, datalist_) {
        if (v->type == LnkData::TIndexDir) {
            datalist_.removeOne(v);
        }
    }

    QList<IndexInfo> infoList = Acc::instance()->getSettingModel()->getIndexList();
    foreach(const IndexInfo &info, infoList) {
        QSharedPointer<LnkData> data(new LnkData());
        data->type = LnkData::TIndexDir;
        data->name = info.key + QString(" (%1)").arg(info.filter);
        data->searchText = info.key;
        data->path = info.path;
        datalist_.append(data);
    }
}

//////////////////////////////////////////////////////////////////////////
LnkModel::LnkModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

LnkModel::~LnkModel()
{
}

void LnkModel::insertModelData(const QString &name, QSharedPointer<ModelData> modelData)
{
    modelData->setBreak(std::bind(&LnkModel::isBreak, this));
    modeldata_[name] = modelData;
    namelist_.removeOne(name);
    namelist_.append(name);
}

void LnkModel::removeModelData(const QString &name)
{
    modeldata_.remove(name);
    namelist_.removeOne(name);
}

QSharedPointer<ModelData> LnkModel::getModelData(const QString &name)
{
    return modeldata_[name];
}


void LnkModel::filter(const QString &text)
{
    head_.clear();
	pfilterdata_.clear(); 
    QSet<QString> repeat;
    
    if (text.isEmpty()) {
        return;
    }

    auto addItem = [&](const QSharedPointer<LnkData> &data) {
        if (isBreak()) {
            return;
        }
        QString key = (data->name + data->path).toLower();
        if (repeat.find(key) != repeat.end()) {
            return;
        }

        repeat.insert(key);
        QFileInfo info(data->path);
        if (info.exists()) {
            data->icon = g_iconProvider.icon(QFileInfo(data->path));
        }
        if (data->icon.isNull()) {
            data->icon = g_iconProvider.icon(QFileIconProvider::File);
        }

        pfilterdata_.append(data);
    };

    foreach(const QString &name, namelist_) {
        QSharedPointer<ModelData> model = modeldata_[name];
        if (model) {
            QList<QSharedPointer<LnkData>> result = model->filter(text);
            foreach(const QSharedPointer<LnkData> &data, result) {
                addItem(data);
            }
        }
    }

    qSort(pfilterdata_.begin(), pfilterdata_.end(),
        [](const QSharedPointer<LnkData> &left, const QSharedPointer<LnkData> &right) -> bool {
        if (left->type != right->type) {
            return left->type > right->type;
        }

        int leftHits = Acc::instance()->getHitsModel()->hits(T_LNK, left->name, left->path);
        int rightHits = Acc::instance()->getHitsModel()->hits(T_LNK, right->name, right->path);
        if (leftHits != rightHits) {
            return leftHits > rightHits;
        } else {
            return left->name.length() < right->name.length();
        }
    });

	int count = pfilterdata_.size();
	emit dataChanged(this->index(0, 0), this->index(qMax(count, 0), 0));
}

bool LnkModel::isBreak() const
{
    const int MAX_RESULT = 100;
    return pfilterdata_.size() >= MAX_RESULT;
}

int LnkModel::showCount() const
{
	return pfilterdata_.size();
}

int LnkModel::rowCount(const QModelIndex &parent) const
{
	return pfilterdata_.count();
}

QVariant LnkModel::data(const QModelIndex &index, int role) const
{
	int row = index.row();
	if (role == Qt::DisplayRole && row < pfilterdata_.size()) {
		return pfilterdata_[row]->toVariant();
	}
	return QVariant();
}
