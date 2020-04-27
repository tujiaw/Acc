#pragma once

#include <QObject>
#include <functional>
#include <QTime>

class QTimer;
class CronTask : public QObject
{
    Q_OBJECT

public:
    struct TaskInfo {
        TaskInfo() : hour(-1), minute(-1), second(-1) {}
        TaskInfo(const QString &name_, int hour_, int minute_, int second_, const std::function<void()> &handle_) :
            name(name_), hour(hour_), minute(minute_), second(second_), handle(handle_) {}
        QString name;
        int hour, minute, second;
        std::function<void()> handle;
    };

    static CronTask* instance();
    void addEveryday(const TaskInfo &info);

private:
    CronTask(QObject *parent);
    ~CronTask();
    void slotTimer();

private:
    QTimer *timer_;
    QList<TaskInfo> everydayTaskList;
};
