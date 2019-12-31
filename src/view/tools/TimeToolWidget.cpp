#include "TimeToolWidget.h"
#include <QTimer>
#include <QTime>

const QString BEIJING_TIME_FORMAT = "yyyy-MM-dd HH:mm:ss";
TimeToolWidget::TimeToolWidget(QWidget *parent)
    : QWidget(parent)
{
    ui.setupUi(this);
    m_timer = new QTimer(this);
    m_timer->setInterval(1000);
    connect(m_timer, &QTimer::timeout, this, &TimeToolWidget::onTimer);
    m_timer->start();

    connect(ui.pbStart, &QPushButton::clicked, this, &TimeToolWidget::onStart);
    connect(ui.pbPause, &QPushButton::clicked, this, &TimeToolWidget::onPause);
    connect(ui.pbFlush, &QPushButton::clicked, this, &TimeToolWidget::onTimer);
    connect(ui.pbToUnixTime, &QPushButton::clicked, this, &TimeToolWidget::onUnixTime);
    connect(ui.pbToBeijingTime, &QPushButton::clicked, this, &TimeToolWidget::onBeijingTime);
}

TimeToolWidget::~TimeToolWidget()
{
}

void TimeToolWidget::onStart()
{
    m_timer->start();
}

void TimeToolWidget::onPause()
{
    m_timer->stop();
}

void TimeToolWidget::onTimer()
{
    ui.leCurrentTime->setText(QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss"));
    ui.leCurrentUnixTime->setText(QString::number(QDateTime::currentMSecsSinceEpoch() / 1000));
}

void TimeToolWidget::onUnixTime()
{
    QString text = ui.leBeijingTime->text();
    if (text.isEmpty()) {
        ui.leUnixTime->setText("");
    } else {
        qint64 t = QDateTime::fromString(text, BEIJING_TIME_FORMAT).toMSecsSinceEpoch() / 1000;
        ui.leUnixTime->setText(QString::number(t));
    }
}

void TimeToolWidget::onBeijingTime()
{
    QString text = ui.leUnixTime->text();
    if (text.isEmpty()) {
        ui.leBeijingTime->setText("");
    } else {
        ui.leBeijingTime->setText(QDateTime::fromMSecsSinceEpoch(text.toLongLong() * 1000).toString(BEIJING_TIME_FORMAT));
    }
}
