#pragma once

#include <QWidget>

class QListWidget;
class QStackedWidget;

class ToolsWidget : public QWidget
{
    Q_OBJECT

public:
    ToolsWidget(QWidget *parent);
    ~ToolsWidget();
    void init();

private:
    QListWidget *m_list;
    QStackedWidget *m_stacked;
};
