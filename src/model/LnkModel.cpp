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

QFileIconProvider g_iconProvider;
//////////////////////////////////////////////////////////////////////////
LnkModel::LnkModel(QObject *parent)
    : QAbstractListModel(parent), watcher_(this)
{
    LocalSearcher::instance().open();
    initSearchEngine();
    //std::thread t(std::bind(&LnkModel::initLnk, this));
    //t.detach();
    initLnk();
    connect(&watcher_, &QFileSystemWatcher::directoryChanged, this, &LnkModel::onDirChanged);
}

LnkModel::~LnkModel()
{
}

void LnkModel::initLnk()
{
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

void LnkModel::initSearchEngine()
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

    pinitdata_.append(baidu);
    pinitdata_.append(bing);
    pinitdata_.append(google);
}

void LnkModel::load(const QString &dir)
{
    LocalSearcher::instance().initTable(Util::md5(dir), dir);
}

void LnkModel::filter(const QString &text)
{
    head_.clear();
	pfilterdata_.clear(); 
    QSet<QString> repeat;
    
    auto isBreak = [&]() -> bool {
        const int MAX_RESULT = 100;
        return pfilterdata_.size() >= MAX_RESULT;
    };
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

    QStringList searchList = text.split(" ", QString::SkipEmptyParts);
    if (searchList.isEmpty()) {
        return;
    }

    for (int i = 0; i < pinitdata_.size(); i++) {
        if (pinitdata_.at(i)->searchText.contains(searchList.first())) {
            addItem(pinitdata_.at(i));
        }
    }

    QList<QVariantMap> datas;
    LocalSearcher::instance().query(searchList.last(), datas);
    foreach(const QVariantMap &data, datas) {
        QSharedPointer<LnkData> p(new LnkData());
        p->name = data["name"].toString();
        p->path = data["path"].toString();
        addItem(p);
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
            return left->path < right->path;
        }
    });

	int count = pfilterdata_.size();
	emit dataChanged(this->index(0, 0), this->index(qMax(count, 0), 0));
}

int LnkModel::showCount() const
{
	return pfilterdata_.size();
}

bool LnkModel::removeSearcher(const QString &name)
{
    return LocalSearcher::instance().dropTable(name);
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

void LnkModel::onDirChanged(const QString &path)
{
    qDebug() << "dir changed:" << path;
}
