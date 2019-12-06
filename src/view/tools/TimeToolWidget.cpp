#include "TimeToolWidget.h"
#include <QTimer>
#include <QTime>

TimeToolWidget::TimeToolWidget(QWidget *parent)
    : QWidget(parent)
{
    ui.setupUi(this);
    m_timer = new QTimer(this);
    m_timer->setInterval(1000);
    connect(m_timer, &QTimer::timeout, this, &TimeToolWidget::onTimer);
    m_timer->start();
}

TimeToolWidget::~TimeToolWidget()
{
}

void TimeToolWidget::onTimer()
{
    ui.leCurrentTime->setText(QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss"));
    ui.leCurrentUnixTime->setText(QString::number(QDateTime::currentMSecsSinceEpoch() / 1000));
}