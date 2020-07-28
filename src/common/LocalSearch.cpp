#include "LocalSearch.h"
#include "Util.h"
#include "lucene/ChineseAnalyzer.h"
#include <QDebug>
using namespace Lucene;

LocalSearcher::LocalSearcher()
{

}

LocalSearcher& LocalSearcher::instance()
{
    static LocalSearcher s_inst;
    return s_inst;
}

QString LocalSearcher::addDir(const QString &dir)
{
    std::vector<std::wstring> files;
    Util::getFiles(dir.toStdWString(), files);
    QString name = Util::md5(dir);

    auto addDocument = [](std::wstring &path) -> DocumentPtr {
        int start = path.find_last_of(L'/');
        if (start >= 0) {
            DocumentPtr doc = newLucene<Document>();
            String name = path.substr(start + 1);
            doc->add(newLucene<Field>(L"name", name, Field::STORE_YES, Field::INDEX_NOT_ANALYZED));
            doc->add(newLucene<Field>(L"path", path, Field::STORE_YES, Field::INDEX_NO));

            String dir = path.substr(0, start);
            start = dir.find_last_of(L'/');
            if (start >= 0) {
                dir = dir.substr(start + 1);
                doc->add(newLucene<Field>(L"dir", dir, Field::STORE_YES, Field::INDEX_NOT_ANALYZED));
            }
            return doc;
        }
        return nullptr;
    };

    Lucene::String localIndexDir = Util::getIndexDir(name).toStdWString();
    IndexWriter::setDefaultWriteLockTimeout(3000);
    IndexWriterPtr writer = newLucene<IndexWriter>(FSDirectory::open(localIndexDir), newLucene<StandardAnalyzer>(LuceneVersion::LUCENE_CURRENT), true, IndexWriter::MaxFieldLengthLIMITED);
    for (auto iter = files.begin(); iter != files.end(); ++iter) {
        DocumentPtr doc = addDocument(*iter);
        if (doc) {
            writer->addDocument(doc);
        }
    }

    writer->optimize();
    writer->close();
    return name;
}

bool LocalSearcher::addSearch(const QString &name)
{
    try {
        LocalSearcher::Info info;
        info.indexName = name;
        info.reader = IndexReader::open(FSDirectory::open(Util::getIndexDir(name).toStdWString()), true);
        info.searcher = newLucene<IndexSearcher>(info.reader);
        info.analyzer = newLucene<StandardAnalyzer>(LuceneVersion::LUCENE_CURRENT);
        info.parser1 = newLucene<QueryParser>(LuceneVersion::LUCENE_CURRENT, L"name", info.analyzer);
        info.parser1->setAllowLeadingWildcard(true);
        info.parser2 = newLucene<QueryParser>(LuceneVersion::LUCENE_CURRENT, L"dir", info.analyzer);
        info.parser2->setAllowLeadingWildcard(true);

        bool isExist = false;
        std::lock_guard<std::mutex> lock(mutex_);
        for (int i = 0; i < list_.size(); i++) {
            if (list_[i].indexName == info.indexName) {
                list_[i] = info;
                isExist = true;
                break;
            }
        }
        if (!isExist) {
            list_.push_back(info);
        }

        sortSearch();
        return true;
    }
    catch (...) {
        return false;
    }
}

bool LocalSearcher::delSearch(const QString &name)
{
    bool result = false;
    std::lock_guard<std::mutex> lock(mutex_);
    for (int i = 0; i < list_.size(); i++) {
        if (list_[i].indexName == name) {
            list_.removeAt(i);
            result = true;
            break;
        }
    }
    sortSearch();
    return result;
}

void LocalSearcher::sortSearch()
{
    //QStringList indexList = Acc::instance()->getSettingModel()->getIndexList();
    //QMap<QString, int> indexMap;
    //for (int i = 0; i < indexList.size(); i++) {
    //    indexMap[Util::md5(indexList[i])] = i + 1;
    //}
    //qSort(list_.begin(), list_.end(), [&indexMap](const LocalSearcher::Info &left, const LocalSearcher::Info &right) -> bool {
    //    return indexMap[left.indexName] < indexMap[right.indexName];
    //});
}

void LocalSearcher::query(const QString &text, QList<QVariantMap> &result)
{
    auto query = [&](Lucene::SearcherPtr searcher, Lucene::QueryParserPtr parser, const QString &text) {
        try {
            QueryPtr query = parser->parse(text.toStdWString());
            const int hitsPerPage = 10;
            TopScoreDocCollectorPtr collector = TopScoreDocCollector::create(5 * hitsPerPage, false);
            searcher->search(query, collector);
            Collection<ScoreDocPtr> hits = collector->topDocs()->scoreDocs;
            for (int i = 0; i < hits.size(); i++) {
                DocumentPtr doc = searcher->doc(hits[i]->doc);
                QVariantMap vm;
                vm["name"] = QString::fromStdWString(doc->get(L"name"));
                vm["path"] = QString::fromStdWString(doc->get(L"path"));
                result.push_back(vm);
            }
        }
        catch (LuceneException& e) {
            qDebug() << "Exception: " << QString::fromStdWString(e.getError()) << L"\n";
        }
    };

    std::lock_guard<std::mutex> lock(mutex_);
    for (int i = 0; i < list_.size(); i++) {
        query(list_[i].searcher, list_[i].parser1, text + "*");
        query(list_[i].searcher, list_[i].parser2, text + "*");
    }
}

bool LocalSearcher::isExit(const QString &name)
{
    std::lock_guard<std::mutex> lock(mutex_);
    for (int i = 0; i < list_.size(); i++) {
        if (list_[i].indexName == name) {
            return list_[i].reader->isOptimized();
        }
    }
    return false;
}
