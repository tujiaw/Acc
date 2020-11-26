#pragma once

#include <mutex>
#include <QString>
#include <QList>
#include <QVariantMap>

class sqlite3;
class Sqlite {
public:
    Sqlite();
    ~Sqlite();
    bool open(const std::string &path);
    void close();
    std::string errmsg();
    void query(const std::string &sql, const std::function<void(const std::string &name, const std::string &value)> &fn);
    void query2(const std::string &sql, const std::function<void(int row, int col, char **result)> &fn);
    bool execute(const std::string &sql);
    bool execute(const std::string &sql, const std::vector<std::vector<std::string>> &bindText);

private:
    sqlite3 *db_;
};

class LocalSearcher {
public:
    static LocalSearcher& instance();
    bool open();
    bool createTable(const QString &name);
    bool dropTable(const QString &name);
    bool clearTable(const QString &name);
    bool initData(const std::string &name, const std::vector<std::vector<std::string>> &bindText);
    bool initTable(const QString &name, const QString &dir, const QString &filter);
    void sortSearch();
    void query(const QString &text, QList<QVariantMap> &result);
    void query(const QString &table, const QString &text, int length, QList<QVariantMap> &result);
    bool isExit(const QString &name);
    void setTableList(const QStringList &tableList);
    const QStringList& tableList() const { return tableList_; }

private:
    LocalSearcher();
    Sqlite sqlite_;
    QStringList tableList_;
};
