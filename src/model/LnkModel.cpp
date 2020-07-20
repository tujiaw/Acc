#include "LnkModel.h"

#include <QFileIconProvider>
#include <QDebug>
#include <QStandardPaths>
#include <QImageReader>
#include <QProcess>

#include "controller/Acc.h"
#include "common/Util.h"
#include "common/FileVersionInfo.h"
#include "common/pinyin.h"

#include "lucene/LuceneHeaders.h"
#include "lucene/FileUtils.h"
using namespace Lucene;

const QString DEFAULT_INDEX = "Default";
struct SearcherInfo {
    QString indexName;
    Lucene::IndexReaderPtr reader;
    Lucene::SearcherPtr searcher;
    Lucene::AnalyzerPtr analyzer;
    Lucene::QueryParserPtr nameParser, contentParser;
};

QFileIconProvider g_iconProvider;
QList<SearcherInfo> g_searcherList;

QStringList fetchFile(const QString &dir)
{
    QStringList result;
    QProcess pro;
    QString fetchfile = Util::getRunDir() + "/fetchfile.exe";
    pro.start(fetchfile, QStringList() << dir);
    if (pro.waitForStarted()) {
        while (pro.waitForReadyRead()) {
            QByteArray b = pro.readAllStandardOutput();
            b = b.replace("\\", "/");
            QStringList strList = QString::fromUtf8(b).split('\n');
            result.append(strList);
        }
    }
    return result;
}

WorkerThread::WorkerThread(QObject *parent) 
    : QThread(parent), maxLimit_(-1)
{
	qRegisterMetaType<QList<QSharedPointer<LnkData>>>("QList<QSharedPointer<LnkData>>");
}

WorkerThread::~WorkerThread()
{
    qDebug() << "worker thread exit";
}

void WorkerThread::go(const QString &indexDir, int maxLimit)
{
    indexDir_ = indexDir;
    maxLimit_ = maxLimit;
    start();
}

void WorkerThread::run()
{
    QStringList pathList = fetchFile(indexDir_);
    QString indexName = Util::md5(indexDir_);
    QStringList filterSuffix = Acc::instance()->getSettingModel()->filterSuffix();
    if (pathList.isEmpty()) {
        emit resultReady("Directory is empty", indexName);
        return;
    }

    auto addStrDocument = [](QString &str) -> DocumentPtr {
        int start = str.lastIndexOf("/");
        if (start >= 0) {
            DocumentPtr doc = newLucene<Document>();
            String name = str.mid(start + 1).toStdWString();
            String path = str.toStdWString();
            String contents = str.replace("/", " ").toStdWString();
            doc->add(newLucene<Field>(L"name", name, Field::STORE_YES, Field::INDEX_NOT_ANALYZED));
            doc->add(newLucene<Field>(L"path", path, Field::STORE_YES, Field::INDEX_NOT_ANALYZED));
            doc->add(newLucene<Field>(L"contents", contents, Field::STORE_YES, Field::INDEX_ANALYZED));
            return doc;
        }
        return nullptr;
    };

    Lucene::String localIndexDir = Util::getIndexDir(indexName).toStdWString();
    IndexWriter::setDefaultWriteLockTimeout(3000);
    IndexWriterPtr writer = newLucene<IndexWriter>(FSDirectory::open(localIndexDir), newLucene<StandardAnalyzer>(LuceneVersion::LUCENE_CURRENT), true, IndexWriter::MaxFieldLengthLIMITED);
    for (auto iter = pathList.begin(); iter != pathList.end(); ++iter) {
        DocumentPtr doc = addStrDocument(*iter);
        if (doc) {
            writer->addDocument(doc);
        }
	}

    writer->optimize();
    writer->close();

    emit resultReady("", indexName);
}

//////////////////////////////////////////////////////////////////////////
LnkModel::LnkModel(QObject *parent)
    : QAbstractListModel(parent), watcher_(this)
{
    init();

    QStringList successList;
    auto indexList = Acc::instance()->getSettingModel()->getIndexList();
    for (auto iter = indexList.begin(); iter != indexList.end(); ++iter) {
        if (addSearcher(Util::md5(*iter))) {
            successList.push_back(*iter);
        }
    }
    if (successList != indexList) {
        Acc::instance()->getSettingModel()->setIndexList(successList);
    }


    watcher_.addPaths(Util::getAllLnkDir());
    connect(&watcher_, &QFileSystemWatcher::directoryChanged, this, &LnkModel::onDirChanged);
}

LnkModel::~LnkModel()
{
}

void LnkModel::init()
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

