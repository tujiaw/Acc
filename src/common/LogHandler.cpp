#include "LogHandler.h"
#include <QMutex>
#include <QFile>
#include <QTextStream>
#include <QDir>
#include <QDate>
#include <QApplication>
#ifdef DEBUG
#include <windows.h>
#endif

static struct LogWrap {
    LogWrap::LogWrap() { reset(); }
    LogWrap::~LogWrap() { reset(); }
    void reset() {
        isInit = false;
        level = "";
        date = "";
        if (ts) {
            delete ts;
            ts = nullptr;
        }
        if (file) {
            delete file;
            file = nullptr;
        }
    }
    bool isInit;
    QMutex locker;
    QString level;
    QString date;
    QFile *file;
    QTextStream *ts;
} s_logWrap;

#define _TIME_ qPrintable(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz"))
void myMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QString formatMsg;
    switch (type) {
    case QtDebugMsg:
        formatMsg = "Debug:" + msg;
        break;
    case QtWarningMsg:
        formatMsg = "Warning:" + msg;
        break;
    case QtCriticalMsg:
        formatMsg = "Critical:" + msg;
        break;
    case QtFatalMsg:
        formatMsg = "Fatal:" + msg;
        break;
    default:
        return;
    }

    QString curDate = QDate::currentDate().toString("yyyyMMdd");
    if (curDate != s_logWrap.date) {
        s_logWrap.reset();
    }

    if (!s_logWrap.isInit) {
        s_logWrap.isInit = true;
        QDir dir(QApplication::applicationDirPath());
        if (!dir.exists("logs")) {
            dir.mkdir("logs");
        }
        dir.cd("logs");
        s_logWrap.level = "debug";
        s_logWrap.date = QDate::currentDate().toString("yyyyMMdd");
        s_logWrap.file = new QFile(QString("%1/log%2.log").arg(dir.absolutePath()).arg(s_logWrap.date));
        if (s_logWrap.file->open(QIODevice::WriteOnly | QIODevice::Append)) {
            s_logWrap.ts = new QTextStream(s_logWrap.file);
        }
    }

    if (s_logWrap.ts) {
        QString text = _TIME_ + QString("[%1:%2]%3\n").arg(context.file).arg(context.line).arg(formatMsg);
#ifdef DEBUG
        OutputDebugStringA(text.toStdString().c_str());
#endif
        QMutexLocker locker(&s_logWrap.locker);
        (*s_logWrap.ts << text).flush();
    }
}
