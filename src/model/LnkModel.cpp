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
    initSearchEngine();
    std::thread t(std::bind(&LnkModel::initLnk, this));
    t.detach();

    QStringList successList;
    auto indexList = Acc::instance()->getSettingModel()->getIndexList();
    for (auto iter = indexList.begin(); iter != indexList.end(); ++iter) {
        QString name = Util::md5(*iter);
        if (QFile::exists(Util::getIndexDir() + "/" + name)) {
            if (LocalSearcher::instance().addSearch(name)) {
                successList.push_back(*iter);
            }
        }
    }
    if (successList != indexList) {
        Acc::instance()->getSettingModel()->setIndexList(successList);
    }

    watcher_.addPaths(Util::getAllLnkDir());
    connect(&watcher_, &QFileSystemWatcher::directoryChanged, this, &LnkModel::onDirChanged);
    connect(this, &LnkModel::sigLoaded, this, &LnkModel::handleResult, Qt::QueuedConnection);
}

LnkModel::~LnkModel()
{
}

void LnkModel::initLnk()
{
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

        p->pinyin = QString::fromStdString(getFullAndInitialWithSeperator(searchText.toStdString()));
        p->searchText += searchText + " " + p->pinyin;
        pinitdata_.append(p);
    }
    qDebug() << "LnkModel init finished, size:" << pinitdata_.size();
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
    std::thread t([dir, this]() {
        QString name = LocalSearcher::instance().addDir(dir);
        emit this->sigLoaded("", name);
    });
    t.detach();
}

void LnkModel::filter(const QString &text)
{
    head_.clear();
	pfilterdata_.clear(); 
    QSet<QString> repeat;
    
    auto isBreak = [&]() -> bool {
        const int MAX_RESULT = 50;
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

	if (!text.isEmpty() ) {
        QString baseText, extendText;
        int spacePos = text.lastIndexOf(' ');
        if (spacePos >= 0) {
            extendText = text.mid(spacePos + 1).trimmed();
            baseText = text.mid(0, spacePos).trimmed();
        } else {
            baseText = text.trimmed();
        }
        
        if (!baseText.isEmpty()) {
            foreach(const QSharedPointer<LnkData> &data, pinitdata_) {
                if (!extendText.isEmpty() && data->type != LnkData::TSearchEngine) {
                    continue;
                }

                if (data->searchText.contains(baseText, Qt::CaseInsensitive)) {
                    addItem(data);
                }
            }
        }
        if (!extendText.isEmpty()) {
            QList<QVariantMap> extendResult;
            LocalSearcher::instance().query(extendText, extendResult);
            foreach(const QVariantMap &data, extendResult) {
                QSharedPointer<LnkData> p(new LnkData());
                p->name = data["name"].toString();
                p->path = data["path"].toString();
                addItem(p);
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
                return left->path < right->path;
            }
        });
    }
	int count = pfilterdata_.size();
	emit dataChanged(this->index(0, 0), this->index(qMax(count, 0), 0));
}

int LnkModel::showCount() const
{
	return pfilterdata_.size();
}

bool LnkModel::removeSearcher(const QString &name)
{
    LocalSearcher::instance().delSearch(name);
    Util::removeDir(Util::getIndexDir(name));
    return true;
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

void LnkModel::handleResult(const QString &err, const QString &indexName)
{
    if (err.isEmpty()) {
        LocalSearcher::instance().addSearch(indexName);
    } else {
        qDebug() << "ERROR:" << err;
    }
    emit Acc::instance()->sigIndexResult(err, indexName);
}

void LnkModel::onDirChanged(const QString &path)
{
    qDebug() << "dir changed:" << path;
}
