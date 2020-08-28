#include "LocalSearch.h"
#include "Util.h"
#include <QDebug>
#include "sqlite3/sqlite3.h"

Sqlite::Sqlite() 
    : db_(nullptr)
{
}
Sqlite::~Sqlite()
{
    close();
}

bool Sqlite::open(const std::string &path)
{
    return SQLITE_OK == sqlite3_open(path.c_str(), &db_);
}

void Sqlite::close()
{
    if (db_) {
        sqlite3_close(db_);
        db_ = nullptr;
    }
}

std::string Sqlite::errmsg()
{
    if (!db_) {
        return "db is null";
    }
    return sqlite3_errmsg(db_);
}

void Sqlite::query(const std::string &sql, const std::function<void(const std::string &name, const std::string &value)> &fn)
{
    if (!db_) return;
    char **dbResult;
    int row = 0;
    int col = 0;
    sqlite3_get_table(db_, sql.c_str(), &dbResult, &row, &col, NULL);
    int index = col;
    for (int i = 0; i < row; i++) {
        for (int j = 0; j < col; j++) {
            fn(dbResult[j], dbResult[index]);
            index++;
        }
    }
    sqlite3_free_table(dbResult);
}

void Sqlite::query2(const std::string &sql, const std::function<void(int row, int col, char **result)> &fn)
{
    if (!db_) return;
    char **dbResult;
    int row = 0;
    int col = 0;
    sqlite3_get_table(db_, sql.c_str(), &dbResult, &row, &col, NULL);
    fn(row, col, dbResult);
    sqlite3_free_table(dbResult);
}

bool Sqlite::execute(const std::string &sql)
{
    if (!db_) return false;
    return SQLITE_OK == sqlite3_exec(db_, sql.c_str(), NULL, NULL, NULL);
}

bool Sqlite::execute(const std::string &sql, const std::vector<std::vector<std::string>> &bindText)
{
    if (!db_) return false;
    sqlite3_exec(db_, "begin;", NULL, NULL, NULL);
    sqlite3_stmt *stmt;
    int errcode = sqlite3_prepare_v2(db_, sql.c_str(), sql.size(), &stmt, nullptr);
    if (errcode != SQLITE_OK) {
        return false;
    }

    for (std::size_t i = 0; i < bindText.size(); i++) {
        for (std::size_t j = 0; j < bindText[i].size(); j++) {
            sqlite3_bind_text(stmt, j + 1, bindText[i][j].c_str(), -1, nullptr);
        }
        errcode = sqlite3_step(stmt);
        if (errcode != SQLITE_DONE) {
            sqlite3_finalize(stmt);
            return false;
        }
        sqlite3_reset(stmt);
    }

    sqlite3_finalize(stmt);
    sqlite3_exec(db_, "commit;", NULL, NULL, NULL);
    return true;
}
//////////////////////////////////////////////////////////////////////////

LocalSearcher::LocalSearcher()
{
}

LocalSearcher& LocalSearcher::instance()
{
    static LocalSearcher s_inst;
    return s_inst;
}

bool LocalSearcher::open()
{
    if (!sqlite_.open("./data.db")) {
        return false;
    }

    sqlite_.query("select name from sqlite_master where type='table'", [this](const std::string &name, const std::string &value) {
        if (!value.empty() && value != "sqlite_sequence") {
            tableList_.push_back(QString::fromStdString(value));
        }
    });
    return true;
}

bool LocalSearcher::initData(const std::string &name, const std::vector<std::vector<std::string>> &bindText)
{
    QString tableName = QString::fromStdString(name);
    dropTable(tableName);
    if (!createTable(tableName)) {
        return false;
    }

    QString sql = QString("insert into '%1' values(NULL, ?, ?, ?)").arg(tableName);
    return sqlite_.execute(sql.toStdString(), bindText);
}

bool LocalSearcher::createTable(const QString &name)
{
    if (tableList_.contains(name, Qt::CaseInsensitive)) {
        return false;
    }

    tableList_.append(name);
    QString sql = QString("create table if not exists '%1'("
        "id integer primary key autoincrement, "
        "name string not null, "
        "content string not null, "
        "search string)").arg(name);
    return sqlite_.execute(sql.toStdString());
}

bool LocalSearcher::dropTable(const QString &name)
{
    QString sql = QString("drop table '%1'").arg(name);
    if (sqlite_.execute(sql.toStdString())) {
        tableList_.removeOne(name);
        return true;
    }
    return false;
}

bool LocalSearcher::clearTable(const QString &name)
{
    QString sql = QString("delete from '%1'").arg(name);
    return sqlite_.execute(sql.toStdString());
}

bool LocalSearcher::initTable(const QString &name, const QString &dir)
{
    if (isExit(name)) {
        return false;
    }

    std::vector<std::wstring> files;
    Util::getFiles(dir.toStdWString(), files);

    std::vector<std::vector<std::string>> datas;
    for (auto iter = files.begin(); iter != files.end(); ++iter) {
        int start = iter->find_last_of(L'/');
        if (start > 0) {
            std::vector<std::string> bindText;
            std::wstring wname = iter->substr(start + 1);
            std::string name = QString::fromStdWString(wname).toStdString();
            bindText.push_back(name);
            bindText.push_back(QString::fromStdWString(*iter).toStdString());
            bindText.push_back(name);
            datas.push_back(bindText);
        }
    }
    
    return initData(name.toStdString(), datas);
}

void LocalSearcher::query(const QString &text, QList<QVariantMap> &result)
{
    const int MAX_COUNT = 100;
    int searchLen = 0;
    for (int i = 0; i < text.size(); i++) {
        ushort u = text.at(i).unicode();
        if (u >= 0x4E00 && u <= 0x9FA5) {
            searchLen += 3;
        } else {
            searchLen += 1;
        }
    }
    foreach(const QString &table, tableList_) {
        QString sql;
        if (searchLen >= 3) {
            sql = QString("select name, content from '%1' where search like '%%2%' order by name desc limit %3;")
                .arg(table).arg(text).arg(MAX_COUNT / 2);
        } else {
            sql = QString("select name, content from '%1' where search like '%2%' order by name desc limit %3;")
                .arg(table).arg(text).arg(MAX_COUNT / 2);
        }
        
        sqlite_.query2(sql.toStdString().c_str(), [&](int row, int col, char **dbResult) {
            int index = col;
            for (int i = 0; i < row; i++) {
                if (col == 2) {
                    QVariantMap vm;
                    vm["name"] = QString(dbResult[index++]);
                    vm["path"] = QString(dbResult[index++]);
                    result.push_back(vm);
                }
            }
        });
    }
}

bool LocalSearcher::isExit(const QString &name)
{
    return tableList_.contains(name, Qt::CaseInsensitive);
}

void LocalSearcher::setTableList(const QStringList &tableList)
{
    tableList_ = tableList;
}
