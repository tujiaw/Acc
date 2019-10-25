#include "LnkModel.h"
#include <QFileIconProvider>
#include <QDebug>
#include <QStandardPaths>
#include <QImageReader>
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
    QStringList pathList;
    QString indexName;
    if (indexDir_ == Util::getIndexDir(DEFAULT_INDEX)) {
        pathList = Util::getAllLnk();
        indexName = DEFAULT_INDEX;
    } else {
        QStringList filterSuffix = Acc::instance()->getSettingModel()->filterSuffix();
        pathList = Util::getFiles(indexDir_, true, maxLimit_, filterSuffix);
        indexName = Util::md5(indexDir_);
    }

    if (pathList.isEmpty()) {
        emit resultReady("Directory is empty", indexName);
        return;
    }

    auto addDocument = [](const QSharedPointer<LnkData> &data) -> DocumentPtr {
        DocumentPtr doc = newLucene<Document>();
        String name = data->lnkName.toStdWString();
        String path = data->targetPath.toStdWString();
        String contents = (data->targetPath + " " + data->lnkPath + " " + data->pinyin).toStdWString();
        doc->add(newLucene<Field>(L"path", path, Field::STORE_YES, Field::INDEX_NOT_ANALYZED));
        doc->add(newLucene<Field>(L"modified", DateTools::timeToString(FileUtils::fileModified(path), DateTools::RESOLUTION_MINUTE),
            Field::STORE_YES, Field::INDEX_NOT_ANALYZED));
        doc->add(newLucene<Field>(L"name", name, Field::STORE_YES, Field::INDEX_NOT_ANALYZED));
        doc->add(newLucene<Field>(L"contents", contents, Field::STORE_YES, Field::INDEX_ANALYZED));
        return doc;
    };

    Lucene::String localIndexDir = Util::getIndexDir(indexName).toStdWString();
    IndexWriterPtr writer = newLucene<IndexWriter>(FSDirectory::open(localIndexDir), newLucene<StandardAnalyzer>(LuceneVersion::LUCENE_CURRENT), true, IndexWriter::MaxFieldLengthLIMITED);

	QFileInfo info;
    QSet<QString> repeat;
    for (auto iter = pathList.begin(); iter != pathList.end(); ++iter) {
		info.setFile(*iter);
        QString target = info.filePath();
        QString linkTarget = info.symLinkTarget();
        // 排除链接目标是：C:/Windows/Installer中的文件
        if (info.isSymLink() && !linkTarget.contains("installer", Qt::CaseInsensitive)) {
            target = info.symLinkTarget();
        }
		if (target.isEmpty()) {
			continue;
		}

		QSharedPointer<LnkData> p(new LnkData());
		// 链接名
		p->lnkName = info.fileName().remove(".lnk", Qt::CaseInsensitive);
		// 链接路径
		p->lnkPath = info.filePath();
		// 目录可执行程序路径
		p->targetPath = target;
		// 目标名字
		p->targetName = QFileInfo(p->targetPath).baseName();

		if (p->targetName.contains(p->lnkName, Qt::CaseInsensitive)) {
			CFileVersionInfo verinfo;
			if (verinfo.Create(p->targetPath.toStdWString().c_str())) {
				std::wstring desc = verinfo.GetFileDescription();
				if (!desc.empty()) {
					p->lnkName = QString::fromStdWString(desc);
				}
			}
		}

		// 过滤重复
        if (repeat.find(p->key()) != repeat.end()) {
            continue;
        }

        p->pinyin = QString::fromStdString(getFullAndInitialWithSeperator(p->lnkName.trimmed().toStdString()));
        DocumentPtr doc = addDocument(p);
        if (doc) {
            writer->addDocument(doc);
            repeat.insert(p->key());
        }
	}

    writer->optimize();
    writer->close();

    emit resultReady("", indexName);
}

//////////////////////////////////////////////////////////////////////////
LnkModel::LnkModel(QObject *parent)
	: QAbstractListModel(parent)
{
	load();
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
}

LnkModel::~LnkModel()
{
}

void LnkModel::load(const QString &dir)
{
    QString realDir = dir;
    if (dir.isEmpty()) {
        realDir = Util::getIndexDir(DEFAULT_INDEX);
    }
    WorkerThread *workerThread = new WorkerThread(this);
    connect(workerThread, &WorkerThread::resultReady, this, &LnkModel::handleResult, Qt::QueuedConnection);
    connect(workerThread, &WorkerThread::finished, workerThread, &QObject::deleteLater);
    workerThread->go(realDir, Acc::instance()->getSettingModel()->getDirMaxLimit());
}

void LnkModel::filter(const QString &text)
{
	pfilterdata_.clear(); 
	if (!text.isEmpty() ) {

        QSet<QString> repeat;
        int queryIndex = 0;
        auto query = [this, &repeat, &queryIndex](Lucene::SearcherPtr searcher, Lucene::QueryParserPtr parser, const QString &text) {
            const int MAX_RESULT = 50;
            if (pfilterdata_.size() >= MAX_RESULT) {
                return;
            }

            QList<QSharedPointer<LnkData>> result;
            try {
                QueryPtr query = parser->parse(("*" + text + "*").toStdWString());
                const int hitsPerPage = 10;
                TopScoreDocCollectorPtr collector = TopScoreDocCollector::create(5 * hitsPerPage, false);
                searcher->search(query, collector);
                Collection<ScoreDocPtr> hits = collector->topDocs()->scoreDocs;
                for (int i = 0; i < hits.size(); i++) {
                    if (pfilterdata_.size() >= MAX_RESULT) {
                        break;
                    }

                    DocumentPtr doc = searcher->doc(hits[i]->doc);
                    QSharedPointer<LnkData> p(new LnkData());
                    p->lnkName = QString::fromStdWString(doc->get(L"name"));
                    p->targetPath = QString::fromStdWString(doc->get(L"path"));
                    p->icon = g_iconProvider.icon(QFileInfo(p->targetPath));
                    if (p->icon.isNull()) {
                        p->icon = g_iconProvider.icon(QFileIconProvider::File);
                    }

                    if (repeat.find(p->targetPath) == repeat.end()) {
                        pfilterdata_.append(p);
                        repeat.insert(p->targetPath);
                    }
                }
            }
            catch (LuceneException& e) {
                qDebug() << "Exception: " << QString::fromStdWString(e.getError()) << L"\n";
            }
        };

        auto search = [&query](const SearcherInfo &info, const QString &text) {
            query(info.searcher, info.nameParser, text);
            query(info.searcher, info.contentParser, text);
        };

        for (int i = 0; i < g_searcherList.size(); i++) {
            search(g_searcherList[i], text);
        }

        qSort(pfilterdata_.begin(), pfilterdata_.end(),
            [](const QSharedPointer<LnkData> &left, const QSharedPointer<LnkData> &right) -> bool {
            int leftHits = Acc::instance()->getHitsModel()->hits(T_LNK, left->lnkName, left->targetPath);
            int rightHits = Acc::instance()->getHitsModel()->hits(T_LNK, right->lnkName, right->targetPath);
            return leftHits > rightHits;
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
        if (g_searcherList[i].indexName == name) {
            return "Ok";
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
