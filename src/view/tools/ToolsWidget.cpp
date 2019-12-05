#include "ToolsWidget.h"
#include <QListWidget>
#include <QStackedWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include "ColorToolWidget.h"

ToolsWidget::ToolsWidget(QWidget *parent)
    : QWidget(parent)
{
    m_list = new QListWidget(this);
    m_list->setFixedWidth(100);

    m_stacked = new QStackedWidget(this);
    m_stacked->addWidget(new ColorToolWidget(this));

    QHBoxLayout *mLayout = new QHBoxLayout();
    this->setLayout(mLayout);
    mLayout->addWidget(m_list);
    mLayout->addWidget(m_stacked);

    init();
}

ToolsWidget::~ToolsWidget()
{
}

void ToolsWidget::init()
{
    m_list->addItems(QStringList() << "color" << "time" << "json" << "url");
    for (int i = 0; i < m_list->count(); i++) {
        m_list->item(i)->setSizeHint(QSize(this->width(), 30));
    }
}
