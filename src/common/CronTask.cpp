#include "CronTask.h"
#include <QTimer>
#include <QDebug>

CronTask* CronTask::instance()
{
    static CronTask s_inst(nullptr);
    return &s_inst;
}

void CronTask::addEveryday(const TaskInfo &info)
{
    for (int i = 0; i < everydayTaskList.size(); i++) {
        if (everydayTaskList[i].name == info.name) {
            everydayTaskList[i] = info;
            return;
        }
    }
    everydayTaskList.push_back(info);
}

CronTask::CronTask(QObject *parent)
    : QObject(parent)
{
    timer_ = new QTimer(this);
    connect(timer_, &QTimer::timeout, this, &CronTask::slotTimer);
    timer_->setInterval(1000);
    timer_->start();
}

CronTask::~CronTask()
{
}

void CronTask::slotTimer()
{
    QTime cur = QTime::currentTime();
    for (int i = 0; i < everydayTaskList.size(); i++) {
        const TaskInfo &info = everydayTaskList[i];
        if ((info.hour < 0 || info.hour == cur.hour()) &&
            (info.minute < 0 || info.minute == cur.minute()) &&
            (info.second < 0 || info.second == cur.second())) {
            info.handle();
            qDebug() << "everyday task handle, name:" << info.name;
        }
    }
}