void LnkModel::load(const QString &dir)
{
    WorkerThread *workerThread = new WorkerThread(this);
    connect(workerThread, &WorkerThread::resultReady, this, &LnkModel::handleResult, Qt::QueuedConnection);
    connect(workerThread, &WorkerThread::finished, workerThread, &QObject::deleteLater);
    workerThread->go(dir, Acc::instance()->getSettingModel()->getDirMaxLimit());
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
    auto addItem = [&](const QString &name, const QString &path) {
        if (isBreak()) {
            return;
        }
        QSharedPointer<LnkData> p(new LnkData());
        p->name = name;
        p->path = path;
        QString key = (p->name + p->path).toLower();
        if (repeat.find(key) != repeat.end()) {
            return;
        }

        repeat.insert(key);
        QFileInfo info(p->path);
        if (info.exists()) {
            p->icon = g_iconProvider.icon(QFileInfo(p->path));
        }
        if (p->icon.isNull()) {
            p->icon = g_iconProvider.icon(QFileIconProvider::File);
        }

        pfilterdata_.append(p);
    };

    auto query = [&](Lucene::SearcherPtr searcher, Lucene::QueryParserPtr parser, const QString &text) {
        QList<QSharedPointer<LnkData>> result;
        try {
            QueryPtr query = parser->parse(text.toStdWString());
            const int hitsPerPage = 10;
            TopScoreDocCollectorPtr collector = TopScoreDocCollector::create(5 * hitsPerPage, false);
            searcher->search(query, collector);
            Collection<ScoreDocPtr> hits = collector->topDocs()->scoreDocs;
            for (int i = 0; i < hits.size(); i++) {
                DocumentPtr doc = searcher->doc(hits[i]->doc);
                QString name = QString::fromStdWString(doc->get(L"name"));
                QString path = QString::fromStdWString(doc->get(L"path"));
                addItem(name, path);
            }
        }
        catch (LuceneException& e) {
            qDebug() << "Exception: " << QString::fromStdWString(e.getError()) << L"\n";
        }
    };

    auto search = [&query](const SearcherInfo &info, const QString &text) {
        query(info.searcher, info.nameParser, text + "*");
        query(info.searcher, info.contentParser, text + "*");
    };

	if (!text.isEmpty() ) {
        foreach(const QSharedPointer<LnkData> &data, pinitdata_) {
            if (data->searchText.contains(text, Qt::CaseInsensitive)) {
                addItem(data->name, data->path);
            }
        }

        if (text.length() > 1) {
            for (int i = 0; i < g_searcherList.size(); i++) {
                search(g_searcherList[i], text);
                if (isBreak()) {
                    break;
                }
            }
        }

        qSort(pfilterdata_.begin(), pfilterdata_.end(),
            [](const QSharedPointer<LnkData> &left, const QSharedPointer<LnkData> &right) -> bool {
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
    for (int i = 0; i < g_searcherList.size(); i++) {
        if (g_searcherList[i].indexName == name) {
            if (!Util::removeDir(Util::getIndexDir(name))) {
                return false;
            }
            g_searcherList.removeAt(i);
        }
    }
    return true;
}

bool LnkModel::addSearcher(const QString &name)
{
    try {
        SearcherInfo info;
        info.indexName = name;
        info.reader = IndexReader::open(FSDirectory::open(Util::getIndexDir(name).toStdWString()), true);
        info.searcher = newLucene<IndexSearcher>(info.reader);
        info.analyzer = newLucene<StandardAnalyzer>(LuceneVersion::LUCENE_CURRENT);
        info.nameParser = newLucene<QueryParser>(LuceneVersion::LUCENE_CURRENT, L"name", info.analyzer);
        info.nameParser->setAllowLeadingWildcard(true);
        info.contentParser = newLucene<QueryParser>(LuceneVersion::LUCENE_CURRENT, L"contents", info.analyzer);
        info.contentParser->setAllowLeadingWildcard(true);

        bool isExist = false;
        for (int i = 0; i < g_searcherList.size(); i++) {
            if (g_searcherList[i].indexName == name) {
                g_searcherList[i] = info;
                isExist = true;
            }
        }
        if (!isExist) {
            g_searcherList.push_back(info);
        }

        sortSearcher();
        return true;
    }
    catch (LuceneException& e) {
        qDebug() << "Exception: " << QString::fromStdWString(e.getError()) << L"\n";
    }
    return false;
}

void LnkModel::sortSearcher()
{
    QStringList indexList = Acc::instance()->getSettingModel()->getIndexList();
    QMap<QString, int> indexMap;
    for (int i = 0; i < indexList.size(); i++) {
        indexMap[Util::md5(indexList[i])] = i + 1;
    }
    qSort(g_searcherList.begin(), g_searcherList.end(), [&indexMap](const SearcherInfo &left, const SearcherInfo &right) -> bool {
        return indexMap[left.indexName] < indexMap[right.indexName];
    });
}

QString LnkModel::getSearcherStatus(const QString &name) const
{
    for (int i = 0; i < g_searcherList.size(); i++) {
        SearcherInfo &searcher = g_searcherList[i];
        if (searcher.indexName == name) {
            if (searcher.reader->isOptimized()) {
                return "Ok";
            }
        }
    }
    return "Wait";
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
        addSearcher(indexName);
    } else {
        qDebug() << "ERROR:" << err;
    }
    emit Acc::instance()->sigIndexResult(err, indexName);
}

void LnkModel::onDirChanged(const QString &path)
{
    qDebug() << "dir changed:" << path;
}
