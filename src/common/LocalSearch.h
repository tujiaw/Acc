#pragma once

#include <mutex>
#include <QString>
#include <QList>
#include <QVariantMap>
#include "lucene/LuceneHeaders.h"
#include "lucene/FileUtils.h"

class LocalSearcher {
public:
    struct Info {
        QString indexName;
        Lucene::IndexReaderPtr reader;
        Lucene::SearcherPtr searcher;
        Lucene::AnalyzerPtr analyzer;
        Lucene::QueryParserPtr parser1, parser2;
    };

    static LocalSearcher& instance();
    QString addDir(const QString &dir);
    bool addSearch(const QString &name);
    bool delSearch(const QString &name);
    void sortSearch();
    void query(const QString &text, QList<QVariantMap> &result);
    bool isExit(const QString &name);

private:
    LocalSearcher();
    std::mutex mutex_;
    QList<LocalSearcher::Info> list_;
};
