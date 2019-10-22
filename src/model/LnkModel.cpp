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

struct SearcherInfo {
    QString indexDir;
    Lucene::IndexReaderPtr reader;
    Lucene::SearcherPtr searcher;
    Lucene::AnalyzerPtr analyzer;
    Lucene::QueryParserPtr nameParser, contentParser;
};

QFileIconProvider g_iconProvider;
QList<SearcherInfo> g_searcherList;

WorkerThread::WorkerThread(QObject *parent) 
	: QThread(parent)
{
	qRegisterMetaType<QList<QSharedPointer<LnkData>>>("QList<QSharedPointer<LnkData>>");
}

WorkerThread::~WorkerThread()
{
    qDebug() << "worker thread exit";
}

void WorkerThread::go(const QString &indexDir, const QStringList &pathList)
{
    indexDir_ = indexDir;
    pathList_ = pathList;
    start();
}

void WorkerThread::run()
{
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

    Lucene::String indexDir = indexDir_.toStdWString();
    IndexWriterPtr writer = newLucene<IndexWriter>(FSDirectory::open(indexDir), newLucene<StandardAnalyzer>(LuceneVersion::LUCENE_CURRENT), true, IndexWriter::MaxFieldLengthLIMITED);

	QFileInfo info;
    QSet<QString> repeat;
    for (auto iter = pathList_.begin(); iter != pathList_.end(); ++iter) {
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

	emit resultReady(indexDir_);
}
#include <QTimer>
//////////////////////////////////////////////////////////////////////////
LnkModel::LnkModel(QObject *parent)
	: QAbstractListModel(parent)
{
	load();
    loadDir("E:\\11111111");
}

LnkModel::~LnkModel()
{
}

void LnkModel::load()
{
    WorkerThread *workerThread = new WorkerThread(this);
    connect(workerThread, &WorkerThread::resultReady, this, &LnkModel::handleResult, Qt::QueuedConnection);
    connect(workerThread, &WorkerThread::finished, workerThread, &QObject::deleteLater);
    workerThread->go(Util::getIndexDir(), Util::getAllLnk());
}

void LnkModel::loadDir(const QString &dir)
{
    WorkerThread *workerThread = new WorkerThread(this);
    connect(workerThread, &WorkerThread::resultReady, this, &LnkModel::handleResult, Qt::QueuedConnection);
    connect(workerThread, &WorkerThread::finished, workerThread, &QObject::deleteLater);
    workerThread->go(Util::getIndexDir(Util::md5(dir)), Util::getFiles(dir));
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
                int32_t totalHits = collector->getTotalHits();
                qDebug() << "index:" << queryIndex++ << ", total hits:" << totalHits << ", hit size:" << hits.size();
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
        qDebug() << "result count:" << pfilterdata_.size();
	}
	int count = pfilterdata_.size();
	emit dataChanged(this->index(0, 0), this->index(qMax(count, 0), 0));
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

void LnkModel::handleResult(const QString &indexDir)
{
    try {
        SearcherInfo info;
        info.indexDir = indexDir;
        info.reader = IndexReader::open(FSDirectory::open(indexDir.toStdWString()), true);
        info.searcher = newLucene<IndexSearcher>(info.reader);
        info.analyzer = newLucene<StandardAnalyzer>(LuceneVersion::LUCENE_CURRENT);
        info.nameParser = newLucene<QueryParser>(LuceneVersion::LUCENE_CURRENT, L"name", info.analyzer);
        info.nameParser->setAllowLeadingWildcard(true);
        info.contentParser = newLucene<QueryParser>(LuceneVersion::LUCENE_CURRENT, L"contents", info.analyzer);
        info.contentParser->setAllowLeadingWildcard(true);

        bool isExist = false;
        for (int i = 0; i < g_searcherList.size(); i++) {
            if (g_searcherList[i].indexDir == indexDir) {
                g_searcherList[i] = info;
                isExist = true;
            }
        }
        if (!isExist) {
            g_searcherList.push_back(info);
        }
    } catch (LuceneException& e) {
        qDebug() << "Exception: " << QString::fromStdWString(e.getError()) << L"\n";
    }
}
