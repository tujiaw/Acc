#pragma once

#include <QWidget>
#include "ui_TimeToolWidget.h"

class QTimer;
class TimeToolWidget : public QWidget
{
    Q_OBJECT

public:
    TimeToolWidget(QWidget *parent = Q_NULLPTR);
    ~TimeToolWidget();

private slots:
    void onTimer();

private:
    Ui::TimeToolWidget ui;
    QTimer *m_timer;
};
